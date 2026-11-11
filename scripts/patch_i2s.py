"""
Pre-build script: Patches ESP8266Audio library to disable legacy I2S driver files.

The ESP8266Audio library (v1.9.7) uses the legacy I2S driver (driver/i2s.h) in
AudioOutputI2S.cpp, AudioOutputI2SNoDAC.cpp, and AudioOutputSPDIF.cpp.
This conflicts with the new I2S driver (driver/i2s_std.h) used by mic.cpp.

This script wraps the entire content of those files in #if 0 / #endif to prevent
the legacy driver from being compiled and linked, avoiding the runtime conflict:
  E (703) i2s(legacy): CONFLICT! The new i2s driver can't work along with the legacy i2s driver
"""

import os
from os.path import isfile, join

Import("env")

PIOENV = env.get("PIOENV", "")
LIBDEPS_DIR = join(env.get("PROJECT_DIR"), ".pio", "libdeps", PIOENV, "ESP8266Audio", "src")

# Files that use the legacy I2S driver and must be disabled
FILES_TO_PATCH = [
    "AudioOutputI2S.cpp",
    "AudioOutputI2SNoDAC.cpp",
    "AudioOutputSPDIF.cpp",
]

PATCH_MARKER = "// PATCHED_BY_WILLY: Legacy I2S disabled to avoid conflict with new I2S driver"


def patch_file(filepath):
    """Wrap file content in #if 0 to disable legacy I2S code."""
    if not isfile(filepath):
        print(f"[patch_i2s] File not found, skipping: {filepath}")
        return

    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Already patched?
    if PATCH_MARKER in content:
        return

    print(f"[patch_i2s] Patching: {filepath}")

    patched = f"{PATCH_MARKER}\n#if 0\n{content}\n#endif\n"

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(patched)


for fname in FILES_TO_PATCH:
    patch_file(join(LIBDEPS_DIR, fname))
