#include <Arduino.h>

#include "DHTView.hpp"

void DHTView::logValues() {
  const TempAndHumidity newValues = model->processedValueWithEMA;

  float heatIndex = model->dhtSensor.computeHeatIndex(newValues.temperature,
                                                      newValues.humidity);
  float dewPoint = model->dhtSensor.computeDewPoint(newValues.temperature,
                                                    newValues.humidity);
  ComfortState cf;
  float cr = model->dhtSensor.getComfortRatio(cf, newValues.temperature,
                                              newValues.humidity);
  (void)cr;

  String comfortStatus;
  switch (cf) {
  case Comfort_OK:
    comfortStatus = "OK";
    break;
  case Comfort_TooHot:
    comfortStatus = "Too Hot";
    break;
  case Comfort_TooCold:
    comfortStatus = "Too Cold";
    break;
  case Comfort_TooDry:
    comfortStatus = "Too Dry";
    break;
  case Comfort_TooHumid:
    comfortStatus = "Too Humid";
    break;
  case Comfort_HotAndHumid:
    comfortStatus = "Hot and Humid";
    break;
  case Comfort_HotAndDry:
    comfortStatus = "Hot and Dry";
    break;
  case Comfort_ColdAndHumid:
    comfortStatus = "Cold and Humid";
    break;
  case Comfort_ColdAndDry:
    comfortStatus = "Cold and Dry";
    break;
  default:
    comfortStatus = "Unknown";
    break;
  };

  Serial.printf("DHT22 Temp = %.1f°C Umid = %.1f%% I.C. = %.1f P.Orv = %.1f°C Conf = %s",
                newValues.temperature, newValues.humidity, heatIndex, dewPoint,
                comfortStatus.c_str());
  Serial.println("");
}

void DHTView::addError(String error) {
  errors += error + "\r\n";
}

void DHTView::logErrors() {
  Serial.print(errors);
  Serial.println("");
  errors = "";
}

void DHTView::render() {
  if (!isValid()) {
    if (errors != "") {
      logErrors();
    } else {
      logValues();
    }

    setValid();
  }
}
