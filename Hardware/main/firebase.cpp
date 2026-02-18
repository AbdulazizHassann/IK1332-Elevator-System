#include "firebase.h"
#include "debug.h"

#include <Arduino.h>

#define ENABLE_USER_AUTH
#define ENABLE_FIRESTORE
#include <FirebaseClient.h>



#include "ExampleFunctions.h"
#include "secrets.h"

namespace FS
{
  static SSL_CLIENT ssl_client;
  using AsyncClient = AsyncClientClass;
  static AsyncClient aClient(ssl_client);

  static UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD, 3000);

  static FirebaseApp app;
  static Firestore::Documents Docs;

  static bool getUtcTimestampRFC3339(String &outTs)
  {
    uint32_t epoch = get_ntp_time();
    if (epoch == 0)
      return false;

    time_t t = (time_t)epoch;
    struct tm tmUtc;
    gmtime_r(&t, &tmUtc);

    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tmUtc);

    outTs = String(buf);
    return true;
  }

  static void processData(AsyncResult &r)
  {
    if (!r.isResult())
      return;

    if (r.isError())
    {
      DBG_PRINTF("❌ %s | %s (code %d)\n",
                 r.uid().c_str(),
                 r.error().message().c_str(),
                 r.error().code());
      return;
    }

    if (r.available())
      DBG_PRINTF("✅ %s | payload: %s\n", r.uid().c_str(), r.c_str());

    if (r.isDebug())
      DBG_PRINTF("🐛 %s | %s\n", r.uid().c_str(), r.debug().c_str());
  }

  static String makeDocId()
  {
    uint32_t r1 = (uint32_t)esp_random();
    uint32_t r2 = (uint32_t)esp_random();

    char buf[40];
    snprintf(buf, sizeof(buf), "%08lX%08lX%lu",
             (unsigned long)r1,
             (unsigned long)r2,
             (unsigned long)millis());

    return String(buf);
  }

  bool begin()
  {
    DBG_PRINTF("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    // Quick-start TLS (insecure). OK for testing.
    set_ssl_client_insecure_and_buffer(ssl_client);

    DBG_PRINTLN("Initializing Firebase app (Auth)...");
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");

    // Bind Firestore Documents service to the app
    app.getApp<Firestore::Documents>(Docs);

    DBG_PRINTLN("Firestore setup complete. Waiting for auth ready...");
    return true;
  }

  void loop()
  {
    // Keep Firebase internals running
    app.loop();
    Docs.loop();
  }

  bool ready()
  {
    return app.ready();
  }

  // floors only update if larger then 0
  void setElevatorStatus(int currentFloor, int floors)
  {
    if (!ready())
      return;

    String documentPath = "elevator/1";

    Document<Values::Value> doc;
    String updateMask;

    Values::IntegerValue currentFloorValue(currentFloor);
    doc.add("currentFloor", Values::Value(currentFloorValue));
    updateMask = "currentFloor";

    if (floors >= 0)
    {
      Values::IntegerValue floorsValue(floors);
      doc.add("floors", Values::Value(floorsValue));
      updateMask += ",floors";
    }

    PatchDocumentOptions patchOptions(
      DocumentMask(updateMask.c_str()), // only update what we included
      DocumentMask(),
      Precondition()
    );

    Docs.patch(aClient,
              Firestore::Parent(FIREBASE_PROJECT_ID),
              documentPath,
              patchOptions,
              doc,
              processData,
              "setElevatorStatus");
  }

  void logTemperature(float temperature)
  {
    if (!ready())
      return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/temperatures/" + readingId;

    Values::DoubleValue valueTemperature((double)temperature);
    Document<Values::Value> doc("temperature", Values::Value(valueTemperature));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue valueTimestamp(ts.c_str());
      doc.add("timestamp", Values::Value(valueTimestamp));
    }
    else
    {
      DBG_PRINTLN("NTP time not available; writing without timestamp.");
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logTemperature");
  }

  void logPressure(float pressure)
  {
    if (!ready()) return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/pressure/" + readingId;

    Values::DoubleValue pressureV((double)pressure);
    Document<Values::Value> doc("pressure", Values::Value(pressureV));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue tsV(ts.c_str());
      doc.add("timestamp", Values::Value(tsV));
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logPressure");
  }

  void logAcceleration(float acceleration)
  {
    if (!ready()) return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/acceleration/" + readingId;

    Values::DoubleValue accelV((double)acceleration);
    Document<Values::Value> doc("acceleration", Values::Value(accelV));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue tsV(ts.c_str());
      doc.add("timestamp", Values::Value(tsV));
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logAcceleration");
  }

  void logGyroscope(float gyroscopeX, float gyroscopeY, float gyroscopeZ)
  {
    if (!ready()) return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/gyroscope/" + readingId;

    Values::DoubleValue gxV((double)gyroscopeX);
    Values::DoubleValue gyV((double)gyroscopeY);
    Values::DoubleValue gzV((double)gyroscopeZ);

    Document<Values::Value> doc("x", Values::Value(gxV));
    doc.add("y", Values::Value(gyV));
    doc.add("z", Values::Value(gzV));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue tsV(ts.c_str());
      doc.add("timestamp", Values::Value(tsV));
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logGyroscope");
  }

  void logMagnetometer(float magnetometer)
  {
    if (!ready()) return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/magnetometer/" + readingId;

    Values::DoubleValue magV((double)magnetometer);
    Document<Values::Value> doc("magnetometer", Values::Value(magV));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue tsV(ts.c_str());
      doc.add("timestamp", Values::Value(tsV));
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logMagnetometer");
  }

  void logTravelHistory(int oldFloor, int newFloor)
  {
    if (!ready()) return;

    String readingId = makeDocId();
    String documentPath = "elevator/1/travelHistory/" + readingId;

    Values::IntegerValue oldV(oldFloor);
    Values::IntegerValue newV(newFloor);

    Document<Values::Value> doc("oldFloor", Values::Value(oldV));
    doc.add("newFloor", Values::Value(newV));

    String ts;
    if (getUtcTimestampRFC3339(ts))
    {
      Values::TimestampValue tsV(ts.c_str());
      doc.add("timestamp", Values::Value(tsV));
    }

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        documentPath,
                        DocumentMask(),
                        doc,
                        processData,
                        "logTravelHistory");
  }

}

