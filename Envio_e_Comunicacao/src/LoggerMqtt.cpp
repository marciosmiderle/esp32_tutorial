#include "LoggerMqtt.hpp"

const String& LoggerMqtt::getUnsentBuffer() const {
  return unsent;
}

size_t LoggerMqtt::write(const uint8_t * buffer, size_t size) {
  if (size == 0) return 0;

  size_t toCopy = size;
  const uint8_t* src = buffer;

  if (toCopy > MAX_BUFFER) {
    src = buffer + (toCopy - MAX_BUFFER);
    toCopy = MAX_BUFFER;
  }

  truncateTheOldestData(toCopy);

  for (size_t i = 0; i < toCopy; i++) {
    unsent += (char)src[i];
  }
  return size;
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
    unsent = "";
  }
}
