# Como rodar

Para compilar é preciso instalar:
- arduino-cli (ou o Arduino Studio) e adicionar os caminhos ao `PATH`
- cpputest

 
 cmake --build build --target debug run_tests

```bash
$ git clone <repo>
$ cd <repo>/Envio_e_Comunicacao
$ cmake -S . -B build
$ cmake --build build --target debug

$ tree -L 1 -P 'Envio*' build/debug
build/debug
├── core
├── Envio_e_Comunicacao.ino.bin
├── Envio_e_Comunicacao.ino.bin.md5
├── Envio_e_Comunicacao.ino.bootloader.bin
├── Envio_e_Comunicacao.ino.elf
├── Envio_e_Comunicacao.ino.map
├── Envio_e_Comunicacao.ino.merged.bin
├── Envio_e_Comunicacao.ino.partitions.bin

```

Rode o arquivo *.merged.bin usando o upload de firmware no emulador ou hardware.
Use os comandos MQTT descritos em <Comandos MQTT> e siga os exemplo. É preciso um cliente mqtt como o `mosquitto_pub`.

Alternativamente, um arquivo para rodar no simulador wokwi via comando wokwi-cli está na raiz do projeto. É preciso gerar um token de API conforme a documentação. Antes é preciso compilar usando o aquivo compile.bat, que usa o arduino-cli.

Para rodar os testes:

```bash
$ cmake --build build --target run_tests
```

# Envio por HTTP e Comunicação MQTT

## Arquitetura

O sistema envia dados de telemetria via HTTP POST e MQTT, utilizando a classe `Message`. Ambos os protocolos utilizam a mesma lógica de retry (`RetryLogic`) para lidar com falhas de comunicação.
No `loop()` são processados os inputs (pooling de eventos e entradas), são atualizados os modelos (é propagado o clock). Também é feita a saída de mensagens para o usuário (via console ou outra) e enviadas as mensagens de telemetria.

## Classe Message

A classe `Message` (arquivos `src/Message.hpp` e `src/Message.cpp`) encapsula todos os dados relevantes da estação meteorológica:

- **Telemetria básica**: temperatura, umidade, detecção de movimento, timestamp
- **Status dos sensores**: saúde e ratio de cada sensor (NTC, DHT, PIR)
- **Interpretações**: status térmico, de umidade, movimento, divergência, nível de atenção
- **Observações**: strings formatadas para display

O método `toJson()` serializa todos esses dados em JSON para envio tanto por HTTP quanto por MQTT.

## Envio por HTTP

### Configuração
- **API URL**: `http://httpbin.org/post` (API de teste que ecoa o payload)
- **Método**: POST com Content-Type: application/json
- **Formato**: JSON completo da mensagem

### Estratégia de Retry
- **Max retries**: 5 tentativas
- **Retry timeout**: 2000ms entre tentativas
- **TryLater timeout**: 30000ms após esgotar retries

Quando a API está indisponível:
1. O sistema tenta até 5 vezes com intervalo de 2s
2. Se todas falharem, entra em modo "tryLater" por 30s
3. Após 30s, reseta os counters e tenta novamente
4. A aquisição de dados continua normalmente mesmo durante falhas HTTP

## Comunicação MQTT

### Broker e Tópicos

| Item                         | Valor                                     |
|------------------------------|-------------------------------------------|
| **Broker**                   | `broker.hivemq.com`                       |
| **Porta**                    | 1883                                      |
| **Client ID**                | `estacao_meteorologica_001-<MAC ADDRESS>` |
| **Tópico de Telemetria**     | `estacao/telemetria`                      |
| **Tópico de Eventos**        | `estacao/eventos`                         |
| **Tópico de Comandos (sub)** | `estacao/comandos/entrada`                |
| **Tópico de Comandos (pub)** | `estacao/comandos/resposta`               |

Importante: os tópicos são prefixados com o ClientID para evitar colisões.

### Publicação de Telemetria
- Envia JSON completo da mensagem a cada 10 segundos
- Mesmo formato utilizado no HTTP
- Reconexão automática quando o broker fica indisponível

### Publicação de Eventos
Eventos relevantes são publicados automaticamente:
- `motion_start`: Quando movimento é detectado pelo PIR
- `motion_stop`: Quando movimento cessa

