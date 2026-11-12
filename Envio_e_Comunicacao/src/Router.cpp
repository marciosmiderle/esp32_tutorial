#include "Router.hpp"
#include "Logger.hpp"

Router::Router() {}

Router::~Router() {}

Router& Router::getInstance() {
  static Router instance;
  return instance;
}

void Router::on(const String& commandName, RouteCB callback) {
  rotas[commandName] = callback;
}

void Router::update() {
  pollInput(currentCommand);
  if (currentCommand.isEmpty()) return;

  processar();
}

void Router::processar() {
  auto it = rotas.find(currentCommand.name);
  if (it != rotas.end()) {
    it->second(currentCommand);
  } else {
    it = rotas.find("default");
    if (it != rotas.end()) {
      it->second(currentCommand);
    } else {
      Log.print("[RTR] Comando inexistente: '");
      Log.print(currentCommand.name);
      Log.println("'");
    }
  }
  currentCommand.clear();
}
