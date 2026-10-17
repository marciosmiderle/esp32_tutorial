#pragma once

#include <chrono>
#include <functional>

void while_timed(std::chrono::milliseconds duration_limit_ms, std::function<bool()> while_body) {
  auto start_time = std::chrono::steady_clock::now();
  while (while_body()) {
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - start_time >= duration_limit_ms) {
      break;
    }
  }
}
