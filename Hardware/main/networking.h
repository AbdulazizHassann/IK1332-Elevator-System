#pragma once
#include <Arduino.h>

namespace NET
{
  bool begin(uint32_t timeoutMs = 20000);
  void loop();
  bool connected();
  String ipString();
}
