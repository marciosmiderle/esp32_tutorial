#include "PirSensor.hpp"

static PirSensor *instance = nullptr;
static void ARDUINO_ISR_ATTR isrTrampoline() {
  if (instance != nullptr) {
    instance->motionStart();
  }
}

void PirSensor::begin(int aPin) {
  instance = this;
  pinMode(aPin, INPUT_PULLUP);
  attachInterrupt(aPin, isrTrampoline, RISING);
}

void PirSensor::motionStart() {
  lastTrigger_ms = millis();
  startTimer = true;
}

void PirSensor::update() {
  if (startTimer && !motion) {
    motion = true;
    motionStartEvent();
  }

  unsigned long int now = millis();
  if (startTimer && (now - lastTrigger_ms > timeActiveAfterNoMotion_ms)) {
    startTimer = false;
    motion = false;
    motionStopEvent();
  }
}
