#include "LoggerStdioDevice.hpp"

size_t LoggerStdioDevice::write(const uint8_t * buffer, size_t size) {
  return logger.write(buffer, size);
}
