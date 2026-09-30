#pragma once

#include "CppUTestExt/MockSupport.h"
#include <cstdint>
#include <functional>

struct NetworkStatus {};
class NetworkAdapter {};

class NetworkManagerClass {
public:
  enum class Event : uint8_t {
    CONNECTED,
    DISCONNECTED,
    FALLBACK,
    RESTORED
  };
  using EventCb = void(*)(Event event, NetworkAdapter& adapter);

  EventCb registeredCallback = nullptr;

  static NetworkManagerClass& instance() {
    static NetworkManagerClass _instance;
    return _instance;
  }

  void begin() {
    mock().actualCall("NetworkManagerClass::begin");
  }

  void end() {
    mock().actualCall("NetworkManagerClass::end");
  }

  void update() {
    mock().actualCall("NetworkManagerClass::update");
  }

  bool isConnected() {
    mock().actualCall("NetworkManagerClass::isConnected");
    return mock().boolReturnValue();
  }

  void onEvent(EventCb cb) {
    registeredCallback = cb;
  }

  void simulateEvent(Event event, NetworkAdapter& adapter) {
    if (registeredCallback) {
      registeredCallback(event, adapter);
    }
  }
};
