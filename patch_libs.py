
import os
import glob

from typing import List, Tuple

def patch_file(filepath: str, replacements: List[Tuple[str, str]]) -> None:
    """Replaces strings in a file based on a list of tuples (old_str, new_str)"""
    if not os.path.isfile(filepath):
        print(f"Patch Script: File not found: {filepath}")
        return

    with open(filepath, 'r', encoding='utf-8') as f:
        content: str = str(f.read())

    modified: bool = False
    for r in replacements:
        old_str: str = str(r[0])
        new_str: str = str(r[1])
        if old_str in content:
            content = content.replace(old_str, new_str)
            modified = True

    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Patch Script: Patched {filepath}")

# 1. Patch mquickjs.c
patch_file(".pio/libdeps/CYD-3248S035C/mquickjs/mquickjs.c", [
    ('buf = "null";', 'buf = (char *)"null";'),
    ('buf = "undefined";', 'buf = (char *)"undefined";'),
    ('buf = "uninitialized";', 'buf = (char *)"uninitialized";'),
    ('buf = "[tag]";', 'buf = (char *)"[tag]";'),
    ('buf = "[short_float]";', 'buf = (char *)"[short_float]";'),
    ('buf = "[mtag]";', 'buf = (char *)"[mtag]";'),
    ('buf = JS_VALUE_GET_SPECIAL_VALUE(val) ? "true" : "false";', 'buf = JS_VALUE_GET_SPECIAL_VALUE(val) ? (char *)"true" : (char *)"false";')
])

# 2. Patch IRrecv.cpp
patch_file(".pio/libdeps/CYD-3248S035C/IRremoteESP8266/src/IRrecv.cpp", [
    ("  }\n  params.rawlen++;", "  }\n  params.rawlen = params.rawlen + 1;")
])

# 3. Patch chameleonUltra.cpp
patch_file(".pio/libdeps/CYD-3248S035C/ESP Chameleon Ultra/src/chameleonUltra.cpp", [
    ("uint8_t cmd[3] = {slot-1, freq, 0x01};", "uint8_t cmd[3] = {static_cast<uint8_t>(slot-1), freq, 0x01};"),
    ("uint8_t cmd[1] = {slot-1};", "uint8_t cmd[1] = {static_cast<uint8_t>(slot-1)};"),
    ("uint8_t cmd[3] = {slot-1, (tagType >> 8) & 0xFF, tagType & 0xFF};", "uint8_t cmd[3] = {static_cast<uint8_t>(slot-1), static_cast<uint8_t>((tagType >> 8) & 0xFF), static_cast<uint8_t>(tagType & 0xFF)};"),
    ("uint8_t cmd[length + 5] = {length};", "uint8_t cmd[length + 5] = {static_cast<uint8_t>(length)};")
])

# 4. Patch libssh_esp32_compat.c
patch_file(".pio/libdeps/CYD-3248S035C/LibSSH-ESP32/src/libssh_esp32_compat.c", [
    ('static struct passwd p =\n  { LIBSSH_ESP32_COMPAT_USERNAME, /* password: */"", LIBSSH_ESP32_COMPAT_UID,\n    LIBSSH_ESP32_COMPAT_GID, /* comment: */"", /* gecos: */"",\n    LIBSSH_ESP32_COMPAT_HOMEDIR, /* shell: */"" };',
     'static struct passwd p =\n  { (char *)LIBSSH_ESP32_COMPAT_USERNAME, /* password: */(char *)"", LIBSSH_ESP32_COMPAT_UID,\n    LIBSSH_ESP32_COMPAT_GID, /* comment: */(char *)"", /* gecos: */(char *)"",\n    (char *)LIBSSH_ESP32_COMPAT_HOMEDIR, /* shell: */(char *)"" };')
])

# 5. Patch OneWire.cpp
patch_file(".pio/libdeps/CYD-3248S035C/OneWire/OneWire.cpp", [
    ('#  undef noInterrupts() {portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;portENTER_CRITICAL(&mux)', '#  undef noInterrupts'),
    ('#  undef interrupts() portEXIT_CRITICAL(&mux);}', '#  undef interrupts')
])

# 6. Patch pfs.c
patch_file(".pio/libdeps/CYD-3248S035C/ESP32-PSRamFS/src/pfs.c", [
    ('#warning "Will use PSRAM or heap"', '// #warning "Will use PSRAM or heap"'),
    ('#warning "No SPIRAM detected, will use heap"', '// #warning "No SPIRAM detected, will use heap"')
])
