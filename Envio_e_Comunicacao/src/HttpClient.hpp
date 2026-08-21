#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include "Message.hpp"
#include "RetryLogic.hpp"

class HttpClient {
public:
  HttpClient(const char* apiUrl, int maxRetries = 5, 
             unsigned long retryTimeoutMs = 2000, 
             unsigned long tryLaterTimeoutMs = 30000);
  
  bool send(const Message& message);
  void update();
  bool isConnected() const;
  const char* getLastResponse() const;
  
private:
  const char* apiUrl;
  RetryLogic retry;
  String lastResponse;
  bool connected;
  
  bool performRequest(const Message& message);
  void logResult(bool success, const String& response);
};
