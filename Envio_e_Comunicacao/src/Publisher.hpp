#pragma once

#include "HttpClientLocal.hpp"
#include "Estacao.hpp"
#include "Message.hpp"

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
  void setEstacao(Estacao* est);
};

#define Pub Publisher::getInstance()
