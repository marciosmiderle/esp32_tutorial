#pragma once

#include <cstdint>

class WatchDog {
private:
  uint32_t timeout_ms;

public:
  WatchDog(uint32_t timeout_ms = 5000);
  void begin();
  void feed();
};
