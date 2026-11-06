#pragma once

#include <Arduino.h>
#include <functional>
#include "CheckSum.hpp"
#include "HttpClientLocal.hpp"
#include "RetryLogic.hpp"

class OtaUpdater {
public:
  enum class State {Idle = 0, FetchingHash, Downloading, DownloadValidation, Reboot};
  enum class StepState {
    Idle = 0,
    Starting,
    Running,
    Running_2,
    Finished,
    Failed
  };

  using StatusCallback = std::function<void(State state, const char* detail)>;

  OtaUpdater(const int maxRetries = 5,
             const unsigned long retryTimeoutMs = 3000,
             const unsigned long tryLaterTimeoutMs = 30000);

  void start();
  void stop();
  void restart();
  void update();

  bool startUpdate(const char* firmwareUrl);
  bool stopUpdate();
  bool markOk();
  bool markInvalidReboot();

  State getState() const;
  StepState getStepState() const;
  bool getDownloadComplete() const;

private:
  bool isUpdating() const;
  float getProgress() const;

  static constexpr size_t kChunkSize = 4096;
  static constexpr size_t maxCheckSumFileSize = 2048;
  static constexpr unsigned long REBOOT_DELAY_MS = 1000;
  static constexpr const char* OTA = "[OTA] ";

  RetryLogic retry;
  bool started;

  HttpClientLocal http;
  State state = State::Idle;
  StepState stepState = StepState::Idle;
  CheckSum sum;

  String firmwareUrl;

  size_t downloaded = 0;
  bool downloadComplete = false;

  char lastError[96] = {0};
  unsigned long lastProgressLogMs = 0;

  void stepFetchHash();
  void startDownload();
  void stepDownload();
  void stepDownloadValidation();
  void stepReboot();
};
