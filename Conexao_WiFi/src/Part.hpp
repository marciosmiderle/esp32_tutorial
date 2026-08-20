#pragma once

#include <cstdint>

class Part {
  uint8_t pin = 0;

public:
  uint8_t getPin() const { return pin; }

  void begin(uint8_t aPin) {
    pin = aPin;
  }
};
