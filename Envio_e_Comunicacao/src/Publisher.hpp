#pragma once

#include "HttpClientLocal.hpp"
#include "Estacao.hpp"
#include "Message.hpp"

inline constexpr const char* PUB_HOST = "httpbin.org"; // API de teste que ecoa o payload
inline constexpr const int   PUB_PORT = 80;
inline constexpr const char* PUB_ENDP = "/post";
inline constexpr const char* MQTT_TELEMETRY_TOPIC = "estacao/telemetria";
inline constexpr const char* MQTT_EVENT_TOPIC = "estacao/eventos";

class Publisher {
  HttpClientLocal httpClient;

  unsigned long lastSendTime = 0;
  const unsigned long send_interval_ms = 10000;  // Envia a cada 10 segundos
  Estacao* estacao = nullptr;
  Message currentMessage;
  bool started;

  Publisher();
  virtual ~Publisher();

public:
  static Publisher& getInstance();

  void start();
  void stop();
  void restart();
  void update();

  void publishTelemetry();
  void publishEvent(const char* eventType, const char* eventData);

  void setEstacao(Estacao* est);
};

#define Pub Publisher::getInstance()
