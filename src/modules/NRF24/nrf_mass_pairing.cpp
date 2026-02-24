#include <Arduino.h>
#include "nrf_mass_pairing.h"
#include "core/display.h"
#include "core/mykeyboard.h"

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

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(true);
        NRFradio.setPALevel(RF24_PA_MAX);
    }

    tft.setCursor(10, 60);
    tft.println("Flooding pairing requests...");

    uint32_t attempts = 0;
    int current_channel = 2;

    while (!check(EscPress)) {
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);

            uint8_t random_mac[5];
            for (int i=0; i<5; i++) random_mac[i] = random(0, 256);

            NRFradio.openWritingPipe(random_mac);

            uint8_t pair_payload[32] = {0x0F, 0x0F, 0x0F, 0x0F}; // Standard mock pairing req
            NRFradio.write(pair_payload, random(4, 16));

            attempts++;
            current_channel = (current_channel + 2) % 80;

            if (attempts % 150 == 0) {
                tft.fillRect(10, 90, tftWidth - 20, 20, bruceConfig.bgColor);
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
