#pragma once

#include <Arduino.h>
#include <ctime>

template <class SType, unsigned int NUM_SAMPLES_val = 4,
          unsigned int samplingInterval_ms_val = 500,
          time_t readSampleInFailMaxTime_ms_val = 5000,
          time_t readSampleInFailWaitTimeForRetry_ms_val = 5000>
class Sampling {
private:
  static constexpr unsigned int NUM_SAMPLES = NUM_SAMPLES_val;
  int sampleIndex = 0;

  time_t lastSampleReadTime_ms = 0;
  time_t readSampleLastTimeOk_ms = 0;

  static constexpr unsigned int samplingInterval_ms = samplingInterval_ms_val;
  static constexpr time_t readSampleInFailMaxTime_ms =
      readSampleInFailMaxTime_ms_val;
  static constexpr time_t readSampleInFailWaitTimeForRetry_ms =
      readSampleInFailWaitTimeForRetry_ms_val;

  bool samplingInFail = false;

  bool samplingEnabled = false;
  bool samplingComplete = false;

public:
  SType samples[NUM_SAMPLES];

  bool isSamplingEnabled() { return samplingEnabled; }

  bool isSamplingInFail() const { return samplingInFail; }

  bool isSamplingComplete() const { return samplingComplete; }

  void addSample(SType &sample) {
    sampleIndex++;
    readSampleLastTimeOk_ms = millis();
    samples[sampleIndex] = sample;

    if (sampleIndex >= NUM_SAMPLES) {
      samplingComplete = true;
    }
  }

  int samplesSize() { return NUM_SAMPLES; }

  void resetSampling() {
    samplingComplete = false;
    sampleIndex = 0;
  }

  void enableSampling() {
    if (!samplingInFail || samplingInFailShouldRetry()) {
      samplingEnabled = true;
      samplingInFail = false;
    }
  }

  void disableSampling() { samplingEnabled = false; }

  bool sampleIntervalHasPassed() {  // override
    unsigned long now = millis();
    if (now - lastSampleReadTime_ms >= samplingInterval_ms) {
      lastSampleReadTime_ms = now;
      return true;
    }
    return false;
  }

  bool samplingInFailShouldRetry() {
    unsigned long now_ms = millis();
    return now_ms > (readSampleLastTimeOk_ms + readSampleInFailMaxTime_ms +
                     readSampleInFailWaitTimeForRetry_ms);
  }

  void setSamplingInFail() { samplingInFail = true; }

  bool readSampleInFailMaxTimeExceeded() {
    unsigned long agora = millis();
    return agora - readSampleLastTimeOk_ms >= readSampleInFailMaxTime_ms;
  }

  void testing_setSamplingInFailAndReadSampleLastTimeOk_toFailState() {
    setSamplingInFail();
    readSampleLastTimeOk_ms = millis() + readSampleInFailMaxTime_ms;
  }
};
