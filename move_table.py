import re
with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "r") as f:
    code = f.read()

# Extract the enum and array.
enum_match = re.search(r'enum \{[\s\S]*?CFUNC_COUNT\n\};\n\n', code)
if enum_match:
    enum_str = enum_match.group(0)
    code = code.replace(enum_str, '')

array_match = re.search(r'const JSCFunctionDef bruce_c_functions\[CFUNC_COUNT\] = \{[\s\S]*?\};\n\n', code)
if array_match:
    array_str = array_match.group(0)
    code = code.replace(array_str, '')

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

if enum_match and array_match:
    insert_str = enum_str + array_str + stdlib_def_str + '\n'
    code = code.replace('void js_bruce_init(JSContext *ctx) {', insert_str + 'void js_bruce_init(JSContext *ctx) {')

    with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "w") as f:
        f.write(code)
    print("Moved array down.")
else:
    print("Could not find enum or array.")
