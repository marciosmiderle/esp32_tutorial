#pragma once

#include <Arduino.h>
#include "HTTPClientEx.hpp"
#include <deque>

#include "HttpRequest.hpp"
#include "RetryLogic.hpp"

enum class Phase { Idle = 0, Resolving, Connecting, Ready, Sending, Receiving, Done, Failed };

class HttpClientLocal {
public:
  HttpClientLocal(String _host = "localhost", uint16_t _port = 80, String _endp = "/",
                  size_t maxQueue = 16, int maxRetries = 5,
                  unsigned long retryTimeoutMs = 2000,
                  unsigned long tryLaterTimeoutMs = 30000);
  virtual ~HttpClientLocal();

  void start();
  void stop();
  void restart();
  void update();

  bool send(const HttpRequest& request);

  bool isConnected() const;

  const char* getLastResponse() const;  // agora: "HTTP <código>" ou texto do erro
  int pending() const;
  int dropped() const;
  Phase getPhase() const;
  size_t getContentLengh() const;
  int getHttpCode() const;
  int getRequestReturn() const;

  NetworkClient& getNetworkClient();

private:
  NetworkClient nc;
  HTTPClientEx http;
  int requestReturn;
  int httpCode;
  size_t contentLengh = 0;
  Phase phase = Phase::Idle;
  String host;
  IPAddress hostIP;
  uint16_t port;
  String endp;
  RetryLogic retry;
  size_t maxQueue;

  std::deque<HttpRequest> queue;
  String lastResponse;
  bool connected = false;
  bool started = false;
  size_t droppedCount = 0;

  int sendRequest(const HttpRequest& request);
  void setupHttpClient();

  static const size_t MAX_HEAD = 96;
  static const size_t SEND_CHUNK = 512;
  static const size_t RECV_CHUNKS = 4;

  unsigned long connectTimeoutMs_;
  unsigned long responseTimeoutMs_;

  bool initiateConnection();
  void initiateNameResolution();
  static void ao_resolver_dns(const char* name, const char* ip_str, HttpClientLocal* arg);
  short fdEvents();
};
