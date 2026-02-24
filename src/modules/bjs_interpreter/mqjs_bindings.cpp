/**
 * @file mqjs_bindings.cpp
 * @brief Micro QuickJS bindings for Bruce
 * @version 1.0
 *
 * Implementation for mquickjs API compatibility.
 */

#include "interpreter.h"
#include "core/display.h"
#include "globals.h"
extern "C" {
#include "mquickjs.h"
}
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <SD.h>
#include "mqjs_stdlib.h"
#include "core/mykeyboard.h"
#include "modules/ir/ys_irtm.h"
#include "modules/badusb_ble/ducky_typer.h"
#include "modules/rf/rf_send.h"
#include "modules/rf/rf_utils.h"

// Forward declarations
extern int getBattery();

// Helper to convert JSValue to C string (now uses local buffer passed by caller)
#define JS_TO_CSTRING(ctx, val, buf) JS_ToCString(ctx, val, buf)

// Forward declaration for the dispatch function
static JSValue js_bruce_dispatch(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv, int magic);

// Macro for easier function definition in the table
#define BRUCE_CFUNC_DEF(magic, args) { { .generic_magic = js_bruce_dispatch }, JS_UNDEFINED, JS_CFUNC_generic_magic, args, magic }

// Enum for function indices
enum {
    FUNC_DELAY = 0,
    FUNC_MILLIS,
    FUNC_PRINT,
    PRINTLN,
    FUNC_DISPLAY_FILLSCREEN,
    FUNC_DISPLAY_DRAWSTRING,
    FUNC_DISPLAY_DRAWCENTRE,
    FUNC_DISPLAY_SETTEXTCOLOR,
    FUNC_DISPLAY_SETTEXTSIZE,
    FUNC_DISPLAY_WIDTH,
    FUNC_DISPLAY_HEIGHT,
    FUNC_GPIO_DIGITALWRITE,
    FUNC_GPIO_DIGITALREAD,
    FUNC_GPIO_ANALOGWRITE,
    FUNC_GPIO_ANALOGREAD,
    FUNC_GPIO_PINMODE,
    FUNC_WIFI_SCAN,
    FUNC_WIFI_CONNECT,
    FUNC_WIFI_DISCONNECT,
    FUNC_WIFI_STATUS,
    FUNC_WIFI_IP,
    FUNC_HTTP_GET,
    FUNC_STORAGE_READ,
    FUNC_STORAGE_WRITE,
    FUNC_STORAGE_REMOVE,
    FUNC_STORAGE_EXISTS,
    FUNC_SUBGHZ_TX,
    FUNC_BADUSB_RUN,
    FUNC_DEVICE_BATTERY,
    FUNC_DEVICE_FREEHEAP,
    FUNC_DEVICE_BOARD,
    FUNC_DEVICE_SDK,
    FUNC_IR_TX,
    FUNC_IR_SEND,
    FUNC_KEYBOARD_INPUT,
    FUNC_KEYBOARD_HEX,
    FUNC_KEYBOARD_NUM,
    FUNC_DIALOG_ERROR,
    FUNC_DIALOG_WARNING,
    FUNC_DIALOG_INFO,
    FUNC_DIALOG_SUCCESS,
    FUNC_COMPAT_GETBOARDNAME,
    FUNC_COMPAT_GETCPUFREQ,
    FUNC_COMPAT_GETFREEHEAP,
    FUNC_COMPAT_GETFLASHCHIPSIZE,
    FUNC_COMPAT_GETSDKVERSION,
    FUNC_GPIO_LEDC_ATTACH,
    FUNC_GPIO_LEDC_WRITE,
    FUNC_GPIO_LEDC_DETACH,
    FUNC_COUNT
};

