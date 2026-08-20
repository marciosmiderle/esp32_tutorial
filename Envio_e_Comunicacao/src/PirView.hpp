#pragma once

#include "View.hpp"
#include "PirSensor.hpp"

class PirView : public View<PirSensor> {
public:
  String msg;

  void render() override {
    if (!isValid()) {
      if (msg != "") {
        Serial.println(msg);
        msg = "";        
      }
      setValid();
    }
  };
};
