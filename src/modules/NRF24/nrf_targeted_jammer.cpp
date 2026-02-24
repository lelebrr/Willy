#include <Arduino.h>
#include "nrf_targeted_jammer.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_targeted_jammer() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Targeted Jammer");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.startListening();
    }

    int state = 0; // 0=scan, 1=jam
    int target_channel = -1;
    uint32_t timer = millis();
    int current_channel = 0;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            if (state == 0) {
                // Scan phase
                NRFradio.setChannel(current_channel);
                delayMicroseconds(130);
                if (NRFradio.testCarrier()) {
                    target_channel = current_channel;
                    state = 1; // move to jam
                    NRFradio.stopListening();
                    NRFradio.setPALevel(RF24_PA_MAX);
                    NRFradio.startConstCarrier(RF24_PA_MAX, target_channel);
                    timer = millis();

                    tft.fillRect(10, 60, tftWidth-20, 60, bruceConfig.bgColor);
                    tft.setCursor(10, 70);
                    tft.printf("Target found CH: %d", target_channel);
                    tft.setCursor(10, 90);
                    tft.println("JAMMING...");
                }
                current_channel = (current_channel + 1) % 125;
            } else if (state == 1) {
                // Jamming phase
                if (millis() - timer > 2000) { // jam for 2 seconds then rescan
                    state = 0;
                    NRFradio.stopConstCarrier();
                    NRFradio.startListening();
                    tft.fillRect(10, 60, tftWidth-20, 60, bruceConfig.bgColor);
                    tft.setCursor(10, 70);
                    tft.println("Scanning...");
                }
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.stopListening();
        NRFradio.stopConstCarrier();
    }
    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
