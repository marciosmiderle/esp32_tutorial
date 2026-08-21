#pragma once

#include <Arduino.h>
#include <functional>
#include <PubSubClient.h>
#include <WiFi.h>
#include "Message.hpp"
#include "RetryLogic.hpp"

typedef std::function<void(const char* topic, const byte* payload, unsigned int length)> MqttCallback;

class MqttClient {
public:
  MqttClient(const char* broker, int port, const char* clientId,
             const char* telemetryTopic, const char* eventTopic,
             const char* commandTopic, const char* commandSubscribeTopic,
             int maxRetries = 5, unsigned long retryTimeoutMs = 3000,
             unsigned long tryLaterTimeoutMs = 30000);

  bool connect();
  void disconnect();
  bool publishTelemetry(const Message& message);
  bool publishEvent(const char* eventType, const char* eventData);
  void setCallback(MqttCallback callback);
  void update();
  bool isConnected();

private:
  WiFiClient wifiClient;
  PubSubClient client;
  const char* broker;
  int port;
  const char* clientId;
  char resolvedClientId[48];   // ID único em runtime
  const char* telemetryTopic;
  const char* eventTopic;
  const char* commandTopic;
  const char* commandSubscribeTopic;
  RetryLogic retry;
  bool connected;
  MqttCallback userCallback;

  static MqttClient* instance;
  static void mqttCallbackWrapper(char* topic, byte* payload, unsigned int length);
  void handleCommand(char* topic, byte* payload, unsigned int length);
  bool ensureConnection();
  bool wifiReady() const;
  void buildUniqueClientId();
};
