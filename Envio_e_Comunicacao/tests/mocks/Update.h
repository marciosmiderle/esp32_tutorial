#pragma once

#include <Arduino.h>

#define U_FLASH 0

class UpdateClass {
public:
    bool begin(size_t size = 0, int command = U_FLASH) { return true; }
    size_t write(uint8_t *data, size_t len) { return len; }
    bool end(bool evenIfRemaining = false) { return true; }
    bool hasError() { return false; }
    const char* errorString() { return ""; }
    void abort() {}
    bool isRunning() { return false; }
    void printError(Print &out) {}
};

extern UpdateClass Update;
