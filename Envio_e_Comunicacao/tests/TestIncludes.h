#pragma once

// Injeção Global de Mocks e Compatibilidade

#define TEST_BUILD

// handle name conflits
#define Logger EmulatorLogger
#define main main_arduino_emulator

#ifndef ARDUINO_ISR_ATTR
#define ARDUINO_ISR_ATTR
#endif

#include <Arduino.h>
#include "mocks/ESP.h"
#include "mocks/ArduinoJsonCompat.h"

#undef Logger
#undef main

#include "mocks/PrintPolyfill.hpp"
#define Print PrintPolyfill
