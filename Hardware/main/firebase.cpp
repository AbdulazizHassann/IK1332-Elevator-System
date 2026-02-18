#include "firebase.h"
#include "debug.h"

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE
#include <FirebaseClient.h>

#include "ExampleFunctions.h"
#include "secrets.h"

namespace FB
{
  static SSL_CLIENT ssl_client;
  using AsyncClient = AsyncClientClass;
  static AsyncClient aClient(ssl_client);

  static UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD, 3000);
  static FirebaseApp app;
  static RealtimeDatabase db;

  static void processData(AsyncResult &r)
  {
    if (!r.isResult()) return;

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

  bool begin()
  {
    DBG_PRINTF("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    // Quick-start TLS (insecure). OK for testing.
    set_ssl_client_insecure_and_buffer(ssl_client);

    DBG_PRINTLN("Initializing Firebase app...");
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");

    app.getApp<RealtimeDatabase>(db);
    db.url(FIREBASE_DATABASE_URL);

    DBG_PRINTLN("Firebase setup complete. Waiting for auth ready...");
    return true;
  }

  void loop()
  {
    app.loop();
  }

  bool ready()
  {
    return app.ready();
  }

  void setInt(const String &path, int value, const String &tag)
  {
    const char *uid = tag.length() ? tag.c_str() : "setInt";
    db.set<int>(aClient, path.c_str(), value, processData, uid);
  }
}


