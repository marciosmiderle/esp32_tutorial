#pragma once

#include <future>

// Injeção Global de Mocks e Compatibilidade

#define TEST_BUILD
#define HTTPCLIENT_NOSECURE

// handle name conflits
#define Logger EmulatorLogger
#define main main_arduino_emulator

#ifndef ARDUINO_ISR_ATTR
#define ARDUINO_ISR_ATTR
#endif

#include <Arduino.h>
#include "mocks/ESP.h"
#include "mocks/ArduinoJsonCompat.h"
#include "mocks/millis.h"

#undef Logger
#undef main

#include "mocks/PrintPolyfill.hpp"
#define Print PrintPolyfill
#include "mocks/StringPolyfill.hpp"

#include "os_posix_compat.hpp"
