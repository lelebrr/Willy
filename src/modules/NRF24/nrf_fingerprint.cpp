#include <Arduino.h>
#include "nrf_fingerprint.h"
#include "core/display.h"
#include "core/mykeyboard.h"

// Pseudo-promiscuous fingerprinting logic
void nrf_device_fingerprint() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("NRF Fingerprint");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.disableCRC();
        NRFradio.setAddressWidth(2); // Shortest possible address to catch more traffic
        uint8_t mac[2] = {0x00, 0xAA}; // Common preamble pattern
        NRFradio.openReadingPipe(1, mac);
        NRFradio.startListening();
    }

    int current_channel = 0;
    int devs_found = 0;
    uint32_t last_hop = millis();

    tft.setCursor(10, 60);
    tft.println("Listening for addresses...");

    while (!check(EscPress)) {
        if (CHECK_NRF_SPI(mode)) {
            if (millis() - last_hop > 100) {
                current_channel = (current_channel + 1) % 125;
                NRFradio.setChannel(current_channel);
                last_hop = millis();
            }

            if (NRFradio.available()) {
                uint8_t buf[32];
                NRFradio.read(buf, 32);
                devs_found++;

                tft.fillRect(10, 80, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 80);
                tft.printf("Found packets: %d", devs_found);
                tft.fillRect(10, 100, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 100);
                tft.printf("Last CH:%d Len:%d", current_channel, NRFradio.getPayloadSize());
            }
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("SCAN");
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_SPI(mode)) NRFradio.stopListening();
    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
