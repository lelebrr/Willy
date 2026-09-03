#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

// =============================================================================
// Willy ESP32-S3-WROOM-1-N8R2 — Definição Completa de Pinos (13 Módulos)
// =============================================================================

// --- LED RGB Built-in (NeoPixel WS2812) ---
#ifndef PIN_RGB_LED
#define PIN_RGB_LED 48
#endif
static const uint8_t LED_BUILTIN = SOC_GPIO_PIN_COUNT + PIN_RGB_LED;
#define BUILTIN_LED LED_BUILTIN
#define LED_BUILTIN LED_BUILTIN
#define RGB_BUILTIN    LED_BUILTIN
#ifndef RGB_BRIGHTNESS
#define RGB_BRIGHTNESS 64
#endif

// --- USB Serial (CH343P / Native USB) ---
static const uint8_t TX = 43;
static const uint8_t RX = 44;

#define SERIAL_RX RX
#define SERIAL_TX TX
#define BAD_RX SERIAL_RX
#define BAD_TX SERIAL_TX
#define USB_as_HID 1

// --- Barramento I2C (Compartilhado: PN532, DS3231, PAJ7620) ---
static const uint8_t SDA = 8;
static const uint8_t SCL = 17;

// --- Barramento SPI (Compartilhado: TFT, Touch, SD, NRF×2, CC1101) ---
static const uint8_t SS   = 10;  // TFT CS (default SS)
static const uint8_t MOSI = 11;
static const uint8_t SCK  = 12;
static const uint8_t MISO = 13;

// --- Display TFT ILI9341 ---
#ifndef TFT_CS
#define TFT_CS   10
#endif
#ifndef TFT_DC
#define TFT_DC    9
#endif
#ifndef TFT_RST
#define TFT_RST  14
#endif
#ifndef TFT_BL
#define TFT_BL   3
#endif

// --- Touch XPT2046 (SPI compartilhado) ---
#ifndef TOUCH_CS
#define TOUCH_CS 15
#endif

// --- Cartão Micro SD ---
#ifndef SDCARD_CS
#define SDCARD_CS   38
#endif
#ifndef SDCARD_SCK
#define SDCARD_SCK  12
#endif
#ifndef SDCARD_MOSI
#define SDCARD_MOSI 11
#endif
#ifndef SDCARD_MISO
#define SDCARD_MISO 13
#endif

// --- NRF24L01+PA+LNA #1 ---
#ifndef NRF24_CS_PIN
#define NRF24_CS_PIN  16
#endif
#ifndef NRF24_CE_PIN
#define NRF24_CE_PIN  21
#endif
#ifndef NRF24_SS_PIN
#define NRF24_SS_PIN  NRF24_CS_PIN
#endif

// --- NRF24L01+PA+LNA #2 ---
#ifndef NRF24_CS2_PIN
#define NRF24_CS2_PIN 35
#endif
#ifndef NRF24_CE2_PIN
#define NRF24_CE2_PIN 36
#endif

// --- CC1101 Sub-GHz ---
#ifndef CC1101_CS_PIN
#define CC1101_CS_PIN 18
#endif
#ifndef CC1101_GDO0
#define CC1101_GDO0   33
#endif
#ifndef CC1101_GDO2
#define CC1101_GDO2   34
#endif

// --- GPS NEO-6M (UART1) ---
#ifndef GPS_TX_PIN
#define GPS_TX_PIN 40   // ESP TX → GPS RX
#endif
#ifndef GPS_RX_PIN
#define GPS_RX_PIN 39   // ESP RX ← GPS TX
#endif
#ifndef GPS_BAUD
#define GPS_BAUD   9600
#endif

// --- YS-IRTM IR Serial (UART2) ---
#ifndef IR_SERIAL_TX_PIN
#define IR_SERIAL_TX_PIN 1    // ESP TX → YS-IRTM RXD
#endif
#ifndef IR_SERIAL_RX_PIN
#define IR_SERIAL_RX_PIN 47   // ESP RX ← YS-IRTM TXD
#endif
#ifndef IR_SERIAL_BAUD
#define IR_SERIAL_BAUD   9600
#endif

// --- INMP441 Microfone I2S ---
#ifndef I2S_SCK_PIN
#define I2S_SCK_PIN  41   // BCLK
#endif
#ifndef I2S_WS_PIN
#define I2S_WS_PIN   42   // LRCK / Word Select
#endif
#ifndef I2S_SD_PIN
#define I2S_SD_PIN    2   // Data In (do microfone)
#endif

// --- Joystick KY-023 (Analógico + Digital) ---
#ifndef JOY_X_PIN
#define JOY_X_PIN  4    // ADC1_CH3
#endif
#ifndef JOY_Y_PIN
#define JOY_Y_PIN  5    // ADC1_CH4
#endif
#ifndef JOY_BTN_PIN
#define JOY_BTN_PIN 6   // Digital, INPUT_PULLUP
#endif

// --- Grove / I2C aliases (para compatibilidade com o firmware Bruce) ---
#ifndef GROVE_SDA
#define GROVE_SDA SDA
#endif
#ifndef GROVE_SCL
#define GROVE_SCL SCL
#endif

#endif /* Pins_Arduino_h */
