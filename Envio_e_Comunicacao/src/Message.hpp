#pragma once

#include <Arduino.h>
#include "Estacao.hpp"

class Message {
public:
  // Dados de telemetria
  float temperature;
  float humidity;
  bool motionDetected;
  unsigned long timestamp;
  
  // Status dos sensores
  const char* ntcHealth;
  float ntcHealthRatio;
  const char* dhtHealth;
  float dhtHealthRatio;
  const char* pirHealth;
  float pirHealthRatio;
  
  // Interpretações
  const char* tempStatus;
  const char* humidityStatus;
  const char* motionStatus;
  bool sensorDivergence;
  int attentionLevel;
  const char* attentionStatus;
  
  // Observações
  String tempObs;
  String humidityObs;
  String motionObs;
  String divergenceObs;
  
  Message();
  void buildFrom(Estacao& estacao);
  String toJson() const;
  
private:
  void formatObservations(Estacao& estacao);
};
