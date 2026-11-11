#include "ys_irtm.h"
#include "globals.h"

YS_IRTM::YS_IRTM() {
    _serial = new HardwareSerial(1); // Shared with GPS on Port 1 (GPIO 1/3) but using UART1
    _initialized = false;
}

void YS_IRTM::begin() {
    if (!_initialized) {
        _serial->begin(9600, SERIAL_8N1, willyConfigPins.gps_bus.rx, willyConfigPins.gps_bus.tx);
        _initialized = true;
    }
}

void YS_IRTM::end() {
    if (_initialized) {
        _serial->end();
        _initialized = false;
    }
}

void YS_IRTM::sendNEC(uint16_t address, uint8_t command) {
    if (!_initialized) begin();
    uint8_t addr_h = (address >> 8) & 0xFF;
    uint8_t addr_l = address & 0xFF;

    uint8_t packet[5] = {0xA1, 0xF1, addr_h, addr_l, command};

    _serial->write(packet, 5);
    _serial->flush();

    Serial.printf("[YS-IRTM] Sent NEC: Addr=0x%04X, Cmd=0x%02X\n", address, command);
}

bool YS_IRTM::receiveNEC(uint16_t *address, uint8_t *command) {
    if (!_initialized) begin();
    if (_serial->available() < 3) return false;

    // YS-IRTM receive format: FA Addr_H Addr_L Cmd (usually 3 or 4 bytes depends on version)
    // Common version sends 3 bytes: Addr_H, Addr_L, Cmd when a signal is received if FA is not used as header
    // Let's check for 0xFA header or just capture 3 bytes if no header.
    // Based on manual: 3 bytes (AddrH, AddrL, Cmd)

    if (_serial->peek() == 0xFA) {
        if (_serial->available() < 4) return false;
        _serial->read(); // discard header
    }

    if (_serial->available() >= 3) {
        uint8_t h = _serial->read();
        uint8_t l = _serial->read();
        uint8_t c = _serial->read();

        *address = (h << 8) | l;
        *command = c;
        return true;
    }

    return false;
}

YS_IRTM ysIrtm;
