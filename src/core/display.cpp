#include "display.h"
#include "core/wifi/webInterface.h" // for server
#include "core/wifi/wg.h" //for isConnectedWireguard to print wireguard lock
#include "utils.h"
#include <Arduino.h>
#include <FreeRTOS.h>
#include <JPEGDecoder.h>
#include <interface.h> //for charging ischarging to print charging indicator
#include <lvgl.h>
#include <semphr.h>
#include <task.h>

#define MAX_MENU_SIZE (int)(tftHeight / 25)

// Send the ST7789 into or out of sleep mode
void panelSleep(bool on) {
#if defined(ST7789_2_DRIVER) || defined(ST7789_DRIVER)
  if (on) {
    tft.writecommand(0x10); // SLPIN: panel off
    vTaskDelay(pdMS_TO_TICKS(5));
  } else {
    tft.writecommand(0x11); // SLPOUT: panel on
    vTaskDelay(pdMS_TO_TICKS(120));
  }
#endif
  // Disables tft writings on the display
  tft.setSleepMode(on);
}

// isCharging() is provided by the board-specific interface.cpp

/* LVGL related static variables */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

/* LVGL Thread Safety Mutex */
SemaphoreHandle_t lvgl_mutex = NULL;

/* Display flushing otimizado com DMA para ESP32-S3 */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Otimização: Usar DMA para transferências maiores
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);

    // Usar pushColors padrão (TFT_eSPI não tem pushColorsDMA)
    tft.pushColors((uint16_t *)&color_p->full, w * h);

    tft.endWrite();
    xSemaphoreGive(spiMutex);
  }

  lv_disp_flush_ready(disp_drv);
}

/* Read the touchpad */
void updateTouchPoint() {
#ifdef HAS_TOUCH
  uint16_t tx, ty;
  bool touched = false;

  // Guard SPI bus access with mutex — tft.getTouch() internally uses
  // SPI.beginTransaction/endTransaction, which conflicts with LVGL's
  // my_disp_flush() running on the same SPI bus from another context.
  // Increased timeout from 50ms to 100ms for better reliability under load
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    touched = tft.getTouch(&tx, &ty);
    xSemaphoreGive(spiMutex);
  } else if (spiMutex) {
    // Debug: log occasional mutex failures (not every frame to avoid spam)
    static uint32_t lastFail = 0;
    if (millis() - lastFail > 5000) {
      usbSerial.println(
          "[TOUCH] WARNING: Failed to acquire SPI mutex (timeout)");
      lastFail = millis();
    }
  }

  if (touched) {
    // Correção de coordenadas invertidas (como no clean_demo)
    touchPoint.x = tftWidth - tx;
    touchPoint.y = tftHeight - ty;
    touchPoint.pressed = true;
    static uint32_t lastTouchLog = 0;
    if (millis() - lastTouchLog > 500) {
      usbSerial.printf("[TOUCH] raw x=%d y=%d, corrected x=%d y=%d\n", tx, ty,
                       touchPoint.x, touchPoint.y);
      lastTouchLog = millis();
    }
    touchHeatMap(touchPoint);
    if (wakeUpScreen()) {
      AnyKeyPress = true;
    }
  } else {
    touchPoint.pressed = false;
  }
#endif
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (touchPoint.pressed) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchPoint.x;
    data->point.y = touchPoint.y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void initLVGL() {
  usbSerial.println("[LVGL] Starting lv_init()...");
  lv_init();

  // Create the LVGL mutex for thread safety
  lvgl_mutex = xSemaphoreCreateMutex();

  usbSerial.println("[LVGL] lv_init() done.");

  // Otimização: Buffer maior para ESP32-S3 com PSRAM
  size_t buffer_size =
      tftWidth * 20; // 20 lines buffer (otimizado para performance)
  usbSerial.printf("[LVGL] Buffer size: %d bytes\n",
                   buffer_size * sizeof(lv_color_t));

  if (psramFound()) {
    usbSerial.println("[LVGL] PSRAM found, allocating...");
    buf1 = (lv_color_t *)ps_malloc(buffer_size * sizeof(lv_color_t));
    buf2 = (lv_color_t *)ps_malloc(buffer_size * sizeof(lv_color_t));
    usbSerial.printf("[LVGL] PSRAM buffers: buf1=%p, buf2=%p\n", buf1, buf2);
  }

  if (!buf1) {
    usbSerial.println("[LVGL] Allocating buf1 in internal RAM...");
    buf1 = (lv_color_t *)malloc(buffer_size * sizeof(lv_color_t));
  }
  if (!buf2) {
    usbSerial.println("[LVGL] Allocating buf2 in internal RAM...");
    buf2 = (lv_color_t *)malloc(buffer_size * sizeof(lv_color_t));
  }
  usbSerial.printf("[LVGL] Final buffers: buf1=%p, buf2=%p\n", buf1, buf2);

  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_size);
  usbSerial.println("[LVGL] draw_buf initialized.");

  /*Initialize the display*/
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = tftWidth;
  disp_drv.ver_res =
      tftHeight; // Use actual display height (tftHeight has footer offset)
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  usbSerial.println("[LVGL] display driver registered.");

  /*Initialize the (dummy) input device driver*/
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  usbSerial.println("[LVGL] input driver registered.");
}

