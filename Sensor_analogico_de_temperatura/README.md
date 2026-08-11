# Tarefa

Ao apertar o botão uma leitura do sensor de temperatura é efetuada.
O valor cru é mostrado no monitor junto a uma estimativa linear da tensão.
O valor da temperatura é calculado conforme a documentação no sensor. Uma correção foi aplicada à formula já que a resolução das leituras analógicas deste ESP32 é de 12 bits, o exemplo é dado é de 10 bits.
Para testar diversas temperaturas é preciso alterá-la no menu do sensor durante a execução no emulador.

# Escolha dos pinos

Foi escolhido o pino 14 de saída por não estar na faixa 6-11 (connected to the integrated SPI flash) nem na faixa 34-39 (só pionos de input)
Foi escolhido o pino 4 de entrada por não estar na faixa 6-11.
