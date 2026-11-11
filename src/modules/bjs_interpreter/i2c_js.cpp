#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "i2c_js.h"

#include "helpers_js.h"
#include <Wire.h>

static void i2c_require_ready(JSContext *ctx) {
  JSValue global = JS_GetGlobalObject(ctx);
  JSValue v = JS_GetPropertyStr(ctx, global, "\xffi2c_ready");
  bool ready = JS_IsBool(v) ? JS_ToBool(ctx, v) : false;
  if (!ready)
    JS_ThrowInternalError(
        ctx, "i2c not initialized: call i2c.begin(sda,scl,hz) first");
}

JSValue native_i2c_begin(JSContext *ctx, JSValue *this_val, int argc,
                         JSValue *argv) {
  if (argc < 3 || !JS_IsNumber(ctx, argv[0]) || !JS_IsNumber(ctx, argv[1]) ||
      !JS_IsNumber(ctx, argv[2])) {
    return JS_ThrowTypeError(ctx,
                             "i2c.begin(sda:int, scl:int, hz:int) is required");
  }
  int sda = 0, scl = 0;
  uint32_t hz = 0;
  JS_ToInt32(ctx, &sda, argv[0]);
  JS_ToInt32(ctx, &scl, argv[1]);
  JS_ToUint32(ctx, &hz, argv[2]);
  if (sda < 0 || scl < 0)
    return JS_ThrowRangeError(ctx, "i2c.begin: pins must be >= 0");
  if (hz < 1000 || hz > 1000000)
    return JS_ThrowRangeError(ctx, "i2c.begin: hz must be 1k..1MHz");

  Wire.begin(sda, scl, hz);
  Wire.setClock(hz);
  Wire.setTimeOut(50);

  JSValue global = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global, "\xffi2c_ready", JS_NewBool(true));
  return JS_NewBool(true);
}

JSValue native_i2c_scan(JSContext *ctx, JSValue *this_val, int argc,
                        JSValue *argv) {
  i2c_require_ready(ctx);
  JSValue arr = JS_NewArray(ctx, 0);
  uint32_t idx = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission(true) == 0) {
      JS_SetPropertyUint32(ctx, arr, idx++, JS_NewInt32(ctx, a));
    }
  }
  return arr;
}

JSValue native_i2c_end(JSContext *ctx, JSValue *this_val, int argc) {
  i2c_require_ready(ctx);
  Wire.end();
  JSValue global = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global, "\xffi2c_ready", JS_NewBool(false));
  return JS_UNDEFINED;
}

JSValue native_i2c_write(JSContext *ctx, JSValue *this_val, int argc,
                         JSValue *argv) {
  i2c_require_ready(ctx);
  int addr = 0;
  int len = 0;
  bool sendStop = true;
  if (argc > 0 && JS_IsNumber(ctx, argv[0]))
    JS_ToInt32(ctx, &addr, argv[0]);
  size_t tmp_len = 0;
  const char *tmp_buf = NULL;
  if (argc > 1)
    tmp_buf = JS_ToCStringLen(ctx, &tmp_len, argv[1], NULL);
  len = (int)tmp_len;
  if (argc > 2)
    sendStop = JS_ToBool(ctx, argv[2]);

  Wire.beginTransmission((uint8_t)addr);
  if (tmp_buf)
    Wire.write((const uint8_t *)tmp_buf, (size_t)len);
  uint8_t err = Wire.endTransmission(sendStop);
  return JS_NewInt32(ctx, err);
}

JSValue native_i2c_read(JSContext *ctx, JSValue *this_val, int argc,
                        JSValue *argv) {
  i2c_require_ready(ctx);
  int addr = 0;
  int len = 0;
  if (argc > 0 && JS_IsNumber(ctx, argv[0]))
    JS_ToInt32(ctx, &addr, argv[0]);
  if (argc > 1 && JS_IsNumber(ctx, argv[1]))
    JS_ToInt32(ctx, &len, argv[1]);
  size_t got = Wire.requestFrom((uint8_t)addr, (uint8_t)len, (uint8_t)true);

  // Create JS array from the buffer data
  JSValue arr = JS_NewArray(ctx, 0);
  for (size_t i = 0; i < got; i++) {
    JS_SetPropertyUint32(ctx, arr, i, JS_NewInt32(ctx, Wire.read()));
  }
  return arr;
}

JSValue native_i2c_write_read(JSContext *ctx, JSValue *this_val, int argc,
                              JSValue *argv) {
  i2c_require_ready(ctx);
  int addr = 0;
  if (argc > 0 && JS_IsNumber(ctx, argv[0]))
    JS_ToInt32(ctx, &addr, argv[0]);

  const char *wbuf = NULL;
  size_t wlen = 0;
  if (argc < 3)
    return JS_ThrowTypeError(ctx, "i2c.writeRead: arg1 must be string");

  if (JS_IsString(ctx, argv[1])) {
    JSCStringBuf sb;
    wbuf = JS_ToCStringLen(ctx, &wlen, argv[1], &sb);
    if (!wbuf)
      return JS_ThrowTypeError(ctx, "i2c.writeRead: invalid string argument");
  } else {
    return JS_ThrowTypeError(ctx, "i2c.writeRead: arg1 must be string");
  }

  int rlen = 0;
  if (JS_IsNumber(ctx, argv[2]))
    JS_ToInt32(ctx, &rlen, argv[2]);
  int wait = 0;
  if (argc > 3 && JS_IsNumber(ctx, argv[3]))
    JS_ToInt32(ctx, &wait, argv[3]);

  Wire.beginTransmission((uint8_t)addr);
  Wire.write((const uint8_t *)wbuf, (size_t)wlen);
  uint8_t err = Wire.endTransmission(false);
  if (err != 0) {
    return JS_NewInt32(ctx, -err);
  }
  if (wait > 0)
    delay(wait);
  size_t got = Wire.requestFrom((uint8_t)addr, (uint8_t)rlen, (uint8_t)true);

  // Create JS array from the buffer data
  JSValue arr = JS_NewArray(ctx, 0);
  for (size_t i = 0; i < got; i++) {
    JS_SetPropertyUint32(ctx, arr, i, JS_NewInt32(ctx, Wire.read()));
  }
  return arr;
}

#endif
