// Setup para ESP32-S3 N8R2 com ILI9341 + touch XPT2046 (Willy)
// Fallback para envs sem USER_SETUP_LOADED (ver User_Setup_Select.h).
// Valores espelham a configuracao N8R2 do projeto.
#define USER_SETUP_ID 210
#define ILI9341_DRIVER

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 14
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13
#define TFT_BL 3

#define USE_HSPI_PORT

#define TOUCH_CS 15
#define SPI_TOUCH_FREQUENCY 2500000

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY 27000000
#define SPI_READ_FREQUENCY 10000000

#define TFT_RGB_ORDER TFT_BGR
