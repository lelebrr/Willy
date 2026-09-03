#ifndef __MQJS_NATIVE_DECL_H__
#define __MQJS_NATIVE_DECL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <mquickjs.h>

/* Forward declarations of all native functions to satisfy both the host generator tool
   and the target firmware without pulling in ESP32-specific headers on the host. */

/* Math */
JSValue native_math_acosh(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_math_asinh(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_math_atanh(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_math_is_equal(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Globals / Timers */
JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_load(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_setTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_clearTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_setInterval(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_clearInterval(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
void run_timers(JSContext *ctx);
void native_timers_state_finalizer(JSContext *ctx, void *opaque);
JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_assert(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_require(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_delay(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_random(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_parse_int(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_to_string(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_to_hex_string(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_to_lower_case(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_to_upper_case(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_exit(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Audio */
JSValue native_playAudioFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_tone(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* BadUSB */
JSValue native_badusbSetup(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbPrint(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbPrintln(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbHold(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbRelease(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbReleaseAll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbPressRaw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_badusbRunFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Device */
JSValue native_getDeviceName(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getBoard(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getWillyVersion(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getBattery(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getBatteryDetailed(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getFreeHeapSize(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getEEPROMSize(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Display / Sprite */
JSValue native_color(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_fillScreen(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setCursor(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_println(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setTextColor(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setTextSize(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setTextAlign(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawString(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawPixel(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawWideLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFastVLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFastHLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawRect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFillRect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFillRectGradient(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawRoundRect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFillRoundRect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawTriangle(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFillTriangle(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawCircle(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawFillCircle(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawArc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawXBitmap(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawJpg(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawGif(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_gifOpen(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_gifPlayFrame(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_gifDimensions(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_gifReset(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_gifClose(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_width(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_height(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_createSprite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getRotation(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getBrightness(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setBrightness(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_restoreBrightness(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_pushSprite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_deleteSprite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
void native_sprite_finalizer(JSContext *ctx, void *opaque);
void native_gif_finalizer(JSContext *ctx, void *opaque);

/* Dialog / TextViewer */
JSValue native_dialogMessage(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogInfo(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogSuccess(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogWarning(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogError(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogChoice(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_keyboard(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogPickFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogViewFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogViewText(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewer(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_drawStatusBar(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerDraw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerScrollUp(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerScrollDown(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerScrollToLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerGetLine(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerGetMaxLines(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerGetVisibleText(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerClear(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerFromString(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dialogCreateTextViewerClose(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
void native_textviewer_finalizer(JSContext *ctx, void *opaque);

/* GPIO */
JSValue native_pinMode(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_digitalRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_analogRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_touchRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_digitalWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_analogWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_dacWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_ledcSetup(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_ledcAttachPin(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_ledcWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_pins(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* I2C */
JSValue native_i2c_begin(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_i2c_scan(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_i2c_write(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_i2c_read(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_i2c_write_read(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* IR */
JSValue native_irRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_irReadRaw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_irTransmitFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_irTransmit(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Keyboard */
JSValue native_num_keyboard(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_hex_keyboard(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getKeysPressed(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getPrevPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getSelPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getEscPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getNextPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_getAnyPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_setLongPress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Notification */
JSValue native_notifyBlink(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Mic */
JSValue native_micRecordWav(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Runtime */
JSValue native_runtimeToBackground(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_runtimeToForeground(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_runtimeIsForeground(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_runtimeMain(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Serial */
JSValue native_serialPrint(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_serialPrintln(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_serialReadln(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_serialCmd(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Storage */
JSValue native_storageReaddir(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageRename(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageRemove(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageMkdir(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageRmdir(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageSpaceLittleFS(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_storageSpaceSDCard(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* SubGHz */
JSValue native_subghzTransmitFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_subghzTransmit(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_subghzRead(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_subghzReadRaw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_subghzSetFrequency(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* WiFi */
JSValue native_wifiConnected(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_wifiConnectDialog(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_wifiConnect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_wifiScan(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_wifiDisconnect(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_httpFetch(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_wifiMACAddress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
JSValue native_ipAddress(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Additional functions needed by quickjs itself if not defined */
JSValue js_function_bound(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv, JSValue params);

#ifdef __cplusplus
}
#endif

#endif
