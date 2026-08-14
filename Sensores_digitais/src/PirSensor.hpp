#pragma once

#include "Part.hpp"
#include <Arduino.h>
#include <functional>

class PirSensor : public Part {
public:
  const unsigned long timeActiveAfterNoMotion_ms = 1000;

  volatile unsigned long lastTrigger_ms = 0;
  volatile bool startTimer = false;
  bool motion = false;

  std::function<void()> motionStartEvent = []() {};
  std::function<void()> motionStopEvent = []() {};

  void begin(int aPin);
  void motionStart();

  void update();
};
