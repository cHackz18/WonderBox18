#include "CommandButton.h"

bool CommandButton::SetPin(uint8_t pin) {
  return button.set_pin(pin);
}

void CommandButton::SetCommand(const Command & command) {
  m_pressedCommand = command;
}

void CommandButton::SetInverseCommand(const Command & command) {
  m_releaseCommand = command;
}

bool SetToggleMode(const ToggleMode mode) {
  if (!super::m_initialised)
    return false;

  m_mode = mode;
  if (m_mode == ToggleMode::Off)
    return
}
