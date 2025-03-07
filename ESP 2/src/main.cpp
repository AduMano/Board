#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include "buzzer_manager.h"

// Fingerprint sensor on D5 (TX) and D6 (RX)
SoftwareSerial fingerSerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// LCD on I2C (address 0x27, 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Buzzer on D0
BuzzerManager buzzer(D0);

// Communication with Polling Board via SoftwareSerial (for example, D2 = RX, D3 = TX)
SoftwareSerial commSerial(D2, D3);

enum BiometricsState {
  SCANNING_MODE,
  ENROLLMENT_MODE,
  WAIT_REMOVAL,
  DISPLAY_RESULT
};

BiometricsState currentState = SCANNING_MODE;
unsigned long stateStartTime = 0;
const unsigned long ENROLLMENT_TIMEOUT = 2 * 60 * 1000; // 2 minutes timeout
int scannedUserID = -1;

void displayStatus(const String &line1, const String &line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void initBiometrics() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");
  
  finger.begin(57600);
  delay(1000);
  if (finger.verifyPassword()) {
    Serial.println("[Fingerprint] Sensor OK");
    displayStatus("Fingerprint", "Sensor OK");
  } else {
    Serial.println("[Fingerprint] Sensor Error!");
    displayStatus("Sensor Error", "Check wiring");
  }
  delay(2000);
}

void processScanningMode() {
  // In Scanning Mode, continuously prompt for finger.
  displayStatus("Scanning Mode", "Place Finger");

  // Check if a command has arrived to switch to Enrollment Mode
  if (commSerial.available()) {
    String cmd = commSerial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "ENROLL") {
      currentState = ENROLLMENT_MODE;
      stateStartTime = millis();
      Serial.println("[Biometrics] Switching to Enrollment Mode");
      displayStatus("Enrollment Mode", "Scan Finger");
      return;
    }
  }
  
  // Poll for finger image without blocking long delays:
  int p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    // Try to convert image
    if (finger.image2Tz(1) == FINGERPRINT_OK) {
      int searchResult = finger.fingerFastSearch();
      if (searchResult == FINGERPRINT_OK) {
        scannedUserID = finger.fingerID;
        Serial.print("[Biometrics] Match found, ID: ");
        Serial.println(scannedUserID);
      } else {
        scannedUserID = -1;
        Serial.println("[Biometrics] Unknown Finger");
      }
      currentState = WAIT_REMOVAL;
      stateStartTime = millis();
    } else {
      Serial.println("[Biometrics] Error converting image");
    }
  }
}

void processWaitRemoval() {
  // Prompt user to remove finger
  displayStatus("Processing...", "Remove Finger");
  int p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) {
    currentState = DISPLAY_RESULT;
    stateStartTime = millis();
  }
}

void processDisplayResult() {
  if (scannedUserID != -1) {
    displayStatus("User: " + String(scannedUserID), "Parcel Found");
    buzzer.beepSuccess();
    // Send result to Polling Board
    commSerial.println("RESULT:" + String(scannedUserID));
  } else {
    displayStatus("Unknown User", "Try Again");
    buzzer.beepError();
    commSerial.println("RESULT:UNKNOWN");
  }
  // After showing the result for 2 seconds, return to scanning mode.
  if (millis() - stateStartTime > 2000) {
    currentState = SCANNING_MODE;
  }
}

void processEnrollmentMode() {
  // In Enrollment Mode, immediately attempt to enroll a finger.
  displayStatus("Enrollment Mode", "Scan for Enroll");
  
  int p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    if (finger.image2Tz(1) == FINGERPRINT_OK) {
      int enrollResult = finger.createModel();
      if (enrollResult == FINGERPRINT_OK) {
        Serial.println("[Enrollment] Finger enrolled successfully.");
        displayStatus("Enrollment", "Success");
        commSerial.println("ENROLL_SUCCESS");
        buzzer.beepSuccess();
      } else {
        Serial.println("[Enrollment] Enrollment failed.");
        displayStatus("Enrollment", "Failed");
        commSerial.println("ENROLL_FAIL");
        buzzer.beepError();
      }
      // Wait until finger is removed
      while (finger.getImage() != FINGERPRINT_NOFINGER) {
        delay(100);
      }
      currentState = SCANNING_MODE;
    }
  }
  // If enrollment mode is active for over 2 minutes, timeout back to scanning.
  if (millis() - stateStartTime > ENROLLMENT_TIMEOUT) {
    currentState = SCANNING_MODE;
  }
}

void setup() {
  Serial.begin(115200);
  commSerial.begin(9600);
  
  initBiometrics();
  
  // Optionally, if this board needs WiFi for any other function, initialize it here.
  // Otherwise, the Polling Board handles WiFi and API calls.
  
  // Signal readiness to the Polling Board
  displayStatus("BIOMETRICS", "READY");
  commSerial.println("BIOMETRICS_READY");
  
  currentState = SCANNING_MODE;
  stateStartTime = millis();
}

void loop() {
  // Run the state machine for fingerprint processing:
  switch (currentState) {
    case SCANNING_MODE:
      processScanningMode();
      break;
    case WAIT_REMOVAL:
      processWaitRemoval();
      break;
    case DISPLAY_RESULT:
      processDisplayResult();
      break;
    case ENROLLMENT_MODE:
      processEnrollmentMode();
      break;
    default:
      break;
  }
  yield();
  delay(10);
}
