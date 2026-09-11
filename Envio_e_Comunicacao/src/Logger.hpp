#pragma once

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

class LoggerBase : public Print {
public:
  using Print::write;
  size_t write(uint8_t c) override { return write(&c, 1); }
  virtual void update() {};
};

class Logger : public LoggerBase {
  static const int LOGGER_COUNT = 2;

  LoggerBase* loggers[LOGGER_COUNT];
  LoggerBase* logToMqtt;
  LoggerBase* logToSerial;
public:
  Logger(LoggerBase* _logToMqtt, LoggerBase* _logToSerial);
  virtual ~Logger() {};

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
  void update() override;
};
