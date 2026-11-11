#include "core/main_menu.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <HardwareSerial.h>
#include <globals.h>

#include "core/display.h"
#include "core/headless_mode.h"
#include "core/powerSave.h"
#include "core/serial_commands/cli.h"
#include "core/wifi/wifi_common.h"
#include "core/willy_logo.h"

// Novos includes para arquitetura MVC e módulos
#include "core/BenchmarkManager.h"
#include "core/DynamicConfigManager.h"
#include "core/SystemController.h"
#include "core/SystemManager.h"
#include "core/SystemModel.h"
#include "core/SystemView.h"
#include "core/advanced_logger.h"
#include "current_year.h"
#include "esp32-hal-psram.h"
#include "esp_task_wdt.h"
#include "esp_wifi.h"
#include "modules/ml/MLModule.h"
#include "modules/rf/RFModule.h"
#include "modules/rf/rf_utils.h"
#include "modules/rfid/RFIDModule.h"
#include "modules/wifi/WiFiModule.h"
#include <ESP32Time.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <vector>

#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "modules/bjs_interpreter/interpreter.h"
#endif

// LVGL includes
#ifdef HAS_SCREEN
#include <lvgl.h>
#endif

bool tftInitialized = false;
volatile bool lvgl_rendering_active = false;
io_expander ioExpander;
WillyConfig willyConfig;
WillyConfigPins willyConfigPins;

Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(LED_COUNT, RGB_LED, NEO_GRB + NEO_KHZ800);

SerialCli serialCli;
WillyUSBSerial usbSerial(&Serial);
SerialDevice *serialDevice = &usbSerial;

StartupApp startupApp;
String startupAppLuaScript = "";

MainMenu mainMenu;
SPIClass sdcardSPI;
#ifdef USE_HSPI_PORT
#ifndef VSPI
#define VSPI FSPI
#endif
SPIClass CC_NRF_SPI(FSPI);
#else
SPIClass CC_NRF_SPI(HSPI);
#endif

// Navigation Variables
volatile bool NextPress = false;
volatile bool PrevPress = false;
volatile bool UpPress = false;
volatile bool DownPress = false;
volatile bool SelPress = false;
volatile bool EscPress = false;
volatile bool AnyKeyPress = false;
volatile bool NextPagePress = false;
volatile bool PrevPagePress = false;
volatile bool LongPress = false;
volatile bool SerialCmdPress = false;
volatile int forceMenuOption = -1;
volatile uint8_t menuOptionType = 0;
String menuOptionLabel = "";
#ifdef HAS_ENCODER_LED
volatile int EncoderLedChange = 0;
#endif

TouchPoint touchPoint;

keyStroke KeyStroke;

SemaphoreHandle_t spiMutex = NULL;
SemaphoreHandle_t displayMutex = NULL;

TaskHandle_t xHandle;
void __attribute__((weak)) InputHandler() {
  // Read touch screen and update touchPoint
  updateTouchPoint();
}

void __attribute__((weak)) taskInputHandler(void *parameter) {
  Serial.println("[DBG] taskInputHandler started");
  while (true) {
    checkPowerSaveTime();

    // Input detection
    InputHandler();

    // Only process LVGL when explicitly enabled (splash or cyber menu)
    if (lvgl_rendering_active && lvgl_mutex &&
        xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      lv_timer_handler();
      xSemaphoreGive(lvgl_mutex);
    }

    vTaskDelay(pdMS_TO_TICKS(15)); // Faster iteration (66Hz) for smooth touch
  }
}
// Public Globals Variables
unsigned long previousMillis = millis();
int prog_handler; // 0 - Flash, 1 - LittleFS, 3 - Download
String cachedPassword = "";
int8_t interpreter_state = -1;
bool sdcardMounted = false;
bool gpsConnected = false;

// wifi globals moved to wifi_common.cpp

