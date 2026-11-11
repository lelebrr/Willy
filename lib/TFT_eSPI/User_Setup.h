#ifndef USER_SETUP_LOADED // Skip entire file when settings come from
                          // platformio.ini
//                            CONFIGURAÇÕES DEFINIDAS PELO USUÁRIO
//   Define o tipo de driver, fontes a serem carregadas, pinos usados e método
//   de controle SPI etc
//
//   Veja o arquivo User_Setup_Select.h se você desejar ser capaz de definir
//   múltiplas configurações e então selecionar facilmente qual arquivo de
//   configuração é usado pelo compilador.
//
//   Se este arquivo for editado corretamente, então todos os exemplos da
//   biblioteca devem rodar sem a necessidade de fazer mais alterações para uma
//   configuração de hardware específica! Note que alguns esboços são projetados
//   para uma largura/altura de pixel TFT específica

// ##################################################################################
//
// Seção 1. Chame o arquivo de driver correto e quaisquer opções para ele
//
// ##################################################################################

// IMPORTANTE: As definições de driver e pinos são controladas via
// platformio.ini build_flags. Este arquivo serve apenas como fallback quando as
// flags não estão definidas. NÃO defina drivers aqui para evitar conflitos.

// Seção 2. Define the pins that are used to interface with the display here
// As pinagens são definidas em platformio.ini:
// - TFT_CS=10, TFT_DC=9, TFT_RST=14
// - TFT_MOSI=11, TFT_MISO=13, TFT_SCLK=12
// - TOUCH_CS=15
// - USE_HSPI_PORT=1

// Seção 3. Define fonts
// Comente as fontes que não precisa para economizar espaço
#define LOAD_GLCD  // Font 1. Original Adafruit 8 pixel font
#define LOAD_FONT2 // Font 2. Small 16 pixel high font
#define LOAD_FONT4 // Font 4. Medium 26 pixel high font
#define SMOOTH_FONT 1

// Seção 4. Other options

// SPI clock frequency - platformio.ini define 20MHz
#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 20000000
#endif

#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 10000000
#endif

// Touch frequency - XPT2046 requires lower clock
#ifndef SPI_TOUCH_FREQUENCY
#define SPI_TOUCH_FREQUENCY 1000000
#endif

// Use HSPI port for TFT (required on ESP32-S3 with multiple SPI devices)
#ifndef USE_HSPI_PORT
#define USE_HSPI_PORT
#endif

// Color order - BGR for ILI9341 (typical)
#ifndef TFT_RGB_ORDER
#define TFT_RGB_ORDER TFT_BGR
#endif

// Backlight control - platformio.ini define TFT_BL=3, BACKLIGHT_ON=HIGH
#ifndef TFT_BACKLIGHT_ON
#define TFT_BACKLIGHT_ON HIGH
#endif

#endif // USER_SETUP_LOADED