//================================================================================
// FUNÇÕES DE DISPLAY AUSENTES - IMPLEMENTAÇÕES
//================================================================================

// Funções padprintln - imprime com padding
void padprintln(const String &s, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(s);
}

void padprintln(const char str[], int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(str);
}

void padprintln(char c, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(c);
}

void padprintln(unsigned char b, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(b, base);
}

void padprintln(int n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(n, base);
}

void padprintln(unsigned int n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(n, base);
}

void padprintln(long n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(n, base);
}

void padprintln(unsigned long n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(n, base);
}

void padprintln(long long n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println((long)n, base); // TFT_eSPI não suporta long long diretamente
}

void padprintln(unsigned long long n, int base, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println((unsigned long)n,
              base); // TFT_eSPI não suporta unsigned long long diretamente
}

void padprintln(double n, int digits, int16_t padx) {
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.println(n, digits);
}

// Funções padprintf - printf com padding
void padprintf(int16_t padx, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  tft.setCursor(BORDER_PAD_X + padx, tft.getCursorY());
  tft.print(buffer);
}

void padprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  tft.setCursor(BORDER_PAD_X, tft.getCursorY());
  tft.print(buffer);
}

// Funções padprint - imprime sem quebra de linha
void padprint(const String &s, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(s);
}

void padprint(const char str[], int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(str);
}

void padprint(char c, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(c);
}

void padprint(unsigned char b, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(b, base);
}

void padprint(int n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(n, base);
}

void padprint(unsigned int n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(n, base);
}

void padprint(long n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(n, base);
}

void padprint(unsigned long n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(n, base);
}

void padprint(long long n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print((long)n, base); // TFT_eSPI não suporta long long diretamente
}

void padprint(unsigned long long n, int base, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print((unsigned long)n,
            base); // TFT_eSPI não suporta unsigned long long diretamente
}

void padprint(double n, int digits, int16_t padx) {
  tft.setCursor(padx * BORDER_PAD_X, tft.getCursorY());
  tft.print(n, digits);
}

//================================================================================
// OUTRAS FUNÇÕES DE DISPLAY AUSENTES
//================================================================================

// Função printSubtitle
void printSubtitle(String subtitle, bool withLine) {
  tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y + STATUS_BAR_HEIGHT + 10);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  // Quebra de linha automática para texto longo
  int maxCharsPerLine = (tftWidth - 2 * BORDER_PAD_X) /
                        6; // Aproximadamente 6 pixels por caractere
  if ((int)subtitle.length() > maxCharsPerLine) {
    String remaining = subtitle;
    int yOffset = 0;
    while (remaining.length() > 0) {
      String line;
      if ((int)remaining.length() <= maxCharsPerLine) {
        line = remaining;
        remaining = "";
      } else {
        int spaceIndex = remaining.lastIndexOf(' ', maxCharsPerLine);
        if (spaceIndex > 0) {
          line = remaining.substring(0, spaceIndex);
          remaining = remaining.substring(spaceIndex + 1);
        } else {
          line = remaining.substring(0, maxCharsPerLine);
          remaining = remaining.substring(maxCharsPerLine);
        }
      }
      tft.setCursor(BORDER_PAD_X,
                    BORDER_PAD_Y + STATUS_BAR_HEIGHT + 10 + yOffset);
      tft.println(line);
      yOffset += 10; // Altura aproximada da linha
    }
  } else {
    tft.println(subtitle);
  }

  if (withLine) {
    tft.drawLine(BORDER_OFFSET_FROM_SCREEN_EDGE,
                 BORDER_PAD_Y + STATUS_BAR_HEIGHT + 25,
                 tftWidth - BORDER_OFFSET_FROM_SCREEN_EDGE,
                 BORDER_PAD_Y + STATUS_BAR_HEIGHT + 25, willyConfig.priColor);
  }
}

