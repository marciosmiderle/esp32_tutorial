#include <thread>

#undef new // Arduino.h error
#include <httplib.h>

class HttpTestServer {
  httplib::Server server;
  bool stopped = false;
  std::thread th;

public:
  static constexpr const char* ROOT_DIR = WWW_DIR;
  static constexpr const int   PORT     = 8080;

  std::string request;
  std::string response;

  bool response400 = false;
  bool response503 = false;
  bool closeWithoutResponse = false;
  bool delayResponse = false;

  virtual ~HttpTestServer();
  void start();
  void stop();
  void join();

  static std::string okResponse();
  static std::string nokResponse503();
  static std::string nokResponse400();

 private:
  void serve();
};
