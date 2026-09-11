#include "Logger.hpp"

Logger::Logger(LoggerBase* _logToMqtt, LoggerBase* _logToSerial) :
    logToMqtt(_logToMqtt),
    logToSerial(_logToSerial) {
  loggers[0] = logToSerial;
  loggers[1] = logToMqtt;
}

size_t Logger::write(const uint8_t* buffer, size_t size) {
  size_t ret = -1;
  for( int i = 0; i < LOGGER_COUNT; i++) {
    ret = loggers[i]->write(buffer, size);
  }
  return ret;
}

void Logger::update() {
  for( int i = 0; i < LOGGER_COUNT; i++) {
    loggers[i]->update();
  }
}