bool BLEConnected = false;
bool returnToMenu;
String currentLoaderApp = "";
bool appRequiresClose = false;
bool isSleeping = false;
bool isScreenOff = false;
bool dimmer = false;
char timeStr[16];
time_t localTime;
struct tm *timeInfo;
#if defined(HAS_RTC)
#if defined(HAS_RTC_PCF85063A)
pcf85063_RTC _rtc;
#else
cplus_RTC _rtc;
#endif
RTC_TimeTypeDef _time;
RTC_DateTypeDef _date;
bool clock_set = true;
#else
ESP32Time rtc(0);
bool clock_set = false;
#endif

std::vector<Option> options;
// Protected global variables
#if defined(HAS_SCREEN)
tft_logger tft = tft_logger(); // Invoke custom library
tft_sprite sprite((tft_display *)&tft);
tft_sprite draw((tft_display *)&tft);
volatile int tftWidth = TFT_HEIGHT;
volatile int tftHeight = TFT_WIDTH;
#else
tft_logger tft;
SerialDisplayClass &sprite = tft;
SerialDisplayClass &draw = tft;
volatile int tftWidth = VECTOR_DISPLAY_DEFAULT_HEIGHT;
volatile int tftHeight = VECTOR_DISPLAY_DEFAULT_WIDTH;
#endif

#include "ui/willy_splash.h" // Splash screen Willy
#include "willy_logger.h"    // Sistema de logging centralizado
#include <Wire.h>

/*********************************************************************
 **  Function: begin_storage
 **  Config LittleFS and SD storage
 *********************************************************************/
void begin_storage() {
  if (!LittleFS.begin(true)) {
    LittleFS.format(), LittleFS.begin();
  }
  // SD card disabled during boot — SD shares TFT SPI bus and
  // failed mount attempts corrupt SPI state, breaking touch.
  // SD will be mounted on-demand when user accesses SD features.
  bool checkFS = false;
  Serial.println("[SD] SD card skipped at boot (shares SPI with touch)");
  willyConfig.fromFile(checkFS);
  willyConfigPins.fromFile(checkFS);
}

/*********************************************************************
 **  Function: _setup_gpio()
 **  Now handled in board-specific interface.cpp to avoid conflicts.
 *********************************************************************/
void __attribute__((weak)) _setup_gpio() {
  // Default empty implementation
}

/*********************************************************************
 **  Function: _post_setup_gpio()
 **  Post IO setup (like display backlight, and custom IO expanders logic)
 *********************************************************************/
void _post_setup_gpio() {
  // Turn on backlight if configured
#ifdef TFT_BL
  if (TFT_BL >= 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    Serial.printf("[DISPLAY] Backlight turned ON on pin %d\n", TFT_BL);
  } else {
    Serial.println("[DISPLAY] Backlight pin not configured (TFT_BL < 0)");
  }
#else
  Serial.println("[DISPLAY] Backlight not configured (TFT_BL not defined)");
#endif
}

/*********************************************************************
 **  Function: setup_gpio
 **  Setup GPIO pins
 *********************************************************************/
void setup_gpio() {

  // init setup from /ports/*/interface.h
  _setup_gpio();

  // Smoochiee v2 uses a AW9325 tro control GPS, MIC, Vibro and CC1101 RX/TX
  // powerlines
  ioExpander.init(IO_EXPANDER_ADDRESS, &Wire);

#if TFT_MOSI > 0
  if (willyConfigPins.CC1101_bus.mosi == (gpio_num_t)TFT_MOSI)
    initCC1101once(&tft.getSPIinstance()); // (T_EMBED), CORE2 and others
  else
#endif
      if (willyConfigPins.CC1101_bus.mosi == willyConfigPins.SDCARD_bus.mosi)
    initCC1101once(
        &sdcardSPI); // (ARDUINO_M5STACK_CARDPUTER) and (ESP32S3DEVKITC1) and
                     // devices that share CC1101 pin with only SDCard
  else
    initCC1101once(NULL);
  // (ARDUINO_M5STICK_C_PLUS) || (ARDUINO_M5STICK_C_PLUS2) and others that
  // doesn´t share SPI with other devices (need to change it when Willy board
  // comes to shore)
}

