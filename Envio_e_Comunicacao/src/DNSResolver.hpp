#pragma once

#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#else
#include <future>
#endif

#include <string>

typedef void (*AsyncDNSCallback)(const char* name, const char* ip_str, void* arg);

class DNSResolver {
private:
  std::string hostname;
  AsyncDNSCallback callback;
  void* callback_arg;
  bool started;

#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
  ip_addr_t _lwip_resolved_ip;
  static void lwip_dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
  static void lwip_dns_safe_call(void *ctx);
#else
  std::future<std::string> future_worker;
#endif

  DNSResolver();

public:
  static DNSResolver& getInstance();
  bool resolve(const std::string& _hostname, AsyncDNSCallback cb, void* arg);
  void update();
  bool isInProgress() const;
  void reset();
};

#define DNS DNSResolver::getInstance()
