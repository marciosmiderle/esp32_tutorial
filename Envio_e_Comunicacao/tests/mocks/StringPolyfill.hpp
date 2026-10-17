#pragma once

namespace arduino {
  inline String make_string_compat() {
    return String();
  }

  // size_t (unsigned long long) do UCRT64 e resolve a ambiguidade
  inline String make_string_compat(unsigned long long val) {
    return String((unsigned long)val);
  }

  template<typename... Args>
  inline String make_string_compat(Args&&... args) {
    return String(std::forward<Args>(args)...);
  }
}

#define String(...) arduino::make_string_compat(__VA_ARGS__)
