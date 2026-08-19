#pragma once

#include "DHTSensor.hpp"
#include "NtcSensor.hpp"
#include "PirSensor.hpp"

class Estacao {
public:
  NtcSensor &ntc;
  DHTSensor &dht;
  PirSensor &pir;

  Estacao(NtcSensor &_ntc, DHTSensor &_dht, PirSensor &_pir);
  const char *interpretTemperature(float tempC);
  const char *interpretHumidity(float humidity);
  float meanTemperature();

  bool sensorDivergence();
  const char *interpretMotion();
  const char *interpretAttentionLevel(int level);
  int calculateAttentionLevel();

  void consolida() {}

  void update();
};
