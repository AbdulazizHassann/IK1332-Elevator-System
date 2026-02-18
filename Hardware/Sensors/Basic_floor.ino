#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>

BMP581 bmp;

#define SDA_PIN 2
#define SCL_PIN 1

// Updated floor ranges (Pa)
float floorMin[4] = {100890, 100860, 100820, 100785, 100750, 100710 };
float floorMax[4] = {100920, 100889, 100859, 100819, 100784, 100749 };

int predictFloor(float pressure) {
  for (int i = 0; i < 6; i++) {
    if (pressure >= floorMin[i] && pressure <= floorMax[i]) {
      return i + 2;  // floors 2–7
    }
  }
  return -1; // unknown
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
