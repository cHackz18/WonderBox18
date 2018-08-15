#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include "WonderButton.h"

  // set of enumerated button values.
  enum Buttons {
    Mic1,
    Mic2,
    Mic3,
    Mic4,
    Lock,
    ButtonsSize
  };

  // set of enumerated button actions.
  enum Action {
    Press,
    Release,
    ActionSize
  };

class ButtonController {
public:

  // no need to call destructor as instance should be around until program end.
  // ~ButtonController();
  
  // Get the instance of the ButtonController singleton.
  ButtonController * GetInstance();

  // Initialise all buttons, returns false if pins aren't set, true otherwise.
  bool InitialiseButtons();

  // Set the pin number of the button denoted by button.
  SetButtonPin(const Buttons button, const unsigned int pin);

  // Set the Command payload that a button delivers for an action.
  SetCommandOnAction(const Buttons button, const Action action, const OSCCommand & oscCommand);

  // Have the button controller act on any button actions.
  bool ProcessButtonPresses();

private:
  // default constructor called in GetInstance() once and only once.
  ButtonController();

  // collection of buttons of ButtonsSize.
  WonderButton m_buttons[ButtonsSize];
};

#endif
