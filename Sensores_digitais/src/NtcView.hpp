#pragma once

#include "View.hpp"
#include "NtcSensor.hpp"

class NtcView : public View<NtcSensor> {
  void logValues();

public:
  void render() override;
};
