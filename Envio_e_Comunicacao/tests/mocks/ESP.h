#pragma once

#include <cstdint>

class EspClass {
public:
    uint64_t getEfuseMac() { return 0x112233445566ULL; }
    void restart() {}
};

extern EspClass ESP;
