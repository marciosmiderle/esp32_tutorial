#include <atomic>
#include <thread>

#include "CppUTest/TestHarness.h"

class HttpTestServer {
public:
  static const int PORT = 8977;

  std::thread th;
  std::atomic<bool> stop{false};
  std::string request;         // última requisição recebida
  std::string response;        // resposta a enviar
  bool closeWithoutResponse = false;
  bool delayResponse = false;  // nunca responde (só segura a conexão)

  virtual ~HttpTestServer();
  void start();
  void join();

  void shutdownNow();
  static std::string okResponse();
  static std::string nokResponse503();
  static std::string nokResponse400();

 private:
  void wakeUp();
  void serve();
};
