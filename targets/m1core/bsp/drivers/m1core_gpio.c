#include "m1core_gpio.h"

void GPIO_Init(GPIO_TypeDef *gpio, uint32_t out_mask) {
  gpio->DIR = out_mask;
}

void GPIO_Set(GPIO_TypeDef *gpio, uint32_t mask) {
  gpio->SET = mask;
}

void GPIO_Clear(GPIO_TypeDef *gpio, uint32_t mask) {
  gpio->CLR = mask;
}

void GPIO_Toggle(GPIO_TypeDef *gpio, uint32_t mask) {
  uint32_t cur = gpio->DATA;

  gpio->SET = ~cur & mask;
  gpio->CLR = cur & mask;
}

uint32_t GPIO_Read(GPIO_TypeDef *gpio) {
  return gpio->DATA;
}
