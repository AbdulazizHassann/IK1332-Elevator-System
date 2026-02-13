#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>

BMP581 bmp;

#define SDA_PIN 2
#define SCL_PIN 1

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting I2C on custom pins...");
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("Initializing BMP581...");
    if (bmp.beginI2C(0x47, Wire) != BMP5_OK) {
        Serial.println("ERROR: BMP581 not detected!");
        while (1) delay(100);
    }

    Serial.println("BMP581 initialized successfully!");
}

void loop() {
    bmp5_sensor_data data;
    bmp.getSensorData(&data);

    Serial.print("Pressure (Pa): ");
    Serial.print(data.pressure);
    Serial.print("   Temperature (C): ");
    Serial.println(data.temperature);

    delay(300);
}
