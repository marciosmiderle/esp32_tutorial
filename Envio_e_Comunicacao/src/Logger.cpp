#include "Logger.hpp"

static Logger* instance = nullptr;

Logger& Logger::getInstance() {
  if (instance == nullptr) {
    instance = new Logger();
  }
  return *instance;
}

Logger::Logger() {}

Logger::~Logger() {
  for (LoggerBase* lg : loggers) {
    delete lg;
  }
}

size_t Logger::write(const uint8_t* buffer, size_t size) {
  size_t ret = -1;
  for (LoggerBase* lg : loggers) {
    if (lg->isEnabled()) {
      ret = lg->write(buffer, size);
    }
  }
  return ret;
}

void Logger::update() {
  for (LoggerBase* lg : loggers) {
    if (lg->isEnabled()) {
      lg->update();
    }
  }
}

bool Logger::isEnabled() const {
  for (LoggerBase* lg : loggers) {
    if (lg->isEnabled()) {
      return true;
    }
  }
  return false;
}

void Logger::disable() {
  for (LoggerBase* lg : loggers) {
    lg->disable();
  }
}

void Logger::enable() {
  for (LoggerBase* lg : loggers) {
    lg->enable();
  }
}

void Logger::addLogger(LoggerBase* _logger) {
  loggers.push_back(_logger);
}
