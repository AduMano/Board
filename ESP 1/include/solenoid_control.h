#ifndef SOLENOID_CONTROL_H
#define SOLENOID_CONTROL_H

#include <Arduino.h>

class SolenoidControl
{
public:
  static void activate(int pin, unsigned long duration = 2000)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    Serial.printf("Activating Solenoid on pin: %d\n", pin);
    delay(duration);
    digitalWrite(pin, LOW);
    Serial.printf("Deactivating Solenoid on pin: %d\n", pin);
    delay(200);
  }
};

#endif
