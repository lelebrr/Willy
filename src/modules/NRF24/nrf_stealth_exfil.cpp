#include <Arduino.h>
#include <string.h>
#include "nrf_stealth_exfil.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "nrf_common.h"

using namespace nrf_common;

void nrf_stealth_exfil() {
    NRF24_MODE mode = nrf_setMode();
    if (!nrf_start(mode)) {
        displayError("NRF24 not found");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        return;
    }

    drawMainBorder();
    tft.setCursor(10, 35);
    tft.setTextSize(FM);
    tft.println("NRF Stealth Exfil");
    tft.setTextSize(FP);

    displayWarning("DEMO EDUCACIONAL");
    delay(1500);

    if (CHECK_NRF_SPI(mode)) {
        NRFradio.setAutoAck(false);
        NRFradio.setPALevel(RF24_PA_MIN); // Stealthy
        uint8_t mac[5] = {0xDE, 0xAD, 0xBE, 0xEF, 0x11};
        NRFradio.openWritingPipe(mac);
        NRFradio.setChannel(120); // uncommon high channel
    }

    tft.setCursor(10, 60);
    tft.println("Exfiltrating internal data");
    tft.setCursor(10, 80);
    tft.println("Channel: 120 | Power: MIN");

    String exfilData = "SSID:" + willyConfig.wifiAp.ssid + "|PWD:" + willyConfig.wifiAp.pwd + "|MAC:" + willyConfig.wifiMAC;
    int total_len = exfilData.length();
    int sent_bytes = 0;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            if (sent_bytes >= total_len) {
                // Done!
                tft.fillRect(10, 110, tftWidth - 20, 20, willyConfig.bgColor);
                tft.setCursor(10, 110);
                tft.println("Exfiltration Complete!");
                delay(2000);
                break;
            }

            uint8_t payload[32];
            memset(payload, 0, 32);
            payload[0] = 0x58; // Magic header
            payload[1] = sent_bytes; // Offset marker

            int chunk_size = 28; // up to 28 bytes per packet
            if (sent_bytes + chunk_size > total_len) {
                chunk_size = total_len - sent_bytes;
            }

            strncpy((char*)&payload[2], exfilData.c_str() + sent_bytes, chunk_size);

            NRFradio.write(payload, 32);
            sent_bytes += chunk_size;

            tft.fillRect(10, 110, tftWidth - 20, 20, willyConfig.bgColor);
            tft.setCursor(10, 110);
            tft.printf("Exfil bytes: %d/%d", sent_bytes, total_len);

            delay(200); // 5 packets per second
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("EXFIL_SYNC");
        }
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
