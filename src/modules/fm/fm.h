#ifndef __FM_H__
#define __FM_H__

#ifndef LITE_VERSION

// Verificar se a biblioteca Adafruit_Si4713 está disponível
#ifdef HAS_ADAFRUIT_SI4713

#include "core/display.h"
#include "core/mykeyboard.h"
#include <Adafruit_Si4713.h>
#include <globals.h>

void fm_live_run(bool reserved = true);
void fm_ta_run();
bool fm_begin();
bool fm_setup(bool traffic_alert = false, bool silent = false);
void fm_stop();
void fm_spectrum();

#else // HAS_ADAFRUIT_SI4713

// Stubs para quando a biblioteca não estiver disponível
static inline void fm_live_run(bool reserved = true) {}
static inline void fm_ta_run() {}
static inline bool fm_begin() { return false; }
static inline bool fm_setup(bool traffic_alert = false, bool silent = false) {
  return false;
}
static inline void fm_stop() {}
static inline void fm_spectrum() {}

#endif // HAS_ADAFRUIT_SI4713

#endif // LITE_VERSION

#endif // __FM_H__
