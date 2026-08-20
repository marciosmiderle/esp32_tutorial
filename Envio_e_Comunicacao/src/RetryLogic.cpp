#include "RetryLogic.hpp"

RetryLogic::RetryLogic(int maxRetries, unsigned long retryTimeoutMs, unsigned long tryLaterTimeoutMs)
    : maxRetries(maxRetries),
      retryTimeoutMs(retryTimeoutMs),
      tryLaterTimeoutMs(tryLaterTimeoutMs) {}

bool RetryLogic::hasRetriesLeft() const {
  return retryCount < maxRetries;
}

bool RetryLogic::isRetryTimeoutElapsed() const {
  return (millis() - lastAttemptMs) >= retryTimeoutMs;
}

bool RetryLogic::canRetry() const {
  return hasRetriesLeft() && isRetryTimeoutElapsed();
}

bool RetryLogic::canStart() const {
  if (tryLaterUntilMs == 0) return true;
  return millis() >= tryLaterUntilMs;
}

void RetryLogic::registerAttempt() {
  retryCount++;
  lastAttemptMs = millis();
}

void RetryLogic::enterTryLater() {
  tryLaterUntilMs = millis() + tryLaterTimeoutMs;
}

void RetryLogic::reset() {
  retryCount = 0;
  lastAttemptMs = 0;
  tryLaterUntilMs = 0;
}

bool RetryLogic::isInTryLater() const {
  return tryLaterUntilMs > 0 && millis() < tryLaterUntilMs;
}

bool RetryLogic::tryLaterExpired() const {
  return tryLaterUntilMs > 0 && millis() >= tryLaterUntilMs;
}