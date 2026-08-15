/**
 * @file m1core_gpio.h
 */
#ifndef M1CORE_GPIO_H
#define M1CORE_GPIO_H

#include "M1CORE.h"

#define GPIO_Pin_0 (1UL << 0)
#define GPIO_Pin_1 (1UL << 1)
#define GPIO_Pin_2 (1UL << 2)
#define GPIO_Pin_3 (1UL << 3)

void GPIO_Init(GPIO_TypeDef *gpio, uint32_t out_mask);
void GPIO_Set(GPIO_TypeDef *gpio, uint32_t mask);
void GPIO_Clear(GPIO_TypeDef *gpio, uint32_t mask);
void GPIO_Toggle(GPIO_TypeDef *gpio, uint32_t mask);
uint32_t GPIO_Read(GPIO_TypeDef *gpio);

#endif
