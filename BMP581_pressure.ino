#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>

BMP581 bmp;

// Your chosen I2C pins
#define SDA_PIN 3
#define SCL_PIN 4

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting I2C on custom pins...");
    Wire.begin(SDA_PIN, SCL_PIN);   // SDA = 3, SCL = 4

    Serial.println("Initializing BMP581...");
    if (!bmp.beginI2C()) {
        Serial.println("ERROR: BMP581 not detected. Check wiring!");
        while (1) {
            delay(100);
        }
    }

    Serial.println("BMP581 initialized successfully!");
}

void loop() {
    bmp5_sensor_data data;   // Struct to hold sensor readings

    // IMPORTANT: pass pointer using &data
    bmp.getSensorData(&data);

    float pressure = data.pressure;         // Pascals
    float temperature = data.temperature;   // Celsius

    Serial.print("Pressure (Pa): ");
    Serial.print(pressure);
    Serial.print("   Temperature (C): ");
    Serial.println(temperature);

    delay(300);
}
