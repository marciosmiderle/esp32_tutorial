#include "HttpClientLocal.hpp"
#include "Logger.hpp"
#include "DNSResolver.hpp"
#include "os_posix_compat.hpp"

HttpClientLocal::HttpClientLocal(String _host, uint16_t _port, String _endp,
                                 size_t _maxQueue, int maxRetries,
                                 unsigned long retryTimeoutMs,
                                 unsigned long tryLaterTimeoutMs)
  : nc(),
    host(_host), port(_port), endp(_endp),
    retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs),
    maxQueue(_maxQueue) {
  setupHttpClient();
}

HttpClientLocal::~HttpClientLocal() {
  stop();
  DNS.reset();
}

void HttpClientLocal::setupHttpClient() {
  http.setConnectTimeout(500);
  http.setTimeout(500);
  http.setReuse(true);
  http.begin(nc, host, port, endp);
}

void HttpClientLocal::start() {
  started = true;
  setupHttpClient();
}

void HttpClientLocal::stop() {
  started = false;
  nc.stop();
}

void HttpClientLocal::restart() {
  stop();
  start();
}

void HttpClientLocal::update() {
  if (!started) {
    phase = Phase::Idle;
    return;
  }

  if (queue.empty())
    return;

  if (phase == Phase::Idle) {
    if(!retry.canRetry()) {
      restart();
      return;
    }
    phase = Phase::Resolving;
    initiateNameResolution();
    return;
  }
  if (phase == Phase::Resolving) {
    if (hostIP != IPAddress()) { // resolveu
      retry.reset();
      if (!initiateConnection()) {
        if (os_posix_compat::is_wouldblock(os_posix_compat::get_last_error())) {
          return;
        }
        phase = Phase::Idle;
        restart();
        return;
      }
      phase = Phase::Connecting;
    } else {
      DNS.update();
    }
    return;
  }
  if (phase == Phase::Connecting) {
    if (isConnected()) {
      phase = Phase::Ready;
      retry.reset();
      return;
    } else {
      Log.print("[HTTP] connecting to ");
      Log.println(hostIP);
      return;
    }
    return;
  }

  if (phase == Phase::Ready) {
    if (queue.empty())
      return;

    phase = Phase::Sending;
    retry.reset();
    // retorna pra ser nice
    return;
  }

  if (phase == Phase::Sending) {
    // HTTP client errors HTTPC_ERROR_* see HTTPClient.h
    requestReturn  = sendRequest(queue.front());
    httpCode       = http.getHttpCode();
    int contentLen = http.getSize();
    contentLengh   = contentLen <= 0 ? 0 : contentLen;
    phase = Phase::Receiving;
    retry.reset();
    // retorna pra ser nice
    return;
  }

  if (phase == Phase::Receiving) {
    lastResponse = "HTTP " + String(httpCode);
    Log.printf("[HTTP] last response %s [tamanho da fila %lu]\n", lastResponse.c_str(), (long unsigned)queue.size());

    if (httpCode >= 200 && httpCode < 300) {
      if (httpCode == HTTP_CODE_PARTIAL_CONTENT) {
        retry.reset();
        // onData
        return;
      }

      phase = Phase::Ready;
      queue.pop_front();
      retry.reset();
      return;
    }

    if (httpCode >= 400 && httpCode < 500) {
      phase = Phase::Ready;
      queue.pop_front();
      retry.reset();

      if (requestReturn < 0) {
        phase = Phase::Idle;
        restart();
      }
      return;
    }

    if (requestReturn >= 0 && contentLengh == 0) {
      phase = Phase::Ready;
      queue.pop_front();
      retry.reset();
      return;
    }

    if (requestReturn < 0) {
      phase = Phase::Idle;
      restart();
      return;
    }

    Log.println("[HTTP] unhandled return");
    phase = Phase::Ready;
    retry.reset();
    return;
  }
}

bool HttpClientLocal::initiateConnection() {
  uint32_t addr = hostIP;

  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return false;
  }
  os_posix_compat::set_nonblocking(fd, true);

  struct sockaddr_in sa = {};
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = addr;

  int r = ::connect(fd, (struct sockaddr*)&sa, sizeof(sa));
  if (r == 0 || os_posix_compat::is_inprogress(os_posix_compat::get_last_error())) {
    nc = NetworkClient(fd);
    return true;
  }
  return false;
}

void HttpClientLocal::ao_resolver_dns(const char* name, const char* ip_str, HttpClientLocal* arg) {
  if (ip_str != nullptr) {
    arg->hostIP = ip_str;
  } else {
    arg->phase = Phase::Failed;
  }
}

void HttpClientLocal::initiateNameResolution() {
  DNS.resolve(host.c_str(), (AsyncDNSCallback)ao_resolver_dns, this);
}

bool HttpClientLocal::send(const HttpRequest& request) {
  if (maxQueue == 0) return false;

  while (queue.size() >= maxQueue) {  // descarta a mais antiga
    queue.pop_front();
    droppedCount++;
  }
  queue.push_back(request);
  return true;
}

int HttpClientLocal::sendRequest(const HttpRequest& request) {
  for (auto hv : request.headers) {
    http.addHeader(hv.first, hv.second);
  }

  return http.sendRequest(request.method.c_str(), request.content);
}

bool HttpClientLocal::isConnected() const {
  return const_cast<HttpClientLocal*>(this)->nc.connected();
}

const char* HttpClientLocal::getLastResponse() const { return lastResponse.c_str(); }

int HttpClientLocal::pending() const { return queue.size(); }

int HttpClientLocal::dropped() const { return droppedCount; }

Phase HttpClientLocal::getPhase() const {
  return phase;
}

size_t HttpClientLocal::getContentLengh() const {
  return contentLengh;
}

int HttpClientLocal::getHttpCode() const {
  return httpCode;
}

int HttpClientLocal::getRequestReturn() const {
  return requestReturn;
}

NetworkClient& HttpClientLocal::getNetworkClient() {
  return nc;
}
