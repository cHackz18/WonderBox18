#ifndef WONDER_BUTTON_H
#define WONDER_BUTTON_H

#include "Button.h"
#include "Command.h"

enum ToggleMode {
  Off,
  On
};

class CommandButton : public Button{
public:

  bool SetPressCommand(const Command & command);

  bool SetReleaseCommand(const Command & command);

  bool SetToggleMode(const ToggleMode mode);

private:
  Command m_pressedCommand;
  Command m_releasedCommand;
  ToggleMode m_mode;
};
#endif
