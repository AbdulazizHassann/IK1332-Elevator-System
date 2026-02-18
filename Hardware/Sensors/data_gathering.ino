#include <Wire.h>
#include <ICM_20948.h>
#include <SparkFun_BMP581_Arduino_Library.h>

// --- Sensors ---
ICM_20948_I2C imu;
BMP581 bmp;

// --- I2C Pins ---
#define SDA_PIN 2
#define SCL_PIN 1

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting I2C on custom pins...");
    Wire.begin(SDA_PIN, SCL_PIN);

    // --- Initialize ICM-20948 ---
    Serial.println("Initializing ICM-20948...");
    if (imu.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: ICM-20948 not detected!");
        while (1) delay(100);
    }
    Serial.println("ICM-20948 initialized successfully!");

    // --- Initialize BMP581 ---
    Serial.println("Initializing BMP581...");
    if (bmp.beginI2C(0x47, Wire) != BMP5_OK) {
        Serial.println("ERROR: BMP581 not detected!");
        while (1) delay(100);
    }
    Serial.println("BMP581 initialized successfully!");
}

void loop() {

    // --- Read IMU ---
    if (imu.dataReady()) {
        imu.getAGMT();

        // ⭐ Convert raw accel to m/s²
        const float SCALE = 9.81 / 16384.0;
        float ax = imu.accX() * SCALE;
        float ay = imu.accY() * SCALE;
        float az = imu.accZ() * SCALE;

        Serial.println("=== ICM-20948 ===");
        Serial.print("Accel (m/s^2): ");
        Serial.print(ax); Serial.print(", ");
        Serial.print(ay); Serial.print(", ");
        Serial.println(az);

        Serial.print("Gyro (rad/s): ");
        Serial.print(imu.gyrX()); Serial.print(", ");
        Serial.print(imu.gyrY()); Serial.print(", ");
        Serial.println(imu.gyrZ());

        Serial.print("Mag (uT): ");
        Serial.print(imu.magX()); Serial.print(", ");
        Serial.print(imu.magY()); Serial.print(", ");
        Serial.println(imu.magZ());
    }

    // --- Read BMP581 ---
    bmp5_sensor_data data;
    bmp.getSensorData(&data);

    Serial.println("=== BMP581 ===");
    Serial.print("Pressure (Pa): ");
    Serial.print(data.pressure);
    Serial.print("   Temperature (C): ");
    Serial.println(data.temperature);

    Serial.println("-----------------------------");
    delay(200);
}
