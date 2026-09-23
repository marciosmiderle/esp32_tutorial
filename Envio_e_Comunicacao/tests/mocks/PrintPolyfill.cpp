#include "PrintPolyfill.hpp"
#include <cstdarg>

size_t PrintPolyfill::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  size_t ret = vprintf(format, arg);
  va_end(arg);
  return ret;
}

size_t PrintPolyfill::vprintf(const char *format, va_list arg) {
  char loc_buf[64];
  char *temp = loc_buf;
  va_list copy;
  va_copy(copy, arg);
  int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
  va_end(copy);
  if (len < 0) {
    va_end(arg);
    return 0;
  }
  if (len >= (int)sizeof(loc_buf)) {
    temp = (char *)malloc(len + 1);
    if (temp == NULL) {
      va_end(arg);
      return 0;
    }
    len = vsnprintf(temp, len + 1, format, arg);
  }
  va_end(arg);
  len = write((uint8_t *)temp, len);
  if (temp != loc_buf) {
    free(temp);
  }
  return len;
}
