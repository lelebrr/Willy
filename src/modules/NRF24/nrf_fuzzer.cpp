#include <Arduino.h>
#include "nrf_fuzzer.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_smart_fuzzer() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("NRF Smart Fuzzer");
    tft.setTextSize(FP);
    tft.setCursor(10, 60);
    tft.println("Flooding random packets...");

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.setPALevel(RF24_PA_MAX);
        NRFradio.setDataRate(RF24_2MBPS);
    }

    uint32_t packets_sent = 0;
    uint32_t last_update = millis();
    uint32_t start_time = millis();

    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            uint8_t random_ch = random(0, 125);
            uint8_t payload_len = random(1, 33);
            uint8_t payload[32];
            for (int i = 0; i < payload_len; i++) {
                payload[i] = random(0, 256);
            }

            uint8_t mac_len = random(3, 6);
            uint8_t rand_mac[5];
            for (int i=0; i<mac_len; i++) rand_mac[i] = random(0, 256);

            NRFradio.setChannel(random_ch);
            NRFradio.setAddressWidth(mac_len);
            NRFradio.openWritingPipe(rand_mac);
            NRFradio.write(payload, payload_len);
            packets_sent++;
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("FUZZ_" + String(random(0,125)));
        }

        if (millis() - last_update > 200) {
            tft.fillRect(10, 80, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, 80);
            tft.printf("Sent: %d", packets_sent);
            last_update = millis();
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
        NRFSerial.println("OFF");
    }
}
