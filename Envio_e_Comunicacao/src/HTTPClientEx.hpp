#pragma once

#include <HTTPClient.h>

class HTTPClientEx : public HTTPClient {
public:
  int getHttpCode() { return _returnCode; }
};
