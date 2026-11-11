#include "HttpTestServer.hpp"
#include <thread>
#include <chrono>
#include <iostream>

HttpTestServer::~HttpTestServer() {
  if (!stopped) {
    stop();
  }
}

void HttpTestServer::start() {
  stopped = false;

  using namespace httplib;

  server.set_logger([](const Request& req, const Response& res) {
    std::cout << "[HttpTestServer] " << req.method << " " << req.path
              << " -> Status: " << res.status << std::endl;
  });

  auto handler = [this](const Request& req, Response& res) {
    this->request = req.body.empty() ? req.path : req.body;

    if (this->delayResponse) {
      std::cout << "[HttpTestServer] [MODO: delayResponse] Segurando conexão..." << std::endl;
      while (!this->stopped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      return;
    }

    if (this->closeWithoutResponse) {
      std::cout << "[HttpTestServer] [MODO: closeWithoutResponse] Derrubando conexão." << std::endl;
      res.status = -1;
      return;
    }

    if (this->response503) {
      res.status = 503;
      res.set_content(nokResponse503(), "text/html");
      return;
    }

    if (this->response400) {
      res.status = 400;
      res.set_header("Connection", "close");
      res.set_content(nokResponse400(), "text/html");
      return;
    }

    if (!this->response.empty()) {
      res.status = 200;
      res.set_content(this->response, "text/html");
      return;
    }

    if (req.path == "/" || req.path == "/status") {
      res.status = 200;
      res.set_content(okResponse(), "text/html");
      return;
    }

    res.status = 404;
    res.set_content("<html><body><h1>404 Not Found</h1></body></html>", "text/html");
  };

  server.Get(R"(.*)", handler);
  server.Post(R"(.*)", handler);

  if (!server.set_mount_point("/", ROOT_DIR)) {
    std::clog << "[HttpTestServer] AVISO: Pasta (" << ROOT_DIR << ") indisponível." << std::endl;
  }
  std::clog << "[HttpTestServer] Serving dir (" << WWW_DIR << ")" << std::endl;

  th = std::thread([this] { serve(); });
  server.wait_until_ready();
}

void HttpTestServer::stop() {
  stopped = true;
  server.stop();
  join();
}

void HttpTestServer::join() {
  if (th.joinable()) {
    th.join();
  }
}

std::string HttpTestServer::okResponse() {
  return "<html><body><h1>200 OK</h1></body></html>";
}

std::string HttpTestServer::nokResponse503() {
  return "<html><body><h1>503 Service Unavailable</h1></body></html>";
}

std::string HttpTestServer::nokResponse400() {
  return "<html><body><h1>400 Bad Request</h1></body></html>";
}

void HttpTestServer::serve() {
  if (!server.listen("127.0.0.1", PORT)) {
    std::cerr << "[HttpTestServer] ERRO CRÍTICO: Falha ao alocar a porta " << PORT << std::endl;
  }
}
