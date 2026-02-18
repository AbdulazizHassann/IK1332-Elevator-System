#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <ICM_20948.h>
#include <SparkFun_BMP581_Arduino_Library.h>

// ================== WIFI + NTP ==================
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* ntpServer           = "pool.ntp.org";
const long  gmtOffset_sec       = 3600;
const int   daylightOffset_sec  = 3600;

// ================== SENSORS ==================
ICM_20948_I2C imu;
BMP581 bmp;

#define SDA_PIN 2
#define SCL_PIN 1

// ================== TIMING ==================
const unsigned long SAMPLE_INTERVAL_MS = 1000;
const unsigned long SEND_INTERVAL_MS   = 5000;
unsigned long lastSampleTime = 0;
unsigned long lastSendTime   = 0;

// ================== MOVING AVERAGE (Z ONLY) ==================
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

// ================== MOVEMENT DETECTION (Z ONLY) ==================
int detectMovement(float meanZ) {
  const float GRAVITY   = 9.81;
  const float THRESHOLD = 0.05;

  float diff = meanZ - GRAVITY;

  if (fabs(diff) < THRESHOLD) return 0;
  else if (diff > THRESHOLD)  return 1;
  else                        return -1;
}

// ================== FLOOR DETECTION ==================
float floorMin[6] = {100890, 100860, 100820, 100785, 100750, 100710};
float floorMax[6] = {100920, 100889, 100859, 100819, 100784, 100749};

int predictFloor(float pressure) {
  for (int i = 0; i < 6; i++) {
    if (pressure >= floorMin[i] && pressure <= floorMax[i]) {
      return i + 2;
    }
  }
  return -1;
}

// ================== TEMPERATURE ANOMALY ==================
float tempMu     = 22.0;
float tempSigma2 = 0.25;
float tempGamma  = 9.0;

bool isTempAnomaly(float temp) {
  float diff = temp - tempMu;
  float T = (diff * diff) / tempSigma2;
  return (T > tempGamma);
}

// ================== TIME HELPERS ==================
String getCurrentTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "TIME_ERROR";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ================== FIREBASE HOOK ==================
void sendToFirebase(
  int currentFloor,
  int movement,
  float meanPressure,
  float meanTemp,
  float meanAccelZ,   // ⭐ ONLY Z SENT
  float meanGyroX,
  float meanGyroY,
  float meanGyroZ,
  float meanMagX,
  float meanMagY,
  float meanMagZ,
  bool tempAnomaly,
  String timestamp
) {
  Serial.println("=== Sending to Firebase (stub) ===");
  Serial.print("  Timestamp: "); Serial.println(timestamp);
  Serial.print("  Floor: "); Serial.println(currentFloor);
  Serial.print("  Movement: "); Serial.println(movement);
  Serial.print("  meanPressure: "); Serial.println(meanPressure);
  Serial.print("  meanTemp: "); Serial.println(meanTemp);
  Serial.print("  meanAccelZ: "); Serial.println(meanAccelZ);
  Serial.print("  tempAnomaly: "); Serial.println(tempAnomaly ? "YES" : "NO");
  Serial.println("-----------------------------");
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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
  unsigned long now = millis();

  static float lastAccelZ = 0;
  static float lastGyroX = 0, lastGyroY = 0, lastGyroZ = 0;
  static float lastMagX = 0, lastMagY = 0, lastMagZ = 0;
  static float lastPressure = 0, lastTemp = 0;

  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;

    if (imu.dataReady()) {
      imu.getAGMT();

      // ⭐ ONLY Z IS PROCESSED
      const float SCALE = 9.81 / 16384.0;
      lastAccelZ = imu.accZ() * SCALE;

      updateAccelBuffer(lastAccelZ);

      // Gyro + Mag unchanged
      lastGyroX = imu.gyrX();
      lastGyroY = imu.gyrY();
      lastGyroZ = imu.gyrZ();
      lastMagX  = imu.magX();
      lastMagY  = imu.magY();
      lastMagZ  = imu.magZ();
    }

    bmp5_sensor_data data;
    bmp.getSensorData(&data);
    lastPressure = data.pressure;
    lastTemp     = data.temperature;

    updatePressureBuffer(lastPressure);

    Serial.println("===== RAW SENSOR DATA =====");
    Serial.print("AccelZ (m/s^2): "); Serial.println(lastAccelZ);
    Serial.print("Gyro: "); Serial.print(lastGyroX); Serial.print(", ");
                           Serial.print(lastGyroY); Serial.print(", ");
                           Serial.println(lastGyroZ);
    Serial.print("Mag: ");  Serial.print(lastMagX); Serial.print(", ");
                           Serial.print(lastMagY); Serial.print(", ");
                           Serial.println(lastMagZ);
    Serial.print("Pressure: "); Serial.println(lastPressure);
    Serial.print("Temp: ");     Serial.println(lastTemp);
  }

  if (now - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = now;

    float meanAccelZ    = getMeanAccelZ();   // ⭐ ONLY Z
    float meanPressure  = getMeanPressure();
    float meanTemp      = lastTemp;

    int movement = detectMovement(meanAccelZ);
    int currentFloor = predictFloor(meanPressure);
    bool tempAnom = isTempAnomaly(meanTemp);

    String timestamp = getCurrentTimeString();

    Serial.println("===== PROCESSED DATA =====");
    Serial.print("meanAccelZ: "); Serial.println(meanAccelZ);
    Serial.print("meanPressure: "); Serial.println(meanPressure);
    Serial.print("meanTemp: "); Serial.println(meanTemp);

    Serial.print("Movement: ");
    if (movement == 0)      Serial.println("STOPPED");
    else if (movement == 1) Serial.println("UP");
    else                    Serial.println("DOWN");

    Serial.print("Current floor: ");
    if (currentFloor == -1) Serial.println("UNKNOWN");
    else                    Serial.println(currentFloor);

    Serial.print("Temp anomaly: ");
    Serial.println(tempAnom ? "YES" : "NO");

    sendToFirebase(
      currentFloor,
      movement,
      meanPressure,
      meanTemp,
      meanAccelZ,   // ⭐ ONLY Z SENT
      lastGyroX,
      lastGyroY,
      lastGyroZ,
      lastMagX,
      lastMagY,
      lastMagZ,
      tempAnom,
      timestamp
    );
  }
}
