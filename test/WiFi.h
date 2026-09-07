#pragma once
#include "Arduino.h"
#include <cstdint>

struct IPAddress {
    uint8_t bytes[4];
    IPAddress() { bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0; }
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        bytes[0] = a; bytes[1] = b; bytes[2] = c; bytes[3] = d;
    }
    uint8_t operator[](int i) const { return bytes[i]; }
};

class WiFiClient {
public:
    bool connect(const char* host, uint16_t port);
    void stop();
};
