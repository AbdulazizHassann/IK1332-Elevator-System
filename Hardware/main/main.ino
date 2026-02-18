#include "debug.h"
#include "networking.h"
#include "firebase.h"

void setup()
{
  Serial.begin(115200);
  delay(200);

  if (!NET::begin(20000))
    return;

  FS::begin();
}

void loop()
{
  NET::loop();
  FS::loop();

  static bool didOnce = false;
  if (FS::ready() && !didOnce)
  {
    didOnce = true;

    FS::setElevatorStatus(2, -1);
    FS::logTemperature(23.81f);
    FS::logPressure(101.1f);
    FS::logAcceleration(101.1f);
    FS::logGyroscope(101.1f, 101.1f, 101.1f);
    FS::logMagnetometer(101.1f);
    FS::logTravelHistory(1, 2);
  }
}

