#pragma once
#include <cstdint>

class IPAddress {
public:
    uint8_t bytes[4];
    IPAddress() { bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0; }
    IPAddress(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
        bytes[0] = b0; bytes[1] = b1; bytes[2] = b2; bytes[3] = b3;
    }
    uint8_t operator[](int index) const { return bytes[index]; }
};

class WiFiClient {
public:
    bool connect(const char* host, uint16_t port) { return true; }
    void stop() {}
};
