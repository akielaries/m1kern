/**
 * @file board.h
 * @brief board bring-up for the examples. not part of the kernel
 */
#pragma once

#include <stdint.h>

#include "M1CORE.h"
#include "console.h"

/* m1core's gpio is 2 bits wide in the current soc build. the upper two are
   declared so the examples compile unchanged; they simply drive nothing */
#define LED0 (1UL << 0)
#define LED1 (1UL << 1)
#define LED2 (1UL << 2)
#define LED3 (1UL << 3)

typedef enum {
  PIN_LOW  = 0,
  PIN_HIGH = 1
} pin_state_e;

/* SystemInit + console + LED pins. call before kernel_init */
void board_init(void);

void led_toggle(uint32_t pin);
void led_write(uint32_t pin, pin_state_e state);
pin_state_e led_read(uint32_t pin);
