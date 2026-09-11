#pragma once

#include "Logger.hpp"

class LoggerSerial : public LoggerBase {
  Print& logger;
public:
  LoggerSerial(Print& _logger) : logger(_logger) {}

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
};
