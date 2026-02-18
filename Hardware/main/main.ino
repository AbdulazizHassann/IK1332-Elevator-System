#include "networking.h"
#include "firebase.h"
#include "debug.h"

static bool wroteOnce = false;

void setup()
{
  Serial.begin(115200);
  delay(200);

  if (!NET::begin(20000))
  {
    DBG_PRINTLN("Halting: WiFi failed.");
    return;
  }

  FB::begin();
}

void loop()
{
  NET::loop();
  FB::loop();

  if (FB::ready() && !wroteOnce)
  {
    wroteOnce = true;
    FB::setInt("/elevator/status/currentFloor", 2, "setCurrentFloor");
    FB::setInt("/elevator/status/totalFloors", 4, "setTotalFloors");
  }
}
