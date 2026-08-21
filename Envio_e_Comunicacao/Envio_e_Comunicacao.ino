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
#include "src/HttpClient.hpp"
#include "src/MqttClient.hpp"

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
HttpClient httpClient(API_URL);

// Configurações do Broker MQTT
const char* MQTT_BROKER = "broker.hivemq.com";  // Broker público HiveMQ
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "estacao_meteorologica_001";
const char* MQTT_TELEMETRY_TOPIC = "estacao/telemetria";
const char* MQTT_EVENT_TOPIC = "estacao/eventos";
const char* MQTT_COMMAND_TOPIC = "estacao/comandos/resposta";
const char* MQTT_COMMAND_SUBSCRIBE_TOPIC = "estacao/comandos/entrada";

MqttClient mqttClient(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID,
                      MQTT_TELEMETRY_TOPIC, MQTT_EVENT_TOPIC,
                      MQTT_COMMAND_TOPIC, MQTT_COMMAND_SUBSCRIBE_TOPIC);

Message currentMessage;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL_MS = 10000;  // Envia a cada 10 segundos

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
  wif.connectingEvent      = []() { Serial.println("wif.connectingEvent"); };
  wif.connectedEvent = []() {
    Serial.print("wif.connectedEvent ");
    Serial.println(WiFi.localIP());  
  };  
  wif.connectionLostEvent  = []() { Serial.println("wif.connectionLostEvent"); };
  wif.disconnectionEvent   = []() { Serial.println("wif.disconnectionEvent"); };
  wif.connectionStopEvent  = []() { Serial.println("wif.connectionStopEvent"); };
}

// Callback para comandos MQTT recebidos
void mqttCommandCallback(const char* topic, const byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  Serial.print("[COMANDO] Recebido: ");
  Serial.println(message);
  
  // Processa comandos simples
  if (strcmp(message, "led_on") == 0) {
    Serial.println("[COMANDO] Acionando LED (simulado)");
    // digitalWrite(LED_PIN, HIGH);
  } else if (strcmp(message, "led_off") == 0) {
    Serial.println("[COMANDO] Desligando LED (simulado)");
    // digitalWrite(LED_PIN, LOW);
  } else if (strcmp(message, "read_now") == 0) {
    Serial.println("[COMANDO] Forçando leitura imediata dos sensores");
    estView.invalidate();
  } else {
    Serial.print("[COMANDO] Comando desconhecido: ");
    Serial.println(message);
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
  }
}

void setup() {
  Serial.begin(115200);

  setupNtc();
  setupDht();
  setupPir();
  setupEst();
  setupWiFi();
  
  // Configura callback MQTT
  mqttClient.setCallback(mqttCommandCallback);
  
  Serial.println("=== Estacao Meteorologica Iniciada ===");
  Serial.println("HTTP API: httpbin.org/post (eco)");
  Serial.println("MQTT Broker: broker.hivemq.com:1883");
  Serial.println("Topicos MQTT:");
  Serial.println("  - Telemetria: estacao/telemetria");
  Serial.println("  - Eventos: estacao/eventos");
  Serial.println("  - Comandos (sub): estacao/comandos/entrada");
  Serial.println("  - Comandos (pub): estacao/comandos/resposta");
}

void loop() {
  consoleInput();
  ntcBtn.processInput();
  dhtBtn.processInput();

  est.update();
  wif.update();
  mqttClient.update();

  ntcView.render();
  dhtView.render();
  pirView.render();
  estView.render();
  
  // Envia dados para HTTP e MQTT periodicamente
  if (!wif.isConnected()) {
    Serial.println("\nWiFi desconectado, pulando envio de mensagens");
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
        Serial.println("\n--- Publicando via MQTT ---");
        mqttClient.publishTelemetry(currentMessage);
      } else {
        Serial.println("\n[MQTT] Não conectado, tentando reconectar...");
        mqttClient.disconnect();
        mqttClient.connect();
      }
    }
  }
}
