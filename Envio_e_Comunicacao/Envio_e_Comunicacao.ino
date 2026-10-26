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

#include <NetworkManager.h> // soosp/NetworkManager
#include "src/NetManager.hpp"
#include <WiFiAdapter.h>
#include <WiFiProfile.h>

#include "src/Message.hpp"
#include "src/Publisher.hpp"
#include "src/HttpClientLocal.hpp"
#include "src/MqttClient.hpp"
#include "src/OtaUpdater.hpp"

#include "src/Logger.hpp"
#include "src/LoggerMqtt.hpp"
#include "src/LoggerSerial.hpp"
#include "src/Console.hpp"
#include "src/MqttStream.hpp"
#include "src/Router.hpp"
#include "src/Command.hpp"

Console console(Serial);

const uint8_t NTC_PIN     = 34;
const uint8_t NTC_BTN_PIN = 27;
NtcSensor     ntc;
NtcView       ntcView(&ntc, &console);
Button        ntcBtn;

const uint8_t DHT_PIN     = 33;
const uint8_t DHT_BTN_PIN = 16;
DHTSensor     dht;
DHTView       dhtView(&dht, &console);
Button        dhtBtn;

const uint8_t PIR_PIN = 17;
PirSensor     pir;
PirView       pirView(&pir, &console);

Estacao est(ntc, dht, pir);
EstacaoView estView(&est, &console);

OtaUpdater ota;

WatchDog wdg(5000);

void setupWatchDog() {
  wdg.begin();
}

LoggerSerial* logToSer0 = nullptr;
LoggerMqtt*   logToMqtt = nullptr;

void setupLogger() {
  logToSer0 = new LoggerSerial(Serial);
  logToMqtt = new LoggerMqtt(MQTT);
  Log.addLogger(logToSer0);
  Log.addLogger(logToMqtt);
}

void setupNtc() {
  ntc.begin(NTC_PIN);
  ntc.modelUpdateEvent = []() { est.consolida(); };
  ntc.enableSampling();

  ntcBtn.begin(NTC_BTN_PIN);
  pinMode(NTC_BTN_PIN, INPUT);
  ntcBtn.buttonReleased = []() { ntcView.invalidate(); };
}

void setupDht() {
  dht.begin(DHT_PIN);
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
  pir.motionStartEvent = []() {
    est.consolida();
    // Publica evento de movimento detectado
    Pub.publishEvent("motion_start", "Movimento detectado");
  };
  pir.motionStopEvent = []() {
    est.consolida();
    // Publica evento de movimento cessado
    Pub.publishEvent("motion_stop", "Movimento cessado");
  };
}

void setupEst() {}

WiFiProfile wifiProfile;
WiFiAdapter wifiAdapter(wifiProfile);

WiFiProfile::WiFiConfig cfg = {
  .ssid = "Wokwi-GUEST",
  .password = ""
};

void setupNetwork() {
  NetMgr.startEvent     = []() { Log.println("NetMgr.startEvent"); };

  NetMgr.connectedEvent      = []() {
    Log.print("NetMgr.connectedEvent - IP: ");
    Log.println(NetworkManagerClass::instance().getLocalIP());

    Pub.start();
    MQTT.start();
  };
  NetMgr.disconnectedEvent  = []() {
    Log.println("NetMgr.disconnectedEvent");

    Pub.stop();
    MQTT.stop();
  };
  NetMgr.connectionFallbackEvent       = []() {
    Log.print("NetMgr.connectionFallbackEvent - IP: ");
    Log.println(NetworkManagerClass::instance().getLocalIP());

    Pub.restart();
    MQTT.restart();
  };
  NetMgr.connectionRestoreEvent = []() {
    Log.println("NetMgr.connectionRestoreEvent");

    Pub.restart();
    MQTT.restart();
  };

  NetMgr.stopEvent = []() {
    Log.println("NetMgr.stopEvent");

    Pub.stop();
    MQTT.stop();
  };

  wifiProfile.setConfig(cfg);
  NetworkManagerClass::instance().addAdapter(wifiAdapter);
  NetMgr.start();
}

void setupPublisher() {
  Pub.setEstacao(&est);
}

