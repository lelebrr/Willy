#ifndef WILLY_HAL_H
#define WILLY_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_delay(uint32_t ms);
void hal_delay_us(uint32_t us);
void hal_gpio_set_direction(uint8_t pin, uint8_t mode);
void hal_gpio_set_level(uint8_t pin, uint8_t level);
uint8_t hal_gpio_get_level(uint8_t pin);
void hal_uart_write(uint8_t uart, uint8_t data);
int hal_uart_read(uint8_t uart);
void *hal_malloc(size_t size);
void hal_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // WILLY_HAL_H
