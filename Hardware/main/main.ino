#include "networking.h"
#include "firebase.h"
#include <Wire.h>
#include <ICM_20948.h>
#include <SparkFun_BMP581_Arduino_Library.h>

// ================== SENSORS ==================
ICM_20948_I2C imu;
BMP581 bmp;

#define SDA_PIN 2
#define SCL_PIN 1

// ================== FLOOR ==================
int lastFloor = 0;

// ================== ML FLOOR MODEL ==================
float w_floor = -0.02636f;
float b_floor = 1.9261f;

float Pbase = 100000.0f;
bool baselineSet = false;

int predictFloorML(float pressure) {
  if (!baselineSet) return -1;

  float x = pressure - Pbase;
  float floor_est = w_floor * x + b_floor;
  int f = round(floor_est);

  if (f < 2) f = 2;
  if (f > 7) f = 7;

  return f;
}

// ================== TIMING ==================
const unsigned long SAMPLE_INTERVAL_MS = 1000;
const unsigned long SEND_INTERVAL_MS   = 3000;
unsigned long lastSampleTime = 0;
unsigned long lastSendTime   = 0;

// ================== MOVING AVERAGE (ACCEL Z) ==================
const int ACCEL_WINDOW = 5;
float accelZBuffer[ACCEL_WINDOW];
int accelIndex = 0;
bool accelFilled = false;

void updateAccelBuffer(float newZ) {
  accelZBuffer[accelIndex] = newZ;
  accelIndex++;
  if (accelIndex >= ACCEL_WINDOW) {
    accelIndex = 0;
    accelFilled = true;
  }
}

float getMeanAccelZ() {
  int count = accelFilled ? ACCEL_WINDOW : accelIndex;
  if (count == 0) return 0.0;
  float sum = 0;
  for (int i = 0; i < count; i++) sum += accelZBuffer[i];
  return sum / count;
}

// ================== MOVING AVERAGE (RAW Z) ==================
const int RAW_WINDOW = 10;
int16_t rawZBuffer[RAW_WINDOW];
int rawIndex = 0;
bool rawFilled = false;

void updateRawZBuffer(int16_t newRaw) {
  rawZBuffer[rawIndex] = newRaw;
  rawIndex++;
  if (rawIndex >= RAW_WINDOW) {
    rawIndex = 0;
    rawFilled = true;
  }
}

float getMeanRawZ() {
  int count = rawFilled ? RAW_WINDOW : rawIndex;
  if (count == 0) return 0.0;
  float sum = 0;
  for (int i = 0; i < count; i++) sum += rawZBuffer[i];
  return sum / count;
}

// ================== MOVING AVERAGE (PRESSURE) ==================
const int PRESS_WINDOW = 5;
float pressureBuffer[PRESS_WINDOW];
int pressIndex = 0;
bool pressFilled = false;

void updatePressureBuffer(float p) {
  pressureBuffer[pressIndex] = p;
  pressIndex++;
  if (pressIndex >= PRESS_WINDOW) {
    pressIndex = 0;
    pressFilled = true;
  }
}

float getMeanPressure() {
  int count = pressFilled ? PRESS_WINDOW : pressIndex;
  if (count == 0) return 0.0;
  float sum = 0;
  for (int i = 0; i < count; i++) sum += pressureBuffer[i];
  return sum / count;
}

// ================== TEMPERATURE ANOMALY ==================
float tempMu     = 22.0;
float tempSigma2 = 0.25;
float tempGamma  = 16.0;

bool isTempAnomaly(float temp) {
  float diff = temp - tempMu;
  float T = (diff * diff) / tempSigma2;
  return (T > tempGamma);
}

// ================== ACCELERATION ANOMALY ==================
float accelMu_raw = 1031.5935;
float accelSigma2_raw = 39.7448;
float accelGamma_raw = 16.0;

bool isAccelAnomalyRaw(int16_t rawAz) {
  float diff = rawAz - accelMu_raw;
  float T = (diff * diff) / accelSigma2_raw;
  return (T > accelGamma_raw);
}

// ================== SETUP ==================
void setup() {

  Serial.begin(115200);
  delay(200);

  if (!NET::begin(20000))
    return;

  FS::begin();

  Serial.println("Starting I2C...");
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Initializing ICM-20948...");
  if (imu.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
    Serial.println("ERROR: ICM-20948 not detected!");
    while (1) delay(100);
  }

  Serial.println("Initializing BMP581...");
  if (bmp.beginI2C(0x47, Wire) != BMP5_OK) {
    Serial.println("ERROR: BMP581 not detected!");
    while (1) delay(100);
  }
}

