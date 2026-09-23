#include "Console.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

size_t Console::printf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  size_t ret = _out.vprintf(format, arg);
  va_end(arg);
  return ret; 
}
