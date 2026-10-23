#include "Command.hpp"
#include <ctype.h>

Command Command::fromCommandLine(const String& input, void* sender) {
  Command cmd;
  cmd.sender = sender;

  int len = input.length();
  int startIdx = 0;

  while (startIdx < len && isblank(input.charAt(startIdx))) {
    startIdx++;
  }

  int endIdx = startIdx;
  while (endIdx < len && !isblank(input.charAt(endIdx))) {
    endIdx++;
  }

  cmd.name = input.substring(startIdx, endIdx);
  startIdx = endIdx;

  while (startIdx < len) {
    while (startIdx < len && isblank(input.charAt(startIdx))) {
      startIdx++;
    }

    if (startIdx >= len) {
      break;
    }

    endIdx = startIdx;
    while (endIdx < len && !isblank(input.charAt(endIdx))) {
      endIdx++;
    }

    if (cmd.argsCount < MAX_ARGS) {
      cmd.args[cmd.argsCount++] = input.substring(startIdx, endIdx);
    } else {
      break;
    }

    startIdx = endIdx;
  }

  return cmd;
}

bool Command::isEmpty() {
  return name.isEmpty();
}

void Command::clear() {
  name = "";
  argsCount = 0;
  sender = nullptr;
}

int Command::getArgsCount() const {
  return argsCount;
}
