/**
 * @file esp32-hal-periman.h
 * @brief Stub header for ESP32 Peripheral Manager
 *
 * This is a compatibility stub for Arduino ESP32 framework 2.0.x
 * The peripheral manager was introduced in Arduino ESP32 3.x
 *
 * This stub provides dummy implementations to allow compilation
 * with older framework versions.
 */

#ifndef ESP32_HAL_PERIMAN_H
#define ESP32_HAL_PERIMAN_H

#include <stdbool.h>
#include <stdint.h>

// Bus types for peripheral manager
#define ESP32_BUS_TYPE_SDMMC_CLK 0
#define ESP32_BUS_TYPE_SDMMC_CMD 1
#define ESP32_BUS_TYPE_SDMMC_D0 2
#define ESP32_BUS_TYPE_SDMMC_D1 3
#define ESP32_BUS_TYPE_SDMMC_D2 4
#define ESP32_BUS_TYPE_SDMMC_D3 5
#define ESP32_BUS_TYPE_ETHERNET_SPI 6

// Stub function declarations - these are no-op stubs
// In Arduino 3.x, these manage pin bus ownership for peripheral remapping

static inline bool perimanSetPinBus(int pin, int busType, void *owner,
                                    int8_t hostId, int8_t busIndex) {
  (void)pin;
  (void)busType;
  (void)owner;
  (void)hostId;
  (void)busIndex;
  return true;
}

static inline bool perimanClearPinBus(int pin) {
  (void)pin;
  return true;
}

// Accept function pointer as void* for compatibility
typedef bool (*perimanDeinitFunc_t)(void *);

static inline bool perimanSetBusDeinit(int busType,
                                       perimanDeinitFunc_t deinitFunc) {
  (void)busType;
  (void)deinitFunc;
  return true;
}

static inline bool perimanSetPinBusExtraType(int pin, const char *extraType) {
  (void)pin;
  (void)extraType;
  return true;
}

static inline void *perimanGetPinBus(int pin) {
  (void)pin;
  return NULL;
}

#endif // ESP32_HAL_PERIMAN_H