/*********************************************************************
 **  Function: begin_tft
 **  Config tft display hardware only (no touch calibration here).
 **  Touch calibration is done in setup_touch() AFTER begin_storage().
 *********************************************************************/
void begin_tft() {
  Serial.println("[BOOT] Iniciando tft.init()...");

  tft.init();
  Serial.println("[DEBUG] tft.init() executado");
  tft.setRotation(1); // Rotation 1 = landscape (320x240)
  Serial.println("[DEBUG] tft.setRotation(1) executado");
  tft.invertDisplay(
      false); // ILI9341: set to false for normal colors (true inverts all)
  Serial.println("[DEBUG] tft.invertDisplay(false) executado");
  tft.fillScreen(TFT_BLACK);
  Serial.println("[DEBUG] tft.fillScreen(TFT_BLACK) executado");

  Serial.printf("[DISPLAY] TFT resolution after rotation: %dx%d\n", tft.width(),
                tft.height());

  tftWidth = tft.width();
  tftHeight = tft.height();

  if (tftWidth > 0 && tftHeight > 0) {
    tftInitialized = true;
    Serial.printf(
        "[DISPLAY] TFT initialized successfully! tftWidth=%d, tftHeight=%d\n",
        tftWidth, tftHeight);
  } else {
    tftInitialized = false;
    Serial.println("[ERROR] TFT initialization failed! Width or height is 0. "
                   "Entering headless mode.");
  }
}

/*********************************************************************
 **  Function: setup_touch
 **  Load or run touch calibration. Must be called AFTER begin_storage()
 **  so LittleFS is already mounted.
 *********************************************************************/
void setup_touch() {
#ifdef HAS_TOUCH
  uint16_t calData[5];
  bool needsCalibration = false;

  // Add a small delay to let the SPI bus settle after initializations
  delay(100);

  // Check if calibration file exists (LittleFS already mounted by
  // begin_storage)
  if (!LittleFS.exists("/touch_cal.dat")) {
    needsCalibration = true;
    Serial.println("[TOUCH] No calibration file found in LittleFS.");
  }

#ifdef JOY_BTN_PIN
  // Hold joystick button during boot to force recalibration
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);
  delay(100);
  int btnVal = digitalRead(JOY_BTN_PIN);
  Serial.printf(
      "[TOUCH] Checking JOY_BTN_PIN (%d): %s\n", JOY_BTN_PIN,
      (btnVal == LOW ? "LOW (Pressed/Force Cal)" : "HIGH (Not Pressed)"));
  if (btnVal == LOW) {
    needsCalibration = true;
    Serial.println("[TOUCH] Joystick button HELD - deleting old data and "
                   "forcing recalibration.");
    LittleFS.remove("/touch_cal.dat");
  }
