#ifndef FINGERPRINT_MANAGER_H
#define FINGERPRINT_MANAGER_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <buzzer_manager.h>
#include <Base64.h>
#include <lcd_manager.h>
#include <supabase_fetch.h> // Ensure this header defines SupabaseFetcher

// Fingerprint sensor wiring (NodeMCU):
// Sensor TX -> D5, Sensor RX -> D6.
static SoftwareSerial fingerSerial(D5, D6); // RX = D6, TX = D5
static Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void printHex(int num, int precision)
{
  char tmp[16];
  char format[128];


  
  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

void initFingerprint()
{
  finger.begin(57600);
  delay(1000);
  if (finger.verifyPassword())
  {
    Serial.println("[Fingerprint] Sensor found!");
    displayMessage(" Finger Scanner ", "  Initialized!  ");
    delay(2000);
  }
  else
  {
    Serial.println("[Fingerprint] Sensor not found or wrong password!");
    displayMessage(" Finger Scanner ", "   Not  Found   ");
    delay(2000);
  }
}

// bool captureFingerprintTemplate(uint8_t *templateBuffer, uint16_t &templateLength)
// {
//   int p = -1;
//   Serial.println("[Fingerprint] Place finger on sensor...");
//   while (p != FINGERPRINT_OK)
//   {
//     p = finger.getImage();
//     delay(100);
//   }
//   p = finger.image2Tz(1);
//   if (p != FINGERPRINT_OK)
//   {
//     Serial.println("[Fingerprint] Error converting first image");
//     return false;
//   }
//   Serial.println("[Fingerprint] Remove finger...");
//   delay(2000);
//   while (finger.getImage() != FINGERPRINT_NOFINGER)
//   {
//     delay(1000);
//   }
//   Serial.println("[Fingerprint] Place same finger again...");
//   p = -1;
//   while (p != FINGERPRINT_OK)
//   {
//     delay(3000);
//     p = finger.getImage();
//   }
//   p = finger.image2Tz(2);
//   if (p != FINGERPRINT_OK)
//   {
//     Serial.println("[Fingerprint] Error converting second image");
//     return false;
//   }
//   p = finger.createModel();
//   if (p != FINGERPRINT_OK)
//   {
//     Serial.println("[Fingerprint] Error creating model");
//     return false;
//   }
//   // Simulate downloading the fingerprint template.
//   templateLength = 512; // Adjust as needed.
//   for (uint16_t i = 0; i < templateLength; i++)
//   {
//     templateBuffer[i] = i % 256;
//   }

//   // --- Now perform a local match against a simulated user list ---
//   const int NUM_USERS = 2;
//   struct User
//   {
//     int id;
//     String name;
//     uint8_t fingerprintTemplate[512];
//   };

//   User users[NUM_USERS];

//   // User 1: "User One" – assign an inverted template to simulate no parcels.
//   for (uint16_t i = 0; i < templateLength; i++)
//   {
//     users[0].fingerprintTemplate[i] = 255 - (i % 256);
//   }
//   users[0].id = 1;
//   users[0].name = "User One";

//   // User 2: "User Two" – assign a template identical to the captured one.
//   for (uint16_t i = 0; i < templateLength; i++)
//   {
//     users[1].fingerprintTemplate[i] = i % 256;
//   }
//   users[1].id = 2;
//   users[1].name = "User Two";

//   int matchedUserId = 0;
//   for (int u = 0; u < NUM_USERS; u++)
//   {
//     if (memcmp(templateBuffer, users[u].fingerprintTemplate, templateLength) == 0)
//     {
//       matchedUserId = users[u].id;
//       Serial.println("[Fingerprint] Matched user: " + users[u].name);
//       break;
//     }
//   }

//   // --- Simulate action based on the matched user ---
//   // Assume you have a global buzzer object, e.g.:
//   // BuzzerManager buzzer(BUZZER_PIN);
//   if (matchedUserId == 2)
//   {
//     // User 2 has three parcels: Small, Medium, and Large.
//     Serial.println("[Fingerprint] 3 parcels found for user " + String(matchedUserId));

//     // Open Small locker
//     Serial.println("[Fingerprint] Opening Small locker...");
//     buzzer.beep(100); // beep to signal action
//     // Simulate solenoid activation for small locker:
//     SolenoidControl::activate(SOLENOID_SMALL_GPIO, 1500);
//     delay(2000);

//     // Open Medium locker
//     Serial.println("[Fingerprint] Opening Medium locker...");
//     buzzer.beep(100);
//     SolenoidControl::activate(SOLENOID_MED_GPIO, 1500);
//     delay(2000);

//     // Open Large locker
//     Serial.println("[Fingerprint] Opening Large locker...");
//     buzzer.beep(100);
//     SolenoidControl::activate(SOLENOID_LARGE_GPIO, 1500);
//     delay(2000);
//   }
//   else if (matchedUserId == 1)
//   {
//     Serial.println("[Fingerprint] No parcels associated with user " + String(matchedUserId));
//     // Optionally, you could call a buzzer error beep here:
//     buzzer.beepError();
//   }
//   else
//   {
//     Serial.println("[Fingerprint] No matching user found.");
//     // Optionally, beep an error:
//     buzzer.beepError();
//   }

//   return true;
// }

int captureFingerprintTemplate()
{
  int p = -1;

  p = finger.getImage();

  if (p == FINGERPRINT_OK)
  {
    Serial.println("[Fingerprint] Finger detected, processing...");

    if (finger.image2Tz(1) == FINGERPRINT_OK)
    {
      Serial.println("[Fingerprint] Image successfully converted.");

      // 🔄 Try to match fingerprint in sensor memory
      Serial.println("[Fingerprint] Searching for a match...");
      p = finger.fingerFastSearch();

      if (p == FINGERPRINT_OK)
      {
        Serial.print("[Fingerprint] Match found! User ID: ");
        Serial.println(finger.fingerID);

        // 🔹 Ensure the user removes their finger before continuing
        buzzer.beepSuccess();
        Serial.println("[Fingerprint] Remove your finger...");
        displayMessage(" Please  Remove ", "   Your  Finger   ");
        delay(1000);

        while (finger.getImage() != FINGERPRINT_NOFINGER)
        {
          delay(100);
        }

        return finger.fingerID; // ✅ Return matched fingerprint ID
      }
      else
      {
        Serial.println("[Fingerprint] No match found. Try again.");
        displayMessage(" No Match Found ", "  Try Again...  ");
        buzzer.beepError();
        delay(1000);
        displayMessage("   Place your   ", "     Finger     ");

        return -1;
      }
    }
    else
    {
    }
  }
  else
  {
    Serial.print("."); // Indicate waiting for finger
  }

  delay(300);
  return -1;
}

const uint8_t MAX_ENROLL_ID = 100; // Define maximum enrollment id allowed.
uint8_t id;

uint8_t getFingerprintEnroll()
{
  // Check if all IDs are taken
  if (id > MAX_ENROLL_ID)
  {
    displayMessage("Enrollment", "Can't connect API");
    delay(2000);
    return FINGERPRINT_BADLOCATION; // or choose an error code that fits your design
  }

  // Display initial enrollment message
  displayMessage("Enroll Finger", "Place finger");
  delay(2000);

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  displayMessage("Enroll Finger", "Waiting...");
  delay(1000);

  // Capture first image until valid
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      displayMessage("Image Status:", "Image taken");
      delay(1000);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayMessage("Error:", "Comm error");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      displayMessage("Error:", "Imaging error");
      delay(1000);
      break;
    default:
      Serial.println("Unknown error");
      displayMessage("Error:", "Unknown error");
      delay(1000);
      break;
    }
  }

  // Convert first image to template slot 1
  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    displayMessage("Conversion:", "Image converted");
    delay(1000);
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    displayMessage("Conversion:", "Messy image");
    delay(1000);
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    displayMessage("Conversion:", "Comm error");
    delay(1000);
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("No fingerprint features");
    displayMessage("Conversion:", "No features");
    delay(1000);
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Invalid image");
    displayMessage("Conversion:", "Invalid image");
    delay(1000);
    return p;
  default:
    Serial.println("Unknown error");
    displayMessage("Conversion:", "Unknown error");
    delay(1000);
    return p;
  }

  Serial.println("Remove finger");
  displayMessage("Remove finger", "");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  displayMessage("Place finger", "again for enroll");
  delay(2000);

  // Capture second image until valid
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      displayMessage("Image Status:", "Image taken");
      delay(1000);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayMessage("Error:", "Comm error");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      displayMessage("Error:", "Imaging error");
      delay(1000);
      break;
    default:
      Serial.println("Unknown error");
      displayMessage("Error:", "Unknown error");
      delay(1000);
      break;
    }
  }

  // Convert second image to template slot 2
  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    displayMessage("Conversion:", "Image converted");
    delay(1000);
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    displayMessage("Conversion:", "Messy image");
    delay(1000);
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    displayMessage("Conversion:", "Comm error");
    delay(1000);
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("No fingerprint features");
    displayMessage("Conversion:", "No features");
    delay(1000);
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Invalid image");
    displayMessage("Conversion:", "Invalid image");
    delay(1000);
    return p;
  default:
    Serial.println("Unknown error");
    displayMessage("Conversion:", "Unknown error");
    delay(1000);
    return p;
  }

  // Create model from the two templates
  Serial.print("Creating model for #");
  Serial.println(id);
  displayMessage("Creating Model", "for enroll");
  delay(1000);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
    displayMessage("Model Creation", "Prints matched!");
    delay(1000);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    displayMessage("Model Creation", "Comm error");
    delay(1000);
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    displayMessage("Model Creation", "Mismatch");
    delay(1000);
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    displayMessage("Model Creation", "Unknown error");
    delay(1000);
    return p;
  }

  Serial.print("Storing model for #");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
    displayMessage("Enrollment", "Stored successfully");
    delay(1000);
    // Enrollment successful: return current id, then increment for next enrollment
    uint8_t enrolledId = id;
    id++; // Increment global enrollment id for next time
    return enrolledId;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    displayMessage("Enrollment", "Comm error");
    delay(1000);
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Bad location");
    displayMessage("Enrollment", "Bad location");
    delay(1000);
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Flash error");
    displayMessage("Enrollment", "Flash error");
    delay(1000);
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    displayMessage("Enrollment", "Unknown error");
    delay(1000);
    return p;
  }
}

#endif