// Function table for mQuickJS
extern "C" const JSCFunctionDef bruce_func_table[] = {
    BRUCE_CFUNC_DEF(FUNC_DELAY, 1),
    BRUCE_CFUNC_DEF(FUNC_MILLIS, 0),
    BRUCE_CFUNC_DEF(FUNC_PRINT, 1),
    BRUCE_CFUNC_DEF(PRINTLN, 1),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_FILLSCREEN, 1),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_DRAWSTRING, 3),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_DRAWCENTRE, 4),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_SETTEXTCOLOR, 2),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_SETTEXTSIZE, 1),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_WIDTH, 0),
    BRUCE_CFUNC_DEF(FUNC_DISPLAY_HEIGHT, 0),
    BRUCE_CFUNC_DEF(FUNC_GPIO_DIGITALWRITE, 2),
    BRUCE_CFUNC_DEF(FUNC_GPIO_DIGITALREAD, 1),
    BRUCE_CFUNC_DEF(FUNC_GPIO_ANALOGWRITE, 2),
    BRUCE_CFUNC_DEF(FUNC_GPIO_ANALOGREAD, 1),
    BRUCE_CFUNC_DEF(FUNC_GPIO_PINMODE, 2),
    BRUCE_CFUNC_DEF(FUNC_WIFI_SCAN, 0),
    BRUCE_CFUNC_DEF(FUNC_WIFI_CONNECT, 3),
    BRUCE_CFUNC_DEF(FUNC_WIFI_DISCONNECT, 0),
    BRUCE_CFUNC_DEF(FUNC_WIFI_STATUS, 0),
    BRUCE_CFUNC_DEF(FUNC_WIFI_IP, 0),
    BRUCE_CFUNC_DEF(FUNC_HTTP_GET, 1),
    BRUCE_CFUNC_DEF(FUNC_STORAGE_READ, 1),
    BRUCE_CFUNC_DEF(FUNC_STORAGE_WRITE, 3),
    BRUCE_CFUNC_DEF(FUNC_STORAGE_REMOVE, 1),
    BRUCE_CFUNC_DEF(FUNC_STORAGE_EXISTS, 1),
    BRUCE_CFUNC_DEF(FUNC_SUBGHZ_TX, 4),
    BRUCE_CFUNC_DEF(FUNC_BADUSB_RUN, 1),
    BRUCE_CFUNC_DEF(FUNC_DEVICE_BATTERY, 0),
    BRUCE_CFUNC_DEF(FUNC_DEVICE_FREEHEAP, 0),
    BRUCE_CFUNC_DEF(FUNC_DEVICE_BOARD, 0),
    BRUCE_CFUNC_DEF(FUNC_DEVICE_SDK, 0),
    BRUCE_CFUNC_DEF(FUNC_IR_TX, 3),
    BRUCE_CFUNC_DEF(FUNC_IR_SEND, 3),
    BRUCE_CFUNC_DEF(FUNC_KEYBOARD_INPUT, 3),
    BRUCE_CFUNC_DEF(FUNC_KEYBOARD_HEX, 3),
    BRUCE_CFUNC_DEF(FUNC_KEYBOARD_NUM, 3),
    BRUCE_CFUNC_DEF(FUNC_DIALOG_ERROR, 1),
    BRUCE_CFUNC_DEF(FUNC_DIALOG_WARNING, 1),
    BRUCE_CFUNC_DEF(FUNC_DIALOG_INFO, 1),
    BRUCE_CFUNC_DEF(FUNC_DIALOG_SUCCESS, 1),
    BRUCE_CFUNC_DEF(FUNC_COMPAT_GETBOARDNAME, 0),
    BRUCE_CFUNC_DEF(FUNC_COMPAT_GETCPUFREQ, 0),
    BRUCE_CFUNC_DEF(FUNC_COMPAT_GETFREEHEAP, 0),
    BRUCE_CFUNC_DEF(FUNC_COMPAT_GETFLASHCHIPSIZE, 0),
    BRUCE_CFUNC_DEF(FUNC_COMPAT_GETSDKVERSION, 0),
    BRUCE_CFUNC_DEF(FUNC_GPIO_LEDC_ATTACH, 3),
    BRUCE_CFUNC_DEF(FUNC_GPIO_LEDC_WRITE, 2),
    BRUCE_CFUNC_DEF(FUNC_GPIO_LEDC_DETACH, 1)
};

extern "C" const JSSTDLibraryDef bruce_stdlib_def = {
    NULL,
    bruce_func_table,
    NULL,
    0,
    64,
    0,
    0,
    JS_CLASS_USER
};

// delay(ms)
static JSValue func_delay(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int ms = 0;
    if (argc > 0) JS_ToInt32(ctx, &ms, argv[0]);
    delay(ms);
    return JS_UNDEFINED;
}

