#include <Arduino.h>
#include "nrf_geolocator.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_signal_geolocator() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Signal Geo-Locator");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.startListening();
    }

    tft.setCursor(10, 60);
    tft.println("Scanning strength...");

    int current_channel = 2;
    int max_signal = 0;
    int best_channel = 2;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);
            delayMicroseconds(130);

            bool carrier = NRFradio.testCarrier();
            bool rpd = NRFradio.testRPD(); // Received Power Detector (> -64dBm)

            int strength_score = 0;
            if (rpd) strength_score = 100;
            else if (carrier) strength_score = 50;

            if (strength_score > max_signal) {
                max_signal = strength_score;
                best_channel = current_channel;
            }

            tft.fillRect(10, 90, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, 90);
            tft.printf("CH: %d | RPD: %s", current_channel, rpd ? "HIGH" : "LOW");

            tft.fillRect(10, 120, tftWidth - 20, 40, bruceConfig.bgColor);
            tft.setCursor(10, 120);
            tft.printf("Best CH: %d\nPeak > -64dBm: %s", best_channel, max_signal == 100 ? "YES" : "NO");

            // Simple drawing loop
            int bar_w = (strength_score * (tftWidth - 20)) / 100;
            tft.fillRect(10, 160, bar_w, 20, TFT_GREEN);
            tft.fillRect(10 + bar_w, 160, (tftWidth - 20) - bar_w, 20, bruceConfig.bgColor);
            tft.drawRect(10, 160, tftWidth - 20, 20, TFT_WHITE);

            current_channel = (current_channel + 1) % 125;

            // decay peak over time to allow continuous tracking
            if (random(0, 50) == 0) max_signal = 0;
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("GEOLOC_" + String(current_channel));
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_SPI(mode)) NRFradio.stopListening();
    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