Payload de exemplo:
```json
{
  "type": "motion_start",
  "data": "Movimento detectado",
  "timestamp": 12345678
}
```

### Comandos Remotos

O sistema subscreve ao tópico `estacao/comandos/entrada` e processa os seguintes comandos:

| Comando                        | Ação                                                   |
|--------------------------------|--------------------------------------------------------|
| `led_on`                       | Aciona LED (simulado)                                  |
| `led_off`                      | Desliga LED (simulado)                                 |
| `read_now`                     | Mostra a interpretação ambiental da estação no console |
| `firmware_update <url>`        | Inicia OTA (ver seção OTA Update)                      |
| `firmware_update_stop`         | Cancela um update se iniciado                          |
| `firmware_mark_ok`             | Confirma firmware atual após o update                  |
| `firmware_mark_invalid_reboot` | Marca o update como inválido e reinicia                |

Para testar, publique no broker MQTT:
```bash
mosquitto_pub -h broker.hivemq.com -t "<ClientID>/estacao/comandos/entrada" -m "read_now"
```

### Recuperação de Conexão
- Tentativa de reconexão automática no `update()` do MqttClient
- Mesma estratégia de retry do HTTP

## Resiliência do Sistema

O sistema continua operando nas seguintes situações:

| Falha                    | Comportamento                                          |
|--------------------------|--------------------------------------------------------|
| Sensor inválido          | Outros sensores continuam lendo; erro é registrado     |
| Rede cai                 | Aquisição local continua; HTTP/MQTT aguardam reconexão |
| API HTTP indisponível    | Retry automático; MQTT continua funcionando            |
| Broker MQTT indisponível | Retry automático; HTTP continua funcionando            |

## Exemplo de Payload JSON

```json
{
  "timestamp": 12345678,
  "temperature": 25.3,
  "humidity": 60.2,
  "motionDetected": false,
  "sensors": {
    "ntc": {"health": "Good", "healthRatio": 1.0},
    "dht": {"health": "Good", "healthRatio": 1.0},
    "pir": {"health": "Good", "healthRatio": 1.0}
  },
  "interpretation": {
    "tempStatus": "Normal",
    "humidityStatus": "Normal",
    "motionStatus": "Parado",
    "sensorDivergence": false,
    "attentionLevel": 0,
    "attentionStatus": "Verde"
  },
  "observations": {
    "tempObs": "25.3C",
    "humidityObs": "60.2%",
    "motionObs": "Nenhum",
    "divergenceObs": "OK"
  }
}
```

# Conexão de Rede

A conexão de rede é independente do meio físico do link. Pode ser configurado um acesso via WiFi (mais de um ponto de acesso) e Celular.

Pode-se ver e modificar o estado da rede via comandos no console:
- ws inicia a conexão
- wf termina a conexão

Os pinos do ntc foram alterados por conflito com o WiFi.

| peça      | pino | novo pino |
|-----------|------|-----------|
| ntc       | 14   | 34        |
| botão ntc | 4    | 27        |

# Transformar as medições em informações úteis para monitoramento.

Os limites e regras são vagamente arbitrários, mas podem ser facilmente alterados.

# Regras

- Funções de interpretação que convertem valores numéricos em status descritivos
- Detecção de divergência comparando temperaturas do NTC e DHT (diferença > 5°C)
- Cálculo do nível de atenção baseado em múltiplos fatores com pesos diferentes. Consulte `Estacao::calculateAttentionLevel()`
- Temperatura crítica acima de 32°C ou abaixo de 10°C
- Umidade crítica se >= 80.0 ou < 30.0, atenção se >= 60.0 ou < 40.0
- Divergência entre sensores entra no cálculo de maneira crítica
- Movimento pode indicar intrusão, acresce o nível de atenção

# Cenários de teste.

Para testar basta mudar os valores dos sensores no emulador e digitar i <enter> no console: uma tela com todas as informações da estação será mostrada.
É possível rodar no console com o wokwi-cli e digitar i <enter> para mostrar o dashboard. Digitando o comando p injeta movimento no modelo do sensor de movimento, alterando o Nível Geral de Atenção.

# Tarefa