void consoleInput(StreamString dbgCmd, Console* out) {

  if (dbgCmd.available() > 0) {
    char comando = dbgCmd.read();
    if (comando == 'n') {
      ntcBtn.buttonReleased();
    }
    if (comando == 'd') {
      comando = dbgCmd.peek();
      if (comando == 'f') {  // df <enter> simula falha, d depois mostra a falha
        dht.testing_setSamplingInFailAndReadSampleLastTimeOk_toFailState();
        for (int i = 0; i < 30; ++i) {
          dht.addBadRead();
        }
      } else {
        dhtView.console = out;
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
      comando = dbgCmd.peek();
      if (comando == 's') {
        NetMgr.start();
      }
      if (comando == 'f') {
        NetMgr.stop();
      }
    }
    if (comando == 'q') {
      comando = dbgCmd.peek();
      if (comando == 's') {
        MQTT.connect();
      }
      if (comando == 'f') {
        MQTT.disconnect();
      }
      if (comando == 'p') {
        Pub.publishTelemetry();
      }
    }

    if (comando == 'o') {
      comando = dbgCmd.peek();
      if (comando == 'm') {
        dbgCmd.read();
        ota.markOk();
      }
      if (comando == 'i') {
        dbgCmd.read();
        ota.markInvalidReboot();
      }
    }

    if (comando == 'l') {
      String tp = "Logger";
      LoggerBase* obj = &Log;
      if (dbgCmd.available() > 0) {
        comando = dbgCmd.read();
        if (comando == '0') {
          obj = logToSer0;
          tp = "LoggerSerial";
        }
        if (comando == 'q') {
          obj = logToMqtt;
          tp = "LoggerMqtt";
        }
        if (dbgCmd.available() > 0) {
          comando = dbgCmd.read();
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

MqttStream mqttStream(MQTT);
Console consoleMqtt(mqttStream);

void setupRouter() {
  RTR.pollInput = [](Command& cmdOut) {
    if (console.available()) {
      String cmdline = console.readAvailableUntil('\n');
      if (cmdline.isEmpty()) return;
      cmdOut = Command::fromCommandLine(cmdline, &console);
    }
    if (consoleMqtt.available()) {
      String cmdline = consoleMqtt.readAvailableUntil('\n');
      if (cmdline.isEmpty()) return;
      cmdOut = Command::fromCommandLine(cmdline, &consoleMqtt);
    }
    return;
  };

  RTR.on("firmware_update_stop", [](const Command& cmd) {
      if (ota.stopUpdate()) {
        Pub.publishEvent("ota", "update cancelado");
      } else {
        Pub.publishEvent("ota", ota.getLastError());
      }
    });

  RTR.on("firmware_update", [](const Command& cmd) {
      if (cmd.getArgsCount() <= 0) {
        Log.println("[COMANDO] Uso: firmware_update <url_do_bin>");
        Pub.publishEvent("ota", "uso: firmware_update <url>");
        return;
      }
      const String& url = cmd.args[0];
      if (ota.startUpdate(url.c_str())) {
        Pub.publishEvent("ota", "download iniciado");
      } else {
        Pub.publishEvent("ota", ota.getLastError());
      }
    });

  RTR.on("firmware_mark_ok", [](const Command& cmd) {
      if (ota.markOk()) {
        Pub.publishEvent("ota", "firmware marcado como valido");
      } else {
        Pub.publishEvent("ota", "mark ok falhou");
      }
    });

  RTR.on("firmware_mark_invalid_reboot", [](const Command& cmd) {
      if (ota.markInvalidReboot()) {
        Pub.publishEvent("ota", "firmware marcado como valido, reboot");
      } else {
        Pub.publishEvent("ota", "firmware marcado como valido falhou");
      }
    });

  RTR.on("led_on", [](const Command& cmd) {
      Log.println("[COMANDO] Acionando LED (simulado)");
    });

  RTR.on("led_off", [](const Command& cmd) {
      Log.println("[COMANDO] Desligando LED (simulado)");
    });

  RTR.on("read_now", [](const Command& cmd) {
      Log.println("[COMANDO] Mostrando a leitura ambiental atual da estação");
      estView.console = cmd.sender;
      estView.invalidate();
    });

  RTR.on("default", [](const Command& cmd) {
      StreamString scmd;
      scmd += cmd.name;
      consoleInput(scmd, cmd.sender);
    });
}

void setup() {
  Serial.begin(115200);

  setupWatchDog();
  setupLogger();
  setupNtc();
  setupDht();
  setupPir();
  setupEst();
  setupNetwork();
  setupPublisher();
  setupRouter();

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
  ntcBtn.processInput();
  dhtBtn.processInput();

  RTR.update();
  
  est.update();
  NetMgr.update();
  Pub.update();
  MQTT.update();
  ota.update();

  ntcView.render();
  dhtView.render();
  pirView.render();
  estView.render();

  wdg.feed();
  Log.update();
}
