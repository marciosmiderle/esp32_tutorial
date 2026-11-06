#pragma once

#include <chrono>
#include <functional>

inline void while_timed(unsigned int duration_limit_ms,
                        std::function<bool()> while_body,
                        std::function<bool()> isProgressing = []() { return false; }) {
  auto start_time = std::chrono::steady_clock::now();
  auto limit_time = std::chrono::milliseconds(duration_limit_ms);
  while (while_body()) {
    auto current_time = std::chrono::steady_clock::now();
    if (isProgressing()) {
      start_time = current_time;
      continue;
    }
    if (current_time - start_time >= limit_time) {
      break;
    }
  }
}
