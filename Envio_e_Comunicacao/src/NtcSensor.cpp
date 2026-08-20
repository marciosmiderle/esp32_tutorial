#include "NtcSensor.hpp"
#include <Arduino.h>

void NtcSensor::begin(int aPin) {
  Part::begin(aPin);

  pinMode(aPin, INPUT);
}

bool NtcSensor::shouldSample() {  // override
  return sampleIntervalHasPassed();
}

int NtcSensor::readSample() {  // override
  return analogRead(getPin());
}

bool NtcSensor::readSampleWasOk() {
  return true;
}

void NtcSensor::sample() {
  if (isSamplingComplete()) return;

  if (shouldSample()) {
    int lastSample = readSample();
    if (readSampleWasOk()) {
      addSample(lastSample);
      addGoodRead();
    } else {
      addBadRead();
      if (readSampleInFailMaxTimeExceeded()) {
        setSamplingInFail();
        modelReadSampleInFailEvent();
      }
      return;
    }

    if (isSamplingComplete()) {
      disableSampling();

      coletaDeAmostrasCompletaEvent();

      // prepara para a próxima
      resetSampling();
      enableSampling();
    }
  }
}

void NtcSensor::update() { coletarEProcessarAmostras(); }

float NtcSensor::tension(float analogValue) {
  return (analogValue / 4095.0) * 3.3;  // linear
}

float NtcSensor::temperatureInCensius(float analogValue) {
  const float BETA = 3950;
  const float maxAnalogValue = 4096 - 1;
  return 1.0 / (log(1.0 / (maxAnalogValue / analogValue - 1.0)) / BETA +
                1.0 / 298.15) -
         273.15;
}

void NtcSensor::coletarEProcessarAmostras() {
  if (!isSamplingEnabled()) return;

  sample();
}

void NtcSensor::coletaDeAmostrasCompletaEvent() {
  processedValue = processedValueRemovingOutliersFromSamples();
  filteredTemperatureWithEMA = processedTemperatureWithEMAFilter(processedValue);

  modelUpdateEvent();
}

float NtcSensor::processedTemperatureWithEMAFilter(float ntcProcessedVal) {
  float tempAtual =
      constrain(temperatureInCensius(ntcProcessedVal), -273, 1000);
  // Filtro EMA (Exponential Moving Average)
  const float EMA_ALPHA = 0.15;
  return EMA_ALPHA * tempAtual +
         (1.0 - EMA_ALPHA) * constrain(filteredTemperatureWithEMA, -273, 1000);
}

// Processa as amostras já coletadas (média aparada)
float NtcSensor::processedValueRemovingOutliersFromSamples() {
  const int len = samplesSize();
  std::sort(samples, samples + len);

  // Descarta 25% de cada lado
  int inicio = len / 4;
  int fim = len - inicio;
  long soma = 0;
  int contagem = 0;

  for (int i = inicio; i < fim; i++) {
    soma += samples[i];
    contagem++;
  }

  float mediaADC = (float)soma / contagem;
  return (mediaADC <= 0 || mediaADC >= 4095) ? -999 : mediaADC;
}
