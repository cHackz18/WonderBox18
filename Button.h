/*
  Button - a small library for Arduino to handle button debouncing
  
  MIT licensed.
*/

#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

ButtonReadMode {
  ReadDown,
  ReadUp
};

class Button
{
public:
  Button(uint8_t pin);
  // start
  // Author : cHackz18, Project : WonderBox18
  Button();
  bool SetPin(const uint8_t pin);
  // end
  bool Initialise();
  bool Read();
  bool Toggled();
  bool Pressed();
  bool Released();
  bool HasChanged();


  const static bool PRESSED = LOW;
  const static bool RELEASED = HIGH;

protected:
  bool initCalled;
  uint8_t  _pin;
  uint16_t _delay;
  bool       _state;
  bool       _has_changed;
  uint32_t _ignore_until;
};

#endif
