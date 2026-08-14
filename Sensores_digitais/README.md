# Tarefa

Ao apertar o botão é iniciado um processo de leitura do sensor de temperatura com amostragem.
Após o término do processo um conjunto de valores crus são filtrados e é calculado o valor final considerando o valor anterior. É mostrado o valor final no monitor junto a uma estimativa linear da tensão.
O valor da temperatura é calculado conforme a documentação no sensor. Uma correção foi aplicada à formula já que a resolução das leituras analógicas deste ESP32 é de 12 bits, o exemplo é dado é de 10 bits.
Para testar diversas temperaturas é preciso alterá-la no menu do sensor durante a execução no emulador.

Internamente:
1. Lê 32 amostras do ADC
2. Ordena elas
3. Joga fora os 25% menores e 25% maiores  - remove outliers
4. Faz a média do que sobrou               - valor mais estável daquele instante
5. Passa esse valor pelo filtro EMA        - suaviza ao longo do tempo

O filtro EMA (Exponential Moving Average): 

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

# Tratamento de falhas demonstrado.

Feita apenas para o DHT. Temos um timeout e retry para uma amostragem isolada, e outro timeout para o conjunto de amostras em falha. O comando df no console injeta este segundo tipo de falha. Um comando d neste estado faz o console mostrar que o DHT está em falha. Após o readSampleInFailWaitTimeForRetry o comando d volta a responder normalmente.

# Como rodar

Um arquivo para rodar no simulador wokwi via comando wokwi-cli está na raiz do projeto. É preciso gerar um token de API conforme a documentação. Antes é preciso compilar usando o aquivo compile.bat, que usa o arduino-cli.
