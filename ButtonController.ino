#include "ButtonController.h"

static ButtonController * bController = nullptr;

// default constructor called in GetInstance() once and only once.
ButtonController::ButtonController() {}

// Initialise all buttons, returns false if pins aren't set, true otherwise.
bool InitialiseButtons() {
  // loop through each button and set up
}

// Get the instance of the ButtonController singleton.
ButtonController * ButtonController::GetInstance() {
  if (bController != nullptr)
    return bController;

  bController = new ButtonController;
}

// Set the pin number of the button denoted by button.
void ButtonController::SetButtonPin(const Button button, const unsigned int pin) {
  if (button == ButtonSize)
    return;

  m_buttons[button].set_pin(pin);
}

// Set the Command payload that a button delivers for an action.
void ButtonController::SetCommandOnAction(const Buttons button, const Action action, const OSCCommand & oscCommand) {
  switch(action) {
  case Action::Press:
    m_buttons[button].SetPressCommand(command);
    break;
  case Action::Release:
    m_buttons[button].SetReleaseCommand(command);
  }
}

// Have the button controller act on any button actions.
bool ProcessButtonPresses() {
  
}
