#ifndef __HELPERS_JS_H__
#define __HELPERS_JS_H__
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "core/willy_serial_cmds.h"
#include <FS.h>

extern "C" {
#include <mquickjs.h>

static inline int JS_IsObject(JSContext *ctx, JSValue val) {
  return JS_GetClassID(ctx, val) >= 0;
}

static inline int JS_ToBool(JSContext *ctx, JSValue val) {
  if (JS_IsBool(val))
    return JS_VALUE_GET_SPECIAL_VALUE(val);
  if (JS_IsInt(val))
    return JS_VALUE_GET_INT(val) != 0;
  if (JS_IsNull(val) || JS_IsUndefined(val))
    return 0;
  return 1;
}

static inline const char *JS_GetTypedArrayBuffer(JSContext *ctx, size_t *plen,
                                                 JSValue obj) {
  if (plen)
    *plen = 0;
  return NULL;
}

static inline const char *JS_GetOwnPropertyByIndex(JSContext *ctx,
                                                   uint32_t index,
                                                   uint32_t *prop_count,
                                                   JSValue obj) {
  return NULL;
}

static inline JSValue JS_NewUint8ArrayCopy(JSContext *ctx, const uint8_t *buf,
                                           size_t len) {
  return JS_NewStringLen(ctx, (const char *)buf, len);
}
}

#include <globals.h>
#include <string.h>

extern "C" {
void js_fatal_error_handler(JSContext *ctx);
bool JS_IsTypedArray(JSContext *ctx, JSValue val);
}

struct FileParamsJS {
  FS *fs;
  String path;
  bool exist;
  u_int8_t paramOffset;
};
FileParamsJS js_get_path_from_params(JSContext *ctx, JSValue *argv,
                                     bool checkIfexist = true,
                                     bool legacy = false);

JSValue js_value_from_json_variant(JSContext *ctx, JsonVariantConst value);

void internal_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv,
                    uint8_t printTft, uint8_t newLine);

#endif
#endif
