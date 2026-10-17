#pragma once

#include "RetryLogic.hpp"
#include "Telemetry.hpp"
#include <functional>

class NetManager : public Telemetry {
  NetManager(int maxRetries, unsigned long retryTimeoutMs,
             unsigned long tryLaterTimeoutMs);
  virtual ~NetManager();

public:
  static NetManager& getInstance(int maxRetries = 5, unsigned long retryTimeoutMs = 3000,
                                 unsigned long tryLaterTimeoutMs = 30000);

  std::function<void()> startEvent = {};

  std::function<void()> connectedEvent = {};
  std::function<void()> disconnectedEvent = {};
  std::function<void()> connectionFallbackEvent = {};
  std::function<void()> connectionRestoreEvent = {};

  std::function<void()> stopEvent = {};

  void start();
  void stop();
  void update();
  bool isConnected() const;
  void retryReset();

private:
  RetryLogic retry;
  bool started;
};

#define NetMgr NetManager::getInstance()
