#pragma once

#include "View.hpp"
#include "NtcSensor.hpp"

class NtcView : public View<NtcSensor> {
  void logValues();

public:
  NtcView(NtcSensor* sensor, Console& console) : View(sensor, console) {}
  void render() override;
};
