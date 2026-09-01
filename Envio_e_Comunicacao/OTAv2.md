# Requisitos para o OTAv2

- Clean code                                                                                  [^Qualidade]
- código independente de tipo de conexão com a internet (Wifi, celular)     [^NetworkManager] [^OtaUpdater] [^WatchDog] [^Qualidade]
- WatchDog                                                                                    [^WatchDog]
  - deve reiniciar o sistema                                                                  [^Requisito]
    - quando passar o tempo para rollback sem receber mark ok
    - por falta de conexão com
      - mqtt client
      - http client
  - deve                                                                                      [^Restrição]
    - ser o primeiro componente a rodar
    - interromper por hardware os outros componentes para agir
- Logger com logrável que possam ser ativados e desativados, saída para serial ou HTTP        [^Requisito]
- Versão/firmware id legível no boot (Serial + evento MT)                                     [^Requisito]
- MqttClient
  - Deve ser capaz de iniciar um novo update                                                  [^Requisito]
  - QoS 1 nos comandos críticos, se o broker permitir                                         [^Restrição]
  - Publicar status (downloading, verifying, pending_ok, failed) no tópico de eventos         [^Requisito]
- HttpClientLocal
  - Deve ser capaz de baixar outro firmware                                                   [^Requisito]
- OtaUpdater deve:
  - ser assíncrono                                                                            [^Requisito]
    - assincronia 100% precisa de outra lib, mas pode-se fazer meio assincrono com a HTTPClient e timetouts pequenos com o RetryLogic
  - cancelável                                                                                [^Requisito]
  - ser validado manualmente                                                                  [^Requisito]
  - ser capaz de voltar à versão anterior:
    - caso a versão atual tenha sido marcada como válida, mas não consegue fazer novo upgrade [^Requisito]
    - uma vez marcado como válido, após o reboot, deve poder voltar ao anterior               [^Requisito]
  - não ser capaz de iniciar novo upgrade se a que está rodando não foi marcada como válida   [^Restrição]
  - ser testada num hardware igual ou melhor compatível ao que vai rodar                      [^Restrição]

# Testes

É preciso ter testes automáticos numa esteira de CI/CD.

- testes com 100% de cobertura para as classes:                                               [^Requisito] [^Qualidade]
  - NetworkManager
  - OtaUpdater
  - MqttClient
  - HttpClientLocal
- OtaUpdater
  - cenários
    - se `esp_ota_mark_app_valid_cancel_rollback()` e em seguida sem reboot `esp_ota_mark_app_invalid_rollback_and_reboot()`, marca a atual como inválida e retorna à outra
    - se `esp_ota_mark_app_valid_cancel_rollback()` sem app na outra partição, nada deve fazer

# Implantação

É preciso ter um procedimento de geração de versão final candidata, e esta ser validada em bancada e estações próximas.
- executar um roteiro de testes para a versão candidata
  - desde a geração da versão numa esteira CI/CD
  - testes de bancada
  - implantar em estações próximas
  - por fim nas distantes


# Notas

[^OtaUpdater]: Classe reponsável por atualizar e validar o firmware junto com o usuário.
[^WatchDog]: Classe responsável por reiniciar o sistema caso ele fique sem responder ou preso numa função.
[^NetworkManager]: Classe responsável por manter uma conexão com a internet e abstrair o tipo (WiFi, celular)
[^Requisito]: Requisito funcional.
[^Restrição]: Requisito não-funcional.
[^Qualidade]: Requisito não-funcional.
