// User_Setup.h - Willy ESP S3 + ILI9341 + XPT2046
// Fallback header: defines are also set in platformio.ini build_flags.
// Using #ifndef guards to avoid redefinition warnings.

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED
#endif

#ifndef ILI9341_DRIVER
#define ILI9341_DRIVER
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH 240
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 320
#endif

#ifndef TFT_CS
#define TFT_CS 10
#endif
#ifndef TFT_DC
#define TFT_DC 9
#endif
#ifndef TFT_RST
#define TFT_RST 14
#endif
#ifndef TFT_MOSI
#define TFT_MOSI 11
#endif
#ifndef TFT_SCLK
#define TFT_SCLK 12
#endif
#ifndef TFT_MISO
#define TFT_MISO 13
#endif
#ifndef TFT_BL
#define TFT_BL 3
#endif

// USE_HSPI_PORT: TFT_eSPI cria seu proprio SPIClass(HSPI) em vez de
// compartilhar o global SPI. Obrigatório no ESP32-S3 quando há outros
// dispositivos SPI (CC1101, NRF24, SD).
#ifndef USE_HSPI_PORT
#define USE_HSPI_PORT
#endif

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 20000000
#endif
#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 10000000
#endif

// Touch XPT2046
#ifndef TOUCH_CS
#define TOUCH_CS 15
#endif
#ifndef SPI_TOUCH_FREQUENCY
#define SPI_TOUCH_FREQUENCY 2500000
#endif
#ifndef TOUCH_IRQ
#define TOUCH_IRQ -1
#endif

#ifndef LOAD_GLCD
#define LOAD_GLCD
#endif
#ifndef LOAD_FONT2
#define LOAD_FONT2
#endif
#ifndef LOAD_FONT4
#define LOAD_FONT4
#endif
#ifndef SMOOTH_FONT
#define SMOOTH_FONT
#endif

#ifndef TFT_RGB_ORDER
#define TFT_RGB_ORDER TFT_BGR
#endif
