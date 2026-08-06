# Saída serial

Presumo que configurar a saída padrão com ´Serial.begin(115200);´ seja uma saída serial válida. O rx do monitor está vinculado ao tx do pino 1, funcionando a velocidade ajustada aqui.

# Tarefa

O LED no pino 12 é controlado por um botão no pino 4. Ao apertar o botão o LED troca do estado desligado para ligado, e assim permance até que um novo toque no botão o faça retornar para o estado desligado. Foi acrescendado um delay de 200ms a cada mudança de estado como uma forma de histerese.

# Escolha dos pinos

Foi escolhido o pino 12 de saída por não estar na faixa 6-11 (connected to the integrated SPI flash) nem na faixa 34-39 (só pionos de input)
Foi escolhido o pino 4 de entrada por não estar na faixa 6-11.
