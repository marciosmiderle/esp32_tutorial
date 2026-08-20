#include "Estacao.hpp"

Estacao::Estacao(NtcSensor& _ntc, DHTSensor& _dht, PirSensor& _pir)
    : ntc(_ntc), dht(_dht), pir(_pir) {}

const char* Estacao::interpretTemperature(float tempC) {
  if (tempC < 10.0) return "Frio";
  if (tempC < 18.0) return "Fresco";
  if (tempC < 26.0) return "Normal";
  if (tempC < 32.0) return "Quente";
  return "Critico";
}

const char* Estacao::interpretHumidity(float humidity) {
  if (humidity < 30.0) return "Seco";
  if (humidity < 60.0) return "Normal";
  if (humidity < 80.0) return "Umido";
  return "Critico";
}

float Estacao::meanTemperature() {
  const TempAndHumidity dhtValues = dht.processedValueWithEMA;
  const float dhtTemp = dhtValues.temperature;
  const float ntcTemp = ntc.filteredTemperatureWithEMA;

  return (ntcTemp + dhtTemp) / 2;
}

bool Estacao::sensorDivergence() {
  const TempAndHumidity dhtValues = dht.processedValueWithEMA;
  const float dhtTemp = dhtValues.temperature;
  const float ntcTemp = ntc.filteredTemperatureWithEMA;

  float tempDiff = abs(ntcTemp - dhtTemp);
  return tempDiff > 5.0;
}

const char* Estacao::interpretMotion() {
  return pir.inMotion() ? "Movimento" : "Parado";
}

const char* Estacao::interpretAttentionLevel(int level) {
  switch (level) {
  case 0:
    return "Verde";
  case 1:
    return "Amarelo";
  case 2:
    return "Laranja";
  default:
    return "Vermelho";
  }
}

int Estacao::calculateAttentionLevel() {
  const float ntcTemp = ntc.filteredTemperatureWithEMA;
  const TempAndHumidity dhtValues = dht.processedValueWithEMA;
  const float dhtTemp = dhtValues.temperature;
  const float humidity = dhtValues.humidity;
  const bool motion = pir.inMotion();

  const float tempC = (ntcTemp + dhtTemp) / 2;
  const bool sDivergence = sensorDivergence();

  int level = 0;

  // Temperatura crítica
  if (tempC >= 32.0 || tempC < 10.0)
    level += 2;
  else if (tempC >= 26.0 || tempC < 18.0)
    level += 1;

  // Umidade crítica
  if (humidity >= 80.0 || humidity < 30.0)
    level += 2;
  else if (humidity >= 60.0 || humidity < 40.0)
    level += 1;

  // Divergência entre sensores
  if (sDivergence) level += 2;

  // Movimento (pode indicar intrusão)
  if (motion) level += 1;

  // Limita em 3 (máximo)
  return min(level, 3);
}

void Estacao::update() {
  ntc.update();
  dht.update();
  pir.update();
}
