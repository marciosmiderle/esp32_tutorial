#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>

// Especialização para permitir que ArduinoJson use a classe String do emulador
// mesmo que ela não tenha o método write() nativo.
namespace ArduinoJson {
    namespace V743HB42 { // Namespace interno da versão da lib
        namespace detail {
            template <>
            struct Writer<arduino::String, void> {
                arduino::String* _dest;
            public:
                Writer(arduino::String& dest) : _dest(&dest) {}
                size_t write(uint8_t c) {
                    (*_dest) += (char)c;
                    return 1;
                }
                size_t write(const uint8_t* s, size_t n) {
                    for (size_t i = 0; i < n; i++) {
                        (*_dest) += (char)s[i];
                    }
                    return n;
                }
            };
        }
    }
}
