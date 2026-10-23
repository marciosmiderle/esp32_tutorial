#include "MqttClient.hpp"
#include "Logger.hpp"
#include <ArduinoJson.h>

const char* MQTT_BROKER = "broker.hivemq.com";  // Broker público HiveMQ
const int   MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "estacao_meteorologica_001";
const char* MQTT_TELEMETRY_TOPIC = "estacao/telemetria";
const char* MQTT_EVENT_TOPIC = "estacao/eventos";
const char* MQTT_LOGS_TOPIC = "estacao/logs";
const char* MQTT_COMMAND_TOPIC = "estacao/comandos/resposta";
const char* MQTT_COMMAND_SUBSCRIBE_TOPIC = "estacao/comandos/entrada";

MqttClient& MqttClient::getInstance() {
  static MqttClient instance(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID,
                             MQTT_TELEMETRY_TOPIC, MQTT_EVENT_TOPIC,
                             MQTT_COMMAND_TOPIC, MQTT_COMMAND_SUBSCRIBE_TOPIC,
                             MQTT_LOGS_TOPIC);
  return instance;
}

MqttClient::MqttClient(const char* _broker, int _port, const char* _clientId,
                       const char* _telemetryTopic, const char* _eventTopic,
                       const char* _commandTopic, const char* _commandSubscribeTopic,
                       const char* _logTopic,
                       int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : broker(_broker), port(_port), clientId(_clientId),
    telemetryTopic(_telemetryTopic), eventTopic(_eventTopic),
    commandTopic(_commandTopic), commandSubscribeTopic(_commandSubscribeTopic),
    logTopic(_logTopic), userCallback({}),
    retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), started(false) {

  buildUniqueClientId();

  netClient.setConnectionTimeout(500); // ms
  client.setClient(netClient);
  client.setServer(broker, port);
  client.setBufferSize(1024);       // JSON de telemetria precisa de espaço
  client.setKeepAlive(180);         // segundos
  client.setSocketTimeout(1);       // segundos

  client.setCallback(pubSubClientCallback);
}

MqttClient::~MqttClient() {
  stop();
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

  if (queue.empty())
    return;

  if (isConnected()) {

    if (sendQueuedMessage(queue.front())) {

      if (client.loop()) {
        queue.pop_front();
        retry.reset();
      }
    }

    return;
  }

  connect();
}

bool MqttClient::sendMessage(const MqttMessage& msg) {
  if (maxQueue == 0) return false;

  while (queue.size() >= maxQueue) {  // descarta a mais antiga
    queue.pop_front();
    droppedCount++;
  }
  queue.push_back(msg);
  return true;
}

bool MqttClient::sendQueuedMessage(MqttMessage& msg) {
  String fullTopic(resolvedClientId);
  fullTopic.concat("/");
  fullTopic.concat(msg.topic);

  return client.publish(fullTopic.c_str(), msg.content.c_str());
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

  // cleanSession=true: re-subscribe sempre após connect
  bool success = client
    .setKeepAlive(180)
    .setSocketTimeout(1)
    .connect(resolvedClientId);

  if (success) {
    Log.print("[MQTT] Conectado a ");
    Log.print(broker);
    Log.print(":");
    Log.print(port);
    Log.print(" como ");
    Log.println(resolvedClientId);

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
  String rlogTopic(resolvedClientId);
  rlogTopic.concat("/");
  rlogTopic.concat(logTopic);

  return client.publish(rlogTopic.c_str(), reinterpret_cast<const uint8_t*>(data), length, false);
}

void MqttClient::setCallback(MqttCallback callback) {
  userCallback = callback;
}

void MqttClient::pubSubClientCallback(char* topic, byte* payload, unsigned int length) {
  MQTT.userCallback(topic, payload, length);
}

bool MqttClient::isConnected() {
  return client.connected();
}
