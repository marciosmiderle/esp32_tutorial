#include "src/LoggerSerial.hpp"
#include "src/WatchDog.hpp"

#include "src/Controls.hpp"

#include "src/NtcSensor.hpp"
#include "src/NtcView.hpp"

#include "src/DHTSensor.hpp"
#include "src/DHTView.hpp"

#include "src/PirSensor.hpp"
#include "src/PirView.hpp"

#include "src/Estacao.hpp"
#include "src/EstacaoView.hpp"

#include "src/WiFiManager.hpp"

#include "src/Message.hpp"
#include "src/HttpClientLocal.hpp"
#include "src/MqttClient.hpp"
#include "src/OtaUpdater.hpp"

#include "src/Logger.hpp"
#include "src/LoggerMqtt.hpp"
#include "src/LoggerSerial.hpp"

const uint8_t NTC_PIN     = 34;
const uint8_t NTC_BTN_PIN = 27;
NtcSensor     ntc;
NtcView       ntcView;
Button        ntcBtn;

const uint8_t DHT_PIN     = 33;
const uint8_t DHT_BTN_PIN = 16;
DHTSensor     dht;
DHTView       dhtView;
Button        dhtBtn;

const uint8_t PIR_PIN = 17;
PirSensor     pir;
PirView       pirView;

Estacao est(ntc, dht, pir);
EstacaoView estView(&est);

// Configurações da API HTTP
const char* API_URL = "http://httpbin.org/post";  // API de teste que ecoa o payload
HttpClientLocal httpClient(API_URL);

// Configurações do Broker MQTT
const char* MQTT_BROKER = "broker.hivemq.com";  // Broker público HiveMQ
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "estacao_meteorologica_001";
const char* MQTT_TELEMETRY_TOPIC = "estacao/telemetria";
const char* MQTT_EVENT_TOPIC = "estacao/eventos";
const char* MQTT_LOGS_TOPIC = "estacao/logs";
const char* MQTT_COMMAND_TOPIC = "estacao/comandos/resposta";
const char* MQTT_COMMAND_SUBSCRIBE_TOPIC = "estacao/comandos/entrada";

MqttClient mqttClient(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID,
                      MQTT_TELEMETRY_TOPIC, MQTT_EVENT_TOPIC,
                      MQTT_COMMAND_TOPIC, MQTT_COMMAND_SUBSCRIBE_TOPIC,
                      MQTT_LOGS_TOPIC);

OtaUpdater ota;

Message currentMessage;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL_MS = 10000;  // Envia a cada 10 segundos

WatchDog wdg(5000);

void setupWatchDog() {
  wdg.begin();
}

LoggerSerial* logToSer0 = nullptr;
LoggerMqtt*   logToMqtt = nullptr;

void setupLogger() {
  logToSer0 = new LoggerSerial(Serial);
  logToMqtt = new LoggerMqtt(mqttClient);
  Log.addLogger(logToSer0);
  Log.addLogger(logToMqtt);
}

void setupNtc() {
  ntc.begin(NTC_PIN);
  ntcView.model = &ntc;
  ntc.modelUpdateEvent = []() { est.consolida(); };
  ntc.enableSampling();

  ntcBtn.begin(NTC_BTN_PIN);
  pinMode(NTC_BTN_PIN, INPUT);
  ntcBtn.buttonReleased = []() { ntcView.invalidate(); };
}

void setupDht() {
  dht.begin(DHT_PIN);
  dhtView.model = &dht;
  dht.modelUpdateEvent = []() { est.consolida(); };
  dht.modelReadSampleInFailEvent = []() {
    dht.resetSampling();
    // const String err = String("falha ao ler sensor depois de ") +
    //                    dht.readSampleInFailMaxTime_ms + "ms";
    // dhtView.addError(err);
    // dhtView.invalidate();
  };
  dht.enableSampling();

  dhtBtn.begin(DHT_BTN_PIN);
  pinMode(DHT_BTN_PIN, INPUT);
  dhtBtn.buttonReleased = []() {
    if (dht.isSamplingInFail()) {
      const String err = String("DHT está em falha");
      dhtView.addError(err);
    }
    dhtView.invalidate();
  };
}

