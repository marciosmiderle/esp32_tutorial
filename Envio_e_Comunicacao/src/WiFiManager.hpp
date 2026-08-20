#pragma once

#include <functional>
#include "Telemetry.hpp"
#include "RetryLogic.hpp"
#include <Arduino.h>
#include <WiFi.h>

const int MAX_RETRIES = 5;
const unsigned long RETRY_TIMEOUT_MS = 2000;
const unsigned long TRY_LATER_TIMEOUT_MS = 30000;

class WiFiManager : public Telemetry {
public:
  WiFiManager();

  wl_status_t status = WL_IDLE_STATUS;
  wl_status_t prevStatus = WL_IDLE_STATUS;

  std::function<void()> connectingEvent = {};
  std::function<void()> connectedEvent = {};
  std::function<void()> connectionLostEvent = {};
  std::function<void()> disconnectionEvent = {};
  std::function<void()> connectionStopEvent = {};

  void begin();
  void start();
  void stop();
  void update();

private:
  RetryLogic retry;
  bool started = false;

  void handleFailure();
  wl_status_t getStatus();
};