// millis()
static JSValue func_millis(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, millis());
}

// print(...)
static JSValue func_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    for (int i = 0; i < argc; i++) {
        JSCStringBuf buf;
        const char *s = JS_TO_CSTRING(ctx, argv[i], &buf);
        Serial.print(s);
    }
    return JS_UNDEFINED;
}

// println(...)
static JSValue func_println(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    func_print(ctx, this_val, argc, argv);
    Serial.println();
    return JS_UNDEFINED;
}

// display.fillScreen(color)
static JSValue func_display_fillScreen(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int color = 0;
    if (argc > 0) JS_ToInt32(ctx, &color, argv[0]);
    tft.fillScreen(color);
    return JS_UNDEFINED;
}

// display.drawString(str, x, y)
static JSValue func_display_drawString(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *s = JS_TO_CSTRING(ctx, argv[0], &buf);
    int x = 0, y = 0;
    if (argc > 1) JS_ToInt32(ctx, &x, argv[1]);
    if (argc > 2) JS_ToInt32(ctx, &y, argv[2]);
    tft.drawString(s, x, y);
    return JS_UNDEFINED;
}

// display.drawCentreString(str, x, y, font)
static JSValue func_display_drawCentreString(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *s = JS_TO_CSTRING(ctx, argv[0], &buf);
    int x = 0, y = 0, f = 1;
    if (argc > 1) JS_ToInt32(ctx, &x, argv[1]);
    if (argc > 2) JS_ToInt32(ctx, &y, argv[2]);
    if (argc > 3) JS_ToInt32(ctx, &f, argv[3]);
    tft.drawCentreString(s, x, y, f);
    return JS_UNDEFINED;
}

// display.setTextColor(fg, bg)
static JSValue func_display_setTextColor(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int fg = 0, bg = -1;
    JS_ToInt32(ctx, &fg, argv[0]);
    if (argc > 1) JS_ToInt32(ctx, &bg, argv[1]);
    tft.setTextColor(fg, bg);
    return JS_UNDEFINED;
}

// display.setTextSize(size)
static JSValue func_display_setTextSize(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int s = 1;
    if (argc > 0) JS_ToInt32(ctx, &s, argv[0]);
    tft.setTextSize(s);
    return JS_UNDEFINED;
}

// display.width()
static JSValue func_display_width(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, tftWidth);
}

// display.height()
static JSValue func_display_height(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, tftHeight);
}

// gpio.digitalWrite(pin, value)
static JSValue func_gpio_digitalWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin = 0, val = 0;
    JS_ToInt32(ctx, &pin, argv[0]);
    if (argc > 1) JS_ToInt32(ctx, &val, argv[1]);
    digitalWrite(pin, val);
    return JS_UNDEFINED;
}

// gpio.digitalRead(pin)
static JSValue func_gpio_digitalRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin = 0;
    JS_ToInt32(ctx, &pin, argv[0]);
    return JS_NewInt32(ctx, digitalRead(pin));
}

// gpio.analogWrite(pin, value)
static JSValue func_gpio_analogWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin = 0, val = 0;
    JS_ToInt32(ctx, &pin, argv[0]);
    if (argc > 1) JS_ToInt32(ctx, &val, argv[1]);
    analogWrite(pin, val);
    return JS_UNDEFINED;
}

// gpio.analogRead(pin)
static JSValue func_gpio_analogRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin = 0;
    JS_ToInt32(ctx, &pin, argv[0]);
    // return JS_NewInt32(ctx, analogRead(pin)); // Commented out to avoid ADC conflict
    return JS_NewInt32(ctx, 0);
}

// gpio.pinMode(pin, mode)
static JSValue func_gpio_pinMode(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin = 0, mode = 0;
    JS_ToInt32(ctx, &pin, argv[0]);
    if (argc > 1) JS_ToInt32(ctx, &mode, argv[1]);
    pinMode(pin, mode);
    return JS_UNDEFINED;
}

// WiFi.scan() -> returns array of SSIDs
static JSValue func_wifi_scan(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int n = WiFi.scanNetworks();
    JSValue arr = JS_NewArray(ctx, n);
    for (int i = 0; i < n; i++) {
        JS_SetPropertyUint32(ctx, arr, i, JS_NewString(ctx, WiFi.SSID(i).c_str()));
    }
    return arr;
}

