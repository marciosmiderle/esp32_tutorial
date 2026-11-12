#include "RetryLogic.hpp"
#include "Logger.hpp"

RetryLogic::RetryLogic(const int maxRetries, const unsigned long retryTimeoutMs, const unsigned long tryLaterTimeoutMs, const char* logPrefix)
    : maxRetries(maxRetries),
      retryTimeoutMs(retryTimeoutMs),
      tryLaterTimeoutMs(tryLaterTimeoutMs),
      logPrefix(logPrefix) {}

bool RetryLogic::canRetry() {
  if (isInTryLater()) {
    //TODO: niveis de log Log.println("Aguardando tryLater expirar...");
    return false;
  }

  if (tryLaterExpired()) {
    Log.print(logPrefix);
    Log.println("TryLater expirado, resetando retries");
    reset();
  }

  if (!hasRetriesLeft()) {
    Log.print(logPrefix);
    Log.println("Sem mais retries, entrando no tryLater");
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
