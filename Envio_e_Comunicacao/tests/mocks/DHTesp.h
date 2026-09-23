#pragma once

#include <Arduino.h>

enum ComfortState {
  Comfort_OK = 0,
  Comfort_TooHot = 1,
  Comfort_TooCold = 2,
  Comfort_TooDry = 4,
  Comfort_TooHumid = 8,
  Comfort_HotAndHumid = 9,
  Comfort_HotAndDry = 5,
  Comfort_ColdAndHumid = 10,
  Comfort_ColdAndDry = 6
};

class TempAndHumidity {
 public:
  float temperature;
  float humidity;
};

class DHTesp {
public:

  typedef enum {
    AUTO_DETECT,
    DHT11,
    DHT22,
    AM2302,  // Packaged DHT22
    RHT03    // Equivalent to DHT22
  }
    DHT_MODEL_t;

  typedef enum {
    ERROR_NONE = 0,
    ERROR_TIMEOUT,
    ERROR_CHECKSUM
  }
    DHT_ERROR_t;

  void setup(uint8_t pin, DHT_MODEL_t model=AUTO_DETECT) {};
  TempAndHumidity getTempAndHumidity() { return { 0, 0}; };
  DHT_ERROR_t getStatus() { return DHT_ERROR_t::ERROR_NONE; };
  float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit=false) { return 0; };
  float computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit=false) { return 0; };
  float getComfortRatio(ComfortState& destComfStatus, float temperature, float percentHumidity, bool isFahrenheit=false) { return 0; };
};
