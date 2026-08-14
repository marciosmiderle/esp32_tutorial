#include <Arduino.h>

#include "NtcSensor.hpp"

bool NtcSensor::shouldMeasure() { // override
  unsigned long agora = millis();
  if (agora - lastSampleReadTime_ms >= samplingInterval_ms) {
    lastSampleReadTime_ms = agora;

    return true;
  }

  return false;
}

int NtcSensor::readSample() {  // override
  return analogRead(pin);
}

void NtcSensor::measure()
{
  if (samplingComplete) return;

  if (shouldMeasure()) {
    samples[sampleIndex] = readSample();
    sampleIndex++;

    if (sampleIndex >= NUM_SAMPLES) {
      samplingComplete = true;
    }

    if (samplingComplete) {
      samplingEnabled = false;
      coletaDeAmostrasCompletaEvent();

      // prepara para a próxima
      samplingComplete = false;
      sampleIndex = 0;
    }
  }
}

void NtcSensor::enableSampling()
{
  samplingEnabled = true;
}

void NtcSensor::update() {
  coletarEProcessarAmostras();
}

float NtcSensor::tension(float analogValue)
{
  return (analogValue / 4095.0) * 3.3; // linear
}

float NtcSensor::temperatureInCensius(float analogValue)
{
  const float BETA = 3950;
  const float maxAnalogValue = 4096 - 1;
  return 1.0 / (log(1.0 / (maxAnalogValue / analogValue - 1.0)) / BETA + 1.0 / 298.15) - 273.15;
}

void NtcSensor::coletarEProcessarAmostras()
{
  if (!samplingEnabled) return;

  measure();
}

void NtcSensor::coletaDeAmostrasCompletaEvent()
{
  processedValue = processedValueRemovingOutliersFromSamples();
  filteredTemperatureWithEMA = processedTemperatureWithEMAFilter(processedValue);

  modelUpdateEvent();
}

float NtcSensor::processedTemperatureWithEMAFilter(float ntcProcessedVal)
{
  float tempAtual = constrain(temperatureInCensius(ntcProcessedVal), -273, 1000);
  // Filtro EMA (Exponential Moving Average)
  const float EMA_ALPHA = 0.15;
  return EMA_ALPHA * tempAtual + (1.0 - EMA_ALPHA) *
    constrain(filteredTemperatureWithEMA, -273, 1000);
}

// Processa as amostras já coletadas (média aparada)
float NtcSensor::processedValueRemovingOutliersFromSamples()
{
  const int len = sizeof(samples) / sizeof(samples[0]);
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
