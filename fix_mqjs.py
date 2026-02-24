import re
with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "r") as f:
    code = f.read()

# fix JSValueConst
code = code.replace("JSValueConst", "JSValue")

# find all JS_NewCFunction(...)
matches = re.findall(r'JS_NewCFunction\s*\(\s*ctx\s*,\s*([a-zA-Z0-9_]+)\s*,\s*"[^"]+"\s*,\s*(\d+)\s*\)', code)

# matches is a list of (func_name, args_count)
# We need to build the enum and the JSCFunctionDef array.
funcs = list(dict.fromkeys(matches)) # unique

enum_str = "enum {\n"
for f, _ in funcs:
    enum_str += f"    IDX_{f},\n"
enum_str += "    CFUNC_COUNT\n"
enum_str += "};\n\n"

array_str = "const JSCFunctionDef bruce_c_functions[CFUNC_COUNT] = {\n"
for f, nargs in funcs:
    array_str += f"    [IDX_{f}] = {{ {{ .generic = {f} }}, {{0}}, JS_CFUNC_generic, {nargs}, 0 }},\n"
array_str += "};\n\n"

# replace JS_NewCFunction
code = re.sub(r'JS_NewCFunction\s*\(\s*ctx\s*,\s*([a-zA-Z0-9_]+)\s*,\s*"[^"]+"\s*,\s*\d+\s*\)', r'JS_NewCFunctionParams(ctx, IDX_\1, JS_UNDEFINED)', code)

# generate bruce_stdlib_def
stdlib_def_str = """
JSSTDLibraryDef bruce_stdlib_def = {
    .stdlib_table = NULL,
    .c_function_table = bruce_c_functions,
    .c_finalizer_table = NULL,
    .stdlib_table_len = 0,
    .stdlib_table_align = 0,
    .sorted_atoms_offset = 0,
    .global_object_offset = 0,
    .class_count = 0
};
"""

# inject array_str before `void js_bruce_init`
code = code.replace('void js_bruce_init(JSContext *ctx) {', enum_str + array_str + stdlib_def_str + '\nvoid js_bruce_init(JSContext *ctx) {')

# One more fix: JS_FreeValue and JS_DupValue are not in mquickjs.
code = code.replace('JS_FreeValue(ctx, global_obj);', '')
code = code.replace('JS_FreeValue(ctx, val);', '')
code = code.replace('JS_DupValue(ctx, display)', 'display')

with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "w") as f:
    f.write(code)

print("Rewrote mqjs_bindings.cpp")
