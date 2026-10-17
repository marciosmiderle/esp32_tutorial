#include "HttpTestServer.hpp"

HttpTestServer::~HttpTestServer() {
  if (!stop) {
    shutdownNow();
  }
}

void HttpTestServer::start() {
  th = std::thread([this] { serve(); });
}

void HttpTestServer::join() {
  if (th.joinable()) th.join();
}

void HttpTestServer::shutdownNow() {
  stop = true;
  wakeUp();
  join();
}

void HttpTestServer::wakeUp() {
  // acorda o ppoll do servidor (está sem timeout) com uma conexão descartável
  int c = ::socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa = {};
  sa.sin_family = AF_INET;
  sa.sin_port = htons(PORT);
  sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (c >= 0) { ::connect(c, (struct sockaddr*)&sa, sizeof(sa)); close(c); }
}

void HttpTestServer::serve() {
  int ls = ::socket(AF_INET, SOCK_STREAM, 0);
  if (ls < 0) return;
  int on = 1;
  setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on));
  struct sockaddr_in sa = {};
  sa.sin_family = AF_INET;
  sa.sin_port = htons(PORT);
  sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (::bind(ls, (struct sockaddr*)&sa, sizeof(sa)) != 0) { close(ls); return; }
  listen(ls, 4);

  while (!stop.load()) {
    struct pollfd p = {static_cast<decltype(pollfd::fd)>(ls), POLLIN, 0};
    int r = ppoll(&p, 1, nullptr, nullptr);
    if (r <= 0) continue;
    int c = accept(ls, nullptr, nullptr);
    if (c < 0) continue;

    char buf[512];
    ssize_t n = recv(c, buf, sizeof(buf) - 1, 0);
    request = (n > 0) ? std::string(buf, (size_t)n) : std::string();

    if (!closeWithoutResponse && !delayResponse) {
      size_t sent = 0;
      while (sent < response.size()) {
        ssize_t w = send(c, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
        if (w <= 0) break;
        sent += (size_t)w;
      }
    }
    close(c);
  }
  close(ls);
}

std::string HttpTestServer::okResponse() {
  return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}";
}

std::string HttpTestServer::nokResponse503() {
  return "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close";
}

std::string HttpTestServer::nokResponse400() {
  return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close";
}
