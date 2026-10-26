#include "Publisher.hpp"

#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "MqttClient.hpp"
#include <ArduinoJson.h>

Publisher& Publisher::getInstance() {
  static Publisher instance;
  return instance;
}

Publisher::Publisher() :
  httpClient(PUB_HOST, PUB_PORT, PUB_ENDP),
  started(false) {}

Publisher::~Publisher() {
  httpClient.stop();
}

void Publisher::start() {
  if (started) return;

  started = true;
  httpClient.start();
}

void Publisher::stop() {
  started = false;

  httpClient.stop();
}

void Publisher::restart() {
  stop();
  start();
}

void Publisher::update() {
  if (!started) return;

  publishTelemetry();

  httpClient.update();
}

void Publisher::publishTelemetry() {
  if (estacao == nullptr) return;

  unsigned long currentTime = millis();

  if (currentTime - lastSendTime >= send_interval_ms) {
    lastSendTime = currentTime;

    currentMessage.buildFrom(*estacao);
    const String currentContent = currentMessage.toJson();

    HttpRequest req("POST", currentContent);
    req.addHeader("Content-Type", "application/json");

    httpClient.send(req);
    
    MqttMessage msg(MQTT_TELEMETRY_TOPIC, currentContent);
    MQTT.sendMessage(msg);
  }
}

void Publisher::publishEvent(const char* eventType, const char* eventData) {
  if (MQTT.isConnected()) {
    JsonDocument doc;
    doc["type"] = eventType;
    doc["data"] = eventData;
    doc["timestamp"] = millis();

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    MqttMessage msg(MQTT_EVENT_TOPIC, jsonPayload);
    MQTT.sendMessage(msg);
  }
}

void Publisher::setEstacao(Estacao* est) {
  estacao = est;
}