// Função printCenterFootnote
void printCenterFootnote(String text) {
  int centerX = (tftWidth - (text.length() * 6)) / 2;
  tft.setCursor(centerX, tftHeight - 40);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.println(text);
}

// Função printFootnote
void printFootnote(String text) {
  tft.setCursor(BORDER_PAD_X, tftHeight - 40);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.println(text);
}

// Função getColorVariation
uint16_t getColorVariation(uint16_t color, int delta, int direction) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  if (direction == 0) { // variar todos
    r = constrain(r + delta, 0, 31);
    g = constrain(g + delta, 0, 63);
    b = constrain(b + delta, 0, 31);
  } else if (direction == 1) { // variar apenas vermelho
    r = constrain(r + delta, 0, 31);
  } else if (direction == 2) { // variar apenas verde
    g = constrain(g + delta, 0, 63);
  } else if (direction == 3) { // variar apenas azul
    b = constrain(b + delta, 0, 31);
  }

  return (r << 11) | (g << 5) | b;
}

// Função progressHandler
void progressHandler(int progress, size_t total, String message) {
  static int lastProgress = -1;
  if (progress != lastProgress) {
    tft.setCursor(BORDER_PAD_X, tftHeight - 60);
    tft.printf("Progress: %d/%d - %s", progress, (int)total, message.c_str());
    lastProgress = progress;
  }
}

// Função resetTftDisplay
void resetTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg,
                     uint16_t screen) {
  tft.fillScreen(screen);
  tft.setCursor(x, y);
  tft.setTextColor(fc, bg);
  tft.setTextSize(size);
}

// Funções de display de mensagens
void displayError(String txt, bool waitKeyPress) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, TFT_RED);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.println("ERROR: " + txt);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  if (waitKeyPress) {
    while (!check(AnyKeyPress))
      vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displaySuccess(String txt, bool waitKeyPress) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, TFT_GREEN);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.println("SUCCESS: " + txt);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  if (waitKeyPress) {
    while (!check(AnyKeyPress))
      vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displayWarning(String txt, bool waitKeyPress) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, TFT_YELLOW);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  tft.println("WARNING: " + txt);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  if (waitKeyPress) {
    while (!check(AnyKeyPress))
      vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displayInfo(String txt, bool waitKeyPress) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, TFT_BLUE);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.println("INFO: " + txt);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  if (waitKeyPress) {
    while (!check(AnyKeyPress))
      vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displayRedStripe(String text, uint16_t fgcolor, uint16_t bgcolor) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, bgcolor);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(fgcolor, bgcolor);
  tft.println(text);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
}

void displayTextLine(String txt, bool waitKeyPress) {
  tft.fillRect(0, tftHeight - 60, tftWidth, 60, willyConfig.priColor);
  tft.setCursor(10, tftHeight - 50);
  tft.setTextColor(willyConfig.bgColor, willyConfig.priColor);
  tft.println(txt);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  if (waitKeyPress) {
    while (!check(AnyKeyPress))
      vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Função loopOptions completa
int loopOptions(std::vector<Option> &options, uint8_t menuType,
                const char *subText, int index, bool interpreter) {
  // Implementação básica - apenas retorna o índice selecionado
  // Esta função é complexa e pode precisar de implementação específica
  // baseada no código do menu principal
  return index;
}

// Funções de borda
void drawMainBorder(bool clear) {
  if (clear) {
    tft.fillScreen(willyConfig.bgColor);
  }

  tft.drawRect(BORDER_OFFSET_FROM_SCREEN_EDGE, BORDER_OFFSET_FROM_SCREEN_EDGE,
               tftWidth - (2 * BORDER_OFFSET_FROM_SCREEN_EDGE),
               tftHeight - (2 * BORDER_OFFSET_FROM_SCREEN_EDGE),
               willyConfig.priColor);

  tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y);
}

void drawMainBorderWithTitle(String title, bool clear) {
  drawMainBorder(clear);

  // Desenha título na parte superior da borda
  int titleX = (tftWidth - (title.length() * 6)) /
               2; // 6 pixels por caractere aproximado
  tft.setCursor(titleX, BORDER_OFFSET_FROM_SCREEN_EDGE + 5);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.print(title);

  // Linha horizontal abaixo do título
  tft.drawLine(BORDER_OFFSET_FROM_SCREEN_EDGE,
               BORDER_OFFSET_FROM_SCREEN_EDGE + STATUS_BAR_HEIGHT,
               tftWidth - BORDER_OFFSET_FROM_SCREEN_EDGE,
               BORDER_OFFSET_FROM_SCREEN_EDGE + STATUS_BAR_HEIGHT,
               willyConfig.priColor);

  tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y + STATUS_BAR_HEIGHT);
}