// wifi.connect(ssid, password, timeout)
static JSValue func_wifi_connect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf1, buf2;
    const char *ssid = JS_TO_CSTRING(ctx, argv[0], &buf1);
    const char *pwd = (argc > 1) ? JS_TO_CSTRING(ctx, argv[1], &buf2) : "";
    int timeout = 30;
    if (argc > 2) JS_ToInt32(ctx, &timeout, argv[2]);

    WiFi.begin(ssid, pwd);
    int waited = 0;
    while (WiFi.status() != WL_CONNECTED && waited < timeout) {
        delay(1000);
        waited++;
    }
    return JS_NewBool(WiFi.status() == WL_CONNECTED);
}

// wifi.disconnect()
static JSValue func_wifi_disconnect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    WiFi.disconnect();
    return JS_UNDEFINED;
}

// wifi.status()
static JSValue func_wifi_status(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewBool(WiFi.status() == WL_CONNECTED);
}

// wifi.ip()
static JSValue func_wifi_ip(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    String ip = WiFi.localIP().toString();
    return JS_NewStringLen(ctx, ip.c_str(), ip.length());
}

// http.get(url) -> returns string
static JSValue func_http_get(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *url = JS_TO_CSTRING(ctx, argv[0], &buf);
    if (!url) return JS_EXCEPTION;

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = "";
    if (httpCode > 0) {
        payload = http.getString();
    }
    http.end();
    return JS_NewString(ctx, payload.c_str());
}

// storage.read(path)
static JSValue func_storage_read(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *path = JS_TO_CSTRING(ctx, argv[0], &buf);
    if (!path) return JS_EXCEPTION;

    FS *fs = &LittleFS;
    if (String(path).startsWith("/sd")) fs = &SD;

    if (!fs->exists(path)) return JS_UNDEFINED;
    File f = fs->open(path, "r");
    if (!f) return JS_UNDEFINED;
    String content = f.readString();
    f.close();
    return JS_NewString(ctx, content.c_str());
}

// storage.write(path, content, append)
static JSValue func_storage_write(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf1, buf2;
    const char *path = JS_TO_CSTRING(ctx, argv[0], &buf1);
    const char *content = JS_TO_CSTRING(ctx, argv[1], &buf2);
    bool append = JS_ToBool(ctx, argv[2]);
    if (!path || !content) return JS_EXCEPTION;

    FS *fs = &LittleFS;
    if (String(path).startsWith("/sd")) fs = &SD;

    File f = fs->open(path, append ? "a" : "w");
    if (!f) return JS_FALSE;
    f.print(content);
    f.close();
    return JS_TRUE;
}

// storage.remove(path)
static JSValue func_storage_remove(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *path = JS_TO_CSTRING(ctx, argv[0], &buf);
    if (!path) return JS_EXCEPTION;

    FS *fs = &LittleFS;
    if (String(path).startsWith("/sd")) fs = &SD;

    return fs->remove(path) ? JS_TRUE : JS_FALSE;
}

// storage.exists(path)
static JSValue func_storage_exists(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *path = JS_TO_CSTRING(ctx, argv[0], &buf);
    if (!path) return JS_EXCEPTION;

    FS *fs = &LittleFS;
    if (String(path).startsWith("/sd")) fs = &SD;

    return fs->exists(path) ? JS_TRUE : JS_FALSE;
}

// subghz.tx(data, freq, te, count)
static JSValue func_subghz_tx(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *data = JS_TO_CSTRING(ctx, argv[0], &buf);
    float freq = 433.92;
    int te = 300, count = 10;
    if (argc > 1) {
        int freqInt;
        JS_ToInt32(ctx, &freqInt, argv[1]);
        freq = (float)freqInt;
    }
    if (argc > 2) JS_ToInt32(ctx, &te, argv[2]);
    if (argc > 3) JS_ToInt32(ctx, &count, argv[3]);

    if (String(data).endsWith(".sub")) {
        FS *fs = &LittleFS;
        if (String(data).startsWith("/sd")) fs = &SD;
        return JS_NewBool(txSubFile(fs, data, true));
    } else {
        // Simple RC Switch send simulation/proxy
        RCSwitch_send(strtoull(data, NULL, 16), 24, te, 1, count);
        return JS_TRUE;
    }
}

