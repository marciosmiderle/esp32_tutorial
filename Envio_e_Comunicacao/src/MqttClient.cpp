#include "MqttClient.hpp"
#include "Logger.hpp"
#include <ArduinoJson.h>

MqttClient* MqttClient::instance = nullptr;

MqttClient::MqttClient(const char* _broker, int _port, const char* _clientId,
                       const char* _telemetryTopic, const char* _eventTopic,
                       const char* _commandTopic, const char* _commandSubscribeTopic,
                       const char* _logTopic,
                       int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : broker(_broker), port(_port), clientId(_clientId),
    telemetryTopic(_telemetryTopic), eventTopic(_eventTopic),
    commandTopic(_commandTopic), commandSubscribeTopic(_commandSubscribeTopic),
    logTopic(_logTopic), userCallback(nullptr),
    retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), started(false) {

  buildUniqueClientId();

  netClient.setConnectionTimeout(500); // ms
  client.setClient(netClient);
  client.setServer(broker, port);
  client.setBufferSize(1024);       // JSON de telemetria precisa de espaço
  client.setKeepAlive(180);         // segundos
  client.setSocketTimeout(1);       // segundos

  instance = this;
  client.setCallback(mqttCallbackWrapper);
}

void MqttClient::start() {
  started = true;
}

void MqttClient::stop() {
  started = false;
  disconnect();
}

void MqttClient::restart() {
  stop();
  start();
}

void MqttClient::update() {
  if (!started) return;

  if (isConnected()) {
    // publishMessages(); TODO
    client.loop();
    return;
  }

  connect();
}

void MqttClient::buildUniqueClientId() {
  // Evita colisão no broker público: prefixo + MAC
  uint64_t mac = ESP.getEfuseMac();
  snprintf(resolvedClientId, sizeof(resolvedClientId),
           "%s-%04X%04X",
           clientId ? clientId : "estacao",
           (uint16_t)(mac >> 32),
           (uint16_t)(mac & 0xFFFF));
}

void MqttClient::connect() {
  if (isConnected()) {
    return;
  }

  if (!retry.canRetry()) {
    return;
  }

  Log.print("[MQTT] Conectando a ");
  Log.print(broker);
  Log.print(":");
  Log.print(port);
  Log.print(" como ");
  Log.println(resolvedClientId);

  // cleanSession=true: re-subscribe sempre após connect
  bool success = client
    .setKeepAlive(180)
    .setSocketTimeout(1)
    .connect(resolvedClientId);

  if (success) {
    Log.println("[MQTT] Conectado");

    String cmdTopic(resolvedClientId);
    cmdTopic.concat("/");
    cmdTopic.concat(commandSubscribeTopic);
    if (client.subscribe(cmdTopic.c_str(), 1)) {
      Log.print("[MQTT] Subscrito: ");
      Log.println(cmdTopic);
    } else {
      Log.print("[MQTT] Falha subscribe: ");
      Log.println(cmdTopic);
    }

    retry.reset();
    return;
  }

  Log.print("[MQTT] Falha connect, state=");
  Log.println(client.state());
  return;
}

void MqttClient::disconnect() {
  if (isConnected()) {
    client.disconnect();
  }
  retry.reset();
}

bool MqttClient::ensureConnection() {
  if (isConnected()) {
    return true;
  }
  restart();

  return false;
}

bool MqttClient::publishTelemetry(const Message& message) {
  String jsonPayload = message.toJson();
  if (jsonPayload.length() + 16 > client.getBufferSize()) {
    Log.println("[MQTT] Payload maior que o buffer MQTT");
    return false;
  }

  String telTopic(resolvedClientId);
  telTopic.concat("/");
  telTopic.concat(telemetryTopic);
  Log.print("[MQTT] PUB ");
  Log.print(telTopic);
  Log.print(" (");
  Log.print(jsonPayload.length());
  Log.print(" B) ");

  client.loop();
  //addMessage(telTopic.c_str(), jsonPayload.c_str());

  if (!ensureConnection()) {
    Log.println("[MQTT] publishTelemetry: sem conexao");
    return false;
  }
  // QoS 1: pelo menos uma entrega no broker
  bool success = client.publish(telTopic.c_str(), jsonPayload.c_str(), false);
  client.loop();

  if (success) {
    Log.println("OK");
  } else {
    Log.println("FALHA");
    restart();
  }
  return success;
}

bool MqttClient::publishEvent(const char* eventType, const char* eventData) {
  if (!ensureConnection()) {
    Log.println("[MQTT] publishEvent: sem conexao");
    return false;
  }

  JsonDocument doc;
  doc["type"] = eventType;
  doc["data"] = eventData;
  doc["timestamp"] = millis();

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  String eveTopic(resolvedClientId);
  eveTopic.concat("/");
  eveTopic.concat(eventTopic);
  Log.print("[MQTT] EVT ");
  Log.print(eveTopic);
  Log.print(" ");

  client.loop();
  bool success = client.publish(eveTopic.c_str(), jsonPayload.c_str(), false);
  client.loop();

  if (success) {
    Log.println("OK");
  } else {
    Log.println("FALHA");
    restart();
  }
  return success;
}

bool MqttClient::publishLog(const char* data, size_t length) {
  if (!ensureConnection()) {
    //Log.println("[MQTT] publishLog: sem conexao");
    return false;
  }

  String rlogTopic(resolvedClientId);
  rlogTopic.concat("/");
  rlogTopic.concat(logTopic);

  return client.publish(rlogTopic.c_str(), reinterpret_cast<const uint8_t*>(data), length, false);
}

void MqttClient::setCallback(MqttCallback callback) {
  userCallback = callback;
}

void MqttClient::mqttCallbackWrapper(char* topic, byte* payload, unsigned int length) {
  if (instance) {
    instance->handleCommand(topic, payload, length);
  }
}

void MqttClient::handleCommand(char* topic, byte* payload, unsigned int length) {
  Log.print("[MQTT] CMD ");
  Log.print(topic);
  Log.print(": ");

  // Cópia segura
  char message[128];
  unsigned int n = length < sizeof(message) - 1 ? length : sizeof(message) - 1;
  memcpy(message, payload, n);
  message[n] = '\0';
  Log.println(message);

  if (userCallback) {
    userCallback(topic, payload, length);
  }
}

bool MqttClient::isConnected() {
  return client.connected();
}