#endif

  if (needsCalibration) {
    Serial.println(
        "[TOUCH] Using hardcoded calibration data from working demo");
    tft.fillScreen(TFT_BLACK);

    // Use hardcoded values from clean_demo that works
    calData[0] = 280;
    calData[1] = 3555;
    calData[2] = 298;
    calData[3] = 3505;
    calData[4] = 7;

    // Save to LittleFS (now guaranteed to be mounted)
    File f = LittleFS.open("/touch_cal.dat", "w");
    if (f) {
      f.write((uint8_t *)calData, 10);
      f.close();
      Serial.println("[TOUCH] Hardcoded calibration SAVED to /touch_cal.dat");
    } else {
      Serial.println("[TOUCH] WARNING: Could not save calibration!");
    }
    tft.fillScreen(TFT_BLACK);
  } else {
    // Load existing calibration
    File f = LittleFS.open("/touch_cal.dat", "r");
    if (f) {
      f.read((uint8_t *)calData, 10);
      f.close();
      Serial.println("[TOUCH] Calibration loaded from /touch_cal.dat");
    } else {
      // Fallback defaults for rotation=1 (landscape)
      calData[0] = 280;
      calData[1] = 3555;
      calData[2] = 298;
      calData[3] = 3505;
      calData[4] = 7;
      Serial.println("[TOUCH] Using hardcoded default calibration");
    }
  }

  // Verify calibration data sanity (after loading or using defaults)
  if (calData[0] == 0 || calData[0] == 0xFFFF || calData[0] == calData[1]) {
    Serial.println("[TOUCH] Calibration FAILED sanity check. Using defaults "
                   "and forcing recalibration.");
    LittleFS.remove("/touch_cal.dat");
    calData[0] = 280;
    calData[1] = 3555;
    calData[2] = 298;
    calData[3] = 3505;
    calData[4] = 7;
  }

  tft.setTouch(calData);
  Serial.printf("[TOUCH] CalData applied: %d,%d,%d,%d,%d\n", calData[0],
                calData[1], calData[2], calData[3], calData[4]);

  // Test touch functionality
  uint16_t tx, ty;
  uint8_t touchResult = tft.getTouch(&tx, &ty, 600);
  Serial.printf("[TOUCH] Touch test result: %d, x=%d, y=%d\n", touchResult, tx,
                ty);
#endif
}

/*********************************************************************
 **  Function: boot_screen
 **  Draw boot screen
 *********************************************************************/
void boot_screen() {
  Serial.println("[BOOT] Inside boot_screen()...");

  // Fundo escuro com borda Neon Aqua
  tft.fillScreen(willyConfig.bgColor);
  tft.drawRect(2, 2, tftWidth - 4, tftHeight - 4, willyConfig.priColor);

  // Desenha logo ASCII "Willy" centralizado
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.setTextSize(FP);

  // Posição inicial para o logo ASCII
  int logoY = 15;
  int logoX = tftWidth / 2;

  // Desenha cada linha do logo ASCII
  const char *logo = WILLY_LOGO_ASCII;
  char line[50];
  int lineIdx = 0;

  tft.setTextDatum(MC_DATUM); // Centralizado

  // Desenha o logo linha por linha
  tft.drawString("      /\\ ", logoX, logoY);
  tft.drawString("     /  \\ ", logoX, logoY + 8);
  tft.drawString("    /____\\ ", logoX, logoY + 16);
  tft.drawString("   /\\    /\\ ", logoX, logoY + 24);
  tft.drawString("  /  \\  /  \\ ", logoX, logoY + 32);
  tft.drawString(" |____||____| ", logoX, logoY + 40);
  tft.drawString(" \\____/\\____/ ", logoX, logoY + 48);
  tft.drawString("  \\____/____/ ", logoX, logoY + 56);
  tft.drawString("   \\________/ ", logoX, logoY + 64);

  // Texto "Willy" em destaque
  tft.setTextSize(FM);
  tft.setTextColor(willyConfig.secColor, willyConfig.bgColor);
  tft.drawString("WILLY", logoX, logoY + 80);

  // Versão
  tft.setTextSize(FP);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.drawString(WILLY_VERSION, logoX, logoY + 95);

  // Rodapé
  tft.setTextSize(FP);
  tft.drawString("FUTURISTIC EDITION", logoX, tftHeight - 15);

  Serial.println("[BOOT] Finished boot_screen() with ASCII logo.");
}

/*********************************************************************
 **  Function: boot_screen_anim
 **  Draw boot screen
 *********************************************************************/