// badusb.run(filename)
static JSValue func_badusb_run(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *filename = JS_TO_CSTRING(ctx, argv[0], &buf);
    FS *fs = &LittleFS;
    if (String(filename).startsWith("/sd")) fs = &SD;

    HIDInterface *hid = nullptr;
    ducky_startKb(hid, false);
    if (hid) {
        key_input(*fs, filename, hid);
        delete hid;
        return JS_TRUE;
    }
    return JS_FALSE;
}

// device.battery()
static JSValue func_device_battery(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, getBattery());
}

// device.freeHeap()
static JSValue func_device_freeHeap(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, ESP.getFreeHeap());
}

// device.board()
static JSValue func_device_board(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewStringLen(ctx, "ESP32", 5);
}

// device.sdk()
static JSValue func_device_sdk(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewStringLen(ctx, ESP.getSdkVersion(), strlen(ESP.getSdkVersion()));
}

// ir.tx(protocol, address, command)
static JSValue func_ir_tx(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int addr = 0, cmd = 0;
    if (argc > 1) JS_ToInt32(ctx, &addr, argv[1]);
    if (argc > 2) JS_ToInt32(ctx, &cmd, argv[2]);
    ysIrtm.sendNEC(addr, cmd);
    return JS_TRUE;
}

// ir.send(protocol, data, bits)
static JSValue func_ir_send(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int addr = 0, cmd = 0;
    if (argc > 1) JS_ToInt32(ctx, &addr, argv[1]);
    if (argc > 2) JS_ToInt32(ctx, &cmd, argv[2]);
    ysIrtm.sendNEC(addr, cmd);
    return JS_TRUE;
}

// keyboard.input(msg, def, maxLen)
static JSValue func_keyboard_input(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf1, buf2;
    const char *msg = JS_TO_CSTRING(ctx, argv[0], &buf1);
    const char *def = (argc > 1) ? JS_TO_CSTRING(ctx, argv[1], &buf2) : "";
    int maxLen = 76;
    if (argc > 2) JS_ToInt32(ctx, &maxLen, argv[2]);

    String res = keyboard(def, maxLen, msg);
    return JS_NewString(ctx, res.c_str());
}

// keyboard.hex(msg, def, maxLen)
static JSValue func_keyboard_hex(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf1, buf2;
    const char *msg = JS_TO_CSTRING(ctx, argv[0], &buf1);
    const char *def = (argc > 1) ? JS_TO_CSTRING(ctx, argv[1], &buf2) : "";
    int maxLen = 76;
    if (argc > 2) JS_ToInt32(ctx, &maxLen, argv[2]);

    String res = hex_keyboard(def, maxLen, msg);
    return JS_NewString(ctx, res.c_str());
}

// keyboard.num(msg, def, maxLen)
static JSValue func_keyboard_num(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf1, buf2;
    const char *msg = JS_TO_CSTRING(ctx, argv[0], &buf1);
    const char *def = (argc > 1) ? JS_TO_CSTRING(ctx, argv[1], &buf2) : "0";
    int maxLen = 10;
    if (argc > 2) JS_ToInt32(ctx, &maxLen, argv[2]);

    String res = num_keyboard(def, maxLen, msg);
    return JS_NewInt32(ctx, res.toInt());
}

// dialog.error(msg)
static JSValue func_dialog_error(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    displayError(JS_TO_CSTRING(ctx, argv[0], &buf), true);
    return JS_UNDEFINED;
}

// dialog.warning(msg)
static JSValue func_dialog_warning(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    displayWarning(JS_TO_CSTRING(ctx, argv[0], &buf), true);
    return JS_UNDEFINED;
}

// dialog.info(msg)
static JSValue func_dialog_info(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    displayInfo(JS_TO_CSTRING(ctx, argv[0], &buf), true);
    return JS_UNDEFINED;
}

// dialog.success(msg)
static JSValue func_dialog_success(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    displaySuccess(JS_TO_CSTRING(ctx, argv[0], &buf), true);
    return JS_UNDEFINED;
}

// compat.getBoardName()
static JSValue func_compat_getBoardName(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewStringLen(ctx, "ESP32", 5);
}

