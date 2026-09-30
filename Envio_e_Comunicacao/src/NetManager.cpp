#include "NetManager.hpp"
#include "Logger.hpp"
#define NETWORK_MANAGER_NO_GLOBAL
#include <NetworkManager.h> // soosp/NetworkManager

static NetworkManagerClass& nm = NetworkManagerClass::instance();
static void onNetworkEvent(NetworkManagerClass::Event event, NetworkAdapter& adapter);

NetManager& NetManager::getInstance(int maxRetries, unsigned long retryTimeoutMs,
                                    unsigned long tryLaterTimeoutMs) {
  static NetManager instance(maxRetries, retryTimeoutMs, tryLaterTimeoutMs);
  return instance;
}

NetManager::NetManager(int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs),
    started(false) {
  nm.onEvent(onNetworkEvent);
}

void NetManager::start() {
  started = true;
  if (!retry.canRetry()) {
    return;
  }

  nm.begin();
  startEvent();
}

void NetManager::stop() {
  started = false;
  retry.reset();

  nm.end();
  stopEvent();
}

void NetManager::update() {
  if (!started) return;

  if (!isConnected()) {
    start();
  }

  nm.update();
}

bool NetManager::isConnected() const {
  return nm.isConnected();
}

void NetManager::retryReset() {
  retry.reset();
}

void onNetworkEvent(NetworkManagerClass::Event event, NetworkAdapter& adapter) {
  NetManager& instance = NetManager::getInstance();

  switch (event) {
  case NetworkManagerClass::Event::CONNECTED:
    instance.retryReset();
    instance.connectedEvent();
    break;

  case NetworkManagerClass::Event::DISCONNECTED:
    instance.retryReset();
    instance.disconnectedEvent();
    break;

  case NetworkManagerClass::Event::FALLBACK:
    instance.retryReset();
    instance.connectionFallbackEvent();
    Log.println("[NM] lower-priority adapter took over");
    break;

  case NetworkManagerClass::Event::RESTORED:
    instance.retryReset();
    instance.connectionRestoreEvent();
    Log.println("[NM] higher-priority adapter recovered");
    break;

  default:
    break;
  }
}
