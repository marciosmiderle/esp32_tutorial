#pragma once

#include <Arduino.h>
#include <functional>
#include <NetworkClient.h>
#include <PubSubClient.h>
#include "Message.hpp"
#include "RetryLogic.hpp"

typedef std::function<void(const char* topic, const byte* payload, unsigned int length)> MqttCallback;

class MqttClient {
protected:
  MqttClient(const char* broker, int port, const char* clientId,
             const char* telemetryTopic, const char* eventTopic,
             const char* commandTopic, const char* commandSubscribeTopic,
             const char* _logTopic,
             int maxRetries = 5, unsigned long retryTimeoutMs = 3000,
             unsigned long tryLaterTimeoutMs = 30000);
  virtual ~MqttClient();

public:
  static MqttClient& getInstance();

  void start();
  void stop();
  void restart();
  void update();

  void connect();
  void disconnect();
  bool publishTelemetry(const Message& message);
  bool publishEvent(const char* eventType, const char* eventData);
  virtual bool publishLog(const char* data, size_t length);
  void setCallback(MqttCallback callback);
  bool isConnected();

private:
  NetworkClient netClient;
  PubSubClient client;
  const char* broker;
  int port;
  const char* clientId;
  char resolvedClientId[48];   // ID único em runtime
  const char* telemetryTopic;
  const char* eventTopic;
  const char* commandTopic;
  const char* commandSubscribeTopic;
  const char* logTopic;
  MqttCallback userCallback;

  static MqttClient* instance;
  static void mqttCallbackWrapper(char* topic, byte* payload, unsigned int length);
  void handleCommand(char* topic, byte* payload, unsigned int length);
  bool ensureConnection();
  void buildUniqueClientId();

private:
  RetryLogic retry;
  bool started;
};

#define MQTT MqttClient::getInstance()
