#pragma once

#include "View.hpp"
#include "DHTSensor.hpp"

class DHTView : public View<DHTSensor> {
  String errors;
  void logValues();
  void logErrors();

public:
  void addError(String error);

  void render() override;
};
