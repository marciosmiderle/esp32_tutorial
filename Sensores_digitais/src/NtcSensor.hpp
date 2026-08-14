#pragma once

#include <functional>

class NtcSensor {

public:
  int pin    = 14;
  float processedValue    = 0;
  float filteredTemperatureWithEMA = 25.0;

  static const int NUM_SAMPLES = 32;
  int samples[NUM_SAMPLES];
  int sampleIndex = 0;
  unsigned int samplingInterval_ms = 5; // ms
  unsigned long lastSampleReadTime_ms = 0;

  bool samplingEnabled = false;
  bool samplingComplete = false;
  std::function<void()> modelUpdateEvent;

  bool shouldMeasure();
  int readSample();
  void measure();

  void enableSampling();
  void update() ;
  float tension(float analogValue);
  float temperatureInCensius(float analogValue);

  void coletarEProcessarAmostras();
  void coletaDeAmostrasCompletaEvent();
  float processedTemperatureWithEMAFilter(float ntcProcessedVal);
  // Processa as amostras já coletadas (média aparada)
  float processedValueRemovingOutliersFromSamples();
};
