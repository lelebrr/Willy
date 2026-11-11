import re

with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "r") as f:
    code = f.read()

# 1. Replace int32_t with int
code = code.replace("int32_t ", "int ")

# 2. Fix JS_ToCString
def replace_tocstring(match):
    var_name = match.group(1)
    ctx = match.group(2)
    arg = match.group(3)
    return f"JSCStringBuf buf_{var_name};\n    const char *{var_name} = JS_ToCString({ctx}, {arg}, &buf_{var_name});"

code = re.sub(r'const\s+char\s*\*\s*([a-zA-Z0-9_]+)\s*=\s*JS_ToCString\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\);', replace_tocstring, code)

# 3. Remove JS_FreeCString
code = re.sub(r'\s*JS_FreeCString\s*\([^)]+\);', '', code)

# 4. JS_GetProperty names
def fix_JS_GetProperty(match):
    return match.group(0).replace('tab[i].atom', 'tab[i].atom') # NO-OP for JS_GetProperty

code = re.sub(r'JS_GetProperty\s*\(\s*ctx\s*,\s*argv\[0\]\s*,\s*tab\[i\]\.atom\s*\);', fix_JS_GetProperty, code)
# wait, actually JS_AtomToCString also changed? Let's check.
# Let's see if JS_AtomToCString needs buf in mquickjs.h

with open("src/modules/bjs_interpreter/mqjs_bindings.cpp", "w") as f:
    f.write(code)

print("Signatures fixed.")