void boot_screen_anim() {
  boot_screen();
  int i = millis();
  // checks for boot.jpg in SD and LittleFS for customization
  int boot_img = 0;
  bool drawn = false;
  if (sdcardMounted) {
    Serial.println("[BOOT] Checking SD for /boot.jpg...");
    if (SD.exists("/boot.jpg"))
      boot_img = 1;
    else {
      Serial.println("[BOOT] Checking SD for /boot.gif...");
      if (SD.exists("/boot.gif"))
        boot_img = 3;
    }
  }
  if (boot_img == 0) {
    Serial.println("[BOOT] Checking LittleFS for /boot.jpg...");
    if (LittleFS.exists("/boot.jpg"))
      boot_img = 2;
    else {
      Serial.println("[BOOT] Checking LittleFS for /boot.gif...");
      if (LittleFS.exists("/boot.gif"))
        boot_img = 4;
    }
  }
  tft.drawPixel(0, 0,
                0); // Forces back communication with TFT, to avoid ghosting
  // Start image loop
  while (millis() < (unsigned long)(i + 7000)) { // boot image lasts for 5 secs
    if ((millis() - i > 2000) && !drawn) {
      tft.fillRect(0, 45, tftWidth, tftHeight - 45, willyConfig.bgColor);
      if (boot_img > 0 && !drawn) {
        tft.fillScreen(willyConfig.bgColor);
        if (boot_img == 5) {
          drawImg(*willyConfig.themeFS(),
                  willyConfig.getThemeItemImg(willyConfig.theme.paths.boot_img),
                  0, 0, true, 3600);
          Serial.println("Image from SD theme");
        } else if (boot_img == 1) {
          drawImg(SD, "/boot.jpg", 0, 0, true);
          Serial.println("Image from SD");
        } else if (boot_img == 2) {
          drawImg(LittleFS, "/boot.jpg", 0, 0, true);
          Serial.println("Image from LittleFS");
        } else if (boot_img == 3) {
          drawImg(SD, "/boot.gif", 0, 0, true, 3600);
          Serial.println("Image from SD");
        } else if (boot_img == 4) {
          drawImg(LittleFS, "/boot.gif", 0, 0, true, 3600);
          Serial.println("Image from LittleFS");
        }
        tft.drawPixel(
            0, 0, 0); // Forces back communication with TFT, to avoid ghosting
      }
      drawn = true;
    }
#if !defined(LITE_VERSION)
    if (!boot_img && (millis() - i > 2200) && (millis() - i) < 2700)
      tft.drawRect(2 * tftWidth / 3, tftHeight / 2, 2, 2, willyConfig.priColor);
    if (!boot_img && (millis() - i > 2700) && (millis() - i) < 2900)
      tft.fillRect(0, 45, tftWidth, tftHeight - 45, willyConfig.bgColor);
    if (!boot_img && (millis() - i > 2900) && (millis() - i) < 3400)
      tft.drawXBitmap(2 * tftWidth / 3 - 30, 5 + tftHeight / 2,
                      willy_small_bits, willy_small_width, willy_small_height,
                      willyConfig.bgColor, willyConfig.priColor);
    if (!boot_img && (millis() - i > 3400) && (millis() - i) < 3600)
      tft.fillScreen(willyConfig.bgColor);
    if (!boot_img && (millis() - i > 3600))
      tft.drawXBitmap((tftWidth - 238) / 2, (tftHeight - 133) / 2, bits,
                      bits_width, bits_height, willyConfig.bgColor,
                      willyConfig.priColor);
#endif
    if (check(AnyKeyPress)) // If any key or M5 key is pressed, it'll jump the
                            // boot screen
    {
      tft.fillScreen(willyConfig.bgColor);
      delay(10);
      return;
    }
  }

  // Clear splashscreen
  tft.fillScreen(willyConfig.bgColor);
}

/*********************************************************************
 **  Function: init_clock
 **  Clock initialisation for propper display in menu
 *********************************************************************/
