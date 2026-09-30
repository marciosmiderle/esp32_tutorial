#include "millis.h"

unsigned long& _get_time_warp_offset() {
  static unsigned long offset = 0;
  return offset;
}

void millis_set(unsigned long ms) {
  _get_time_warp_offset() = ms;
}

void millis_forward(unsigned long offset_ms) {
  _get_time_warp_offset() += offset_ms;
}

void millis_backward(unsigned long offset_ms) {
  _get_time_warp_offset() -= offset_ms;
}

unsigned long custom_millis() {
  return _get_time_warp_offset();
}
