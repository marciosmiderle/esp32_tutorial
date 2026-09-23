#pragma once

#include "Logger.hpp"
#include "MqttClient.hpp"

class LoggerMqtt : public LoggerBase {
  static const int MAX_BUFFER = 2048;

  MqttClient& logger;
  String unsent;

public:
  LoggerMqtt(MqttClient& _logger) : logger(_logger) {}
  virtual ~LoggerMqtt() {};

  const String& getUnsentBuffer() const;

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
  void update() override;

private:
  void truncateTheOldestData(size_t size);
};
