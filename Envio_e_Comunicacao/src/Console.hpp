#pragma once

#include <Arduino.h>

class Console {
  Print& _out;

public:
  Console(Print& out) : _out(out) {}

  size_t printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
  size_t print(const String &s) { return _out.print(s); }
  size_t print(const char str[]) { return _out.print(str); }
  size_t println(const String &s) { return _out.println(s); }
  size_t println(const char str[]) { return _out.println(str); }
  size_t println() { return _out.println(); }
};
