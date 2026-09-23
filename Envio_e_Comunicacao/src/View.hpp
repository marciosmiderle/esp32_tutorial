#pragma once

#include "Console.hpp"

template <class model_type> class View {
  bool valid = true;

public:
  model_type *model;
  Console &console;

  View(model_type *m, Console &c) : model(m), console(c) {}

  bool isValid() { return valid; }
  void invalidate() { valid = false; }
  void setValid() { valid = true; }
  virtual void render() = 0;
};
