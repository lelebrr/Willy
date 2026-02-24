#ifndef __RF_JAMMER_H__
#define __RF_JAMMER_H__

#include <Arduino.h>
#include <stdint.h>

class RFJammer {
public:
    RFJammer(uint8_t mode = 0); // 0: ITMT, 1: CONT, 2: HOPPER
    ~RFJammer();

    void setup();

private:
    int nTransmitterPin;
    bool sendRF = true;
    uint8_t jammerMode = 0;

    void display_banner();
    void run_full_jammer();
    void run_itmt_jammer();
    void run_hopper_jammer();
    void send_optimized_pulse(int width);
    void send_random_pattern(int numPulses);
};

#endif
