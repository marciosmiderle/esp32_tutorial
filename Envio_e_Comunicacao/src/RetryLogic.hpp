#pragma once

class RetryLogic {
public:
  RetryLogic(const int maxRetries, const unsigned long retryTimeoutMs, const unsigned long tryLaterTimeoutMs, const char* logPrefix);

  bool hasRetriesLeft() const;
  bool retryTimeoutHasElapsed() const;
  bool canRetry();
  bool isInTryLater();
  bool tryLaterExpired() const;

  // Ações
  void registerAttempt();
  void enterTryLater();
  void reset();

private:
  const int maxRetries;
  const unsigned long retryTimeoutMs;
  const unsigned long tryLaterTimeoutMs;
  const char* logPrefix;

  int retryCount = 0;
  unsigned long lastAttemptMs = 0;
  unsigned long tryLaterUntilMs = 0;
};
