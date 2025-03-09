// ESP8266 NodeMCU - Fingerprint, LCD, and Buzzer Control
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include "wifi_setup.h"
#include "supabase_fetch.h"
#include "buzzer_manager.h"
#include "fingerprint_manager.h"

SupabaseFetcher supabase;

void setup() {
  Serial.begin(74880);
  Wire.begin(D2, D1);
  initLCD();
  
  displayMessageAutoCenter("Initializing...", "Please wait...");

  connectToWiFi();
  supabase.fetchServerUrl();

  initFingerprint();

  displayMessageAutoCenter("System is", "ready!");
}

void loop() {
  displayMessageAutoCenter("Put Finger", "On  Biometrics");

  delay(500);
}
