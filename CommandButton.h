#ifndef WONDER_BUTTON_H
#define WONDER_BUTTON_H

#include "Button.h"
#include "Command.h"

class CommandButton : public Button{
public:

  bool SetPressCommand(const Command & command);

  bool SetReleaseCommand(const Command & command);

private:
  Command m_pressedCommand;
  Command m_releasedCommand;
};
#endif
