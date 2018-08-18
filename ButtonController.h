#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include "CommandButton.h"

  // set of enumerated CommandButton values.
  // these must stay enumerated 0 to Size.
  enum CommandButtons {
    Mic1,
    Mic2,
    CPA,
    LoKc,
    CommandButtonsSize
  };

  // set of enumerated button actions.
  // these must stay enumerated 0 to Size.
  enum Action {
    Press,
    Release,
    ActionSize
  };

class ButtonController {
public:

  // clean up global button controller on program exit.
  ~ButtonController();

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
  bool HandleCommandButtons();
  bool HandleToggleButtons();

private:
  // default constructor called in GetInstance() once and only once.
  ButtonController();

  // collection of CommandButtons of CommandButtonsSize.
  CommandButton m_commandButtons[CommandButtonsSize];
};

#endif
