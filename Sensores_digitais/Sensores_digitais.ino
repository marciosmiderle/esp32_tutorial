#include "src/Controls.hpp"

#include "src/NtcSensor.hpp"
#include "src/NtcView.hpp"

#include "src/DHTSensor.hpp"
#include "src/DHTView.hpp"

const int  NTC_PIN     = 14;
const int  NTC_BTN_PIN = 4;
NtcSensor  ntc;
NtcView    ntcView;
Button     ntcBtn;

const int DHT_PIN     = 33;
const int DHT_BTN_PIN = 16;
DHTSensor dht;
DHTView   dhtView;
Button    dhtBtn;

void setup() {
  Serial.begin(115200);

  // NTC
  ntc.pin = NTC_PIN;
  pinMode(NTC_PIN, INPUT);
  ntcView.model = &ntc;
  ntc.modelUpdateEvent = []() { ntcView.invalidate(); };

  ntcBtn.begin(NTC_BTN_PIN);
  pinMode(NTC_BTN_PIN, INPUT);
  ntcBtn.buttonReleased = []() { ntc.enableSampling(); };
  // /NTC

  // DHT
  dht.begin(DHT_PIN);
  dhtView.model = &dht;
  dht.modelUpdateEvent = []() { dhtView.invalidate(); };
  dht.modelReadSampleInFailEvent = []() {
    dht.resetSampling();
    const String err = String("falha ao ler sensor depois de ") +
                       dht.readSampleTempoMaximoEmFalha_ms + "ms";
    dhtView.addError(err);
    dhtView.invalidate();
  };

  dhtBtn.begin(DHT_BTN_PIN);
  pinMode(DHT_BTN_PIN, INPUT);
  dhtBtn.buttonReleased = []() { dht.enableSampling(); };
  // /DHT
}

void console() {
  if (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 'n') {
      ntcBtn.buttonReleased();
    }
    if (comando == 'd') {
      dhtBtn.buttonReleased();
    }
  }
}

void loop() {
  console();

  ntcBtn.processInput();
  dhtBtn.processInput();

  ntc.update();
  dht.update();

  ntcView.render();
  dhtView.render();
}
