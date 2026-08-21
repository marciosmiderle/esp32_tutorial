#include "MqttClient.hpp"
#include <ArduinoJson.h>

MqttClient* MqttClient::instance = nullptr;

MqttClient::MqttClient(const char* _broker, int _port, const char* _clientId,
                       const char* _telemetryTopic, const char* _eventTopic,
                       const char* _commandTopic, const char* _commandSubscribeTopic,
                       int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : broker(_broker), port(_port), clientId(_clientId),
    telemetryTopic(_telemetryTopic), eventTopic(_eventTopic),
    commandTopic(_commandTopic), commandSubscribeTopic(_commandSubscribeTopic),
    retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs),
    connected(false), userCallback(nullptr) {

  buildUniqueClientId();

  client.setClient(wifiClient);
  client.setServer(broker, port);
  client.setBufferSize(1024);      // JSON de telemetria precisa de espaço
  client.setKeepAlive(30);         // segundos
  client.setSocketTimeout(10);     // segundos

  instance = this;
  client.setCallback(mqttCallbackWrapper);
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

bool MqttClient::wifiReady() const {
  return WiFi.status() == WL_CONNECTED;
}

bool MqttClient::connect() {
  if (client.connected()) {
    connected = true;
    return true;
  }

  // Sem WiFi não tenta (não queima retry)
  if (!wifiReady()) {
    connected = false;
    return false;
  }

  if (!retry.canRetry()) {
    return false;
  }

  Serial.print("[MQTT] Conectando a ");
  Serial.print(broker);
  Serial.print(":");
  Serial.print(port);
  Serial.print(" como ");
  Serial.println(resolvedClientId);

  // cleanSession=true: re-subscribe sempre após connect
  bool success = client.connect(resolvedClientId);

  if (success) {
    Serial.println("[MQTT] Conectado");

    String cmdTopic(resolvedClientId);
    cmdTopic.concat("/");
    cmdTopic.concat(commandSubscribeTopic);
    if (client.subscribe(cmdTopic.c_str(), 1)) {
      Serial.print("[MQTT] Subscrito: ");
      Serial.println(cmdTopic);
    } else {
      Serial.print("[MQTT] Falha subscribe: ");
      Serial.println(cmdTopic);
    }

    retry.reset();
    connected = true;
    return true;
  }

  Serial.print("[MQTT] Falha connect, state=");
  Serial.println(client.state());
  connected = false;
  return false;
}

void MqttClient::disconnect() {
  if (client.connected()) {
    client.disconnect();
  }
  connected = false;
  retry.reset();
  Serial.println("[MQTT] Desconectado");
}

bool MqttClient::ensureConnection() {
  if (client.connected()) {
    connected = true;
    return true;
  }
  connected = false;
  return connect();
}

bool MqttClient::publishTelemetry(const Message& message) {
  if (!ensureConnection()) {
    Serial.println("[MQTT] publishTelemetry: sem conexao");
    return false;
  }

  String jsonPayload = message.toJson();
  if (jsonPayload.length() + 16 > client.getBufferSize()) {
    Serial.println("[MQTT] Payload maior que o buffer MQTT");
    return false;
  }

  String telTopic(resolvedClientId);
  telTopic.concat("/");
  telTopic.concat(telemetryTopic);
  Serial.print("[MQTT] PUB ");
  Serial.print(telTopic);
  Serial.print(" (");
  Serial.print(jsonPayload.length());
  Serial.print(" B) ");

  client.loop();
  // QoS 1: pelo menos uma entrega no broker
  bool success = client.publish(telTopic.c_str(), jsonPayload.c_str(), false);
  client.loop();

  if (success) {
    Serial.println("OK");
  } else {
    Serial.println("FALHA");
    connected = false;
  }
  return success;
}

bool MqttClient::publishEvent(const char* eventType, const char* eventData) {
  if (!ensureConnection()) {
    Serial.println("[MQTT] publishEvent: sem conexao");
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
  Serial.print("[MQTT] EVT ");
  Serial.print(eveTopic);
  Serial.print(" ");

  client.loop();
  bool success = client.publish(eveTopic.c_str(), jsonPayload.c_str(), false);
  client.loop();

  if (success) {
    Serial.println("OK");
  } else {
    Serial.println("FALHA");
    connected = false;
  }
  return success;
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
  Serial.print("[MQTT] CMD ");
  Serial.print(topic);
  Serial.print(": ");

  // Cópia segura
  char message[128];
  unsigned int n = length < sizeof(message) - 1 ? length : sizeof(message) - 1;
  memcpy(message, payload, n);
  message[n] = '\0';
  Serial.println(message);

  if (userCallback) {
    userCallback(topic, payload, length);
  }
}

void MqttClient::update() {
  // Só faz sentido com WiFi
  if (!wifiReady()) {
    if (connected) {
      connected = false;
    }
    return;
  }

  // Mantém keepalive + recebe publishes
  if (client.connected()) {
    client.loop();
    connected = true;
    return;
  }

  connected = false;
  connect();  // respeita RetryLogic e só tenta com WiFi up
}

bool MqttClient::isConnected() {
  return connected && client.connected();
}
