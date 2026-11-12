# Contexto

Estas são as partições usadas no firmware (build/partitions.csv):

| Nome     | Tipo | SubTipo  | Offset   | Tamanho  | Tamanho (dec) | Uso                        |
|----------|------|----------|----------|----------|---------------|----------------------------|
| nvs      | data | nvs      | 0x9000   | 0x5000   | 20480         | configurações não voláteis |
| otadata  | data | ota      | 0xe000   | 0x2000   | 8192          | seletor de boot            |
| app0     | app  | ota\_0   | 0x10000  | 0x140000 | 1310720       | app do usuário             |
| app1     | app  | ota\_1   | 0x150000 | 0x140000 | 1310720       | upgrade do app do usuário  |
| spiffs   | data | spiffs   | 0x290000 | 0x160000 | 1441792       | dados não voláteis         |
| coredump | data | coredump | 0x3F0000 | 0x10000  | 65536         | --x--                      |

Antes destas partições há o bootloader e a tabela de partições gravadas na flash:

| Tamanho | Tamanho | Arquivo        | Descrição           | Offset de gravação |
|---------|---------|----------------|---------------------|--------------------|
| 23488   | 23K     | bootloader.bin | bootloader          | 0x1000             |
| 3072    | 3,0K    | partitions.bin | tabela de partições | 0x8000             |

O arquivo firmware.ino.merged.bin não vai para uma partição específica pois ele é a imagem completa que junta o bootloader, a tabela de partições e o app. para gravar diretamente na memória Flash no offset 0x0 (neste caso).

A ferramenta esptool merge-bin preenche as lacunas (faz o padding) com bytes 0xFF, neste caso para o arquivo final ter 4M.

| Offset  | Arquivo          | Partição |
|---------|------------------|----------|
| 0x1000  | bootloader.bin   | não      |
| 0x8000  | partitions.bin   | não      |
| 0xe000  | boot_app0.bin    | otadata  |
| 0x10000 | firmware.ino.bin | app0     |

O comando usado é como este:

    `esptool --chip esp32 merge-bin -o firmware.ino.merged.bin --pad-to-size 4MB --flash-mode keep --flash-freq keep --flash-size keep 0x1000 firmware.ino.bootloader.bin 0x8000 firmware.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.ino.bin`

Ao fazer a primeira gravação de firmware, o arquivo merged define estas partições ao ser escrito no offset 0x0. Em otadata o seletor de boot diz para o bootloader executar o que está em app0.

O upgrade é feito escrevendo um novo app de usuário na app1. Em otadata é marcado para o bootloader executar o que está em app1 e não mais app0. Quando o reboot acontece, o app1 é executado mas fica marcado como "pendente de verificação". Se um novo reboot acontece, o bootloader volta a executar o app0. Ele somente ira manter a execução do que está em app1 se este for marcado como válido durante sua execução.

Depois de um upgrade temos numa partição o app antigo, marcado como válido, e na outra partição o app recém atualizado. Este recém atualizado está no estado "pending verify". Se houver reboot neste momento, acontece o rollback. Se for iniciado novo upgrade, as duas partições vão ficar no estado "pending verify", e no próximo reboot nenhuma seria selecionada para rodar, resultando num sistema não bootavel.


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


# Requisitos Funcionais Consolidados (RF)

- **RF-01** O sistema deve permitir iniciar atualização de firmware via comando MQTT `firmware_update <url>` (e futuramente via outros canais).
- **RF-02** O download deve validar integridade via arquivo de hash (`.md5` ou `.sha256`) antes de finalizar a escrita na partição.
- **RF-03** Após reboot da nova imagem, o firmware deve permanecer em estado “pending validation” (detectado via `esp_ota_get_state_partition` / `ESP_OTA_IMG_PENDING_VERIFY`) até receber `firmware_mark_ok`.
- **RF-04** Se não receber `markOk` dentro de um timeout configurável (ex.: 5–15 min), deve fazer rollback automático para a imagem anterior e reiniciar.
- **RF-05** Deve ser possível forçar rollback imediato com `firmware_mark_invalid_reboot`.
- **RF-06** Deve ser possível cancelar um download em andamento (`firmware_update_stop` / `stopUpdate` / `abort`).
- **RF-07** Não deve iniciar novo update se a imagem atualmente em execução ainda estiver em estado pending (não marcada como válida) — estado obtido das APIs `esp_ota_*`.
- **RF-08** Deve publicar status no tópico de eventos MQTT: `downloading`, `verifying`, `pending_ok`, `success`, `failed`, `rollback`.
- **RF-09** Deve expor versão/firmware-id legível no boot (Serial + evento MQTT).
- **RF-10** O OTA deve funcionar independentemente do meio de rede (abstração `NetworkManager` / interface de conectividade).
- **RF-11** Deve ser possível monitorar o bom funcionamento do sistema através de ferramentas padronizadas.

# Requisitos Não-Funcionais Consolidados (RNF)

- **RNF-01** (Robustez) Nenhuma falha de rede, energia ou watchdog deve deixar o dispositivo em estado “brickado” (sempre deve existir uma imagem bootável válida).
- **RNF-02** (Tempo real / cooperativo) O download não pode bloquear o loop principal por mais de alguns milissegundos por chamada de `update()`.
- **RNF-03** (Watchdog) Deve existir um WatchDog que reinicia o sistema em caso de:
  - timeout de validação (pending sem markOk),
  - perda prolongada de conectividade crítica,
  - travamento detectado.
- **RNF-04** (Observabilidade) Logger configurável (níveis + saída Serial e/ou HTTP/MQTT).
- **RNF-05** (Testabilidade) Classes críticas (OtaUpdater, NetworkManager, MqttClient, HttpClientLocal, CheckSum, RetryLogic) devem permitir 100% de cobertura com testes unitários (injeção de dependências / interfaces).
- **RNF-06** (Hardware) Testes de OTA devem ser executados em hardware igual ou superior ao de implantação.
- **RNF-07** (Qualidade de código) Clean Code, responsabilidade única, baixo acoplamento.
