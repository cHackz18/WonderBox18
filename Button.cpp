/*
  Button - a small library for Arduino to handle button debouncing
  
  MIT licensed.
*/

#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin)
:  initCalled(false)
,  _pin(pin)
,  _delay(100)
,  _state(HIGH)
,  _has_changed(false)
,  _ignore_until(0)
{
}

// start
// Author : cHackz18, Project : WonderBox18
const uint8_t PIN_MAX = 0xFFFF;
Button::Button()
:  initCalled(false)
,  _pin(PIN_MAX)
,  _delay(100)
,  _state(HIGH)
,  _has_changed(false)
,  _ignore_until(0)
{
}

bool Button::SetPin(const uint8_t pin) {

  // can't change the pin if the button is already initialised and running.
  if (initCalled)
    return false;

  // can't change the pin to PIN_MAX as that denotes an uninitialised button state.
  if(pin == PIN_MAX)
    return false;

  _pin = pin;
  return true;
}
// end

enum InputType {
  Standard,
  Pullup

bool Button::Initialise()
{
  if (initCalled)
    return false;
  pinMode(_pin, INPUT_PULLUP);
  return initCalled = true;
}

// 
// public methods
// 

bool Button::Read()
{
  // ignore pin changes until after this delay time
  if (_ignore_until > millis())
  {
    // ignore any changes during this period
  }
  
  // pin has changed 
  else if (digitalRead(_pin) != _state)
  {
    _ignore_until = millis() + _delay;
    _state = !_state;
    _has_changed = true;
  }
  
  return _state;
}

// has the button been toggled from on -> off, or vice versa
bool Button::Toggled()
{
  Read();
  return has_changed();
}

// mostly internal, tells you if a button has changed after calling the read() function
bool Button::HasChanged()
{
  if (_has_changed) {
    _has_changed = false;
    return true;
  }
  return false;
}

// has the button gone from off -> on
bool Button::Pressed()
{
  if (read() == PRESSED && has_changed() == true)
    return true;
  else
    return false;
}

// has the button gone from on -> off
bool Button::Released()
{
  if (read() == RELEASED && has_changed() == true)
    return true;
  else
    return false;
}