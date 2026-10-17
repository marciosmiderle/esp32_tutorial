#include <NetworkClient.h>

/*
falso positivo vindo do Arduino-Emulator, em Ethernet.h.

O construtor de EthernetClient faz duas coisas que ficam vivas depois do teste:

cpp
static std::vector<EthernetClient*>& active_clients() {
  static std::vector<EthernetClient*> clients;   // static
  ...
}
EthernetClient() {
  ...
  registerCleanup();                    // registra handlers SIGINT/SIGTERM (static)
  active_clients().push_back(this);     // e nunca remove no destrutor
}

Cada teste que faz push_back realoca. O CppUTest vê o buffer novo como alocado dentro do teste e ainda vivo no fim. Com o warmup tudo é alocado antes do CppUTest rastrear.
*/

static bool warmUpEmulatorStatics() {
  std::vector<std::unique_ptr<NetworkClient>> clients;
  for (int i = 0; i < 33; i++) {
    clients.push_back(std::make_unique<NetworkClient>());
  }
  return true;
}

const bool noArduinoEmulatorLeaks = warmUpEmulatorStatics();
