#pragma once

#include "Part.hpp"
#include "Sampling.hpp"
#include "Telemetry.hpp"
#include <DHTesp.h>
#include <functional>

class DHTSensor : public Part, public Sampling<TempAndHumidity>, public Telemetry {
public:
  DHTesp dhtSensor;
  TempAndHumidity processedValue = {.temperature = 25.0, .humidity = 50.0};
  TempAndHumidity processedValueWithEMA = {.temperature = 25.0,
                                            .humidity = 50.0};

  std::function<void()> modelUpdateEvent = [](){};
  std::function<void()> modelReadSampleInFailEvent = [](){};

  void begin(int aPin);

  bool shouldSample();
  bool readSampleWasOk();
  TempAndHumidity readSample();
  void sample();

  void update();

  void coletarEProcessarAmostras();
  void coletaDeAmostrasCompletaEvent();
  TempAndHumidity processedValueWithEMAFilter(TempAndHumidity ntcProcessedVal);
  // Processa as amostras já coletadas (média aparada)
  TempAndHumidity processedValueRemovingOutliersFromSamples();
};
