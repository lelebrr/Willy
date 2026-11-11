#ifndef __YS_IRTM__
#define __YS_IRTM__

#include <Arduino.h>
#include "globals.h"

/**
 * @class YS_IRTM
 * @brief Driver for YS-IRTM IR Serial Transceiver module.
 *
 * This module communicates via UART (9600 baud) and handles NEC protocol.
 * Command packet format for sending: 0xA1, 0xF1, Addr_H, Addr_L, Cmd
 */
class YS_IRTM {
public:
    YS_IRTM();

    void begin();
    void end();

    /**
     * @brief Sends an IR NEC command via Serial.
     * @param address 16-bit address
     * @param command 8-bit command
     */
    void sendNEC(uint16_t address, uint8_t command);

    /**
     * @brief Checks for received IR data on Serial.
     * @param address Pointer to store 16-bit address
     * @param command Pointer to store 8-bit command
     * @return true if a command was received
     */
    bool receiveNEC(uint16_t *address, uint8_t *command);

private:
    HardwareSerial* _serial;
    bool _initialized;
};

extern YS_IRTM ysIrtm;

#endif
