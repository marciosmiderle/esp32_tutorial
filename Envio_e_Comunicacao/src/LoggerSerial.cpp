#include "LoggerSerial.hpp"

size_t LoggerSerial::write(const uint8_t * buffer, size_t size) {
  return logger.write(buffer, size);
}
