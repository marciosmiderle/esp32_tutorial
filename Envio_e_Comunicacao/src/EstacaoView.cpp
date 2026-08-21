#include <Arduino.h>
#include "EstacaoView.hpp"

EstacaoView::EstacaoView(Estacao* estacao) {
  model = estacao;
}

void EstacaoView::logValues() {
  NtcSensor& ntc = model->ntc;
  const char* ntcHealth = ntc.health();
  const float ntcRatio = ntc.healthRatio();
  
  DHTSensor& dht = model->dht;
  const char* dhtHealth = dht.health();
  const float dhtRatio = dht.healthRatio();
  const TempAndHumidity dhtValues = dht.processedValueWithEMA;
  const float dhtHumidity = dhtValues.humidity;
  
  PirSensor& pir = model->pir;
  const char* pirHealth = pir.health();
  const char* pirRatio = pir.health();
  const bool pirMotion = pir.inMotion();

  const float meanTemp = model->meanTemperature();
  // Interpretações
  const char* tempStatus = model->interpretTemperature(meanTemp);
  const char* humidityStatus = model->interpretHumidity(dhtHumidity);
  const char* motionStatus = model->interpretMotion();
  
  // Detecta divergência entre sensores de temperatura (NTC vs DHT)
  const bool divergence = model->sensorDivergence();
  const char* divergenceStatus = divergence ? "SIM" : "Nao";
  const char* divergenceObs = divergence ? "Diferenca > 5C" : "OK";
  
  // Nível geral de atenção
  int attentionLevel = model->calculateAttentionLevel();
  const char* attentionStatus = model->interpretAttentionLevel(attentionLevel);
  
  // Observações
  String tempObs = String(meanTemp, 1) + "C";
  String humidityObs = String(dhtHumidity, 1) + "%";
  String motionObs = pirMotion ? "Detectado" : "Nenhum";
  
  const char* condicaoDosSensoresFmt = R"HEREDOC_SENSORES(
+------------------------------------------------------------------------------+
|                                                                              |
|                        INTERPRETACAO AMBIENTAL                               |
|                                                                              |
+----------------------------------------------------------------------------- +
|                                                                              |
|  CONDICAO DOS SENSORES                                                       |
|  +----------------+----------------+----------------+----------------+       |
|  | Sensor         | NTC            | DHT            | PIR            |       |
|  +----------------+----------------+----------------+----------------+       |
|  | Status         | %-14s | %-14s | %-14s |       |                           
|  | Saude          | %14.1f | %14.1f | %14.1f |       |                        
|  +----------------+----------------+----------------+----------------+       |
|                                                                              |)HEREDOC_SENSORES";
  Serial.printf(condicaoDosSensoresFmt, 
                ntcHealth, dhtHealth, pirHealth,
                ntcRatio, dhtRatio, pirRatio);

  const char* interpretacaoFmtParte1 = R"HEREDOC_INT1(
|  INTERPRETACAO AMBIENTAL                                                     |
|  +---------------------------+----------------+---------------------------+  |
|  | Parametro                 | Valor          | Interpretacao             |  |
|  +---------------------------+----------------+---------------------------+  |
|  | Condicao termica          | %-14s | %-25s |  |
|  | Condicao de umidade       | %-14s | %-25s |  |)HEREDOC_INT1";
  Serial.printf(interpretacaoFmtParte1, 
                tempObs.c_str(), tempStatus,
                humidityObs.c_str(), humidityStatus);

  const char* interpretacaoFmtParte2 = R"HEREDOC_INT2(
|  | Presenca/Movimento        | %-14s | %-25s |  |
|  | Divergencia sensores      | %-14s | %-25s |  |
|  +---------------------------+----------------+---------------------------+  |
|                                                                              |
|  +-----------------------------------------------------------------------+   |
|  | NIVEL GERAL DE ATENCAO: %-45s |   |
|  +-----------------------------------------------------------------------+   |
|                                                                              |
+------------------------------------------------------------------------------+
)HEREDOC_INT2";

  Serial.printf(interpretacaoFmtParte2,
                motionObs.c_str(), motionStatus,
                divergenceStatus, divergenceObs,
                attentionStatus);
}

void EstacaoView::addError(String error) {
  errors += error + "\n";
}

void EstacaoView::logErrors() {
  Serial.print(errors);
  errors = "";
}

void EstacaoView::render() {
  if (!isValid()) {
    if (errors != "") {
      logErrors();
    } else {
      logValues();
    }
    setValid();
  }
}
