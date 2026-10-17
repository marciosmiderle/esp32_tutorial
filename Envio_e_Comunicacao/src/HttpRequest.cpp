#include "HttpRequest.hpp"

HttpRequest::HttpRequest(String _method, String _content)
  : method(_method), content(_content)
{}

void HttpRequest::addHeader(const String& name, const String& value) {
  headers[name] = value;
}

