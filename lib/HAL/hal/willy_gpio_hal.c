#include "willy_gpio_hal.h"
#include "driver/gpio.h"
#include "esp_attr.h"

// GPIO HAL implementation for ESP32-S3

void gpio_hal_init(void) {
  // Initialize GPIO subsystem
  gpio_config_t io_conf = {.pin_bit_mask =
                               (1ULL << GPIO_NUM_MAX) - 1, // All pins
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_DISABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);
}

void gpio_hal_set_direction(uint32_t gpio_num, hal_gpio_mode_t mode) {
  gpio_mode_t gpio_mode;

  switch (mode) {
  case HAL_GPIO_MODE_INPUT:
    gpio_mode = GPIO_MODE_INPUT;
    break;
  case HAL_GPIO_MODE_OUTPUT:
    gpio_mode = GPIO_MODE_OUTPUT;
    break;
  case HAL_GPIO_MODE_INPUT_OUTPUT:
    gpio_mode = GPIO_MODE_INPUT_OUTPUT;
    break;
  case HAL_GPIO_MODE_INPUT_OUTPUT_OD:
    gpio_mode = GPIO_MODE_INPUT_OUTPUT_OD;
    break;
  default:
    gpio_mode = GPIO_MODE_INPUT;
    break;
  }

  gpio_set_direction(gpio_num, gpio_mode);
}

void gpio_hal_set_pull(uint32_t gpio_num, hal_gpio_pull_t pull) {
  gpio_pullup_t pull_up = GPIO_PULLUP_DISABLE;
  gpio_pulldown_t pull_down = GPIO_PULLDOWN_DISABLE;

  switch (pull) {
  case HAL_GPIO_PULLUP_DISABLE:
    pull_up = GPIO_PULLUP_DISABLE;
    pull_down = GPIO_PULLDOWN_DISABLE;
    break;
  case HAL_GPIO_PULLUP_ENABLE:
    pull_up = GPIO_PULLUP_ENABLE;
    pull_down = GPIO_PULLDOWN_DISABLE;
    break;
  case HAL_GPIO_PULLDOWN_ENABLE:
    pull_up = GPIO_PULLUP_DISABLE;
    pull_down = GPIO_PULLDOWN_ENABLE;
    break;
  case HAL_GPIO_PULLUP_PULLDOWN_ENABLE:
    pull_up = GPIO_PULLUP_ENABLE;
    pull_down = GPIO_PULLDOWN_ENABLE;
    break;
  default:
    pull_up = GPIO_PULLUP_DISABLE;
    pull_down = GPIO_PULLDOWN_DISABLE;
    break;
  }

  gpio_set_pull_mode(gpio_num, pull_up);
}

void gpio_hal_set_level(uint32_t gpio_num, uint32_t level) {
  gpio_set_level(gpio_num, level);
}

uint32_t gpio_hal_get_level(uint32_t gpio_num) {
  return gpio_get_level(gpio_num);
}

void gpio_hal_intr_handler(uint32_t gpio_num, hal_gpio_intr_type_t type) {
  // Install GPIO interrupt service
  gpio_isr_handle_t isr_handle = NULL;

  gpio_int_type_t intr_type;
  switch (type) {
  case HAL_GPIO_INTR_DISABLE:
    intr_type = GPIO_INTR_DISABLE;
    break;
  case HAL_GPIO_INTR_POSEDGE:
    intr_type = GPIO_INTR_POSEDGE;
    break;
  case HAL_GPIO_INTR_NEGEDGE:
    intr_type = GPIO_INTR_NEGEDGE;
    break;
  case HAL_GPIO_INTR_ANYEDGE:
    intr_type = GPIO_INTR_ANYEDGE;
    break;
  case HAL_GPIO_INTR_LOW_LEVEL:
    intr_type = GPIO_INTR_LOW_LEVEL;
    break;
  case HAL_GPIO_INTR_HIGH_LEVEL:
    intr_type = GPIO_INTR_HIGH_LEVEL;
    break;
  default:
    intr_type = GPIO_INTR_DISABLE;
    break;
  }

  gpio_set_intr_type(gpio_num, intr_type);

  // Clear any pending interrupt
  gpio_intr_disable(gpio_num);
  gpio_intr_enable(gpio_num);
}

static void IRAM_ATTR gpio_hal_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  // Generic GPIO interrupt handler
  // This can be extended to handle specific interrupt callbacks
}
