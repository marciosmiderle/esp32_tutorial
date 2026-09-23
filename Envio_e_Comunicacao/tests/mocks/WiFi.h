#pragma once

#include <Arduino.h>
#include <Client.h>

typedef enum {
    WL_NO_SHIELD        = 255,
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6,
    WL_STOPPED          = 7
} wl_status_t;

class WiFiClass {
public:
    void begin(const char* ssid, const char* pass = nullptr, int channel = 0) {}
    void disconnect() {}
    wl_status_t status() { return WL_DISCONNECTED; }
    IPAddress localIP() { return IPAddress(192, 168, 1, 100); }
};

extern WiFiClass WiFi;

class WiFiClient : public arduino::Client {
public:
    int connect(IPAddress ip, uint16_t port) { return 1; }
    int connect(const char *host, uint16_t port) { return 1; }
    size_t write(uint8_t c) override { return 1; }
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}
    void stop() {}
    uint8_t connected() { return 0; }
    size_t write(const uint8_t *buf, size_t size) override { return size; };
    int read(uint8_t *buf, size_t size) override { return size; };
    operator bool() override { return true; };
};
