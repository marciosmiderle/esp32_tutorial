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

Foi escolhido o pino 14 de saída por não estar na faixa 6-11 (connected to the integrated SPI flash) nem na faixa 34-39 (só pionos de input)
Foi escolhido o pino 4 de entrada por não estar na faixa 6-11.
