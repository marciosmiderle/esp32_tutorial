#pragma once

#include <Arduino.h>
#include "NetworkClient.h"

enum followRedirects_t {
    HTTPC_DISABLE_FOLLOW_REDIRECTS,
    HTTPC_STRICT_FOLLOW_REDIRECTS,
    HTTPC_FORCE_FOLLOW_REDIRECTS
};

class HTTPClient {
public:
    HTTPClient() {}
    bool begin(String url) { return true; }
    void end() {}
    void addHeader(const char* name, const char* value) {}
    void setTimeout(uint16_t timeout) {}
    void setFollowRedirects(followRedirects_t follow) {}
    int GET() { return 200; }
    int POST(uint8_t * payload, size_t size) { return 200; }
    int POST(String payload) { return 200; }
    int getSize() { return 0; }
    String getString() { return ""; }
    NetworkClient* getStreamPtr() { return nullptr; }
    bool connected() { return false; }
    void setConnectTimeout(int32_t connectTimeout) {};
};
