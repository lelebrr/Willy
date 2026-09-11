#include "nrf_common.h"
#include "../../core/mykeyboard.h"

namespace nrf_common {

RF24 NRFradio(wilyConfigPins.NRF24_bus.io0, wilyConfigPins.NRF24_bus.cs);
HardwareSerial NRFSerial = HardwareSerial(2); // Uses UART2 for External NRF's
SPIClass *NRFSPI;

void nrf_info() {
    tft.fillScreen(wilyConfig.bgColor);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("_Disclaimer_", tftWidth / 2, 10, 1);
    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.setTextSize(FP);
    tft.setCursor(15, 33);
    padprintln("These functions were made to be used in a controlled environment for STUDY only.");
    padprintln("");
    padprintln("DO NOT use these functions to harm people or companies, you can go to jail!");
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    padprintln("");
    padprintln(
        "This device is VERY sensible to noise, so long wires or passing near VCC line can make "
        "things go wrong."
    );
    delay(1000);
    while (!check(AnyKeyPress));
}

bool nrf_start(NRF24_MODE mode) {
    bool result = false;
    if (mode == NRF_MODE_DISABLED) return false;

    if (CHECK_NRF_UART(mode)) {
        if (USBserial.getSerialOutput() == &Serial1) {
            displayError("(E) UART already in use", true);
            return false;
        }
        NRFSerial.begin(115200, SERIAL_8N1, wilyConfigPins.uart_bus.rx, wilyConfigPins.uart_bus.tx);
        Serial.println("NRF24 on Serial Started");
        result = true;
    };

    if (!CHECK_NRF_SPI(mode)) return result;
    pinMode(wilyConfigPins.NRF24_bus.cs, OUTPUT);
    digitalWrite(wilyConfigPins.NRF24_bus.cs, HIGH);
    pinMode(wilyConfigPins.NRF24_bus.io0, OUTPUT);
    digitalWrite(wilyConfigPins.NRF24_bus.io0, LOW);

    if (wilyConfigPins.NRF24_bus.mosi == (gpio_num_t)TFT_MOSI &&
        wilyConfigPins.NRF24_bus.mosi != GPIO_NUM_NC) { // (T_EMBED), CORE2 and others
#if TFT_MOSI > 0 // condition for Headless and 8bit displays (no SPI bus)
        NRFSPI = &tft.getSPIinstance();
#else
        NRFSPI = &SPI;
#endif

    } else if (wilyConfigPins.NRF24_bus.mosi == wilyConfigPins.SDCARD_bus.mosi) {
        // CC1101 shares SPI with SDCard (Cardputer and CYDs)

        NRFSPI = &sdcardSPI;
    } else if (wilyConfigPins.NRF24_bus.mosi == wilyConfigPins.CC1101_bus.mosi &&
               wilyConfigPins.NRF24_bus.mosi != wilyConfigPins.SDCARD_bus.mosi) {
        // Smoochie board shares CC1101 and NRF24 SPI bus with different CS pins at
        // the same time, different from StickCs that uses the same Bus, but one at a
        // time (same CS Pin)
        NRFSPI = &CC_NRF_SPI;
    } else {
        NRFSPI = &SPI;
    }
    NRFSPI->begin(
        (int8_t)wilyConfigPins.NRF24_bus.sck,
        (int8_t)wilyConfigPins.NRF24_bus.miso,
        (int8_t)wilyConfigPins.NRF24_bus.mosi
    );
    delay(10);

    if (NRFradio.begin(
            NRFSPI,
            rf24_gpio_pin_t(wilyConfigPins.NRF24_bus.io0),
            rf24_gpio_pin_t(wilyConfigPins.NRF24_bus.cs)
        )) {
        result = true;
    } else {
        return false;
    }
    return result;
}

NRF24_MODE nrf_setMode() {
    NRF24_MODE mode = NRF_MODE_DISABLED;
    options = {
        {"SPI Mode",  [&]() { mode = NRF_MODE_SPI; } },
        {"SPI UART",  [&]() { mode = NRF_MODE_UART; }},
        {"SPI BOTH",  [&]() { mode = NRF_MODE_BOTH; }},
        {"Main Menu", [=]() { returnToMenu = true; } }
    };
    loopOptions(options);
    return mode;
}

} // namespace nrf_common
