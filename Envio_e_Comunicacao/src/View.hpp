#pragma once

template <class model_type> class View {
  bool valid = true;

public:
  model_type *model;

  bool isValid() { return valid; }
  void invalidate() { valid = false; }
  void setValid() { valid = true; }
  virtual void render() = 0;
};
