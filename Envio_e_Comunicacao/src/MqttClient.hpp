#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "Message.hpp"
#include "RetryLogic.hpp"

// Callback para receber comandos remotos
typedef std::function<void(const char* topic, const byte* payload, unsigned int length)> MqttCallback;

class MqttClient {
public:
  static MqttClient* g_mqttClientInstance;
  
  MqttClient(const char* broker, int port, const char* clientId,
             const char* telemetryTopic, const char* eventTopic,
             const char* commandTopic, const char* commandSubscribeTopic,
             int maxRetries = 5, unsigned long retryTimeoutMs = 2000,
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
  const char* telemetryTopic;
  const char* eventTopic;
  const char* commandTopic;
  const char* commandSubscribeTopic;
  RetryLogic retry;
  bool connected;
  MqttCallback userCallback;
  
  static void mqttCallbackWrapper(char* topic, byte* payload, unsigned int length);
  void handleCommand(char* topic, byte* payload, unsigned int length);
  bool ensureConnection();
};
