#pragma once

#include "Logger.hpp"

class LoggerStdioDevice : public LoggerBase {
  StdioDevice& logger;
public:
  LoggerStdioDevice(StdioDevice& _logger) : logger(_logger) {}

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
};
