#pragma once

#include <HTTPClient.h>

class HTTPClientEx : public HTTPClient {
public:
  int getHttpCode() { return _returnCode; }
  String getHost() { return _host; }
  uint16_t getPort() { return _port; }
  String getEndp() { return _uri; }
};
