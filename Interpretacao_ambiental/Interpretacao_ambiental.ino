#include "src/Controls.hpp"

#include "src/NtcSensor.hpp"
#include "src/NtcView.hpp"

#include "src/DHTSensor.hpp"
#include "src/DHTView.hpp"

#include "src/PirSensor.hpp"
#include "src/PirView.hpp"

#include "src/Estacao.hpp"
#include "src/EstacaoView.hpp"

const uint8_t NTC_PIN     = 14;
const uint8_t NTC_BTN_PIN = 4;
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
  pir.motionStartEvent = []() { est.consolida(); };
  pir.motionStopEvent = []() { est.consolida(); };
}

void setupEst() {}

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
  }
}

void setup() {
  Serial.begin(115200);

  setupNtc();
  setupDht();
  setupPir();
  setupEst();
}

void loop() {
  consoleInput();
  ntcBtn.processInput();
  dhtBtn.processInput();

  est.update();

  ntcView.render();
  dhtView.render();
  pirView.render();
  estView.render();
}
