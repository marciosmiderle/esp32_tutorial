#include <Arduino.h>

#include "NtcView.hpp"

void NtcView::logValues() {
  const float C = model->temperatureInCensius(model->ntcProcessedValue);
  const float CwithEMA = model->ntcFilteredTemperature;
  const float V = model->tension(model->ntcProcessedValue);
  Serial.printf("NTC raw value = %.0f / Tension = %.1fV / Temperature = %.1f°C %.1f°C (EMA)", model->ntcProcessedValue, V, C, CwithEMA);
  Serial.println("");
}

void NtcView::render() {
  if (!isValid()) {
    logValues();
    setValid();
  }
}
