#include "board.h"

#include "m1kern_target.h"
#include "m1core_gpio.h"

#define LED_PORT GPIO0
#define LED_MASK (LED0 | LED1 | LED2 | LED3)

void board_init(void) {
  SystemInit();
  console_init();
  m1kern_target_tick_init();

  GPIO_Init(LED_PORT, LED_MASK);

  console_printf("\r\n");
  console_printf("m1kern on m1core @ %d MHz\r\n", SystemCoreClock / 1000000);
}

void led_toggle(uint32_t pin) {
  GPIO_Toggle(LED_PORT, pin);
}

void led_write(uint32_t pin, pin_state_e state) {
  if (state == PIN_HIGH) {
    GPIO_Set(LED_PORT, pin);
  } else {
    GPIO_Clear(LED_PORT, pin);
  }
}

pin_state_e led_read(uint32_t pin) {
  return (GPIO_Read(LED_PORT) & pin) ? PIN_HIGH : PIN_LOW;
}