void init_clock() {
#if defined(HAS_RTC)
  _rtc.begin();
#if defined(HAS_RTC_BM8563)
  _rtc.GetBm8563Time();
#endif
#if defined(HAS_RTC_PCF85063A)
  _rtc.GetPcf85063Time();
#endif
  _rtc.GetTime(&_time);
  _rtc.GetDate(&_date);

  struct tm timeinfo = {};
  timeinfo.tm_sec = _time.Seconds;
  timeinfo.tm_min = _time.Minutes;
  timeinfo.tm_hour = _time.Hours;
  timeinfo.tm_mday = _date.Date;
  timeinfo.tm_mon = _date.Month > 0 ? _date.Month - 1 : 0;
  timeinfo.tm_year = _date.Year >= 1900 ? _date.Year - 1900 : 0;
  time_t epoch = mktime(&timeinfo);
  struct timeval tv = {.tv_sec = epoch};
  settimeofday(&tv, nullptr);
#else
  struct tm timeinfo = {};
  timeinfo.tm_year = CURRENT_YEAR - 1900;
  timeinfo.tm_mon = 0x05;
  timeinfo.tm_mday = 0x14;
  time_t epoch = mktime(&timeinfo);
  rtc.setTime(epoch);
  clock_set = true;
  struct timeval tv = {.tv_sec = epoch};
  settimeofday(&tv, nullptr);
#endif
}

/*********************************************************************
 **  Function: init_led
 **  Led initialisation
 *********************************************************************/
void init_led() {
#ifdef HAS_RGB_LED
  pixels.begin();
#endif
}

/*********************************************************************
 **  Function: startup_sound
 **  Play sound or tone depending on device hardware
 *********************************************************************/
void startup_sound() {
  if (willyConfig.soundEnabled == 0)
    return; // if sound is disabled, do not play sound
#if !defined(LITE_VERSION)
#if defined(BUZZ_PIN)
  // Bip M5 just because it can. Does not bip if splashscreen is bypassed
  _tone(5000, 50);
  delay(200);
  _tone(5000, 50);
  /*  2fix: menu infinite loop */
#elif defined(HAS_NS4168_SPKR)
  // play a boot sound
  if (willyConfig.theme.boot_sound) {
    playAudioFile(
        willyConfig.themeFS(),
        willyConfig.getThemeItemImg(willyConfig.theme.paths.boot_sound));
  } else if (SD.exists("/boot.wav")) {
    playAudioFile(&SD, "/boot.wav");
  } else if (LittleFS.exists("/boot.wav")) {
    playAudioFile(&LittleFS, "/boot.wav");
  }
#endif
#endif
}

/*********************************************************************
 **  Function: setup
 **  Where the devices are started and variables set
 *********************************************************************/
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== WILLY ESP S3 - BOOT CORRIGIDO ===");
  Serial.println("[DEBUG] Serial initialized");
  spiMutex = xSemaphoreCreateMutex();
  Serial.println("[DEBUG] spiMutex created");
  displayMutex = xSemaphoreCreateMutex();
  Serial.println("[DEBUG] displayMutex created");

  Serial.println("[BOOT] Initializing system...");

  // Inicializar componentes MVC
  Serial.println("[BOOT] Initializing MVC components...");
  SystemModel::getInstance().loadConfig();
  SystemView::getInstance().init();
  SystemController::getInstance().init();

  // Registrar módulos no SystemManager
  Serial.println("[BOOT] Registering modules...");
  auto &systemManager = SystemManager::getInstance();
  auto modelPtr = std::make_shared<SystemModel>(SystemModel::getInstance());
  auto viewPtr = std::make_shared<SystemView>(SystemView::getInstance());

  systemManager.registerModule(std::make_unique<WiFiModule>(modelPtr, viewPtr));
  systemManager.registerModule(std::make_unique<RFModule>(modelPtr, viewPtr));
  systemManager.registerModule(std::make_unique<RFIDModule>(modelPtr, viewPtr));
  systemManager.registerModule(std::make_unique<MLModule>(modelPtr, viewPtr));
  systemManager.registerModule(std::make_unique<DynamicConfigManager>());
  systemManager.registerModule(
      std::make_unique<BenchmarkManager>(modelPtr, viewPtr));

  // Inicializar todos os módulos
  if (!systemManager.initAllModules()) {
    Serial.println("[ERROR] Failed to initialize some modules!");
  }

  // Inicializar AdvancedLogger (willyLogger)
  Serial.println("[BOOT] Initializing Advanced Logger...");
  if (!willyLogger.begin()) {
    Serial.println("[ERROR] Failed to initialize Advanced Logger!");
  }

  // NENHUM tft.init() aqui (deixa para begin_tft())
  // NENHUM hack de backlight GPIO 21/3

  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  if (psramInit())
    log_d("PSRAM Started");
  if (psramFound())
    log_d("PSRAM Found");
  else
    log_d("PSRAM Not Found");
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());

  // declare variables
  prog_handler = 0;
  sdcardMounted = false;
  wifiConnected = false;
  BLEConnected = false;
  willyConfig.bright = 100; // theres is no value yet
  willyConfigPins.rotation = ROTATION;

  Serial.println("[BOOT] Initializing GPIOs...");
  setup_gpio();
  Serial.println("[DEBUG] GPIOs initialized");

