#pragma once

#include <Arduino.h>
#include <functional>
#include "CheckSum.hpp"
#include <HTTPClient.h>
#include "RetryLogic.hpp"

class OtaUpdater {
public:
  enum class State {
    Idle,
    FetchingHash,
    Downloading,
    Success,
    Failed
  };

  using StatusCallback = std::function<void(State state, const char* detail)>;

  OtaUpdater(int maxRetries = 5,
             unsigned long retryTimeoutMs = 3000,
             unsigned long tryLaterTimeoutMs = 30000);

  void begin();
  void setStatusCallback(StatusCallback cb);

  bool startUpdate(const char* firmwareUrl);
  bool stopUpdate();

  bool markOk();
  bool markInvalidReboot();

  void abort();
  void update();

  State getState() const { return state; }
  bool isBusy() const;
  float getProgress() const;
  const char* getLastError() const { return lastError; }

private:
  static constexpr size_t kChunkSize = 4096;

  HTTPClient http;
  State state = State::Idle;
  CheckSum sum;
  RetryLogic retry;
  StatusCallback statusCb;

  String firmwareUrl;

  size_t totalSize = 0;
  size_t downloaded = 0;

  char lastError[96] = {0};
  unsigned long lastProgressLogMs = 0;

  void setState(State s, const char* detail = nullptr);
  void setError(const char* msg);
  void clearError();

  void savePending();
  void clearPending();
  bool loadPending();

  bool stepFetchHash();
  bool stepDownload();
};
