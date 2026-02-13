#include <Wire.h>
#include <ICM_20948.h>

ICM_20948_I2C imu;

#define SDA_PIN 2
#define SCL_PIN 1

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting I2C on custom pins...");
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("Initializing ICM-20948...");

    if (imu.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: ICM-20948 not detected. Check wiring!");
        while (1) delay(100);
    }

    Serial.println("ICM-20948 initialized successfully!");
}

void loop() {
    if (imu.dataReady()) {
        imu.getAGMT();

        Serial.print("Accel: ");
        Serial.print(imu.accX()); Serial.print(", ");
        Serial.print(imu.accY()); Serial.print(", ");
        Serial.println(imu.accZ());

        Serial.print("Gyro: ");
        Serial.print(imu.gyrX()); Serial.print(", ");
        Serial.print(imu.gyrY()); Serial.print(", ");
        Serial.println(imu.gyrZ());

        Serial.print("Mag: ");
        Serial.print(imu.magX()); Serial.print(", ");
        Serial.print(imu.magY()); Serial.print(", ");
        Serial.println(imu.magZ());

        Serial.println("-----------------------------");
    }

    delay(100);
}
