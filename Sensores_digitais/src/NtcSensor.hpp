#pragma once

#include <functional>

class NtcSensor {

public:
  int pin    = 14;
  float ntcProcessedValue    = 0;
  float ntcFilteredTemperature = 25.0;

  static const int NUM_SAMPLES = 32; //TODO template
  int samples[NUM_SAMPLES];
  int indiceAmostra = 0;
  unsigned int intervaloAmostras = 5; // ms
  unsigned long ultimoTempoAmostra = 0;

  bool samplingEnabled = false;
  bool samplingComplete = false;
  std::function<void()> modelUpdateEvent;

  bool shouldMeasure(); // override
  int readSample();  // override
  void measure();

  void enableSampling();
  void update() ;
  float tension(float analogValue);
  float temperatureInCensius(float analogValue);

  void coletarEProcessarAmostras();
  void ntcColetaDeAmostrasCompletaEvent();
  float ntcProcessedTemperatureWithEMAFilter(float ntcProcessedVal);
  // Processa as amostras já coletadas (média aparada)
  float ntcProcessedValueRemovingOutliersFromSamples();
};
