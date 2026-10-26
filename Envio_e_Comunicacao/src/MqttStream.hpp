#pragma once

#include <Arduino.h>
#include "MqttClient.hpp"
#include <StreamString.h>

inline constexpr const char* MQTT_STREAM_TOPIC_IN  = "estacao/comandos/entrada";
inline constexpr const char* MQTT_STREAM_TOPIC_OUT = "estacao/comandos/resposta";

class MqttStream : public Stream {
  MqttClient& client;
  StreamString messageIn;
  String messageOut;

public:
  MqttStream(MqttClient& _client);
  virtual ~MqttStream();

  int available() override;
  int read() override;
  int peek() override;
  using Stream::write;
  size_t write(uint8_t val) override;

  String getMessageOut() const;
};
