#pragma once

#include <Arduino.h>
#include "MqttClient.hpp"
#include <StreamString.h>

class MqttStream : public Stream {
private:
  MqttClient& client;
  StreamString messageIn;
  String messageOut;

public:
  String topicIn;
  String topicOut;
  MqttStream(MqttClient& _client);

  int available() override;
  int read() override;
  int peek() override;
  size_t write(uint8_t val) override;
};