#if defined(HAS_SCREEN)
  begin_tft();
  Serial.println("[DEBUG] TFT initialized");
#else
  tft.begin();
  tftInitialized =
      true; // For devices without HAS_SCREEN explicitly but using tft
#endif

  Serial.println("Starting begin_storage()...");
  begin_storage();
  Serial.println("[DEBUG] Storage initialized");

#if defined(HAS_SCREEN) && defined(HAS_TOUCH)
  // Touch calibration MUST happen after begin_storage() so LittleFS is mounted
  setup_touch();
  Serial.println("[DEBUG] Touch initialized");
#endif

  if (tftInitialized) {
    Serial.println("[BOOT] Iniciando LVGL + Cyber Menu...");
    Serial.println("[DEBUG] Calling initLVGL()");
    initLVGL();
    Serial.println("[DEBUG] LVGL initialized");

    // Show LVGL splash screen (orca animation)
    Serial.println("[BOOT] Showing splash screen...");
    Serial.println("[DEBUG] Setting lvgl_rendering_active = true");
    lvgl_rendering_active = true;
    lv_obj_t *splash_scr = lv_scr_act();
    show_willy_splash(splash_scr);
    // Drive LVGL rendering for splash duration (~5s)
    unsigned long splashStart = millis();
    while (millis() - splashStart < 5000) {
      if (lvgl_mutex &&
          xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        lv_timer_handler();
        xSemaphoreGive(lvgl_mutex);
      }
      vTaskDelay(pdMS_TO_TICKS(15));
      // Allow skipping splash with any key
      if (AnyKeyPress) {
        AnyKeyPress = false;
        Serial.println("[BOOT] Splash skipped by user.");
        break;
      }
    }
    // Clean up splash and disable LVGL rendering for TFT direct menu
    lv_obj_clean(splash_scr);
    if (lvgl_mutex &&
        xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      lv_timer_handler();
      xSemaphoreGive(lvgl_mutex);
    }
    tft.fillScreen(TFT_BLACK);
    delay(200);
    lvgl_rendering_active = false;
    Serial.println("[BOOT] Splash complete. LVGL paused for TFT menu.");
    Serial.println("[DEBUG] Splash complete");

  } else {
    Serial.println("[WARNING] MODO HEADLESS ATIVADO - Tela não encontrada");
    Serial.println("[INFO] Serial + Web Dashboard ainda funcionam normalmente");
  }

  init_clock();
  Serial.println("[DEBUG] Clock initialized");
  init_led();
  Serial.println("[DEBUG] LED initialized");

  options.reserve(20); // preallocate some options space to avoid fragmentation
  Serial.println("[DEBUG] Options reserved");

  // Set WiFi country to avoid warnings and ensure max power
  const wifi_country_t country = {.cc = "US",
                                  .schan = 1,
                                  .nchan = 14,
#ifdef CONFIG_ESP_PHY_MAX_TX_POWER
                                  .max_tx_power =
                                      CONFIG_ESP_PHY_MAX_TX_POWER, // 20
#endif
                                  .policy = WIFI_COUNTRY_POLICY_MANUAL};

  esp_wifi_set_max_tx_power(80); // 80 translates to 20dBm
  esp_wifi_set_country(&country);

  // Some GPIO Settings (such as CYD's brightness control must be set after tft
  // and sdcard)
  _post_setup_gpio();
  // end of post gpio begin

  // #ifndef USE_TFT_eSPI_TOUCH
  // This task keeps running all the time, will never stop
  xTaskCreate(taskInputHandler,              // Task function
              "InputHandler",                // Task Name
              INPUT_HANDLER_TASK_STACK_SIZE, // Stack size
              NULL,                          // Task parameters
              2,       // Task priority (0 to 3), loopTask has priority 2.
              &xHandle // Task handle (not used)
  );
  // #endif
