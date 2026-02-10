#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <WiFi.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include "secrets.h"

static void processData(AsyncResult &r);

SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;

bool wroteOnce = false;

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.printf("Connecting to Wi-Fi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
    if (millis() - start > 20000)
    {
      Serial.println("\n❌ Wi-Fi connect timeout. Check SSID/password and 2.4GHz.");
      return;
    }
  }

  Serial.println("\n✅ Wi-Fi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  // Quick-start TLS (insecure). OK for testing.
  set_ssl_client_insecure_and_buffer(ssl_client);

  Serial.println("Initializing Firebase app...");
  initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");

  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_DATABASE_URL);

  Serial.println("Setup complete. Waiting for auth ready...");
}

void loop()
{
  app.loop();

  static unsigned long lastNotReadyPrint = 0;
  if (!app.ready() && millis() - lastNotReadyPrint > 5000)
  {
    lastNotReadyPrint = millis();
    Serial.println("⏳ App not ready yet (still authenticating)...");
  }

  if (app.ready() && !wroteOnce)
  {
    wroteOnce = true;
    Serial.println("➡️ Writing elevator status fields...");
    Database.set<int>(aClient, "/elevator/status/currentFloor", 2, processData, "setCurrentFloor");
    Database.set<int>(aClient, "/elevator/status/totalFloors", 4, processData, "setTotalFloors");
  }
}

static void processData(AsyncResult &r)
{
  if (!r.isResult())
    return;

  if (r.isError())
  {
    Firebase.printf("❌ %s | %s (code %d)\n",
                    r.uid().c_str(),
                    r.error().message().c_str(),
                    r.error().code());
    return;
  }

  if (r.available())
  {
    Firebase.printf("✅ %s | payload: %s\n", r.uid().c_str(), r.c_str());
  }

  if (r.isDebug())
  {
    Firebase.printf("🐛 %s | %s\n", r.uid().c_str(), r.debug().c_str());
  }
}
