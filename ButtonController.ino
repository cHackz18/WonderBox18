#include "ButtonController.h"

static ButtonController * bController = nullptr;

// default constructor called in GetInstance() once and only once.
ButtonController::ButtonController() {}

// Get the instance of the ButtonController singleton.
ButtonController * ButtonController::GetInstance() {
  if (bController != nullptr)
    return bController;

  bController = new ButtonController;
}

// Set the pin number of the button denoted by button.
void ButtonController::SetButtonPin(const Button button, const unsigned int pin) {
  
}

// Set the Command payload that a button delivers for an action.
void ButtonController::SetCommandOnAction(const Button button, const Action action, const OSCCommand & oscCommand) {
  
}

// Have the button controller act on any button actions.
bool ProcessButtonPresses() {
  
}
