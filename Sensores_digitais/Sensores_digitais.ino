#include "src/Controls.hpp"

#include "src/NtcSensor.hpp"
#include "src/NtcView.hpp"

#include "src/DHTSensor.hpp"
#include "src/DHTView.hpp"

#include "src/PirSensor.hpp"
#include "src/PirView.hpp"

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

void setupNtc() {
  ntc.pin = NTC_PIN;
  pinMode(NTC_PIN, INPUT);
  ntcView.model = &ntc;
  ntc.modelUpdateEvent = []() { ntcView.invalidate(); };

  ntcBtn.begin(NTC_BTN_PIN);
  pinMode(NTC_BTN_PIN, INPUT);
  ntcBtn.buttonReleased = []() { ntc.enableSampling(); };
}

void setupDht() {
  dht.begin(DHT_PIN);
  dhtView.model = &dht;
  dht.modelUpdateEvent = []() { dhtView.invalidate(); };
  dht.modelReadSampleInFailEvent = []() {
    dht.resetSampling();
    const String err = String("falha ao ler sensor depois de ") +
                       dht.readSampleInFailMaxTime_ms + "ms";
    dhtView.addError(err);
    dhtView.invalidate();
  };

  dhtBtn.begin(DHT_BTN_PIN);
  pinMode(DHT_BTN_PIN, INPUT);
  dhtBtn.buttonReleased = []() {
    if (dht.samplingInFail) {
      const String err = String("DHT está em falha");
      dhtView.addError(err);
      dhtView.invalidate();
    }
    dht.enableSampling();
  };
}

void setupPir() {
  pir.begin(PIR_PIN);
  pirView.model = &pir;
  pir.motionStartEvent = []() {
    pirView.msg = "PIR Motion Start";
    pirView.invalidate();
  };
  pir.motionStopEvent = []() {
    pirView.msg = "PIR Motion Stop";
    pirView.invalidate();
  };
}

void consoleInput() {
  if (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 'n') {
      ntcBtn.buttonReleased();
    }
    if (comando == 'd') {
      comando = Serial.peek();
      if (comando == 'f') { // df <enter> simula falha, d depois mostra a falha
        dht.setReadSampleInFail();
        dht.readSampleLastTimeOk_ms = millis() + dht.readSampleInFailMaxTime_ms;
      } else {
        dhtBtn.buttonReleased();
      }
    }
    if (comando == 'p') {
      pir.motionStart();
    }
  }
}

void setup() {
  Serial.begin(115200);

  setupNtc();
  setupDht();
  setupPir();
}

void loop() {
  consoleInput();
  ntcBtn.processInput();
  dhtBtn.processInput();

  ntc.update();
  dht.update();
  pir.update();

  ntcView.render();
  dhtView.render();
  pirView.render();
}