void setupPir() {
  pir.begin(PIR_PIN);
  pirView.model = &pir;
  pir.motionStartEvent = []() {
    est.consolida();
    // Publica evento de movimento detectado
    if (mqttClient.isConnected()) {
      mqttClient.publishEvent("motion_start", "Movimento detectado");
    }
  };
  pir.motionStopEvent = []() {
    est.consolida();
    // Publica evento de movimento cessado
    if (mqttClient.isConnected()) {
      mqttClient.publishEvent("motion_stop", "Movimento cessado");
    }
  };
}

void setupEst() {}

WiFiManager wif;

void setupWiFi() {
  wif.begin();
  wif.connectingEvent      = []() { Log.println("wif.connectingEvent"); };
  wif.connectedEvent = []() {
    Log.print("wif.connectedEvent ");
    Log.println(WiFi.localIP());
  };
  wif.connectionLostEvent  = []() { Log.println("wif.connectionLostEvent"); };
  wif.disconnectionEvent   = []() { Log.println("wif.disconnectionEvent"); };
  wif.connectionStopEvent  = []() { Log.println("wif.connectionStopEvent"); };
}

// Callback para comandos MQTT recebidos
void mqttCommandCallback(const char* topic, const byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Log.print("[COMANDO] Recebido: ");
  Log.println(message);

  if (strcmp(message, "firmware_update_stop") == 0) {
    if (ota.stopUpdate()) {
      mqttClient.publishEvent("ota", "update cancelado");
    } else {
      mqttClient.publishEvent("ota", ota.getLastError());
    }
    return;
  }

  // firmware_update <url>
  if (strncmp(message, "firmware_update", 15) == 0) {
    const char* url = message + 15;
    while (*url == ' ') ++url;
    if (*url == '\0') {
      Log.println("[COMANDO] Uso: firmware_update <url_do_bin>");
      mqttClient.publishEvent("ota", "uso: firmware_update <url>");
      return;
    }
    if (ota.startUpdate(url)) {
      mqttClient.publishEvent("ota", "download iniciado");
    } else {
      mqttClient.publishEvent("ota", ota.getLastError());
    }
    return;
  }

  // firmware_mark_ok
  if (strcmp(message, "firmware_mark_ok") == 0) {
    if (ota.markOk()) {
      mqttClient.publishEvent("ota", "firmware marcado como valido");
    } else {
      mqttClient.publishEvent("ota", "mark ok falhou");
    }
    return;
  }

  // firmware_mark_invalid_reboot
  if (strcmp(message, "firmware_mark_invalid_reboot") == 0) {
    if (ota.markInvalidReboot()) {
      mqttClient.publishEvent("ota", "firmware marcado como valido, reboot");
    } else {
      mqttClient.publishEvent("ota", "firmware marcado como valido falhou");
    }
    return;
  }

  if (strcmp(message, "led_on") == 0) {
    Log.println("[COMANDO] Acionando LED (simulado)");
  } else if (strcmp(message, "led_off") == 0) {
    Log.println("[COMANDO] Desligando LED (simulado)");
  } else if (strcmp(message, "read_now") == 0) {
    Log.println("[COMANDO] Mostrando a leitura ambiental atual da estação");
    estView.invalidate();
  } else {
    Log.print("[COMANDO] Comando desconhecido: ");
    Log.println(message);
  }
}

