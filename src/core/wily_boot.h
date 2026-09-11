#ifndef __WILLY_BOOT_H__
#define __WILLY_BOOT_H__

#include <Arduino.h>

// ============================================================================
// Willy Boot System — Cyberpunk Startup with Hardware Verification
// ============================================================================

// Boot the system with visual hardware checks
// Call this in setup() after TFT init + begin_storage() + begin_tft()
// Returns true if all checks passed, false if some failed (boot continues anyway)
bool wilyBoot();

#endif // __WILLY_BOOT_H__
