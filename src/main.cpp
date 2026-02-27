#include "core/main_menu.h"
#include <globals.h>

#include "core/powerSave.h"
#include "core/serial_commands/cli.h"
#include "core/utils.h"
#include "current_year.h"
#include "esp32-hal-psram.h"
#include "esp_task_wdt.h"
#include "esp_wifi.h"
#include <functional>
#include <lvgl.h>
#include <string>
#include <vector>
#include "ui/cyber_menu.h"
io_expander ioExpander;
BruceConfig bruceConfig;
BruceConfigPins bruceConfigPins;

SerialCli serialCli;
USBSerial USBserial;
SerialDevice *serialDevice = &USBserial;

StartupApp startupApp;
String startupAppLuaScript = "";

MainMenu mainMenu;
SPIClass sdcardSPI;
#ifdef USE_HSPI_PORT
#ifndef VSPI
#define VSPI FSPI
#endif
SPIClass CC_NRF_SPI(VSPI);
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

TaskHandle_t xHandle;
void __attribute__((weak)) taskInputHandler(void *parameter) {
    while (true) {
        checkPowerSaveTime();

        // Input detection
        InputHandler();

        // Sole owner of LVGL timer processing for stability
        if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
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

// wifi globals
// TODO put in a namespace
bool wifiConnected = false;
bool isWebUIActive = false;
String wifiIP;

bool BLEConnected = false;
bool returnToMenu;
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
ESP32Time rtc;
bool clock_set = false;
#endif

std::vector<Option> options;
// Protected global variables
#if defined(HAS_SCREEN)
tft_logger tft = tft_logger(); // Invoke custom library
tft_sprite sprite = tft_sprite(&tft);
tft_sprite draw = tft_sprite(&tft);
volatile int tftWidth = TFT_HEIGHT;
#ifdef HAS_TOUCH
volatile int tftHeight =
    TFT_WIDTH - 20; // 20px to draw the TouchFooter(), were the btns are being read in touch devices.
#else
volatile int tftHeight = TFT_WIDTH;
#endif
#else
tft_logger tft;
SerialDisplayClass &sprite = tft;
SerialDisplayClass &draw = tft;
volatile int tftWidth = VECTOR_DISPLAY_DEFAULT_HEIGHT;
volatile int tftHeight = VECTOR_DISPLAY_DEFAULT_WIDTH;
#endif

#include "core/display.h"
#include "core/led_control.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/serialcmds.h"
#include "core/settings.h"
#include "core/wifi/webInterface.h"
#include "core/wifi/wifi_common.h"
#include "modules/bjs_interpreter/interpreter.h" // for JavaScript interpreter
#include "modules/others/audio.h"                // for playAudioFile
#include "modules/rf/rf_utils.h"                 // for initCC1101once
#include <Wire.h>
#include "willy_logger.h"                        // Sistema de logging centralizado
#include "ui/willy_splash.h"                     // Splash screen Willy

/*********************************************************************
 **  Function: begin_storage
 **  Config LittleFS and SD storage
 *********************************************************************/
void begin_storage() {
    if (!LittleFS.begin(true)) { LittleFS.format(), LittleFS.begin(); }
    bool checkFS = setupSdCard();
    bruceConfig.fromFile(checkFS);
    bruceConfigPins.fromFile(checkFS);
}

/*********************************************************************
 **  Function: _setup_gpio()
 **  Sets up a weak (empty) function to be replaced by /ports/* /interface.h
 *********************************************************************/
void _setup_gpio() __attribute__((weak));
void _setup_gpio() {}

/*********************************************************************
 **  Function: _post_setup_gpio()
 **  Sets up a weak (empty) function to be replaced by /ports/* /interface.h
 *********************************************************************/
void _post_setup_gpio() __attribute__((weak));
void _post_setup_gpio() {}

/*********************************************************************
 **  Function: setup_gpio
 **  Setup GPIO pins
 *********************************************************************/
void setup_gpio() {

    // init setup from /ports/*/interface.h
    _setup_gpio();

    // Smoochiee v2 uses a AW9325 tro control GPS, MIC, Vibro and CC1101 RX/TX powerlines
    ioExpander.init(IO_EXPANDER_ADDRESS, &Wire);

#if TFT_MOSI > 0
    if (bruceConfigPins.CC1101_bus.mosi == (gpio_num_t)TFT_MOSI)
        initCC1101once(&tft.getSPIinstance()); // (T_EMBED), CORE2 and others
    else
#endif
        if (bruceConfigPins.CC1101_bus.mosi == bruceConfigPins.SDCARD_bus.mosi)
        initCC1101once(&sdcardSPI); // (ARDUINO_M5STACK_CARDPUTER) and (ESP32S3DEVKITC1) and devices that
                                    // share CC1101 pin with only SDCard
    else initCC1101once(NULL);
    // (ARDUINO_M5STICK_C_PLUS) || (ARDUINO_M5STICK_C_PLUS2) and others that doesn´t share SPI with
    // other devices (need to change it when Bruce board comes to shore)
}

/*********************************************************************
 **  Function: begin_tft
 **  Config tft
 *********************************************************************/
void begin_tft() {
    tft.setRotation(bruceConfigPins.rotation); // sometimes it misses the first command
    tft.invertDisplay(bruceConfig.colorInverted);
    tft.setRotation(bruceConfigPins.rotation);
    tftWidth = tft.width();
#ifdef HAS_TOUCH
    tftHeight = tft.height() - 20;
#else
    tftHeight = tft.height();
#endif
    resetTftDisplay();
    setBrightness(bruceConfig.bright, false);
}

/*********************************************************************
 **  Function: boot_screen
 **  Draw boot screen
 *********************************************************************/
void boot_screen() {
    Serial.println("[BOOT] Inside boot_screen()...");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FM);
    tft.drawPixel(0, 0, bruceConfig.bgColor);
    Serial.println("[BOOT] Drawing 'Willy' string...");
    tft.drawCentreString("Willy", tftWidth / 2, 10, 1);
    tft.setTextSize(FP);
    Serial.println("[BOOT] Drawing version string...");
    tft.drawCentreString(BRUCE_VERSION, tftWidth / 2, 25, 1);
    tft.setTextSize(FM);
    Serial.println("[BOOT] Drawing 'PREDATORY FIRMWARE' string...");
    tft.drawCentreString(
        "PREDATORY FIRMWARE", tftWidth / 2, tftHeight + 2, 1
    ); // will draw outside the screen on non touch devices
    Serial.println("[BOOT] Finished boot_screen().");
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
        if (SD.exists("/boot.jpg")) boot_img = 1;
        else {
            Serial.println("[BOOT] Checking SD for /boot.gif...");
            if (SD.exists("/boot.gif")) boot_img = 3;
        }
    }
    if (boot_img == 0) {
        Serial.println("[BOOT] Checking LittleFS for /boot.jpg...");
        if (LittleFS.exists("/boot.jpg")) boot_img = 2;
        else {
            Serial.println("[BOOT] Checking LittleFS for /boot.gif...");
            if (LittleFS.exists("/boot.gif")) boot_img = 4;
        }
    }
    Serial.println("[BOOT] Checking theme override...");
    if (bruceConfig.theme.boot_img) boot_img = 5; // override others

    Serial.println("[BOOT] Forcing TFT sync...");
    tft.drawPixel(0, 0, 0);       // Forces back communication with TFT, to avoid ghosting
                                  // Start image loop
    Serial.println("[BOOT] Starting while loop...");
    while (millis() < i + 7000) { // boot image lasts for 7 secs
        if ((millis() - i > 2000) && !drawn) {
            Serial.println("[BOOT] 2 seconds elapsed, starting drawing logic...");
            tft.fillRect(0, 45, tftWidth, tftHeight - 45, bruceConfig.bgColor);
            if (boot_img > 0 && !drawn) {
                Serial.printf("[BOOT] boot_img = %d, drawing image...\n", boot_img);
                tft.fillScreen(bruceConfig.bgColor);
                if (boot_img == 5) {
                    Serial.println("[BOOT] Drawing from theme FS...");
                    drawImg(
                        *bruceConfig.themeFS(),
                        bruceConfig.getThemeItemImg(bruceConfig.theme.paths.boot_img),
                        0,
                        0,
                        true,
                        3600
                    );
                } else if (boot_img == 1) {
                    Serial.println("[BOOT] Drawing /boot.jpg from SD...");
                    drawImg(SD, "/boot.jpg", 0, 0, true);
                } else if (boot_img == 2) {
                    Serial.println("[BOOT] Drawing /boot.jpg from LittleFS...");
                    drawImg(LittleFS, "/boot.jpg", 0, 0, true);
                } else if (boot_img == 3) {
                    Serial.println("[BOOT] Drawing /boot.gif from SD...");
                    drawImg(SD, "/boot.gif", 0, 0, true, 3600);
                } else if (boot_img == 4) {
                    Serial.println("[BOOT] Drawing /boot.gif from LittleFS...");
                    drawImg(LittleFS, "/boot.gif", 0, 0, true, 3600);
                }
                Serial.println("[BOOT] Image drawing call finished.");
                tft.drawPixel(0, 0, 0); // Forces back communication with TFT, to avoid ghosting
            }
            drawn = true;
        }
#if !defined(LITE_VERSION)
        if (!boot_img && (millis() - i > 2200) && (millis() - i) < 2700) {
            // Serial.println("[BOOT] Drawing rect..."); // Too verbose
            tft.drawRect(2 * tftWidth / 3, tftHeight / 2, 2, 2, bruceConfig.priColor);
        }
        if (!boot_img && (millis() - i > 2700) && (millis() - i) < 2900)
            tft.fillRect(0, 45, tftWidth, tftHeight - 45, bruceConfig.bgColor);
        if (!boot_img && (millis() - i > 2900) && (millis() - i) < 3400)
            tft.drawXBitmap(
                2 * tftWidth / 3 - 30,
                5 + tftHeight / 2,
                bruce_small_bits,
                bruce_small_width,
                bruce_small_height,
                bruceConfig.bgColor,
                bruceConfig.priColor
            );
        if (!boot_img && (millis() - i > 3400) && (millis() - i) < 3600) tft.fillScreen(bruceConfig.bgColor);
        if (!boot_img && (millis() - i > 3600)) {
            // Serial.println("[BOOT] Drawing main bits..."); // Too verbose
            tft.drawXBitmap(
                (tftWidth - 238) / 2,
                (tftHeight - 133) / 2,
                bits,
                bits_width,
                bits_height,
                bruceConfig.bgColor,
                bruceConfig.priColor
            );
        }
#endif
        if (check(AnyKeyPress)) // If any key or M5 key is pressed, it'll jump the boot screen
        {
            Serial.println("[BOOT] Key press detected, jumping...");
            tft.fillScreen(bruceConfig.bgColor);
            delay(10);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Increase delay for debug print visibility logic
    }
    Serial.println("[BOOT] Loop finished.");

    // Clear splashscreen
    tft.fillScreen(bruceConfig.bgColor);
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
    beginLed();
#endif
}

/*********************************************************************
 **  Function: startup_sound
 **  Play sound or tone depending on device hardware
 *********************************************************************/
void startup_sound() {
    if (bruceConfig.soundEnabled == 0) return; // if sound is disabled, do not play sound
#if !defined(LITE_VERSION)
#if defined(BUZZ_PIN)
    // Bip M5 just because it can. Does not bip if splashscreen is bypassed
    _tone(5000, 50);
    delay(200);
    _tone(5000, 50);
    /*  2fix: menu infinite loop */
#elif defined(HAS_NS4168_SPKR)
    // play a boot sound
    if (bruceConfig.theme.boot_sound) {
        playAudioFile(bruceConfig.themeFS(), bruceConfig.getThemeItemImg(bruceConfig.theme.paths.boot_sound));
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
    Serial.setRxBufferSize(
        SAFE_STACK_BUFFER_SIZE / 4
    ); // Must be invoked before Serial.begin(). Default is 256 chars
    Serial.begin(115200);

    log_d("Total heap: %d", ESP.getHeapSize());
    log_d("Free heap: %d", ESP.getFreeHeap());
    if (psramInit()) log_d("PSRAM Started");
    if (psramFound()) log_d("PSRAM Found");
    else log_d("PSRAM Not Found");
    log_d("Total PSRAM: %d", ESP.getPsramSize());
    log_d("Free PSRAM: %d", ESP.getFreePsram());

    // declare variables
    prog_handler = 0;
    sdcardMounted = false;
    wifiConnected = false;
    BLEConnected = false;
    bruceConfig.bright = 100; // theres is no value yet
    bruceConfigPins.rotation = ROTATION;
    setup_gpio();
#if defined(HAS_SCREEN)
    tft.init();
    tft.setRotation(bruceConfigPins.rotation);
    tft.fillScreen(TFT_BLACK);
    // bruceConfig is not read yet.. just to show something on screen due to long boot time
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString("Booting", tft.width() / 2, tft.height() / 2, 1);
#else
    tft.begin();
#endif
    Serial.println("Starting begin_storage()...");
    begin_storage();
    Serial.println("begin_storage() completed successfully.");
    Serial.println("Starting begin_tft()...");
    begin_tft();
    Serial.println("begin_tft() completed successfully.");

    Serial.println("Starting initLVGL()...");
    initLVGL(); // Initialize LVGL
    Serial.println("initLVGL() completed successfully.");

    if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
        show_willy_splash(lv_scr_act()); // Show Willy splash screen
        xSemaphoreGive(lvgl_mutex);
    }

    for (int i = 0; i < 55; i++) { // Wait for animation (approx 5.5s)
        if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            lv_timer_handler();
            xSemaphoreGive(lvgl_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // Load a completely new screen to avoid dangling references
    // from the splash screen animations before InputHandler and loop() take over
    if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
        lv_anim_del_all();
        lv_obj_t *new_scr = lv_obj_create(NULL);
        lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);
        lv_timer_handler(); // process screen load
        xSemaphoreGive(lvgl_mutex);
    }

    // SD Card presence check - warn if not found
    if (!sdcardMounted) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(FM);
        tft.drawCentreString("AVISO", tftWidth / 2, tftHeight / 2 - 30, 1);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(FP);
        tft.drawCentreString("Cartao SD nao encontrado!", tftWidth / 2, tftHeight / 2, 1);
        tft.drawCentreString("Insira o cartao e reinicie.", tftWidth / 2, tftHeight / 2 + 16, 1);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawCentreString("Pressione qualquer tecla...", tftWidth / 2, tftHeight / 2 + 40, 1);
        uint32_t warnStart = millis();
        while (millis() - warnStart < 3000) {
            if (check(AnyKeyPress)) break;
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        tft.fillScreen(bruceConfig.bgColor);
    }

    Serial.println("Initializing clock...");
    init_clock();
    Serial.println("Clock initialized.");

    Serial.println("Initializing LED...");
    init_led();
    Serial.println("LED initialized.");

    // Inicializa sistema de logging centralizado Willy
    /*
    if (sdcardMounted) {
        willyLogger.begin();
        willyLogger.logSystemStatus();
        willyLogger.info(COMP_SYSTEM, "Willy iniciado - versao " BRUCE_VERSION);

        // Mostra aviso de log ativo
        willyLogger.showLogWarning();
    }
    */

    Serial.println("Reserving options capacity...");
    options.reserve(20); // preallocate some options space to avoid fragmentation
    Serial.println("Options reserved.");

    // Set WiFi country to avoid warnings and ensure max power
    Serial.println("Setting WiFi country...");
    const wifi_country_t country = {
        .cc = "US",
        .schan = 1,
        .nchan = 14,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };

    esp_wifi_set_max_tx_power(80); // 80 translates to 20dBm
    esp_wifi_set_country(&country);
    Serial.println("WiFi country set.");

    // Some GPIO Settings (such as CYD's brightness control must be set after tft and sdcard)
    Serial.println("Starting _post_setup_gpio()...");
    _post_setup_gpio();
    Serial.println("_post_setup_gpio() completed successfully.");
    // end of post gpio begin

    // #ifndef USE_TFT_eSPI_TOUCH
#if defined(HAS_SCREEN)
    Serial.println("Opening theme file...");
    bruceConfig.openThemeFile(bruceConfig.themeFS(), bruceConfig.themePath, false);
    Serial.println("Theme file opened.");

    if (!bruceConfig.instantBoot) {
        Serial.println("Legacy boot screen disabled (Using Willy Splash).");
    }
    if (bruceConfig.wifiAtStartup) {
        Serial.println("Creating wifiConnectTask...");
        xTaskCreate(
            wifiConnectTask,   // Task function
            "wifiConnectTask", // Task Name
            4096,              // Stack size
            NULL,              // Task parameters
            2,                 // Task priority (0 to 3), loopTask has priority 2.
            NULL               // Task handle (not used)
        );
        Serial.println("wifiConnectTask created.");
    }
#endif
    //  start a task to handle serial commands while the webui is running
    Serial.println("Starting serial commands handler task...");
    startSerialCommandsHandlerTask(true);
    Serial.println("Serial commands handler task started.");

    Serial.println("Waking up screen...");
    wakeUpScreen();
    Serial.println("Screen woken up.");

    if (bruceConfig.startupApp != "" && !startupApp.startApp(bruceConfig.startupApp)) {
        bruceConfig.setStartupApp("");
    }

    // This task keeps running all the time, will never stop
    Serial.println("Creating InputHandler task...");
    xTaskCreate(
        taskInputHandler,              // Task function
        "InputHandler",                // Task Name
        INPUT_HANDLER_TASK_STACK_SIZE, // Stack size
        NULL,                          // Task parameters
        2,                             // Task priority (0 to 3), loopTask has priority 2.
        &xHandle                       // Task handle (not used)
    );
    Serial.println("InputHandler task created. setup() finished.");
}

/**********************************************************************
 **  Function: loop
 **  Main loop
 **********************************************************************/
#if defined(HAS_SCREEN)
void loop() {
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
    if (interpreter_state > 0) {
        vTaskDelete(serialcmdsTaskHandle); // stop serial commands while in interpreter
        vTaskDelay(pdMS_TO_TICKS(10));
        interpreter_state = 2;
        Serial.println("Entering interpreter...");
        while (interpreter_state > 0) { vTaskDelay(pdMS_TO_TICKS(500)); }
        Serial.println("Exiting interpreter...");
        if (interpreter_state == -1) { interpreterTaskHandler = NULL; }
        startSerialCommandsHandlerTask();
        previousMillis = millis(); // ensure that will not dim screen when get back to menu
    }
#endif
    // Set Cyber Menu as default UI
    static bool menu_initialized = false;
    if (!menu_initialized) {
        if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            setup_cyber_menu(lv_scr_act());
            menu_initialized = true;
            xSemaphoreGive(lvgl_mutex);
        }
    }

    // LVGL timer handling removed from here, now centralized in taskInputHandler() for stability

    // Check if the user clicked an LVGL icon
    if (pending_cyber_menu_action >= 0) {
        int action = pending_cyber_menu_action;
        pending_cyber_menu_action = -1; // reset immediately

        // Clear LVGL screen before booting the traditional menu to avoid visual collision
        if (lvgl_mutex && xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            lv_obj_clean(lv_scr_act());
            lv_timer_handler();
            xSemaphoreGive(lvgl_mutex);
        }

        // Action maps to the index of options in MainMenu as defined in main_menu.cpp options.push_back order
        // Ensure index does not drift
        auto menuOptions = mainMenu.getItems();
        if (action < menuOptions.size()) {
            tft.fillScreen(bruceConfig.bgColor);
            menuOptions[action]->optionsMenu(); // Directly open the specific classic menu
        }

        // Upon return, we re-arm LVGL menu initialization
        menu_initialized = false;
        tft.fillScreen(bruceConfig.bgColor); // clear old Bruce rendering
    }

    delay(5);
}
#else

void loop() {
    tft.setLogging();
    Serial.println(
        "\n"
        "██████  ██████  ██    ██  ██████ ███████ \n"
        "██   ██ ██   ██ ██    ██ ██      ██      \n"
        "██████  ██████  ██    ██ ██      █████   \n"
        "██   ██ ██   ██ ██    ██ ██      ██      \n"
        "██████  ██   ██  ██████   ██████ ███████ \n"
        "                                         \n"
        "         PREDATORY FIRMWARE\n\n"
        "Tips: Connect to the WebUI for better experience\n"
        "      Add your network by sending: wifi add ssid password\n\n"
        "At your command:"
    );

    // Enable navigation through webUI
    tft.fillScreen(bruceConfig.bgColor);
    mainMenu.begin();
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
#endif
