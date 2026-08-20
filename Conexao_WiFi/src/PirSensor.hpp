#pragma once

#include <functional>

#include "Part.hpp"
#include "Telemetry.hpp"

class PirSensor : public Part, public Telemetry {
public:
  const unsigned long timeActiveAfterNoMotion_ms = 1000;

  volatile unsigned long lastTrigger_ms = 0;
  volatile bool startTimer = false;
  bool motion = false;

  std::function<void()> motionStartEvent = []() {};
  std::function<void()> motionStopEvent = []() {};

  void begin(int aPin);
  bool inMotion() const;
  void motionStart();

  void update();
};
