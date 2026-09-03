* OTA versão 2

** Implementação
*** WatchDog
*** Logger
*** NetworkManager
*** HttpClientLocal: assincronia
*** MqttClient: aplicar MVC
*** OtaUpdater: clean code
*** Router de comandos desacoplado da view
*** Definir e aplicar um framework de monitoramento
*** Console agnóstico: via serial, Mqtt

** Testes
*** Definir um framework de testes
*** Implementar testes unitários para as classes
**** NetworkManager, HttpClientLocal
**** OtaUpdater, MqttClient


```mermaid
gantt
    title OTA versão 2
    dateFormat  YYYY-MM-DD
    axisFormat  %d/%m

    section Fundação
        WatchDog                        :a1, 2026-09-08, 3d
        Logger                          :a2, after a1, 3d
        Definir framework de testes     :t0, 2026-09-09, 1d
        Testes unitários                :tu_0, after t0, 4d

    section NetworkManager
        Implementação                   :nm_i, after a2, 5d
        Testes unitários                :nm_t, after t0, 3d

    section HttpClientLocal (assíncrono)
    Implementação                       :hc_i, after nm_i, 5d
    Testes                              :hc_t, after nm_i, 3d

    section MqttClient (MVC)
    Implementação                       :mq_i, after hc_i, 5d
    Testes unitários                    :mq_t, after hc_i, 3d

    section OtaUpdater (clean code)
    Implementação                       :ot_i, after mq_i, 5d
    Testes unitários                    :ot_t, after mq_i, 3d

    section Outros
    Router de comandos                  :a7, after ot_i, 5d
    Framework de monitoramento          :a8, after a7, 5d
    Console por Serial/MQTT             :a9, after a8, 5d
```