// ================== LOOP ==================
void loop() {
  NET::loop();
  FS::loop();

  unsigned long now = millis();

  static float lastAccelZ = 0;
  static int16_t lastRawAz = 0;
  static float lastGyroX = 0, lastGyroY = 0, lastGyroZ = 0;
  static float lastMagX = 0, lastMagY = 0, lastMagZ = 0;
  static float lastMagMagnitude = 0;
  static float lastPressure = 0, lastTemp = 0;

  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;

    if (imu.dataReady()) {
      imu.getAGMT();

      const float SCALE = 9.81 / 16384.0;

      lastRawAz = imu.accZ();
      lastAccelZ = lastRawAz * SCALE;

      updateAccelBuffer(lastAccelZ);
      updateRawZBuffer(lastRawAz);

      lastGyroX = imu.gyrX();
      lastGyroY = imu.gyrY();
      lastGyroZ = imu.gyrZ();

      lastMagX  = imu.magX();
      lastMagY  = imu.magY();
      lastMagZ  = imu.magZ();

      lastMagMagnitude = sqrt(
        lastMagX * lastMagX +
        lastMagY * lastMagY +
        lastMagZ * lastMagZ
      );
    }

    bmp5_sensor_data data;
    bmp.getSensorData(&data);
    lastPressure = data.pressure;
    lastTemp     = data.temperature;

    updatePressureBuffer(lastPressure);

    Serial.println("===== RAW SENSOR DATA =====");
    Serial.print("AccelZ (m/s^2): "); Serial.println(lastAccelZ);
    Serial.print("Raw AccelZ: "); Serial.println(lastRawAz);
    Serial.print("Pressure: "); Serial.println(lastPressure);
    Serial.print("Temp: ");     Serial.println(lastTemp);
  }

  if (now - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = now;

    float meanAccelZ    = getMeanAccelZ();
    float meanPressure  = getMeanPressure();
    float meanTemp      = lastTemp;
    float meanRawZ      = getMeanRawZ();

    // ================== MOVEMENT DETECTION (PRESSURE ONLY) ==================
    bool movement = fabs(meanPressure - lastPressure) > 10.0f;
    lastPressure = meanPressure;

    // ================== BASELINE AUTO-CALIBRATION ==================
    if (!movement) {
      if (!baselineSet) {
        Pbase = meanPressure;
        baselineSet = true;
        Serial.print("Baseline initialized at floor 2: ");
        Serial.println(Pbase);
      } else {
        int predictedForBaseline = predictFloorML(meanPressure);
        if (predictedForBaseline == 2) {
          Pbase = 0.95f * Pbase + 0.05f * meanPressure;
        }
      }
    }

    // ================== ML FLOOR PREDICTION ==================
    int currentFloor = predictFloorML(meanPressure);

    if (movement) {
      currentFloor = lastFloor;
    } else if (currentFloor != -1) {
      lastFloor = currentFloor;
    }

    bool tempAnom  = isTempAnomaly(meanTemp);
    bool accelAnom = isAccelAnomalyRaw(lastRawAz);

    Serial.println("===== PROCESSED DATA =====");
    Serial.print("meanRawZ: "); Serial.println(meanRawZ);
    Serial.print("Movement: "); Serial.println(movement ? "MOVING" : "IDLE");

    Serial.print("Current floor: ");
    if (!baselineSet || currentFloor == -1) Serial.println("UNKNOWN");
    else                                     Serial.println(currentFloor);

    Serial.print("Pbase: "); Serial.println(Pbase);

    Serial.print("Temp anomaly: ");
    Serial.println(tempAnom ? "YES" : "NO");

    Serial.print("Accel anomaly: ");
    Serial.println(accelAnom ? "YES" : "NO");

    if (FS::ready())
    {
      FS::patchElevatorStatus(&accelAnom, &tempAnom, &currentFloor, &movement);
      FS::log("temperature", "temperature", meanTemp);
      FS::log("pressure", "pressure", meanPressure);
      FS::log("acceleration","acceleration", meanAccelZ);
      FS::log3("gyroscope", lastGyroX, lastGyroY, lastGyroZ);
      FS::log("magnetometer", "magnetometer", lastMagMagnitude);
    }
  }
}
