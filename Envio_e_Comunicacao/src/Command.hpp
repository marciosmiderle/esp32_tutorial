#pragma once

#include <Arduino.h>
#include "Console.hpp"

class Command {
public:
  static const int MAX_ARGS = 3;

  String name;
  String args[MAX_ARGS];
  size_t argsCount = 0;
  Console* sender;

  static Command fromCommandLine(const String& input, Console* sender = nullptr);
  bool isEmpty();
  void clear();
  int getArgsCount() const;
};
