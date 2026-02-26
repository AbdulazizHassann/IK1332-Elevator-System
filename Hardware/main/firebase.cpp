#include "firebase.h"
#include <Arduino.h>

#define ENABLE_USER_AUTH
#define ENABLE_FIRESTORE
#include <FirebaseClient.h>

#include "ExampleFunctions.h"
#include "secrets.h"

namespace FS {

  static SSL_CLIENT ssl_client;
  using AsyncClient = AsyncClientClass;
  static AsyncClient aClient(ssl_client);
  static UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD, 3000);
  static FirebaseApp app;
  static Firestore::Documents Docs;

  static const char* kParentDoc = "elevator/1";

  // ----------------------------------------------------
  // Timestamp helper
  // ----------------------------------------------------
  static bool getUtcTimestampRFC3339(String &outTs)
  {
    uint32_t epoch = get_ntp_time();
    if (epoch == 0) return false;

    time_t t = (time_t)epoch;
    struct tm tmUtc;
    gmtime_r(&t, &tmUtc);

    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tmUtc);

    outTs = String(buf);
    return true;
  }

  static void addTimestamp(Document<Values::Value>& doc)
  {
    String ts;
    if (!getUtcTimestampRFC3339(ts)) return;

    Values::TimestampValue tsV(ts.c_str());
    doc.add("timestamp", Values::Value(tsV));
  }

  // ----------------------------------------------------
  // Debug callback
  // ----------------------------------------------------
  static void processData(AsyncResult &r)
  {
    if (!r.isResult()) return;

    if (r.isError()) {
      Serial.printf("[Error] %s | %s (code %d)\n",
                r.uid().c_str(),
                r.error().message().c_str(),
                r.error().code());
      return;
    }

    if (r.available()) {
      Serial.printf("[Success] %s\n", r.uid().c_str());
      r.clear();
    }
  }

  // ----------------------------------------------------
  // Doc ID generator
  // ----------------------------------------------------
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

  // ----------------------------------------------------
  // Lifecycle
  // ----------------------------------------------------
  bool begin()
  {
    Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    set_ssl_client_insecure_and_buffer(ssl_client);

    initializeApp(aClient,
                  app,
                  getAuth(user_auth),
                  auth_debug_print,
                  "authTask");

    app.getApp<Firestore::Documents>(Docs);

    Serial.println("Firestore ready. Waiting for auth...");
    return true;
  }

  void loop()
  {
    app.loop();
    Docs.loop();
  }

  bool ready()
  {
    return app.ready();
  }

  // ----------------------------------------------------
  // Generic LOG (single numeric field)
  // ----------------------------------------------------
  void log(const char* collection,
           const char* field,
           double value)
  {
    if (!ready()) return;

    String docId = makeDocId();
    String path = String(kParentDoc) + "/" + collection + "/" + docId;

    Document<Values::Value> doc;
    doc.add(field, Values::Value(Values::DoubleValue(value)));
    addTimestamp(doc);

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        path,
                        DocumentMask(),
                        doc,
                        processData,
                        collection);
  }

  // ----------------------------------------------------
  // Generic LOG3 (vector / xyz)
  // ----------------------------------------------------
  void log3(const char* collection,
            double x,
            double y,
            double z)
  {
    if (!ready()) return;

    String docId = makeDocId();
    String path = String(kParentDoc) + "/" + collection + "/" + docId;

    Document<Values::Value> doc;
    doc.add("x", Values::Value(Values::DoubleValue(x)));
    doc.add("y", Values::Value(Values::DoubleValue(y)));
    doc.add("z", Values::Value(Values::DoubleValue(z)));
    addTimestamp(doc);

    Docs.createDocument(aClient,
                        Firestore::Parent(FIREBASE_PROJECT_ID),
                        path,
                        DocumentMask(),
                        doc,
                        processData,
                        collection);
  }

  // ----------------------------------------------------
  // Patch Elevator Status (cached)
  // ----------------------------------------------------
  void patchElevatorStatus(const bool* anomalyAcceleration,
                           const bool* anomalyTemperature,
                           const int*  currentFloor,
                           const bool* isMoving)
  {
    if (!ready()) return;

    static bool hasAnomAccel = false, lastAnomAccel = false;
    static bool hasAnomTemp  = false, lastAnomTemp  = false;
    static bool hasFloor     = false; static int  lastFloor = 0;
    static bool hasMoving    = false, lastMoving    = false;
    bool logTravelHistory = false;

    Document<Values::Value> doc;

    String updateMask;
    auto addMask = [&](const char* field) {
      if (updateMask.length()) updateMask += ",";
      updateMask += field;
    };

    // ---- anomalyAcceleration ----
    if (anomalyAcceleration) {
      bool v = *anomalyAcceleration;
      if (!hasAnomAccel || v != lastAnomAccel) {
        doc.add("anomalyAcceleration", Values::Value(Values::BooleanValue(v)));
        addMask("anomalyAcceleration");
        lastAnomAccel = v;
        hasAnomAccel = true;
      }
    }

    // ---- anomalyTemperature ----
    if (anomalyTemperature) {
      bool v = *anomalyTemperature;
      if (!hasAnomTemp || v != lastAnomTemp) {
        doc.add("anomalyTemperature", Values::Value(Values::BooleanValue(v)));
        addMask("anomalyTemperature");
        lastAnomTemp = v;
        hasAnomTemp = true;
      }
    }

    // ---- currentFloor ----
    if (currentFloor) {
      int v = *currentFloor;
      if (!hasFloor || v != lastFloor || true) {
        doc.add("currentFloor", Values::Value(Values::IntegerValue(v)));
        addMask("currentFloor");
        lastFloor = v;
        hasFloor = true;
        logTravelHistory = true;
      }
    }

    // ---- isMoving ----
    if (isMoving) {
      bool v = *isMoving;
      if (!hasMoving || v != lastMoving) {
        doc.add("isMoving", Values::Value(Values::BooleanValue(v)));
        addMask("isMoving");
        lastMoving = v;
        hasMoving = true;
      }
    }

    if (updateMask.length() == 0) return;

    PatchDocumentOptions opts(
      DocumentMask(updateMask.c_str()),
      DocumentMask(),
      Precondition()
    );

    Docs.patch(aClient,
              Firestore::Parent(FIREBASE_PROJECT_ID),
              kParentDoc,
              opts,
              doc,
              processData,
              "patchElevatorStatus");

    // Add floor to travel history if floor has changed
    if (logTravelHistory)
    {
      String docId = makeDocId();
      String path = String(kParentDoc) + "/" + "travelHistory" + "/" + docId;

      Document<Values::Value> docTravel;
      docTravel.add("floor", Values::Value(Values::IntegerValue(*currentFloor)));
      addTimestamp(docTravel);

      Docs.createDocument(aClient,
                          Firestore::Parent(FIREBASE_PROJECT_ID),
                          path,
                          DocumentMask(),
                          docTravel,
                          processData,
                          "travelHistory");
      }
  }
}