#if defined(HAS_SCREEN)
  Serial.printf("[MAIN] Abrindo tema: fs=%d, path='%s'\n", willyConfig.theme.fs,
                willyConfig.themePath.c_str());
  willyConfig.openThemeFile(willyConfig.themeFS(), willyConfig.themePath,
                            false);
  if (!willyConfig.instantBoot) {
    // boot_screen_anim() removed — replaced by LVGL splash in setup()
    startup_sound();
  }
  if (willyConfig.wifiAtStartup) {
    log_i("Loading Wifi at Startup");
    xTaskCreate(wifiConnectTask,   // Task function
                "wifiConnectTask", // Task Name
                4096,              // Stack size
                NULL,              // Task parameters
                2,   // Task priority (0 to 3), loopTask has priority 2.
                NULL // Task handle (not used)
    );
  }
#endif
  //  start a task to handle serial commands while the webui is running
  startSerialCommandsHandlerTask(true);

  wakeUpScreen();
  if (willyConfig.startupApp != "" &&
      !startupApp.startApp(willyConfig.startupApp)) {
    willyConfig.setStartupApp("");
  }
  Serial.println("[DEBUG] Setup complete");
}

/**********************************************************************
 **  Function: loop
 **  Main loop
 **********************************************************************/
#if defined(HAS_SCREEN)
void loop() {
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
  if (interpreter_state > 0) {
    vTaskDelete(
        serialcmdsTaskHandle); // stop serial commands while in interpreter
    vTaskDelay(pdMS_TO_TICKS(10));
    interpreter_state = 2;
    Serial.println("Entering interpreter...");
    while (interpreter_state > 0) {
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    Serial.println("Exiting interpreter...");
    if (interpreter_state == -1) {
      interpreterTaskHandler = NULL;
    }
    startSerialCommandsHandlerTask();
    previousMillis =
        millis(); // ensure that will not dim screen when get back to menu
  }
#endif
  tft.fillScreen(willyConfig.bgColor);

  Serial.println("[DEBUG] Calling mainMenu.begin()");
  mainMenu.begin();
  delay(1);
}
#else

void loop() {
  tft.setLogging();
  Serial.println("\n"
                 "██      ██ ██ ██      ██      ██    ██ \n"
                 "██      ██ ██ ██      ██       ██  ██  \n"
                 "██  ██  ██ ██ ██      ██        ████   \n"
                 "██ ████ ██ ██ ██      ██         ██    \n"
                 " ███  ███  ██ ███████ ███████    ██    \n"
                 "                                       \n"
                 "         PREDATORY FIRMWARE\n\n"
                 "Tips: Connect to the WebUI for better experience\n"
                 "      Add your network by sending: wifi add ssid password\n\n"
                 "At your command:");

  // Enable navigation through webUI
  tft.fillScreen(willyConfig.bgColor);
  mainMenu.begin();
  vTaskDelay(10 / portTICK_PERIOD_MS);
}
#endif
