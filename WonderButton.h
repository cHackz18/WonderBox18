#ifndef WONDER_BUTTON_H
#define WONDER_BUTTON_H

#include "Button.h"
#include "Command.h"

class WonderButton {
public:
  WonderButton();

  bool SetPin(uint8_t pin);

  bool SetPressCommand(const Command & command);

  bool SetReleaseCommand(const Command & command);

private:
  Button m_button;
  Command m_pressedCommand;
  Command m_releasedCommand;
};

#endif
