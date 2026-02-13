#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>

BMP581 bmp;

#define SDA_PIN 2
#define SCL_PIN 1

// Updated floor ranges (Pa)
float floorMin[4] = {100150, 100100, 100050,  99000};
float floorMax[4] = {100200, 100150, 100100, 100050};

int predictFloor(float pressure) {
    for (int i = 0; i < 4; i++) {
        if (pressure >= floorMin[i] && pressure <= floorMax[i]) {
            return i + 1;
        }
    }
    return -1;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (bmp.beginI2C(0x47, Wire) != BMP5_OK) {
        Serial.println("ERROR: BMP581 not detected!");
        while (1) delay(100);
    }
}

void loop() {
    bmp5_sensor_data data;
    bmp.getSensorData(&data);

    float pressure = data.pressure;
    float temperature = data.temperature;

    int floor = predictFloor(pressure);

    Serial.print("Pressure (Pa): ");
    Serial.print(pressure);
    Serial.print("   Temp (C): ");
    Serial.print(temperature);
    Serial.print("   -> Floor: ");

    if (floor == -1)
        Serial.println("Unknown");
    else
        Serial.println(floor);

    delay(300);
}
