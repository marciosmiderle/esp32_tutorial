#pragma once

#include <Arduino.h>
#include "Command.hpp"
#include "View.hpp"
#include <map>
#include <functional>
#include <sstream>
#include <vector>

class Router {
private:
  using RouteCB = std::function<void(const Command&)>;
  std::map<String, RouteCB> rotas;
  Command currentCommand;

protected:
  Router();
  virtual ~Router();

public:
  static Router& getInstance();

  std::function<void(Command&)> pollInput = [](Command& cmdOut) {};

  void on(const String& commandName, RouteCB callback);
  void update();
  void processar();
};

#define RTR Router::getInstance()
