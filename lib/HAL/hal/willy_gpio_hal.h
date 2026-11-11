#ifndef WILLY_GPIO_HAL_H
#define WILLY_GPIO_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "soc/gpio_reg.h"
#include <stdint.h>

// GPIO HAL interface for ESP32-S3
typedef enum {
  HAL_GPIO_MODE_INPUT = 0,
  HAL_GPIO_MODE_OUTPUT,
  HAL_GPIO_MODE_INPUT_OUTPUT,
  HAL_GPIO_MODE_INPUT_OUTPUT_OD
} hal_gpio_mode_t;

typedef enum {
  HAL_GPIO_PULLUP_DISABLE = 0,
  HAL_GPIO_PULLUP_ENABLE,
  HAL_GPIO_PULLDOWN_ENABLE,
  HAL_GPIO_PULLUP_PULLDOWN_ENABLE
} hal_gpio_pull_t;

typedef enum {
  HAL_GPIO_INTR_DISABLE = 0,
  HAL_GPIO_INTR_POSEDGE,
  HAL_GPIO_INTR_NEGEDGE,
  HAL_GPIO_INTR_ANYEDGE,
  HAL_GPIO_INTR_LOW_LEVEL,
  HAL_GPIO_INTR_HIGH_LEVEL
} hal_gpio_intr_type_t;

// GPIO functions
void gpio_hal_init(void);
void gpio_hal_set_direction(uint32_t gpio_num, hal_gpio_mode_t mode);
void gpio_hal_set_pull(uint32_t gpio_num, hal_gpio_pull_t pull);
void gpio_hal_set_level(uint32_t gpio_num, uint32_t level);
uint32_t gpio_hal_get_level(uint32_t gpio_num);
void gpio_hal_intr_handler(uint32_t gpio_num, hal_gpio_intr_type_t type);

// Helper macros
#define GPIO_HAL_INPUT HAL_GPIO_MODE_INPUT
#define GPIO_HAL_OUTPUT HAL_GPIO_MODE_OUTPUT
#define GPIO_HAL_OUTPUT_OD HAL_GPIO_MODE_INPUT_OUTPUT_OD
#define GPIO_HAL_PULLUP HAL_GPIO_PULLUP_ENABLE
#define GPIO_HAL_PULLDOWN HAL_GPIO_PULLDOWN_ENABLE
#define GPIO_HAL_PULLUPDOWN HAL_GPIO_PULLUP_PULLDOWN_ENABLE

#ifdef __cplusplus
}
#endif

#endif // WILLY_GPIO_HAL_H
