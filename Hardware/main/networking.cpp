#include "networking.h"
#include "debug.h"

#include <WiFi.h>
#include "secrets.h"

namespace NET
{
  bool begin(uint32_t timeoutMs)
  {
    DBG_PRINTF("Connecting to WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      DBG_PRINT(".");
      delay(300);

      if (millis() - start > timeoutMs)
      {
        DBG_PRINTLN("\n❌ WiFi connection timeout.");
        return false;
      }
    }

    DBG_PRINTLN("\n✅ WiFi connected.");
    DBG_PRINT("IP: ");
    DBG_PRINTLN(WiFi.localIP());
    return true;
  }

  void loop()
  {
  }

  bool connected()
  {
    return WiFi.status() == WL_CONNECTED;
  }

  String ipString()
  {
    return WiFi.localIP().toString();
  }
}