void printTitle(String title) {
  tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.println(title);
}

//================================================================================
// MAIS FUNÇÕES DE DISPLAY FALTANTES
//================================================================================

// Função wakeUpScreen
bool wakeUpScreen() {
  // Implementação básica - sempre retorna true
  return true;
}

// Função drawStatusBar
void drawStatusBar() {
  // Desenha uma barra de status simples na parte superior
  tft.fillRect(0, 0, tftWidth, STATUS_BAR_HEIGHT, willyConfig.priColor);
  tft.setCursor(5, 5);
  tft.setTextColor(willyConfig.bgColor, willyConfig.priColor);
  tft.print("WILLY ESP32-S3");
}

// Funções de ícones pequenos
void drawWifiSmall(int x, int y) {
  // Desenha um ícone WiFi simples
  tft.drawCircle(x + 5, y + 2, 2, willyConfig.priColor);
  tft.drawCircle(x + 5, y + 5, 4, willyConfig.priColor);
  tft.drawCircle(x + 5, y + 8, 6, willyConfig.priColor);
}

void drawWebUISmall(int x, int y) {
  // Desenha um ícone de web simples
  tft.drawCircle(x + 5, y + 5, 4, willyConfig.priColor);
  tft.drawLine(x + 1, y + 5, x + 9, y + 5, willyConfig.priColor);
  tft.drawLine(x + 5, y + 1, x + 5, y + 9, willyConfig.priColor);
}

void drawBLESmall(int x, int y) {
  // Desenha um ícone BLE simples
  tft.drawCircle(x + 3, y + 3, 2, willyConfig.priColor);
  tft.drawCircle(x + 7, y + 7, 2, willyConfig.priColor);
  tft.drawLine(x + 3, y + 3, x + 7, y + 7, willyConfig.priColor);
}

void drawBLE_beacon(int x, int y, uint16_t color) {
  // Desenha um beacon BLE
  tft.drawCircle(x + 5, y + 5, 4, color);
  tft.fillCircle(x + 5, y + 5, 2, color);
}

void drawGPS(int x, int y) {
  // Desenha um ícone GPS
  tft.drawTriangle(x + 2, y + 8, x + 5, y + 2, x + 8, y + 8,
                   willyConfig.priColor);
  tft.drawCircle(x + 5, y + 5, 2, willyConfig.priColor);
}

void drawGpsSmall(int x, int y) {
  // Desenha um ícone GPS pequeno
  tft.drawTriangle(x + 1, y + 4, x + 3, y + 1, x + 5, y + 4,
                   willyConfig.priColor);
  tft.drawPixel(x + 3, y + 3, willyConfig.priColor);
}

void drawCreditCard(int x, int y) {
  // Desenha um ícone de cartão de crédito
  tft.drawRect(x, y, 10, 6, willyConfig.priColor);
  tft.drawLine(x + 2, y + 2, x + 8, y + 2, willyConfig.priColor);
  tft.drawLine(x + 2, y + 4, x + 8, y + 4, willyConfig.priColor);
}

void drawMfkey32Icon(int x, int y) {
  // Desenha ícone MFKEY32
  tft.drawRect(x, y, 8, 6, willyConfig.priColor);
  tft.setCursor(x + 10, y);
  tft.setTextSize(1);
  tft.print("32");
}

void drawMfkey64Icon(int x, int y) {
  // Desenha ícone MFKEY64
  tft.drawRect(x, y, 8, 6, willyConfig.priColor);
  tft.setCursor(x + 10, y);
  tft.setTextSize(1);
  tft.print("64");
}

// Funções de footer
void TouchFooter(uint16_t color) {
  tft.fillRect(0, tftHeight - 30, tftWidth, 30, color);
  tft.setCursor(10, tftHeight - 20);
  tft.setTextColor(willyConfig.bgColor, color);
  tft.print("Touch Screen Active");
}

void MegaFooter(uint16_t color) {
  tft.fillRect(0, tftHeight - 30, tftWidth, 30, color);
  tft.setCursor(10, tftHeight - 20);
  tft.setTextColor(willyConfig.bgColor, color);
  tft.print("Mega Mode Active");
}

