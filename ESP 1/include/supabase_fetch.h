#ifndef SUPABASE_FETCH_H
#define SUPABASE_FETCH_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> // Use Secure Client for HTTPS
#include <ArduinoJson.h>
#include <lcd_manager.h>
#include <Adafruit_Fingerprint.h> // Include the fingerprint library
#include <buzzer_manager.h>
#include <solenoid_control.h>
#include <fingerprint_manager.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

BearSSL::WiFiClientSecure wifiClient;

// Change these constants as needed
const char *supabaseUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/rest/v1/TBL_URL?id=eq.1";
const char *supabaseKeyy = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5sbGp2bXdneGxuaGRpbnZrb2ZkIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MzkzODE4NzUsImV4cCI6MjA1NDk1Nzg3NX0.O30PoY_iunkPTTWVltheYHfaKBoOxL3iAN3Ktw1lwsk";

// Global variable to store the extracted server URL.
String serverUrl = "";

static const uint8_t MAX_ID = 127;
static const uint8_t RAND_MIN = 50;
static const uint8_t RANDOM_MAX = 60;

class SupabaseFetcher
{

#define SOLENOID_SMALL_GPIO 2  // D4 -> small locker
#define SOLENOID_MED_GPIO 13   // D7 -> medium locker
#define SOLENOID_LARGE_GPIO 15 // D8 -> large locker
public:
  // Fetches the API URL from Supabase and updates the global serverUrl.
  void fetchServerUrl()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("[INFO] WiFi Connected. Fetching URL...");
      displayMessage("Getting Ready...", "  Please  Wait  ");

      WiFiClientSecure client;
      client.setTimeout(15000);
      client.setInsecure(); // For testing only

      HTTPClient http;
      http.setTimeout(5000);

      Serial.println("[INFO] Connecting to Supabase...");
      if (!http.begin(client, supabaseUrl))
      {
        Serial.println("[ERROR] Unable to begin HTTPS connection.");
        displayMessage(" Cant load data ", "   Restarting   ");
        buzzer.beepError();
        delay(3000);
        fetchServerUrl(); // recursive retry (consider limiting retries)
        return;
      }

