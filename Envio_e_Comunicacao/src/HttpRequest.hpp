#pragma once

#include <Arduino.h>
#include <map>

class HttpRequest {
public:
  String method;
  String content;
  std::map<String, String> headers;

  HttpRequest(String _method, String _content);
  void addHeader(const String &name, const String &value);
};