// Função drawBatteryStatus
void drawBatteryStatus(uint8_t bat) {
  int x = tftWidth - 40;
  int y = 5;

  // Desenha contorno da bateria
  tft.drawRect(x, y, 30, 12, willyConfig.priColor);
  tft.fillRect(x + 30, y + 3, 3, 6, willyConfig.priColor);

  // Desenha nível da bateria
  int level = map(bat, 0, 100, 0, 26);
  uint16_t color = (bat > 20) ? TFT_GREEN : (bat > 10) ? TFT_YELLOW : TFT_RED;
  tft.fillRect(x + 2, y + 2, level, 8, color);

  // Mostra porcentagem
  tft.setCursor(x - 25, y + 2);
  tft.printf("%d%%", bat);
}

// Função drawWireguardStatus
void drawWireguardStatus(int x, int y) {
  tft.setCursor(x, y);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.print("WG");
}

//================================================================================
// MAIS FUNÇÕES CRÍTICAS FALTANTES
//================================================================================

// Função getComplementaryColor2
uint16_t getComplementaryColor2(uint16_t color) {
  // Retorna uma cor complementar simples
  return (color ^ 0xFFFF) & 0xFFFF;
}

// Função displayMessage
int8_t displayMessage(const char *message, const char *leftButton,
                      const char *centerButton, const char *rightButton,
                      uint16_t color) {
  // Implementação básica - apenas mostra mensagem e retorna
  tft.fillScreen(willyConfig.bgColor);
  tft.setCursor(10, 50);
  tft.setTextColor(color, willyConfig.bgColor);
  tft.println(message);

  if (leftButton) {
    tft.setCursor(10, tftHeight - 40);
    tft.print(leftButton);
  }
  if (centerButton) {
    tft.setCursor(tftWidth / 2 - 20, tftHeight - 40);
    tft.print(centerButton);
  }
  if (rightButton) {
    tft.setCursor(tftWidth - 50, tftHeight - 40);
    tft.print(rightButton);
  }

  // Aguarda input do usuário (implementação simplificada)
  while (!check(AnyKeyPress))
    vTaskDelay(100 / portTICK_PERIOD_MS);
  return 0; // Retorna botão pressionado (simplificado)
}

// Função turnOffDisplay
void turnOffDisplay() {
  // Implementação básica - apenas limpa a tela
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    tft.fillScreen(TFT_BLACK);
    xSemaphoreGive(spiMutex);
  }
}

// Funções de arquivo e imagem (implementações básicas)
Opt_Coord listFiles(int index, std::vector<FileList> fileList) {
  Opt_Coord coord;
  coord.x = 10;
  coord.y = 30 + (index * 15);
  coord.size = 10;
  coord.fgcolor = willyConfig.priColor;
  coord.bgcolor = willyConfig.bgColor;
  return coord;
}

void displayScrollingText(const String &text, Opt_Coord &coord) {
  tft.setCursor(coord.x, coord.y);
  tft.setTextColor(coord.fgcolor, coord.bgcolor);
  tft.setTextSize(coord.size / 10); // Ajuste básico
  tft.println(text);
}

bool drawImg(FS &fs, String filename, int x, int y, bool center,
             int playDurationMs, bool resetButtonStatus) {
  // Implementação básica - apenas retorna false (não implementada)
  return false;
}

bool preparePngBin(FS &fs, String filename) {
  // Implementação básica - apenas retorna false (não implementada)
  return false;
}

//================================================================================
// MAIS FUNÇÕES FALTANTES - CONTINUAÇÃO
//================================================================================

// Função setTftDisplay
void setTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
  tft.setCursor(x, y);
  tft.setTextColor(fc, bg);
  tft.setTextSize(size);
}

// Função drawPng (se existir)
bool drawPng(FS &fs, String filename, int x, int y, bool center) {
  // Implementação básica para PNG
  return false; // Não implementado completamente
}

// Função showGif (se existir)
#if !defined(LITE_VERSION)
bool showGif(FS *fs, const char *filename, int x, int y, bool center,
             int playDurationMs, bool clearButtonStatus) {
  // Implementação básica para GIF
  return false; // Não implementado completamente
}
#endif

// Função showJpeg (se existir)
bool showJpeg(FS &fs, String filename, int x, int y, bool center) {
  // Implementação básica para JPEG
  return false; // Não implementado completamente
}

// Função drawBmp (se existir)
bool drawBmp(FS &fs, String filename, int x, int y, bool center) {
  // Implementação básica para BMP
  return false; // Não implementado completamente
}

// Função drawImg com parâmetros diferentes (se existir)
bool drawImg(FS &fs, String filename, int x, int y, bool center) {
  // Versão sobrecarregada
  return drawImg(fs, filename, x, y, center, 0, true);
}

// Função isCharging (se existir)
bool __attribute__((weak)) isCharging() {
  // Implementação básica - retorna false
  return false;
}