// compat.getCpuFreqMHz()
static JSValue func_compat_getCpuFreqMHz(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, ESP.getCpuFreqMHz());
}

// compat.getFreeHeap()
static JSValue func_compat_getFreeHeap(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, ESP.getFreeHeap());
}

// compat.getFlashChipSize()
static JSValue func_compat_getFlashChipSize(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt32(ctx, ESP.getFlashChipSize());
}

// compat.getSdkVersion()
static JSValue func_compat_getSdkVersion(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewStringLen(ctx, ESP.getSdkVersion(), strlen(ESP.getSdkVersion()));
}

// gpio.ledcAttach(pin, freq, resolution) - stub
static JSValue func_gpio_ledcAttach(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_UNDEFINED;
}

// gpio.ledcWrite(pin, value) - stub
static JSValue func_gpio_ledcWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_UNDEFINED;
}

// gpio.ledcDetach(pin) - stub
static JSValue func_gpio_ledcDetach(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_UNDEFINED;
}

// Function dispatch table
static JSValue js_bruce_dispatch(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv, int magic) {
    switch (magic) {
        case FUNC_DELAY: return func_delay(ctx, this_val, argc, argv);
        case FUNC_MILLIS: return func_millis(ctx, this_val, argc, argv);
        case FUNC_PRINT: return func_print(ctx, this_val, argc, argv);
        case PRINTLN: return func_println(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_FILLSCREEN: return func_display_fillScreen(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_DRAWSTRING: return func_display_drawString(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_DRAWCENTRE: return func_display_drawCentreString(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_SETTEXTCOLOR: return func_display_setTextColor(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_SETTEXTSIZE: return func_display_setTextSize(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_WIDTH: return func_display_width(ctx, this_val, argc, argv);
        case FUNC_DISPLAY_HEIGHT: return func_display_height(ctx, this_val, argc, argv);
        case FUNC_GPIO_DIGITALWRITE: return func_gpio_digitalWrite(ctx, this_val, argc, argv);
        case FUNC_GPIO_DIGITALREAD: return func_gpio_digitalRead(ctx, this_val, argc, argv);
        case FUNC_GPIO_ANALOGWRITE: return func_gpio_analogWrite(ctx, this_val, argc, argv);
        case FUNC_GPIO_ANALOGREAD: return func_gpio_analogRead(ctx, this_val, argc, argv);
        case FUNC_GPIO_PINMODE: return func_gpio_pinMode(ctx, this_val, argc, argv);
        case FUNC_WIFI_SCAN: return func_wifi_scan(ctx, this_val, argc, argv);
        case FUNC_WIFI_CONNECT: return func_wifi_connect(ctx, this_val, argc, argv);
        case FUNC_WIFI_DISCONNECT: return func_wifi_disconnect(ctx, this_val, argc, argv);
        case FUNC_WIFI_STATUS: return func_wifi_status(ctx, this_val, argc, argv);
        case FUNC_WIFI_IP: return func_wifi_ip(ctx, this_val, argc, argv);
        case FUNC_HTTP_GET: return func_http_get(ctx, this_val, argc, argv);
        case FUNC_STORAGE_READ: return func_storage_read(ctx, this_val, argc, argv);
        case FUNC_STORAGE_WRITE: return func_storage_write(ctx, this_val, argc, argv);
        case FUNC_STORAGE_REMOVE: return func_storage_remove(ctx, this_val, argc, argv);
        case FUNC_STORAGE_EXISTS: return func_storage_exists(ctx, this_val, argc, argv);
        case FUNC_SUBGHZ_TX: return func_subghz_tx(ctx, this_val, argc, argv);
        case FUNC_BADUSB_RUN: return func_badusb_run(ctx, this_val, argc, argv);
        case FUNC_DEVICE_BATTERY: return func_device_battery(ctx, this_val, argc, argv);
        case FUNC_DEVICE_FREEHEAP: return func_device_freeHeap(ctx, this_val, argc, argv);
        case FUNC_DEVICE_BOARD: return func_device_board(ctx, this_val, argc, argv);
        case FUNC_DEVICE_SDK: return func_device_sdk(ctx, this_val, argc, argv);
        case FUNC_IR_TX: return func_ir_tx(ctx, this_val, argc, argv);
        case FUNC_IR_SEND: return func_ir_send(ctx, this_val, argc, argv);
        case FUNC_KEYBOARD_INPUT: return func_keyboard_input(ctx, this_val, argc, argv);
        case FUNC_KEYBOARD_HEX: return func_keyboard_hex(ctx, this_val, argc, argv);
        case FUNC_KEYBOARD_NUM: return func_keyboard_num(ctx, this_val, argc, argv);
        case FUNC_DIALOG_ERROR: return func_dialog_error(ctx, this_val, argc, argv);
        case FUNC_DIALOG_WARNING: return func_dialog_warning(ctx, this_val, argc, argv);
        case FUNC_DIALOG_INFO: return func_dialog_info(ctx, this_val, argc, argv);
        case FUNC_DIALOG_SUCCESS: return func_dialog_success(ctx, this_val, argc, argv);
        case FUNC_COMPAT_GETBOARDNAME: return func_compat_getBoardName(ctx, this_val, argc, argv);
        case FUNC_COMPAT_GETCPUFREQ: return func_compat_getCpuFreqMHz(ctx, this_val, argc, argv);
        case FUNC_COMPAT_GETFREEHEAP: return func_compat_getFreeHeap(ctx, this_val, argc, argv);
        case FUNC_COMPAT_GETFLASHCHIPSIZE: return func_compat_getFlashChipSize(ctx, this_val, argc, argv);
        case FUNC_COMPAT_GETSDKVERSION: return func_compat_getSdkVersion(ctx, this_val, argc, argv);
        case FUNC_GPIO_LEDC_ATTACH: return func_gpio_ledcAttach(ctx, this_val, argc, argv);
        case FUNC_GPIO_LEDC_WRITE: return func_gpio_ledcWrite(ctx, this_val, argc, argv);
        case FUNC_GPIO_LEDC_DETACH: return func_gpio_ledcDetach(ctx, this_val, argc, argv);
        default: return JS_UNDEFINED;
    }
}

// Main init function - registers all Bruce API functions
void js_bruce_init(JSContext *ctx) {
    JSValue global_obj = JS_GetGlobalObject(ctx);

    // bruce namespace
    JSValue bruce = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, bruce, "delay", JS_NewCFunctionParams(ctx, FUNC_DELAY, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, bruce, "millis", JS_NewCFunctionParams(ctx, FUNC_MILLIS, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "bruce", bruce);

    // Print functions
    JS_SetPropertyStr(ctx, global_obj, "print", JS_NewCFunctionParams(ctx, FUNC_PRINT, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "println", JS_NewCFunctionParams(ctx, PRINTLN, JS_UNDEFINED));

    // tft (display) namespace
    JSValue display = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, display, "fillScreen", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_FILLSCREEN, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "drawString", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_DRAWSTRING, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "drawCentreString", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_DRAWCENTRE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "setTextColor", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_SETTEXTCOLOR, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "setTextSize", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_SETTEXTSIZE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "width", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_WIDTH, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, display, "height", JS_NewCFunctionParams(ctx, FUNC_DISPLAY_HEIGHT, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "tft", display);

    // gpio namespace
    JSValue gpio = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, gpio, "digitalWrite", JS_NewCFunctionParams(ctx, FUNC_GPIO_DIGITALWRITE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "digitalRead", JS_NewCFunctionParams(ctx, FUNC_GPIO_DIGITALREAD, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "analogWrite", JS_NewCFunctionParams(ctx, FUNC_GPIO_ANALOGWRITE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "analogRead", JS_NewCFunctionParams(ctx, FUNC_GPIO_ANALOGREAD, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "pinMode", JS_NewCFunctionParams(ctx, FUNC_GPIO_PINMODE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "ledcAttach", JS_NewCFunctionParams(ctx, FUNC_GPIO_LEDC_ATTACH, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "ledcWrite", JS_NewCFunctionParams(ctx, FUNC_GPIO_LEDC_WRITE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, gpio, "ledcDetach", JS_NewCFunctionParams(ctx, FUNC_GPIO_LEDC_DETACH, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "gpio", gpio);

    // WiFi namespace
    JSValue wifi = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, wifi, "scan", JS_NewCFunctionParams(ctx, FUNC_WIFI_SCAN, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, wifi, "connect", JS_NewCFunctionParams(ctx, FUNC_WIFI_CONNECT, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, wifi, "disconnect", JS_NewCFunctionParams(ctx, FUNC_WIFI_DISCONNECT, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, wifi, "status", JS_NewCFunctionParams(ctx, FUNC_WIFI_STATUS, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, wifi, "ip", JS_NewCFunctionParams(ctx, FUNC_WIFI_IP, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "WiFi", wifi);

    // http namespace
    JSValue http = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, http, "get", JS_NewCFunctionParams(ctx, FUNC_HTTP_GET, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "http", http);

    // storage namespace
    JSValue storage = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, storage, "read", JS_NewCFunctionParams(ctx, FUNC_STORAGE_READ, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, storage, "write", JS_NewCFunctionParams(ctx, FUNC_STORAGE_WRITE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, storage, "remove", JS_NewCFunctionParams(ctx, FUNC_STORAGE_REMOVE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, storage, "exists", JS_NewCFunctionParams(ctx, FUNC_STORAGE_EXISTS, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "storage", storage);

    // subghz namespace
    JSValue subghz = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, subghz, "tx", JS_NewCFunctionParams(ctx, FUNC_SUBGHZ_TX, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "subghz", subghz);

    // badusb namespace
    JSValue badusb = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, badusb, "run", JS_NewCFunctionParams(ctx, FUNC_BADUSB_RUN, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "badusb", badusb);

    // device namespace
    JSValue device = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, device, "battery", JS_NewCFunctionParams(ctx, FUNC_DEVICE_BATTERY, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, device, "freeHeap", JS_NewCFunctionParams(ctx, FUNC_DEVICE_FREEHEAP, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, device, "board", JS_NewCFunctionParams(ctx, FUNC_DEVICE_BOARD, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, device, "sdk", JS_NewCFunctionParams(ctx, FUNC_DEVICE_SDK, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "device", device);

    // ir namespace
    JSValue ir = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ir, "tx", JS_NewCFunctionParams(ctx, FUNC_IR_TX, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, ir, "send", JS_NewCFunctionParams(ctx, FUNC_IR_SEND, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "ir", ir);

    // keyboard namespace
    JSValue keyboard = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, keyboard, "input", JS_NewCFunctionParams(ctx, FUNC_KEYBOARD_INPUT, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, keyboard, "hex", JS_NewCFunctionParams(ctx, FUNC_KEYBOARD_HEX, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, keyboard, "num", JS_NewCFunctionParams(ctx, FUNC_KEYBOARD_NUM, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "keyboard", keyboard);

    // dialog namespace
    JSValue dialog = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, dialog, "error", JS_NewCFunctionParams(ctx, FUNC_DIALOG_ERROR, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, dialog, "warning", JS_NewCFunctionParams(ctx, FUNC_DIALOG_WARNING, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, dialog, "info", JS_NewCFunctionParams(ctx, FUNC_DIALOG_INFO, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, dialog, "success", JS_NewCFunctionParams(ctx, FUNC_DIALOG_SUCCESS, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "dialog", dialog);

    // compat namespace
    JSValue compat = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, compat, "getBoardName", JS_NewCFunctionParams(ctx, FUNC_COMPAT_GETBOARDNAME, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, compat, "getCpuFreqMHz", JS_NewCFunctionParams(ctx, FUNC_COMPAT_GETCPUFREQ, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, compat, "getFreeHeap", JS_NewCFunctionParams(ctx, FUNC_COMPAT_GETFREEHEAP, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, compat, "getFlashChipSize", JS_NewCFunctionParams(ctx, FUNC_COMPAT_GETFLASHCHIPSIZE, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, compat, "getSdkVersion", JS_NewCFunctionParams(ctx, FUNC_COMPAT_GETSDKVERSION, JS_UNDEFINED));
    JS_SetPropertyStr(ctx, global_obj, "compat", compat);

    // Check for exceptions after init
    if (JS_IsException(global_obj)) {
        JSValue ex = JS_GetException(ctx);
        JSCStringBuf buf;
        const char *err = JS_TO_CSTRING(ctx, ex, &buf);
        Serial.printf("JS Init Error: %s\n", err ? err : "Unknown");
    }
}