      // Set headers
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "*/*");
      http.addHeader("apikey", String(supabaseKeyy));
      http.addHeader("Authorization", String("Bearer ") + String(supabaseKeyy));

      int httpResponseCode = http.GET();
      if (httpResponseCode > 0)
      {
        String response = http.getString();
        Serial.printf("[INFO] HTTP Response Code: %d\n", httpResponseCode);
        Serial.println("Supabase Response: " + response);

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, response);
        if (!err && doc.size() > 0 && doc[0]["url"])
        {
          buzzer.beepSuccess();
          serverUrl = doc[0]["url"].as<String>();
          Serial.println("[INFO] Extracted API URL: " + serverUrl);
          displayMessage(" Connected   to ", "    Database    ");
          delay(2000);
        }
        else
        {
          buzzer.beepError();
          Serial.println("[ERROR] Failed to parse JSON response.");
          displayMessage("Can't Connect to", "    Database    ");
          delay(2000);
        }
      }
      else
      {
        buzzer.beepError();
        Serial.printf("[ERROR] HTTP request failed. Code: %d\n", httpResponseCode);
        displayMessage("  Data can't be ", "     Loaded     ");
        delay(2000);
      }
      http.end();
    }
    else
    {
      buzzer.beepError();
      Serial.println("[ERROR] WiFi Not Connected.");
      displayMessage("WiFi signal lost", "Reconnecting... ");
      delay(2000);
    }
  }

  // Returns the extracted serverUrl.
  String getServerUrl() { return serverUrl; }

  // Polls Supabase for the system state and updates  accordingly.
  void checkForState()
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[STATE POLL] WiFi not connected, skipping poll.");
      return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    // Endpoint for system state – ensure this function is deployed
    String requestUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/functions/v1/get_state";
    Serial.print("[STATE POLL] Requesting: ");
    Serial.println(requestUrl);

    http.begin(client, requestUrl);

    http.addHeader("Authorization", String("Bearer ") + String(supabaseKeyy));
    http.addHeader("apikey", String(supabaseKeyy));
    http.addHeader("Accept", "*/*");
    http.addHeader("Content-Type", "application/json");

    // Build JSON body to pass your serverUrl if needed (adjust as required by your function)
    String jsonBody = "{\"api_url\": \"" + serverUrl + "\"}";

    int httpResponseCode = http.POST(jsonBody);
    if (httpResponseCode > 0)
    {
      String response = http.getString();
      http.end();
      client.stop(); // Explicitly close the WiFiClient

      Serial.print("[STATE POLL] HTTP ");
      Serial.println(httpResponseCode);
      Serial.print("[STATE POLL] Response: ");
      Serial.println(response);

      StaticJsonDocument<512> doc;
      DeserializationError err = deserializeJson(doc, response);
      if (!err)
      {
        const char *currentState = doc["currentState"];
        Serial.print("[STATE POLL] System state: ");
        Serial.println(currentState);

        if (strcmp(currentState, "enrollment") == 0)
        {
          Serial.println("[STATE POLL] System in Enrollment mode.");
          displayMessageAutoCenter("Enrollment", "Mode");
          delay(5000);
        }
        else if (strcmp(currentState, "scanning") == 0)
        {
          Serial.println("[STATE POLL] System in Scanning mode.");
        }
        else
        {
          Serial.println("[STATE POLL] Unknown state, defaulting to Scanning.");
        }
      }
      else
      {
        Serial.println("[STATE POLL] JSON parse error!");
      }
    }
    else
    {
      Serial.printf("[STATE POLL] POST failed, code: %d\n", httpResponseCode);
    }
    http.end();
  }

  // Polls Supabase for locker updates and processes open lockers.
  void checkForUpdates(int ids[])
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[POLL] WiFi not connected, skipping poll.");
      return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String requestUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/functions/v1/get_locker_status";
    String matchFingerprintUrl = serverUrl + "/Locker/GetLockerStatus";
    Serial.println("Locker API URL: " + matchFingerprintUrl);

    http.addHeader("Authorization", "Bearer " + String(supabaseKeyy));
    http.addHeader("apikey", String(supabaseKeyy));
    http.addHeader("Acccept", "*/*");
    http.addHeader("Content-Type", "application/json");

    Serial.println("Function URL: " + requestUrl);

    http.begin(client, requestUrl);

    // Build JSON body with a JSON array of IDs (no extra quotes)
    const int idCount = 3; // Update this if ids array length is dynamic
    String idsString = "[";
    for (int i = 0; i < idCount; i++)
    {
      idsString += String(ids[i]);
      if (i < idCount - 1)
      {
        idsString += ",";
      }
    }
    idsString += "]";
    String jsonBody = "{\"api_url\": \"" + matchFingerprintUrl + "\", \"IDS\": " + idsString + "}";
    Serial.println("[POLL] JSON Body: " + jsonBody);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", String(supabaseKeyy));
    http.addHeader("Authorization", String("Bearer ") + String(supabaseKeyy));
    http.addHeader("Accept", "*/*");

    int httpResponseCode = http.POST(jsonBody);
    if (httpResponseCode > 0)
    {
      String response = http.getString();
      http.end();
      client.stop();

      Serial.print("[POLL] HTTP Code: ");
      Serial.println(httpResponseCode);
      Serial.print("[POLL] Response: ");
      // ✅ Parse JSON Response
      StaticJsonDocument<1024> doc;
      DeserializationError err = deserializeJson(doc, response);


      if (!err && doc.is<JsonArray>()) {
          JsonArray arr = doc.as<JsonArray>();

          for (JsonVariant row : arr) {
              JsonObject obj = row.as<JsonObject>();

              bool isOpen = obj["isOpen"];  // Use correct JSON key format
              String lockerIdStr = obj["id"].as<String>();
              int lockerNumber = obj["lockerNumber"];
              const char* lockerSize = obj["lockerSize"];

              Serial.printf("[POLL] Locker %d (%s) - isOpen: %s\n", lockerNumber, lockerSize, isOpen ? "true" : "false");

              if (isOpen) {
                  Serial.printf("[POLL] Locker %d is open, activating solenoid...\n", lockerNumber);
                  openLocker(lockerNumber);
                  delay(2000);
                  setLockerClosed(lockerNumber);
              }
          }
      } 
      else {
          Serial.println("[POLL] JSON parse error!");
          http.end();
      }
    } 
    else {
        Serial.printf("[POLL] GET request failed, code: %d\n", httpResponseCode);
        http.end();
    }

    http.end();
  }

  // Opens a locker based on its number.
  void openLocker(int lockerNumber)
  {
    switch (lockerNumber)
    {
    case 1:
      Serial.println("Opening Small Locker...");
      SolenoidControl::activate(2);
      break;
    case 2:
      Serial.println("Opening Medium Locker...");
      SolenoidControl::activate(13);
      break;
    case 3:
      Serial.println("Opening Large Locker...");
      SolenoidControl::activate(15);
      break;
    default:
      Serial.println("Unknown locker number.");
      break;
    }
  }
  
  // Sends an update to Supabase to mark a locker as closed.
  void setLockerClosed(int lockerId)
  {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();  // Set to true for testing purposes; use false for production
    HTTPClient https;
    
    String requestUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/functions/v1/update_locker_availablitiy";
    String apiUrl = serverUrl + "/Locker/UpdateLockerStatus/" + String(lockerId);
    
    String jsonBody = "{\"api_url\": \"" + apiUrl + "\"}";

    https.begin(client, requestUrl);

    https.addHeader("apikey", String(supabaseKeyy));
    https.addHeader("Authorization", "Bearer " + String(supabaseKeyy));
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Accept", "*/*");

    int httpsResponseCode = https.POST(jsonBody);

    if (httpsResponseCode > 0) {
        Serial.printf("[SUCCESS] Response Code: %d\n", httpsResponseCode);
        String response = https.getString();
        Serial.println("[RESPONSE] " + response);

        // ✅ Handle plain "true" or "false" responses
        response.trim();
        if (response == "true") {
            Serial.println("[INFO] API Response: Success ✅");
        } else if (response == "false") {
            Serial.println("[INFO] API Response: Failure ❌");
        } else {
            Serial.println("[WARNING] Unexpected response format!");
        }
    } else {
        Serial.printf("[ERROR] POST failed: %d - %s\n", httpsResponseCode, https.errorToString(httpsResponseCode).c_str());
    }

    https.end();
    client.stop();


    // if (WiFi.status() != WL_CONNECTED)
    // {
    //     Serial.println("[POLL] WiFi not connected, skipping poll.");
    //     return;
    // }

    // Serial.println("[DEBUG] WiFi is connected!");

    // WiFiClientSecure client;
    // client.setInsecure();      // Disable SSL verification (for testing)

    // HTTPClient http;
    // String requestUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/functions/v1/update_locker_availablitiy";
    // String apiUrl = serverUrl + "/Locker/UpdateLockerStatus/" + String(lockerId);

    // Serial.println("[DEBUG] Request URL: " + requestUrl);
    // Serial.println("[DEBUG] API URL to send: " + apiUrl);

    // http.begin(client, requestUrl);
    
    // // ✅ Add all necessary headers
    // http.addHeader("apikey", String(supabaseKeyy));
    // http.addHeader("Authorization", "Bearer " + String(supabaseKeyy));
    // http.addHeader("Content-Type", "application/json");
    // http.addHeader("Accept", "*/*");

    // // ✅ Proper JSON formatting
    // // String jsonBody = "{\"api_url\": \"" + apiUrl "\"}";
    // String jsonBody = "{\"api_url\":  " + apiUrl + "}";
    // // String jsonBody = "{\"api_url\": \"" + matchFingerprintUrl + "\", \"IDS\": " + idsString + "}";
    
    // Serial.println("[DEBUG] JSON Payload: " + jsonBody);

    // // Send POST request
    // int httpResponseCode = http.sendRequest("POST", jsonBody);

    // if (httpResponseCode > 0) {
    //     Serial.printf("[SUCCESS] Response Code: %d\n", httpResponseCode);
    //     String response = http.getString();
    //     Serial.println("[RESPONSE] " + response);

    //     // ✅ Handle plain "true" or "false" responses
    //     response.trim();
    //     if (response == "true") {
    //         Serial.println("[INFO] API Response: Success ✅");
    //     } else if (response == "false") {
    //         Serial.println("[INFO] API Response: Failure ❌");
    //     } else {
    //         Serial.println("[WARNING] Unexpected response format!");
    //     }
    // } else {
    //     Serial.printf("[ERROR] POST failed: %d - %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
    // }

    // http.end();
  }

  // Verifies a user based on a given BioID by sending fingerprint data.
  bool verifyFingerprintUser(int userBioId, String apiResponse)
  {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String requestUrl = "https://nlljvmwgxlnhdinvkofd.supabase.co/functions/v1/get_parcel";
    String matchFingerprintUrl = serverUrl + "/ParcelLogs/GetActiveParcels";
    http.begin(client, requestUrl);

    http.addHeader("apikey", String(supabaseKeyy));
    http.addHeader("Authorization", String("Bearer ") + String(supabaseKeyy));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Prefer", "return=representation");

    String jsonBody = "{\"api_url\": \"" + matchFingerprintUrl + "\", \"BioID\": " + userBioId + "}";

    int httpResponseCode = http.POST(jsonBody);
    if (httpResponseCode > 0)
    {
      Serial.printf("[CLOSE] PATCH code: %d\n", httpResponseCode);
      apiResponse = http.getString();
      Serial.println("[CLOSE] PATCH response: " + apiResponse);

      http.end();
      return true;
    }
    else
    {
      Serial.printf("[CLOSE] PATCH failed: %d\n", httpResponseCode);

      http.end();
      return false;
    }
  }

  bool isIdInUse(uint8_t id)
  {
    // loadModel() tries to load the fingerprint data
    // from sensor flash into buffer ID #1 or #2
    uint8_t p = finger.loadModel(id);
    return (p == FINGERPRINT_OK);
  }

  int getNextFreeId()
  {
    // We'll read the index array

    uint8_t templateIndex[128];
    memset(templateIndex, 0, sizeof(templateIndex));

    int ret = -1;

    for (uint8_t i = 1; i <= 127; i++)
    {
      if (!isIdInUse(i))
      {
        ret = i; // Found a free slot
      }
    }
    return -1;

    if (ret != FINGERPRINT_OK)
    {
      Serial.println("Error reading template index!");
      return -1; // or some error code
    }

    // Look for the first free spot
    for (int i = 0; i < 128; i++)
    {
      if (templateIndex[i] == 0)
      {
        // 'i' is zero-based => actual ID is (i + 1)
        return (i + 1);
      }
    }
    // If we reach here, everything is in use
    return -1;
  }

  uint8_t getFingerprintEnroll()
  {
    // Pick the ID to use
    uint8_t idToUse = getNextFreeId();
    // If nextId is beyond our max, pick from [RAND_MIN..RANDOM_MAX]
    if (idToUse > MAX_ID)
    {
      idToUse = random(RAND_MIN, RANDOM_MAX + 1);
    }

    Serial.print("Enrolling ID #");
    Serial.println(idToUse);

    // Step 1: Wait for first finger press
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    Serial.println(idToUse);
    while (p != FINGERPRINT_OK)
    {
      p = finger.getImage();
      switch (p)
      {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
      }
    }

    // Convert first image
    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
    }

    // Ask user to remove finger
    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
      p = finger.getImage();
    }

    // Step 2: Wait for second finger press
    Serial.print("ID ");
    Serial.println(idToUse);
    p = -1;
    Serial.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
      p = finger.getImage();
      switch (p)
      {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
      }
    }

    // Convert second image
    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
    }

    // Create model from the two prints
    Serial.print("Creating model for #");
    Serial.println(idToUse);
    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
      Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
      Serial.println("Communication error");
      return 0;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
      Serial.println("Fingerprints did not match");
      return 0;
    }
    else
    {
      Serial.println("Unknown error");
      return 0;
    }

    // Store model in flash at idToUse
    p = finger.storeModel(idToUse);
    if (p == FINGERPRINT_OK)
    {
      Serial.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
      Serial.println("Communication error");
      return 0;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
      Serial.println("Could not store in that location");
      return 0;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
      Serial.println("Error writing to flash");
      return 0;
    }
    else
    {
      Serial.println("Unknown error");
      return 0;
    }

    return idToUse; // return the newly enrolled ID
  }
};

#endif
