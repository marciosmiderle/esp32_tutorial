#pragma once

#include <Arduino.h>

class Console {
  Stream& io;
  String rx;

public:
  Console(Stream& _io) : io(_io) {}

  size_t printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
  size_t print(const String &s) { return io.print(s); }
  size_t print(const char str[]) { return io.print(str); }
  size_t println(const String &s) { return io.println(s); }
  size_t println(const char str[]) { return io.println(str); }
  size_t println() { return io.println(); }

  int available() { return io.available(); }
  int read() { return io.read(); }
  int peek() { return io.peek(); }

  String readAvailableUntil(char terminator, bool includeTerminator = false);
};
