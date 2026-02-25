#include "networking.h"
#include <WiFi.h>
#include "secrets.h"

namespace NET
{
  bool begin(uint32_t timeoutMs)
  {
    Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);

      if (millis() - start > timeoutMs)
      {
        Serial.println("\nWiFi connection timeout.");
        return false;
      }
    }

    Serial.println("\nWiFi connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
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