As amostragens são feitas continuamente. Após o término de um ciclo, um conjunto de valores crus são filtrados e é calculado o valor final considerando o valor anterior. É mostrado o valor final no monitor junto a uma estimativa linear da tensão.
O valor da temperatura é calculado conforme a documentação no sensor. Uma correção foi aplicada à formula já que a resolução das leituras analógicas deste ESP32 é de 12 bits, o exemplo é dado é de 10 bits.
Para testar diversas temperaturas é preciso alterá-la no menu do sensor durante a execução no emulador.

Internamente:
1. Lê 32 amostras do ADC
2. As ordena
3. Joga fora os 25% menores e 25% maiores  - remove outliers
4. Faz a média do que sobrou               - valor mais estável daquele instante
5. Passa esse valor pelo filtro EMA        - suaviza ao longo do tempo

O filtro EMA: Exponential Moving Average.

# Escolha dos pinos

Foi escolhido o pino 14 de saída por não estar na faixa 6-11 (connected to the integrated SPI flash) nem na faixa 34-39 (só pinos de input)
Foi escolhido o pino 4 de entrada por não estar na faixa 6-11.

Outros pinos:

| peça      | pino | i/o | dig/an |
|-----------|------|-----|--------|
| ntc       | 14   | i   | A      |
| botão ntc | 4    | i   | D      |
| dht       | 33   | i   | D      |
| botão dht | 16   | i   | D      |
| pir       | 17   | i   | D      |

# Explicação das frequências de leitura adotadas.

O DHT tem um intervalo de amostragem recomendado de 2000ms. Foi utilizado 500ms pois são feitas 4 amostragens, e do ponto de vista de demostração fica demorado usar aquele valor.
Da mesma forma para o NTC o valor de intervalo de amostragem é de 500ms. Como faço 32 amostragens optei por 5ms para deixar a demostração mais visível.

# Console

Temos comandos via serial para simular os cliques nos botões do simulador.

| comando | leitura do sensor  |
| n       | ntc                |
| d       | dht                |
| df      | dht : injeta falha |
| p       | pir                |

Na função `consoleInput()` outros comandos de injeção de falha podem ser consultados.

# Tratamento de falhas demonstrado.

Feita apenas para o DHT. Temos um timeout e retry para uma amostragem isolada, e outro timeout para o conjunto de amostras em falha. O comando df no console injeta este segundo tipo de falha. Um comando d neste estado faz o console mostrar que o DHT está em falha. Após o readSampleInFailWaitTimeForRetry o comando d volta a responder normalmente.

# OTA Update (feature/ota-update)

Atualização de firmware via MQTT + HTTP, com verificação de integridade.

## Comandos MQTT

Publique no tópico `<ClientID>/estacao/comandos/entrada` os comandos descritos anteriormente.

O comando `firmware_update <url_do_bin>` baixa o `.bin` e o hash (`.md5` ou `.sha256` de mesmo nome), grava na partição OTA e reinicia. Depois, manualmente, o comando `firmware_mark_ok` confirma o firmware atual como válido, e o `firmware_mark_invalid_reboot` marca o update como inválido fazendo o rollback e reinicia.


Exemplo:

```bash
# 1) Hospede num webserver os arquivos firmware.bin e firmware.md5 no mesmo path base
# 2) Dispare o update
mosquitto_pub -h broker.hivemq.com \
  -t "<ClientID>/estacao/comandos/entrada" \
  -m "firmware_update https://exemplo.com/firmware.bin"

# 3) Após o reboot bem-sucedido
mosquitto_pub -h broker.hivemq.com \
  -t "<ClientID>/estacao/comandos/entrada" \
  -m "firmware_mark_ok"

# 4) Também após o reboot bem-sucedido
mosquitto_pub -h broker.hivemq.com \
  -t "<ClientID>/estacao/comandos/entrada" \
  -m "firmware_mark_invalid_reboot"
```

## Arquivos de hash

O cliente tenta, nesta ordem:

1. `<url sem .bin>.md5`
2. `<url sem .bin>.sha256`

O conteúdo pode ser só o hex, ou no formato clássico `hash  nome_arquivo`.

## Observações

- É necessário partição OTA no esquema de flash (padrão Arduino-ESP32 com OTA).
- O download do bin é síncrono dentro de `update()`; firmwares grandes ocupam o loop por alguns segundos.
- Eventos de progresso/erro também vão para o tópico de eventos MQTT (`type: ota`).
