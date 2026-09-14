#pragma once

#include <Arduino.h>

#include <cstddef>
#include <cstdint>

class LoggerBase : public Print {
  bool enabled = true;

public:
  using Print::write;
  size_t write(uint8_t c);
  virtual void update();

  virtual bool isEnabled() const;
  virtual void disable();
  virtual void enable();
};
