#pragma once

#include <Arduino.h>

class Command {
public:
  static const int MAX_ARGS = 3;

  String name;
  String args[MAX_ARGS];
  size_t argsCount = 0;
  void* sender;

  static Command fromCommandLine(const String& input, void* sender = nullptr);
  bool isEmpty();
  void clear();
  int getArgsCount() const;
};