void consoleInput() {
  if (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 'n') {
      ntcBtn.buttonReleased();
    }
    if (comando == 'd') {
      comando = Serial.peek();
      if (comando == 'f') {  // df <enter> simula falha, d depois mostra a falha
        dht.testing_setSamplingInFailAndReadSampleLastTimeOk_toFailState();
        for (int i = 0; i < 30; ++i) {
          dht.addBadRead();
        }
      } else {
        dhtBtn.buttonReleased();
      }
    }
    if (comando == 'p') {
      pir.motionStart();
    }
    if (comando == 'i') {
      estView.invalidate();
    }
    if (comando == 'w') {
      comando = Serial.peek();
      if (comando == 's') {
        //wif.connectingEvent  wif.connectedEvent
        wif.start();
      }
      if (comando == 'f') {
        // wif.connectionStopEvent
        wif.stop();
      }
      /* TODO: generate these events
         if (comando == 'd') {
         // wif.connectionLostEvent
         // wif.disconnectionEvent
         WiFi.active(false);
         }
         if (comando == 'r') {
         WiFi.setAutoReconnect(true);
         }
      */
    }
    if (comando == 'q') {
      comando = Serial.peek();
      if (comando == 's') {
        mqttClient.connect();
      }
      if (comando == 'f') {
        mqttClient.disconnect();
      }
      if (comando == 'p') {
        currentMessage.buildFrom(est);
        mqttClient.publishTelemetry(currentMessage);
      }
    }

    if (comando == 'o') {
      comando = Serial.peek();
      if (comando == 'm') {
        Serial.read();
        ota.markOk();
      }
      if (comando == 'i') {
        Serial.read();
        ota.markInvalidReboot();
      }
    }

    if (comando == 'l') {
      String tp = "Logger";
      LoggerBase* obj = &Log;
      if (Serial.available() > 0) {
        comando = Serial.read();
        if (comando == '0') {
          obj = logToSer0;
          tp = "LoggerSerial";
        }
        if (comando == 'q') {
          obj = logToMqtt;
          tp = "LoggerMqtt";
        }
        if (Serial.available() > 0) {
          comando = Serial.read();
          if (comando == 's') {
            obj->enable();
            Log.printf("[LOG] iniciado   para %s\n", tp.c_str());
          }
          if (comando == 'f') {
            Log.printf("[LOG] finalizado para %s\n", tp.c_str());
            obj->disable();
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  setupWatchDog();
  setupLogger();
  setupNtc();
  setupDht();
  setupPir();
  setupEst();
  setupWiFi();

  // Configura callback MQTT
  mqttClient.setCallback(mqttCommandCallback);

  ota.begin();
  ota.setStatusCallback([](OtaUpdater::State s, const char* detail) {
    if (s == OtaUpdater::State::Failed) {
      Log.print("[OTA][cb] falha: ");
      Log.println(detail);
    }
  });

  Log.println("=== Estacao Meteorologica Iniciada ===");
  Log.println("HTTP API: httpbin.org/post (eco)");
  Log.println("MQTT Broker: broker.hivemq.com:1883");
  Log.println("Topicos MQTT:");
  Log.println("  - Telemetria: estacao/telemetria");
  Log.println("  - Logs:       estacao/logs");
  Log.println("  - Eventos:    estacao/eventos");
  Log.println("  - Comandos (sub): estacao/comandos/entrada");
  Log.println("  - Comandos (pub): estacao/comandos/resposta");
  Log.println("OTA: comandos ouvidos em estacao/comandos/entrada");
  Log.println("  firmware_update <url>");
  Log.println("  firmware_mark_ok");
  Log.println("  firmware_mark_invalid_reboot");
}

void loop() {
  consoleInput();
  ntcBtn.processInput();
  dhtBtn.processInput();

  est.update();
  wif.update();
  mqttClient.update();
  ota.update();

  ntcView.render();
  dhtView.render();
  pirView.render();
  estView.render();

  // Envia dados para HTTP e MQTT periodicamente
  if (!wif.isConnected()) {
    return;
  } else {
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
      lastSendTime = currentTime;

      // Constrói mensagem única (sem duplicação de código)
      currentMessage.buildFrom(est);

      // Envia por HTTP (com retry automático)
      httpClient.send(currentMessage);

      // Publica por MQTT (com reconexão automática)
      if (mqttClient.isConnected()) {
        Log.println("\n--- Publicando via MQTT ---");
        mqttClient.publishTelemetry(currentMessage);
      } else {
        Log.println("\n[MQTT] Ainda nao conectado (update() esta tentando)");
      }
    }
  }

  wdg.feed();
  Log.update();
}
