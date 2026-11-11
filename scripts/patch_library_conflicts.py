import os
import re
import glob

"""
Corrige conflitos entre as libraries externas (baixadas de branches main/master,
que sao "moving targets") e o core Arduino ESP32.

Estas libraries foram atualizadas no upstream depois do ultimo build e passaram
a usar tipos/funcoes do Arduino sem incluir <Arduino.h> diretamente:

  - ESP Chameleon Ultra: usa `byte` (typedef de Arduino.h) sem incluir Arduino.h
  - ESP PN32BLE: usa std::reverse sem incluir <algorithm>
  - ESP Amiibolink: idem

O script e idempotente: pode rodar em todo build.
"""

print("Patching library conflicts with ESP32 core...")

# Lista de conflitos: (file_pattern, search_pattern, replace_pattern)
conflicts = [
    (
        ".pio/libdeps/*/ESP Chameleon Ultra/src/chameleonUltra.h",
        r'#include <NimBLEDevice\.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP PN32BLE/src/pn532_ble.h",
        r'#include <NimBLEDevice\.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP PN32BLE/src/pn532_ble.cpp",
        r'#include <pn532_ble\.h>',
        '#include <algorithm>\n#include <pn532_ble.h>'
    ),
    (
        ".pio/libdeps/*/ESP Amiibolink/src/amiibolink.h",
        r'#include <NimBLEDevice\.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP PN532Killer/src/PN532Killer.h",
        r'#include <NimBLEDevice\.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP8266SAM/src/render.c",
        r'static void yield\(\)',
        'static void sam_yield()'
    ),
    (
        ".pio/libdeps/*/ESP8266SAM/src/render.c",
        r'\byield\(\);',
        'sam_yield();'
    ),
    (
        ".pio/libdeps/*/JPEGDecoder/src/picojpeg.c",
        r'static uint8 init\(void\)',
        'static uint8 picojpeg_init(void)'
    ),
    (
        ".pio/libdeps/*/JPEGDecoder/src/picojpeg.c",
        r'\binit\(\);',
        'picojpeg_init();'
    ),
]

for file_pattern, search, replace in conflicts:
    files = glob.glob(file_pattern)
    for file_path in files:
        try:
            with open(file_path, 'r') as f:
                content = f.read()

            if replace in content:
                print(f"Already patched: {file_path}")
                continue

            new_content = re.sub(search, replace, content, count=1)
            if new_content != content:
                with open(file_path, 'w') as f:
                    f.write(new_content)
                print(f"Patched: {file_path}")
        except Exception as e:
            print(f"Failed to patch {file_path}: {e}")

print("Library conflict patching done.")
