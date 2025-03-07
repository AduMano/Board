// Arduino Uno R3 Main - Parcel Locker System
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Ultrasonic Pins
const int trigPins[3] = {A0, A1, A2};
const int echoPins[3] = {6, 7, 8};
const int thresholds[3] = {10, 20, 20}; // Thresholds: Small=10cm, Medium & Large=20cm

// LED Pins (Green, Red)
const int greenLED[3] = {3, 4, 5};
const int redLED[3] = {9, 10, 11};

// Button Pins
const int buttonMyself = 12;
const int buttonElse = 13;

unsigned long lastModeChange = 0;
String currentMode = "MYSELF";

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Set Ultrasonic pins
  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(redLED[i], OUTPUT);
    pinMode(greenLED[i], OUTPUT);
    digitalWrite(greenLED[i], HIGH);
    digitalWrite(redLED[i], LOW);
  }

  // Set button pins
  pinMode(buttonMyself, INPUT_PULLUP);
  pinMode(buttonElse, INPUT_PULLUP);

  Serial.begin(9600);
}

// Read ultrasonic distance
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) / 58;
}

void loop() {
  // Locker Status Check
  for (int i = 0; i < 3; i++) {
    long distance = readDistance(trigPins[i], 6 + i);
    if (distance < thresholds[i]) {
      digitalWrite(redLED[i], HIGH);
      digitalWrite(greenLED[i], LOW);
    } else {
      digitalWrite(greenLED[i], HIGH);
      digitalWrite(redLED[i], LOW);
    }
  }
  delay(300);
}
