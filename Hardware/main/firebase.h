#pragma once
#include <Arduino.h>

namespace FB
{
  bool begin();
  void loop();
  bool ready();

  void setInt(const String &path, int value, const String &tag = "");
}
