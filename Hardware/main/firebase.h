#pragma once
#include <Arduino.h>

namespace FS {
  bool begin();
  void loop();
  bool ready();

  void log(const char* collection, const char* field, double value);
  void log3(const char* collection, double x, double y, double z);

  void patchElevatorStatus(const bool* anomalyAcceleration,
                           const bool* anomalyTemperature,
                           const int*  currentFloor,
                           const bool* isMoving);
}