#pragma once

#include "Part.hpp"
#include <Arduino.h>
#include <functional>


class Button : public Part {
public:
  int state = LOW;

  std::function<void()> buttonPressed = [](){};
  std::function<void()> buttonReleased = [](){};

  void processInput()
  {
    int buttonCurrentState = digitalRead(getPin());
    if (buttonCurrentState == HIGH && state == LOW) {
      buttonPressed();
      state = HIGH;
    }
    if (buttonCurrentState == LOW && state == HIGH) {
      buttonReleased();
      state = LOW;
    }
  }
};
