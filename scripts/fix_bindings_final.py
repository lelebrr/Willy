import re
with open('src/modules/bjs_interpreter/mqjs_bindings.cpp', 'r') as f:
    code = f.read()

# 1. Add cstddef
if '<cstddef>' not in code:
    code = code.replace('#include "mquickjs.h"', '#include <cstddef>\n#include "mquickjs.h"')

# 2. JS_NewArray(ctx) -> JS_NewArray(ctx, 0)
code = code.replace('JS_NewArray(ctx)', 'JS_NewArray(ctx, 0)')

# 3. JS_ToCString for conditional parameters
code = re.sub(r'const char \*pwd = argc > 1 \? JS_ToCString\(ctx, argv\[1\]\) : NULL;',
              'JSCStringBuf buf_pwd;\n    const char *pwd = argc > 1 ? JS_ToCString(ctx, argv[1], &buf_pwd) : NULL;', code)

code = re.sub(r'const char \*mode = argc > 2 \? JS_ToCString\(ctx, argv\[2\]\) : FILE_WRITE;',
              'JSCStringBuf buf_mode;\n    const char *mode = argc > 2 ? JS_ToCString(ctx, argv[2], &buf_mode) : FILE_WRITE;', code)

code = re.sub(r'const char \*msg = argc > 0 \? JS_ToCString\(ctx, argv\[0\]\) : \"([^\"]+)\";\s*const char \*def = argc > 1 \? JS_ToCString\(ctx, argv\[1\]\) : \"([^\"]*)\";',
              lambda m: f'JSCStringBuf buf_msg;\n    const char *msg = argc > 0 ? JS_ToCString(ctx, argv[0], &buf_msg) : "{m.group(1)}";\n    JSCStringBuf buf_def;\n    const char *def = argc > 1 ? JS_ToCString(ctx, argv[1], &buf_def) : "{m.group(2)}";', code)

# fix remaining bad JS_ToCString in js_dialog_choice
code = code.replace('const char *valStr = JS_ToCString(ctx, val, &cbuf_val, &buf_val_Str);', 'const char *valStr = JS_ToCString(ctx, val, &cbuf_val);')
code = code.replace('const char *valStr = JS_ToCString(ctx, val, &cbuf_val, &buf_valStr);', 'const char *valStr = JS_ToCString(ctx, val, &cbuf_val);')
code = code.replace('uint count = 0;', 'uint32_t count = 0;')
code = code.replace('uint i = 0;; i++', 'uint32_t i = 0;; i++')
code = code.replace('JSCStringBuf cbuf_val;\n        JSCStringBuf buf_valStr;\n', 'JSCStringBuf cbuf_val;\n')

# JS_ToFloat64 -> JS_ToNumber
code = code.replace('JS_ToFloat64(ctx, &f, argv[0]);', 'JS_ToNumber(ctx, &f, argv[0]);')

# sendDecodedCommand
code = code.replace('sendDecodedCommand(proto, data, bits)', 'sendDecodedCommand(String(proto), String(data), (uint16_t)bits)')

# missing js_gpio
missing_funcs = """
static JSValue js_gpio_digitalWrite(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin, val;
    JS_ToInt32(ctx, &pin, argv[0]);
    JS_ToInt32(ctx, &val, argv[1]);
    digitalWrite(pin, val);
    return JS_UNDEFINED;
}

static JSValue js_gpio_pinMode(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int pin, mode;
    JS_ToInt32(ctx, &pin, argv[0]);
    JS_ToInt32(ctx, &mode, argv[1]);
    pinMode(pin, mode);
    return JS_UNDEFINED;
}
"""
if 'js_gpio_digitalWrite' not in code[:code.find('bruce_c_functions')]:
    code = code.replace('// --- WiFi ---', missing_funcs + '\n// --- WiFi ---')

with open('src/modules/bjs_interpreter/mqjs_bindings.cpp', 'w') as f:
    f.write(code)

print('Patched successfully!')
