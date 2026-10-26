#include "MqttClient.hpp"
#include "Logger.hpp"

MqttClient& MqttClient::getInstance() {
  static MqttClient instance(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID);
  return instance;
}

MqttClient::MqttClient(const char* _broker, int _port, const char* _clientId,
                       int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : broker(_broker), port(_port), clientId(_clientId), userCallback({}),
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
  while (queue.size() >= MQTT_CLIENT_MAX_QUEUE) {  // descarta a mais antiga
    queue.pop_front();
    droppedCount++;
  }
  queue.push_back(msg);
  return true;
}

bool MqttClient::sendQueuedMessage(MqttMessage& msg) {
  String fullTopic = getFullQualifiedTopicName(msg.topic.c_str());

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

String MqttClient::getFullQualifiedTopicName(const char* topic) {
  String fqtn(resolvedClientId);
  fqtn.concat("/");
  fqtn.concat(topic);
  return fqtn;
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

    subscribeToTopics();

    retry.reset();
    return;
  }

  Log.print("[MQTT] Falha connect, state=");
  Log.println(client.state());
  return;
}

void MqttClient::addSubscribeTopic(const String topic) {
  subscribedTopics = topic;
}

void MqttClient::subscribeToTopics() {
  String cmdTopic = getFullQualifiedTopicName(subscribedTopics.c_str());

  if (client.subscribe(cmdTopic.c_str(), 1)) {
    Log.print("[MQTT] Subscrito: ");
    Log.println(cmdTopic);
  } else {
    Log.print("[MQTT] Falha subscribe: ");
    Log.println(cmdTopic);
  }
}

void MqttClient::disconnect() {
  if (isConnected()) {
    client.disconnect();
  }
  retry.reset();
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
