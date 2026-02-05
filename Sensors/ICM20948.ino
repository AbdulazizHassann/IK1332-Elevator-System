#include <Wire.h>
#include <ICM_20948.h>
ICM_20948_I2C imu;

// Your chosen I2C pins
#define SDA_PIN 3
#define SCL_PIN 4

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting I2C on custom pins...");
    Wire.begin(SDA_PIN, SCL_PIN);   // SDA = 3, SCL = 4

    Serial.println("Initializing ICM-20948...");

    // Initialize IMU on I2C address 0x68 (default)
    if (imu.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: ICM-20948 not detected. Check wiring!");
        while (1) {
            delay(100);
        }
    }

    Serial.println("ICM-20948 initialized successfully!");
}

void loop() {
    if (imu.dataReady()) {
        imu.getAGMT();  // Read all sensor data (Accel, Gyro, Mag, Temp)

        // Acceleration (m/s^2)
        Serial.print("Accel (m/s^2): ");
        Serial.print(imu.accX());
        Serial.print(", ");
        Serial.print(imu.accY());
        Serial.print(", ");
        Serial.println(imu.accZ());

        // Gyroscope (rad/s)
        Serial.print("Gyro (rad/s): ");
        Serial.print(imu.gyrX());
        Serial.print(", ");
        Serial.print(imu.gyrY());
        Serial.print(", ");
        Serial.println(imu.gyrZ());

        // Magnetometer (uT)
        Serial.print("Mag (uT): ");
        Serial.print(imu.magX());
        Serial.print(", ");
        Serial.print(imu.magY());
        Serial.print(", ");
        Serial.println(imu.magZ());

        Serial.println("-----------------------------");
    }

    delay(100);
}
