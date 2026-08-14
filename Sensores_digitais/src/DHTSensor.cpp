#include <Arduino.h>

#include "DHTSensor.hpp"
#include "DHTesp.h"
#include <algorithm>

void DHTSensor::begin(int aPin) {
  Part::begin(aPin);

  pinMode(aPin, INPUT);
  dhtSensor.setup(aPin, DHTesp::DHT22);
}

bool DHTSensor::shouldMeasure() { // override
  unsigned long agora = millis();
  if (agora - lastSampleReadTime_ms >= samplingInterval_ms) {
    lastSampleReadTime_ms = agora;

    return true;
  }

  return false;
}

TempAndHumidity DHTSensor::readSample() { // override
  return dhtSensor.getTempAndHumidity();
}

bool DHTSensor::readSampleWasOk() {
  return dhtSensor.getStatus() == DHTesp::ERROR_NONE;
}

bool DHTSensor::readSampleTempoMaximoEmFalhaUltrapassado() {
  unsigned long agora = millis();
  return agora - readSampleLastTimeOk_ms >= readSampleInFailMaxTime_ms;
}

void DHTSensor::measure()
{
  if (samplingComplete) return;

  if (shouldMeasure()) {
    samples[sampleIndex] = readSample();
    if (readSampleWasOk()) {
      sampleIndex++;
      readSampleLastTimeOk_ms = millis();
    } else {
      if (readSampleTempoMaximoEmFalhaUltrapassado()) {
        setReadSampleInFail();
        modelReadSampleInFailEvent();
        return;
      }
    }

    if (sampleIndex >= NUM_SAMPLES) {
      samplingComplete = true;
    }

    if (samplingComplete) {
      samplingEnabled = false;
      coletaDeAmostrasCompletaEvent();

      // prepara para a próxima
      resetSampling();
    }
  }
}

void DHTSensor::resetSampling() {
  samplingComplete = false;
  sampleIndex = 0;
}

void DHTSensor::enableSampling() {
  if (!samplingInFail || samplingInFailShouldRetry()) {
    samplingEnabled = true;
    samplingInFail = false;
  }
}

bool DHTSensor::samplingInFailShouldRetry() {
  unsigned long now_ms = millis();
  return now_ms > (readSampleLastTimeOk_ms + readSampleInFailMaxTime_ms +
                   readSampleInFailWaitTimeForRetry_ms);
}

void DHTSensor::setReadSampleInFail() {
  samplingInFail = true;
}


void DHTSensor::update() { coletarEProcessarAmostras(); }

void DHTSensor::coletarEProcessarAmostras() {
  if (!samplingEnabled) return;

  measure();
}

void DHTSensor::coletaDeAmostrasCompletaEvent()
{
  processedValue = processedValueRemovingOutliersFromSamples();
  processedValueWithEMA = processedValueWithEMAFilter(processedValue);

  modelUpdateEvent();
}

TempAndHumidity DHTSensor::processedValueWithEMAFilter(
    TempAndHumidity ntcProcessedVal) {
  float tempAtual = constrain(ntcProcessedVal.temperature, -273, 1000);
  // Filtro EMA (Exponential Moving Average)
  const float EMA_ALPHA = 0.15;
  ntcProcessedVal.temperature =
      EMA_ALPHA * tempAtual +
      (1.0 - EMA_ALPHA) *
          constrain(processedValueWithEMA.temperature, -273, 1000);

  float humidAtual = constrain(ntcProcessedVal.humidity, -273, 1000);
  // Filtro EMA (Exponential Moving Average)
  ntcProcessedVal.humidity =
      EMA_ALPHA * humidAtual +
      (1.0 - EMA_ALPHA) *
          constrain(processedValueWithEMA.humidity, -273, 1000);

  return ntcProcessedVal;
}

// Processa as amostras já coletadas (média aparada)
TempAndHumidity DHTSensor::processedValueRemovingOutliersFromSamples() {
  TempAndHumidity resp;

  const int len = sizeof(samples) / sizeof(samples[0]);

  std::sort(samples, samples + len,
            [](const TempAndHumidity &a, const TempAndHumidity &b) {
              return a.temperature < b.temperature;
            });


  // Descarta 25% de cada lado se houver no mínimo 4 amostras
  int inicio = (len < 3) ? 0 : len / 4;
  int fim = len - inicio;
  float soma = 0;
  int contagem = 0;

  for (int i = inicio; i < fim; i++) {
    soma += samples[i].temperature;
    contagem++;
  }

  resp.temperature = soma / contagem;
  // float mediaADC = (float)soma / contagem;
  //  return (mediaADC <= 0 || mediaADC >= 4095) ? -999 : mediaADC;

  std::sort(samples, samples + len,
            [](const TempAndHumidity &a, const TempAndHumidity &b) {
              return a.humidity < b.humidity;
            });

  // Descarta 25% de cada lado se houver no mínimo 4 amostras
  soma = 0;
  contagem = 0;

  for (int i = inicio; i < fim; i++) {
    soma += samples[i].humidity;
    contagem++;
  }

  // float mediaADC = (float)soma / contagem;
  //  return (mediaADC <= 0 || mediaADC >= 4095) ? -999 : mediaADC;
  resp.humidity = soma / contagem;

  return resp;
}
