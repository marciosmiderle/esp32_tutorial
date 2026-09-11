#pragma once

#include "Logger.hpp"
#include "MqttClient.hpp"
#include "StreamString.h"

class LoggerMqtt : public LoggerBase {
  static const int MAX_BUFFER = 2048;

  MqttClient& logger;
  StreamString unsent;

public:
  LoggerMqtt(MqttClient& _logger) : logger(_logger) {}
  virtual ~LoggerMqtt() {};

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
  void update() override;

private:
  void truncateTheOldestData(size_t size);
};
