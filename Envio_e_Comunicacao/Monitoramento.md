# Monitoramento

Os dados podem ser servidos via MQTT, por publicação pelas estações nos tópicos abaixo.
O servidor se inscreve nos tópicos com `/envmon/v1/+/telemetry` e processa os dados recebidos.

```
envmon/v1/{device_id}/telemetry    // Dados (JSON ou txt)
envmon/v1/{device_id}/status       // Status (JSON ou txt)
envmon/v1/{device_id}/event        // Eventos (JSON ou txt opcional)
envmon/v1/{device_id}/cmd          // Comandos (texto puro)
envmon/v1/{device_id}/cmd/response // Resposta (texto puro)
```

O tópico Telemetria contém os dados dos sensores, enquanto o status contém dados de saúde do sistema.
Em cmd, e cmd/response a estação recebe e responde comandos enviados pelo software de backend.

Características desta estrutura de nomes:

- Versionamento (v1): permite migrar para v2 no futuro sem quebrar dispositivos antigos.
- Namespace (envmon): evita colisão com outros sistemas no mesmo broker.
- Separação de responsabilidades (telemetry / status / event / cmd).
- Backend se inscreve facilmente com wildcards.

No backend você se inscreve em:
- estacoes/+/telemetry: recebe de todos os dispositivos
- estacoes/+/status: monitora quem está online
- estacoes/+/events

A estação publica sempre no tópico com o próprio device id e se inscreve apenas no seu próprio tópico de comandos.

Formato e campos das mensagens pode ser texto ou json (a definir):

```text
Device: esp32-03
Firmware: 1.2.0
Uptime: 86400 s
Free heap: 185432 bytes
WiFi RSSI: -67 dBm
Battery: 3.87 V
Interval: 300 s
Temperature: 24.7 °C
Humidity: 63.2 %
```

``` json
{
	"Device": "esp32-03",
	"Firmware": "1.2.0",
    "Uptime": 86400,
    "Free heap": 185432,
    "WiFi RSSI": -67,
	"Battery": 3.87,
	"Interval": 300,
	"Temperature": 24.7,
	"Humidity": 63.2
}
```
