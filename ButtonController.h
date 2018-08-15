#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include "Button.h" https://github.com/madleech/Button

class ButtonController {
public:

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

  // Get the instance of the ButtonController singleton.
  ButtonController * GetInstance();

  // Set the pin number of the button denoted by button.
  SetButtonPin(const Button button, const unsigned int pin);

  // Set the Command payload that a button delivers for an action.
  SetCommandOnAction(const Button button, const Action action, const OSCCommand & oscCommand);

  // Have the button controller act on any button actions.
  bool ProcessButtonPresses();

private:
  // default constructor called in GetInstance() once and only once.
  ButtonController();

  // collection of buttons of ButtonsSize.
  Button buttons[ButtonsSize];
};

#endif

auto bc = ButtonController::GetInstance();

bc.Get(Mic1);
