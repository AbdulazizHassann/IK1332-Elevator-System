#pragma once
#include <Arduino.h>

namespace FS
{
  bool begin();
  void loop();
  bool ready();

  void setElevatorStatus(int currentFloor);
  void logTemperature(float temperature);
  void logPressure(float pressure);
  void logAcceleration(float acceleration);
  void logGyroscope(float gyroscopeX, float gyroscopeY, float gyroscopeZ);
  void logMagnetometer(float magnetometer);
  void logTravelHistory(int oldFloor, int newFloor);

}
