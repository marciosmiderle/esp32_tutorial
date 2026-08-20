#include "WiFiManager.hpp"

WiFiManager::WiFiManager()
    : retry(MAX_RETRIES, RETRY_TIMEOUT_MS, TRY_LATER_TIMEOUT_MS) {}

void WiFiManager::begin() {
  start();
}

void WiFiManager::start() {
  if (WiFi.status() == WL_CONNECTED) return;
  
  // Se está em tryLater e o tempo ainda não passou, não faz nada
  if (retry.isInTryLater()) {
    return;
  }

  // Se o tryLater expirou, reseta tudo para começar uma nova sequência
  if (retry.tryLaterExpired()) {
    retry.reset();
  }

  started = true;
  retry.registerAttempt();
  WiFi.begin("Wokwi-GUEST", "", 6);
}

void WiFiManager::stop() {
  status = WL_IDLE_STATUS;
  prevStatus = WL_IDLE_STATUS;
  started = false;
  retry.reset();

  WiFi.disconnect();
  connectionStopEvent();
}

void WiFiManager::update() {
  if (!started) {
    return;
  }

  // Se o tryLater expirou, reseta e tenta novamente
  if (retry.tryLaterExpired()) {
    retry.reset();
    start();
    return;
  }

  // Se ainda está em tryLater, aguarda
  if (retry.isInTryLater()) {
    return;
  }

  wl_status_t currentStatus = getStatus();

  // Detecta transições para disparar eventos
  bool justConnected =
      (currentStatus == WL_CONNECTED && prevStatus != WL_CONNECTED);
  bool justLostConnection =
      (currentStatus != WL_CONNECTED && prevStatus == WL_CONNECTED);

  switch (currentStatus) {
  case WL_NO_SHIELD:
  case WL_IDLE_STATUS:
  case WL_NO_SSID_AVAIL:
    // Aguarda scan ou timeout
    break;

  case WL_SCAN_COMPLETED:
    start();
    break;

  case WL_CONNECT_FAILED:
    handleFailure();
    break;

  case WL_CONNECTION_LOST:
    if (justLostConnection) {
      connectionLostEvent();
    }
    handleFailure();
    break;

  case WL_DISCONNECTED:
    if (justLostConnection) {
      disconnectionEvent();
      handleFailure();
    } else if (started && prevStatus != WL_DISCONNECTED) {
      connectingEvent();
    }
    break;

  case WL_STOPPED:
    break;

  case WL_CONNECTED:
    if (justConnected) {
      retry.reset();
      connectedEvent();
    }
    break;
  }

  prevStatus = currentStatus;
}

void WiFiManager::handleFailure() {
  // Se não tem mais retries disponíveis, entra em tryLater
  if (!retry.hasRetriesLeft()) {
    WiFi.disconnect();
    retry.enterTryLater();
    return;
  }

  // Tem retries disponíveis, mas o timeout entre tentativas ainda não passou?
  // Apenas aguarda, não faz nada.
  if (!retry.isRetryTimeoutElapsed()) {
    return;
  }

  // Tem retries disponíveis E o timeout passou: tenta novamente
  WiFi.disconnect();
  start();
}

wl_status_t WiFiManager::getStatus() {
  return WiFi.status();
}
