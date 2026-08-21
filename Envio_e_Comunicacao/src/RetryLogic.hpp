#pragma once
#include <Arduino.h>

class RetryLogic {
public:
  RetryLogic(int maxRetries, unsigned long retryTimeoutMs, unsigned long tryLaterTimeoutMs);

  bool hasRetriesLeft() const;
  bool retryTimeoutHasElapsed() const;
  bool canRetry();
  bool canStart() const;
  bool isInTryLater();
  bool tryLaterExpired() const;

  // Ações
  void registerAttempt();
  void enterTryLater();
  void reset();

private:
  int maxRetries;
  int retryCount = 0;
  unsigned long retryTimeoutMs;
  unsigned long tryLaterTimeoutMs;
  unsigned long lastAttemptMs = 0;
  unsigned long tryLaterUntilMs = 0;
};
