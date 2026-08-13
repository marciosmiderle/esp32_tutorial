#pragma once

#include "Part.hpp"
#include <DHTesp.h>
#include <functional>

class DHTSensor : public Part {
public:
  DHTesp dhtSensor;
  TempAndHumidity processedValue = {.temperature = 25.0, .humidity = 50.0};
  TempAndHumidity processedValueWithEMA = {.temperature = 25.0,
                                            .humidity = 50.0};

  static const int NUM_SAMPLES = 4; // TODO template
  TempAndHumidity samples[NUM_SAMPLES];
  int indiceAmostra = 0;
  unsigned int intervaloAmostras_ms = 500;
  unsigned long ultimoTempoAmostra_ms = 0;
  unsigned long readSampleLastTimeOk_ms = 0;
  const unsigned long readSampleTempoMaximoEmFalha_ms = 20000;

  bool samplingEnabled = false;
  bool samplingComplete = false;
  std::function<void()> modelUpdateEvent = [](){};
  std::function<void()> modelReadSampleInFailEvent = [](){};

  void begin(int aPin);

  bool shouldMeasure();
  bool readSampleWasOk();
  bool readSampleTempoMaximoEmFalhaUltrapassado();
  TempAndHumidity readSample();


  void measure();

  void resetSampling();
  void enableSampling();

  void update();

  void coletarEProcessarAmostras();
  void coletaDeAmostrasCompletaEvent();
  TempAndHumidity processedValueWithEMAFilter(TempAndHumidity ntcProcessedVal);
  // Processa as amostras já coletadas (média aparada)
  TempAndHumidity processedValueRemovingOutliersFromSamples();
};
