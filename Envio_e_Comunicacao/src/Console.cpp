#include "Console.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

size_t Console::printf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  size_t ret = io.vprintf(format, arg);
  va_end(arg);
  return ret; 
}

String Console::readAvailableUntil(char terminator, bool includeTerminator) {
  rx = "";
  int ava = io.available();
  char c = '\0';

  while (ava) {
    io.readBytes(&c, 1);
    if (c == terminator) {
      if (includeTerminator) {
        rx += c;
      }
      return rx;
    }
    rx += c;
    ava--;
  }

  return rx;
}
