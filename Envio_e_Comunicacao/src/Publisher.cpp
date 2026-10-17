#include "Publisher.hpp"

#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "MqttClient.hpp"

static const char* PUB_HOST = "httpbin.org"; // API de teste que ecoa o payload
static const int   PUB_PORT = 80;
static const char* PUB_ENDP = "/post";

Publisher& Publisher::getInstance() {
  static Publisher instance;
  return instance;
}

Publisher::Publisher() :
  httpClient(PUB_HOST, PUB_PORT, PUB_ENDP),
  started(false) {}

Publisher::~Publisher() {
  httpClient.stop();
}

void Publisher::start() {
  if (started) return;

  started = true;
  httpClient.start();
}

void Publisher::stop() {
  started = false;

  httpClient.stop();
}

void Publisher::restart() {
  stop();
  start();
}

void Publisher::update() {
  if (!started) return;

  publishTelemetry();

  httpClient.update();
}

void Publisher::publishTelemetry() {
  if (estacao == nullptr) return;

  unsigned long currentTime = millis();

  if (currentTime - lastSendTime >= send_interval_ms) {
    lastSendTime = currentTime;

    currentMessage.buildFrom(*estacao);

    HttpRequest req("POST", currentMessage.toJson());
    req.addHeader("Content-Type", "application/json");

    httpClient.send(req);
    
    if (MQTT.isConnected()) {
      Log.println("\n--- Publicando via MQTT ---");
      MQTT.publishTelemetry(currentMessage);
    } else {
      Log.println("\n[MQTT] Ainda nao conectado (update() esta tentando)");
    }
  }
}

void Publisher::setEstacao(Estacao* est) {
  estacao = est;
}
