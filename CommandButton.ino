#include "WonderButton.h"

bool WonderButton::SetPin(uint8_t pin) {
  return button.set_pin(pin);
}

void WonderButton::SetPressedCommand(const Command & command) {
  m_pressedCommand = command;
}

void WonderButton::SetReleasedCommand(const Command & command) {
  m_releaseCommand = command;
}

bool SetToggleMode(const ToggleMode mode) {
  if (super::
  m_mode = mode;
  if (m_mode == ToggleMode::Off)
    return
}