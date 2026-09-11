#include "LoggerMqtt.hpp"

size_t LoggerMqtt::write(const uint8_t * buffer, size_t size) {
  truncateTheOldestData(size);
  return unsent.write(buffer, size);
}

void LoggerMqtt::truncateTheOldestData(size_t size) {
  const size_t total = unsent.length() + size;
  if (total > MAX_BUFFER) {
    const size_t passou = total - MAX_BUFFER;
    unsent.remove(0, passou);
  }
}

void LoggerMqtt::update() {
  if (unsent.length() > 0 && logger.publishLog(unsent.c_str(), unsent.length())) {
    unsent.clear();
  }
}
