#pragma once

#include "Part.hpp"
#include "Sampling.hpp"
#include "Telemetry.hpp"
#include <functional>

const int NUM_SAMPLES = 32;
const unsigned int samplingInterval_ms = 5;

class NtcSensor : public Part,
                  public Sampling <int, NUM_SAMPLES, samplingInterval_ms>, public Telemetry {
public:
  float processedValue = 0;
  float filteredTemperatureWithEMA = 25.0;

  std::function<void()> modelUpdateEvent = [](){};
  std::function<void()> modelReadSampleInFailEvent = [](){};

  void begin(int aPin);

  bool shouldSample();
  bool readSampleWasOk();
  int readSample();
  void sample();

  void update();

  float tension(float analogValue);
  float temperatureInCensius(float analogValue);

  void coletarEProcessarAmostras();
  void coletaDeAmostrasCompletaEvent();
  float processedTemperatureWithEMAFilter(float ntcProcessedVal);
  // Processa as amostras já coletadas (média aparada)
  float processedValueRemovingOutliersFromSamples();
};
