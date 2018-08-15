/*
  Button - a small library for Arduino to handle button debouncing
  
  MIT licensed.
*/

#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class Button
{
  public:
    Button(uint8_t pin);
    // start
    // Author : cHackz18, Project : WonderBox18
    Button();
    bool set_pin(const uint8_t pin);
    // end
    void begin();
    bool read();
    bool toggled();
    bool pressed();
    bool released();
    bool has_changed();


    const static bool PRESSED = LOW;
    const static bool RELEASED = HIGH;

  private:
    uint8_t  _pin;
    uint16_t _delay;
    bool       _state;
    bool       _has_changed;
    uint32_t _ignore_until;
};

#endif
