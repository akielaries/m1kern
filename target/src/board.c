#include "board.h"

#include "m1kern_target.h"
#include "GOWIN_M1_gpio.h"

#define LED_PORT GPIO0
#define LED_MASK (LED0 | LED1 | LED2 | LED3)

void board_init(void) {
  SystemInit();
  console_init();
  m1kern_target_tick_init();

  GPIO_InitTypeDef init;
  init.GPIO_Pin  = LED_MASK;
  init.GPIO_Mode = GPIO_Mode_OUT;
  init.GPIO_Int  = GPIO_Int_Disable;
  GPIO_Init(LED_PORT, &init);

  console_printf("\r\n");
  console_printf("m1kern @ %d MHz\r\n", SystemCoreClock / 1000000);
}

void led_toggle(uint32_t pin) {
  if (LED_PORT->DATAOUT & pin) {
    LED_PORT->DATAOUT &= ~pin;
  } else {
    LED_PORT->DATAOUT |= pin;
  }
}

void led_write(uint32_t pin, pin_state_e state) {
  if (state == PIN_HIGH) {
    LED_PORT->DATAOUT |= pin;
  } else {
    LED_PORT->DATAOUT &= ~pin;
  }
}

pin_state_e led_read(uint32_t pin) {
  return (LED_PORT->DATA & pin) ? PIN_HIGH : PIN_LOW;
}
