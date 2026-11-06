#include "Logger.hpp"

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
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

size_t Logger::println(const char* prefix, const char* msg) {
  return println(prefix, msg, "");
}

size_t Logger::println(const char* prefix, const char* msg, const char* suffix) {
  size_t ret = print(prefix);
  ret += print(msg);
  ret += println(suffix);
  return ret;
}

size_t Logger::println(const char* prefix, const char* msg, const int val) {
  size_t ret = print(prefix);
  ret += print(msg);
  ret += println(val);
  return ret;
}
