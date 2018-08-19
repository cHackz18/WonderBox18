#include "ButtonController.h"

static ButtonController * bController = nullptr;

// clean up global button cleaner on clean up.
~ButtonController() {
  delete bController;
  bController = nullptr;
}

// default constructor called in GetInstance() once and only once.
ButtonController::ButtonController() {}

// Initialise all buttons, returns false if pins aren't set, true otherwise.
bool InitialiseButtons() {
  // loop through each button and set up their read mode.
  for (auto * button = std::beg(m_buttons); button != std::end(m_buttons); ++button)
    if (!button->Initialise(ButtonInit::ReadUp))
      return false;

  return true;
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

  m_buttons[button].SetPin(pin);
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

// Set the OSC enabled device this button controller will output commands to.
bool SetDevice(OSCDevice * device) {
  // make sure device is valid.
  if (!device)
    return false;

  
}

// Have the button controller act on any button actions.
bool ProcessButtonPresses() {
  // handle the toggle button first.
  if (m_toggleButton.Changed())
    if (!HandleToggleButton())
      return false;

  // process each command button in m_buttons.
  for (auto * button = std::beg(m_buttons); button != std::end(m_buttons); ++button)
    if (button->Changed())
      if (!HandleCommandButton(*button))
        return false;

  return true;
}

// Handle logic for setting up command buttons as toggles.
bool HandleToggleButton() {
  ToggleMode mode = (m_toggleButton.Pressed() ? ToggleMode::On : ToggleMode::Off);

  for (auto * button = std::begin(m_buttons); button != ButtonSize; ++button)
    if (!button->SetToggleMode(mode))
      return false;

  return true;
}

// Handle the logic for delivering a command payload.
bool HandleCommandButton(const CommandButton & commandButton) {

  

  return true;
}
