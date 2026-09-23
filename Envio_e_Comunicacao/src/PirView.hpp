#pragma once

#include "View.hpp"
#include "PirSensor.hpp"

class PirView : public View<PirSensor> {
public:
  PirView(PirSensor* sensor, Console& console) : View(sensor, console) {}
  String msg;

  void render() override {
    if (!isValid()) {
      if (msg != "") {
        console.println(msg);
        msg = "";        
      }
      setValid();
    }
  };
};
