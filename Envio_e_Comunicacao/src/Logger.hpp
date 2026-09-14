#pragma once

#include "LoggerBase.hpp"
#include <list>

class Logger : public LoggerBase {
  std::list<LoggerBase*> loggers;

  Logger();
  virtual ~Logger();

public:
  static Logger& getInstance();

  using Print::write;
  size_t write(const uint8_t* buffer, size_t size) override;
  void update() override;

  bool isEnabled() const override;
  void disable() override;
  void enable() override;

  void addLogger(LoggerBase* _logger);
};

#define Log Logger::getInstance()
