#pragma once

#include <Arduino.h>

#include "Estacao.hpp"
#include "View.hpp"

class EstacaoView : public View<Estacao> {
  String errors;
  void logValues();
  void logErrors();

public:
  EstacaoView(Estacao* estacao);
  void addError(String error);

  const char* interpretTemperature(float tempC);
  const char* interpretHumidity(float humidity);
  const char* interpretMotion(bool motionDetected);
  const char* interpretAttentionLevel(int level);
  int calculateAttentionLevel(float tempC, float humidity,
                              bool sensorDivergence, bool motion);

  void render() override;
};
