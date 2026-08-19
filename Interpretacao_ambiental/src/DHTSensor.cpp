#include "DHTSensor.hpp"
#include <algorithm>

void DHTSensor::begin(int aPin) {
  Part::begin(aPin);

  pinMode(aPin, INPUT);
  dhtSensor.setup(aPin, DHTesp::DHT22);
}

bool DHTSensor::shouldSample() { // override
  return sampleIntervalHasPassed();
}

TempAndHumidity DHTSensor::readSample() { // override
  return dhtSensor.getTempAndHumidity();
}

bool DHTSensor::readSampleWasOk() {
  return dhtSensor.getStatus() == DHTesp::ERROR_NONE;
}

void DHTSensor::sample() {
  if (isSamplingComplete()) return;

  if (shouldSample()) {
    TempAndHumidity lastSample = readSample();
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

void DHTSensor::update() { coletarEProcessarAmostras(); }

void DHTSensor::coletarEProcessarAmostras() {
  if (!isSamplingEnabled()) return;

  sample();
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

  const int len = samplesSize();

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
