#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

#include <Arduino.h>

class BuzzerManager
{
private:
  int _pin;

public:
  BuzzerManager(int pin) : _pin(pin)
  {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW); // off
  }
  void beep(unsigned int duration = 100)
  {
    digitalWrite(_pin, HIGH);
    delay(duration);
    digitalWrite(_pin, LOW);
    delay(duration);
  }
  void beepSuccess() { beep(100); }
  void beepError()
  {
    beep(100);
    beep(100);
  }
  void beepAttention()
  {
    beep(5000);
  }
};

#endif
