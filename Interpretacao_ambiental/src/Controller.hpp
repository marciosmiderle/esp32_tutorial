#pragma once

#include <Arduino.h>

#include "NtcSensor.hpp"
#include "Controls.hpp"

class Controller {
public:
  Button btn;
  NtcSensor* model;

  void buttonPressed()
  {

  }

  void buttonReleased()
  {
    model->enableSampling();
    Serial.println("NTC !");
  }

  void processInput()
  {
    int buttonCurrentState = digitalRead(btn.getPin());
    if (buttonCurrentState == HIGH && btn.state == LOW) {
      buttonPressed();
      btn.state = HIGH;
    }
    if (buttonCurrentState == LOW && btn.state == HIGH) {
      buttonReleased();
      btn.state = LOW;
    }
  }
};

