/*
 * board bring-up shared by the examples. not part of the kernel.
 */
#pragma once

#include <stdint.h>

#include "GOWIN_M1.h"
#include "console.h"

/* LEDs the examples toggle. adjust to match your constraints file */
#define LED0 GPIO_Pin_0
#define LED1 GPIO_Pin_1
#define LED2 GPIO_Pin_2
#define LED3 GPIO_Pin_3

typedef enum {
  PIN_LOW  = 0,
  PIN_HIGH = 1
} pin_state_e;

/* SystemInit + console + LED pins. call before kernel_init */
void board_init(void);

void led_toggle(uint32_t pin);
void led_write(uint32_t pin, pin_state_e state);
pin_state_e led_read(uint32_t pin);
