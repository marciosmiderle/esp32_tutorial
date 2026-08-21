#include "RetryLogic.hpp"

RetryLogic::RetryLogic(int maxRetries, unsigned long retryTimeoutMs, unsigned long tryLaterTimeoutMs)
    : maxRetries(maxRetries),
      retryTimeoutMs(retryTimeoutMs),
      tryLaterTimeoutMs(tryLaterTimeoutMs) {}

bool RetryLogic::canRetry() {
  if (isInTryLater()) {
    Serial.println("Aguardando tryLater expirar...");
    return false;
  }
  
  if (tryLaterExpired()) {
    Serial.println("TryLater expirado, resetando retries");
    reset();
  }

  if (!hasRetriesLeft()) {
    Serial.println("Sem mais retries, entrando no tryLater");
    enterTryLater();
    return false;
  }

  if (!retryTimeoutHasElapsed()) {
    return false;
  }

  registerAttempt();

  return true;
}

bool RetryLogic::hasRetriesLeft() const {
  return retryCount < maxRetries;
}

bool RetryLogic::retryTimeoutHasElapsed() const {
  return (millis() - lastAttemptMs) >= retryTimeoutMs;
}

// bool RetryLogic::canStart() const {
//   if (tryLaterUntilMs == 0) return true;
//   return millis() >= tryLaterUntilMs;
// }

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

bool RetryLogic::isInTryLater() {
  return tryLaterUntilMs > 0 && millis() < tryLaterUntilMs;
}

bool RetryLogic::tryLaterExpired() const {
  return tryLaterUntilMs > 0 && millis() >= tryLaterUntilMs;
}
