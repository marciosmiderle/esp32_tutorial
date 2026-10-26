#pragma once

#include <Arduino.h>
#include <functional>
#include <NetworkClient.h>
#include <PubSubClient.h>
#include "Message.hpp"
#include "MqttMessage.hpp"
#include "RetryLogic.hpp"
#include <deque>

inline constexpr const char* MQTT_BROKER    = "broker.hivemq.com";  // Broker público HiveMQ
inline constexpr const int   MQTT_PORT      = 1883;
inline constexpr const char* MQTT_CLIENT_ID = "estacao_meteorologica_001";
inline constexpr const int   MQTT_CLIENT_MAX_QUEUE = 30;

typedef std::function<void(const char* topic, const byte* payload, unsigned int length)> MqttCallback;

class MqttClient {
protected:
  MqttClient(const char* broker, int port, const char* clientId,
             int maxRetries = 5, unsigned long retryTimeoutMs = 3000,
             unsigned long tryLaterTimeoutMs = 30000);
  virtual ~MqttClient();

public:
  static MqttClient& getInstance();

  void start();
  void stop();
  void restart();
  void update();

  String getFullQualifiedTopicName(const char* topic);
  void connect();
  void addSubscribeTopic(const String topic);

  void disconnect();
  virtual bool sendMessage(const MqttMessage& msg);

  void setCallback(MqttCallback callback);
  bool isConnected();

private:
  NetworkClient netClient;
  PubSubClient client;
  const char* broker;
  int port;
  const char* clientId;
  char resolvedClientId[48];   // ID único em runtime
  String subscribedTopics;
  MqttCallback userCallback;

  std::deque<MqttMessage> queue;
  size_t droppedCount = 0;

  static MqttClient* instance;

  RetryLogic retry;
  bool started;

  bool sendQueuedMessage(MqttMessage& msg);
  static void pubSubClientCallback(char* topic, byte* payload, unsigned int length);
  void buildUniqueClientId();
  void subscribeToTopics();
};

#define MQTT MqttClient::getInstance()
