#ifndef __DISPLAY_LOGER
#define __DISPLAY_LOGER

#include <Arduino.h>
#include <stdint.h>

#ifdef HAS_SCREEN
#include <display/tft.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#define WILLY_TFT_DRIVER tft_display
#else
#include "VectorDisplay.h"
#define WILLY_TFT_DRIVER SerialDisplayClass
#endif

enum tftFuncs {
  FILLSCREEN,
  DRAWRECT,
  FILLRECT,
  DRAWROUNDRECT,
  FILLROUNDRECT,
  DRAWCIRCLE,
  FILLCIRCLE,
  DRAWTRIAGLE,
  FILLTRIANGLE,
  DRAWELIPSE,
  FILLELIPSE,
  DRAWLINE,
  DRAWARC,
  DRAWWIDELINE,
  DRAWCENTRESTRING,
  DRAWRIGHTSTRING,
  DRAWSTRING,
  PRINT,
  DRAWIMAGE,
  DRAWPIXEL,
  DRAWFASTVLINE,
  DRAWFASTHLINE,
  SCREEN_INFO = 99
};

#if defined(BOARD_HAS_PSRAM)
#define MAX_LOG_ENTRIES 64
#define MAX_LOG_SIZE 128
#define MAX_LOG_IMAGES 1
#define MAX_LOG_IMG_PATH 512
#else
#define MAX_LOG_ENTRIES 40
#define MAX_LOG_SIZE 64
#define MAX_LOG_IMAGES 1
#define MAX_LOG_IMG_PATH 256
#endif

#define LOG_PACKET_HEADER 0xAA

struct tftLog {
  uint8_t data[MAX_LOG_SIZE];
};

class tft_logger : public WILLY_TFT_DRIVER {
private:
  tftLog *log = nullptr;
  char (*images)[MAX_LOG_IMG_PATH] = nullptr;
  uint8_t logWriteIndex = 0;
  uint8_t logCount = 0;
  bool isSleeping = false;
  bool logging = false;
  bool _logging = false;
  void clearLog();
  void addLogEntry(const uint8_t *buffer, uint8_t size);
  void logWriteHeader(uint8_t *buffer, uint8_t &pos, tftFuncs fn);
  void writeUint16(uint8_t *buffer, uint8_t &pos, uint16_t value);
  bool async_serial = false;
  TaskHandle_t asyncSerialTask = NULL;
  QueueHandle_t asyncSerialQueue = NULL;
  static void asyncSerialTaskFunc(void *pv);
  void restoreLogger();
  bool isLogEqual(const tftLog &a, const tftLog &b);
  void pushLogIfUnique(const tftLog &l);
  void log_drawString(String s, tftFuncs fn, int32_t x, int32_t y);
  void log_print(String s);
  void checkAndLog(tftFuncs fn, ...);

public:
  tft_logger(int16_t w = TFT_WIDTH, int16_t h = TFT_HEIGHT);
  ~tft_logger();
  void setLogging(bool _log = true);

  using WILLY_TFT_DRIVER::endWrite;
  using WILLY_TFT_DRIVER::getCursorX;
  using WILLY_TFT_DRIVER::getCursorY;
  using WILLY_TFT_DRIVER::printf;
  using WILLY_TFT_DRIVER::pushColors;
  using WILLY_TFT_DRIVER::setAddrWindow;
  using WILLY_TFT_DRIVER::setCursor;
  using WILLY_TFT_DRIVER::setTextColor;
  using WILLY_TFT_DRIVER::setTextFont;
  using WILLY_TFT_DRIVER::setTextSize;
  using WILLY_TFT_DRIVER::setTextWrap;
  using WILLY_TFT_DRIVER::startWrite;

  bool inline getLogging(void) { return logging; };
  void inline setSleepMode(bool mode) { isSleeping = mode; }

  void getBinLog(uint8_t *outBuffer, size_t &outSize);
  bool removeLogEntriesInsideRect(int rx, int ry, int rw, int rh);
  void removeOverlappedImages(int x, int y, int center, int ms);
  void fillScreen(uint32_t color);
  void startAsyncSerial();
  void stopAsyncSerial();
  void getTftInfo();
  void imageToBin(uint8_t fs, String file, int x, int y, bool center, int Ms);

  // Drawing methods (overriding base class)
  void drawLine(int32_t x, int32_t y, int32_t x1, int32_t y1, uint32_t color);
  void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
  void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
  void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r,
                     int32_t color);
  void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r,
                     int32_t color);
  void drawCircle(int32_t x, int32_t y, int32_t r, int32_t color);
  void fillCircle(int32_t x, int32_t y, int32_t r, int32_t color);
  void drawEllipse(int16_t x, int16_t y, int32_t rx, int32_t ry,
                   uint16_t color);
  void fillEllipse(int16_t x, int16_t y, int32_t rx, int32_t ry,
                   uint16_t color);
  void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3,
                    int32_t y3, int32_t color);
  void fillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3,
                    int32_t y3, int32_t color);
  void drawArc(int32_t x, int32_t y, int32_t r, int32_t ir, uint32_t startAngle,
               uint32_t endAngle, uint32_t fg_color, uint32_t bg_color,
               bool smoothArc = true);
  void drawWideLine(float ax, float ay, float bx, float by, float wd,
                    int32_t fg, int32_t bg);
  void drawFastVLine(int32_t x, int32_t y, int32_t h, int32_t fg);
  void drawFastHLine(int32_t x, int32_t y, int32_t w, int32_t fg);

  // Drawing string methods (overriding base class)
  int16_t drawString(const String &string, int32_t x, int32_t y);
  int16_t drawString(const String &string, int32_t x, int32_t y, uint8_t font);
  int16_t drawCentreString(const String &string, int32_t x, int32_t y);
  int16_t drawCentreString(const String &string, int32_t x, int32_t y,
                           uint8_t font);
  int16_t drawRightString(const String &string, int32_t x, int32_t y);
  int16_t drawRightString(const String &string, int32_t x, int32_t y,
                          uint8_t font);

  // Print methods (overriding Print class)
  size_t print(const String &s);
  size_t print(char c);
  size_t print(unsigned char b, int base);
  size_t print(int n, int base);
  size_t print(unsigned int n, int base);
  size_t print(long n, int base);
  size_t print(unsigned long n, int base);
  size_t print(long long n, int base);
  size_t print(unsigned long long n, int base);
  size_t print(double n, int digits);

  size_t println(void);
  size_t println(const String &s);
  size_t println(char c);
  size_t println(unsigned char b, int base);
  size_t println(int n, int base);
  size_t println(unsigned int n, int base);
  size_t println(long n, int base);
  size_t println(unsigned long n, int base);
  size_t println(long long n, int base);
  size_t println(unsigned long long n, int base);
  size_t println(double n, int digits);

  size_t printf(const char *format, ...);
};

#endif // __DISPLAY_LOGER
