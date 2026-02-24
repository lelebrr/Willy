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

    const char *secret_data = "PMKID_OR_HANDSHAKE_DATA_MOCK";
    int data_len = strlen(secret_data);
    int sent_bytes = 0;

    uint32_t start_time = millis();
    while (!check(EscPress) && (millis() - start_time < 60000)) {
        if (CHECK_NRF_SPI(mode)) {
            uint8_t payload[32];
            memset(payload, 0, 32);
            payload[0] = 0x58; // Magic header
            strncpy((char*)&payload[1], secret_data, 30);

            NRFradio.write(payload, 32);
            sent_bytes += 30;

            tft.fillRect(10, 110, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, 110);
            tft.printf("Exfil bytes: %d", sent_bytes);

            delay(500); // 1 packet per half second (stealth)
        }

        if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) {
             NRFSerial.println("EXFIL_SYNC");
        }
    }

    if (CHECK_NRF_UART(mode) || CHECK_NRF_BOTH(mode)) NRFSerial.println("OFF");
}
