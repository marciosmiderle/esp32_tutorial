#pragma once

#include <Arduino.h>
#include "HTTPClientEx.hpp"
#include <deque>
#include <functional>

#include "HttpRequest.hpp"
#include "RetryLogic.hpp"

class HttpClientLocal {
public:
  enum class Phase {Idle = 0, Resolving, Connecting, Ready, Sending, Receiving, Done, Failed};

  HttpClientLocal(String _host, uint16_t _port, String _endpoint,
                  size_t maxQueue = 16, int maxRetries = 5,
                  unsigned long retryTimeoutMs = 2000,
                  unsigned long tryLaterTimeoutMs = 30000);
  virtual ~HttpClientLocal();

  void setHostPortEndpoint(String _host, uint16_t _port, String _endpoint);
  void setHostPortEndpointFromUri(String _uri);

  void start();
  void stop();
  void restart();
  void update();

  bool send(const HttpRequest& request);
  std::function<void(bool&)> onData = [](bool& isComplete) {isComplete = true;}; //finishes req
  bool isConnected() const;

  const char* getLastResponse() const;  // agora: "HTTP <código>" ou texto do erro
  int pending() const;
  int dropped() const;
  Phase getPhase() const;
  size_t getContentLength() const;
  String getContent() const;
  String getHost() const;
  uint16_t getPort() const;
  String getEndp() const;
  int getHttpCode() const;
  int getRequestReturn() const;

  NetworkClient& getNetworkClient();
  HTTPClientEx& getHTTPClient();

private:
  NetworkClient nc;
  HTTPClientEx http;
  int requestReturn;
  int httpCode;
  size_t contentLength = 0;
  Phase phase = Phase::Idle;
  IPAddress hostIP;
  RetryLogic retry;
  size_t maxQueue;

  std::deque<HttpRequest> queue;
  String lastResponse;
  bool connected = false;
  bool started = false;
  size_t droppedCount = 0;

  int sendRequest(const HttpRequest& request);
  void setupHttpClient();
  void setupHttpClientFromUri(String _uri);
  void setupHttpClientOptions();

  static constexpr const char* HTTP = "[HTTP] ";

  bool initiateConnection();
  void initiateNameResolution();
  static void ao_resolver_dns(const char* name, const char* ip_str, HttpClientLocal* arg);
  short fdEvents();
};
