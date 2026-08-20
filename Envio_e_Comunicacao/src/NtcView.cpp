#include <Arduino.h>

#include "NtcView.hpp"

void NtcView::logValues() {
  const float C = model->temperatureInCensius(model->processedValue);
  const float CwithEMA = model->filteredTemperatureWithEMA;
  const float V = model->tension(model->processedValue);
  Serial.printf("NTC raw value = %.0f / Tension = %.1fV / Temperature = %.1f°C %.1f°C (EMA)", model->processedValue, V, C, CwithEMA);
  Serial.println("");
}

void NtcView::render() {
  if (!isValid()) {
    logValues();
    setValid();
  }
}
