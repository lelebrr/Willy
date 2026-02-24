#include <Arduino.h>
#include "nrf_impersonation.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_device_impersonation() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Impersonation MITM");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(true);
        NRFradio.enableDynamicPayloads();
        // Target MAC
        uint8_t mac[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        NRFradio.openReadingPipe(1, mac);
        NRFradio.openWritingPipe(mac);
        NRFradio.startListening();
    }

    tft.setCursor(10, 60);
    tft.println("Spoofing: AA:BB:CC:DD:EE");
    tft.setCursor(10, 80);
    tft.println("Waiting for traffic...");

    int spoof_count = 0;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            if (NRFradio.available()) {
                uint8_t dummy[32];
                NRFradio.read(dummy, NRFradio.getPayloadSize());

                // Switch to TX to send spoofed response
                NRFradio.stopListening();
                uint8_t response[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // Example MITM payload injection
                NRFradio.write(response, 4);
                NRFradio.startListening();

                spoof_count++;
                tft.fillRect(10, 100, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 100);
                tft.printf("Spoofed packets: %d", spoof_count);
            }
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("SPOOF_OOK");
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_SPI(mode)) NRFradio.stopListening();
    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
