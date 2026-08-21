#include "Message.hpp"
#include <ArduinoJson.h>

Message::Message() 
  : temperature(0.0),
    humidity(0.0),
    motionDetected(false),
    timestamp(0),
    ntcHealth("Unknown"),
    ntcHealthRatio(0.0),
    dhtHealth("Unknown"),
    dhtHealthRatio(0.0),
    pirHealth("Unknown"),
    pirHealthRatio(0.0),
    tempStatus("Unknown"),
    humidityStatus("Unknown"),
    motionStatus("Unknown"),
    sensorDivergence(false),
    attentionLevel(0),
    attentionStatus("Unknown") {}

void Message::buildFrom(Estacao& estacao) {
  NtcSensor& ntc = estacao.ntc;
  DHTSensor& dht = estacao.dht;
  PirSensor& pir = estacao.pir;
  
  // Dados brutos dos sensores
  const float ntcTemp = ntc.filteredTemperatureWithEMA;
  const TempAndHumidity dhtValues = dht.processedValueWithEMA;
  const float dhtTemp = dhtValues.temperature;
  const float dhtHumidity = dhtValues.humidity;
  const bool pirMotion = pir.inMotion();
  
  // Telemetria principal
  temperature = estacao.meanTemperature();
  humidity = dhtHumidity;
  motionDetected = pirMotion;
  timestamp = millis();
  
  // Status de saúde dos sensores
  ntcHealth = ntc.health();
  ntcHealthRatio = ntc.healthRatio();
  dhtHealth = dht.health();
  dhtHealthRatio = dht.healthRatio();
  pirHealth = pir.health();
  pirHealthRatio = pir.healthRatio();
  
  // Interpretações
  tempStatus = estacao.interpretTemperature(temperature);
  humidityStatus = estacao.interpretHumidity(humidity);
  motionStatus = estacao.interpretMotion();
  sensorDivergence = estacao.sensorDivergence();
  attentionLevel = estacao.calculateAttentionLevel();
  attentionStatus = estacao.interpretAttentionLevel(attentionLevel);
  
  // Formata observações
  formatObservations(estacao);
}

void Message::formatObservations(Estacao& estacao) {
  tempObs = String(temperature, 1) + "C";
  humidityObs = String(humidity, 1) + "%";
  motionObs = motionDetected ? "Detectado" : "Nenhum";
  divergenceObs = sensorDivergence ? "Diferenca > 5C" : "OK";
}

String Message::toJson() const {
  StaticJsonDocument<512> doc;
  
  // Telemetria básica
  doc["timestamp"] = timestamp;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["motionDetected"] = motionDetected;
  
  // Status dos sensores
  JsonObject sensors = doc.createNestedObject("sensors");
  
  JsonObject ntc = sensors.createNestedObject("ntc");
  ntc["health"] = ntcHealth;
  ntc["healthRatio"] = ntcHealthRatio;
  
  JsonObject dht = sensors.createNestedObject("dht");
  dht["health"] = dhtHealth;
  dht["healthRatio"] = dhtHealthRatio;
  
  JsonObject pir = sensors.createNestedObject("pir");
  pir["health"] = pirHealth;
  pir["healthRatio"] = pirHealthRatio;
  
  // Interpretações
  JsonObject interpretation = doc.createNestedObject("interpretation");
  interpretation["tempStatus"] = tempStatus;
  interpretation["humidityStatus"] = humidityStatus;
  interpretation["motionStatus"] = motionStatus;
  interpretation["sensorDivergence"] = sensorDivergence;
  interpretation["attentionLevel"] = attentionLevel;
  interpretation["attentionStatus"] = attentionStatus;
  
  // Observações
  JsonObject observations = doc.createNestedObject("observations");
  observations["tempObs"] = tempObs;
  observations["humidityObs"] = humidityObs;
  observations["motionObs"] = motionObs;
  observations["divergenceObs"] = divergenceObs;
  
  String output;
  serializeJson(doc, output);
  return output;
}
