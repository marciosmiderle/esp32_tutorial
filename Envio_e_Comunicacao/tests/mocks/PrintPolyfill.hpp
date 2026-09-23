#pragma once

#include <Arduino.h>

class PrintPolyfill : public Print {
public:
  size_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)));
  size_t vprintf(const char *format, va_list arg);
};
