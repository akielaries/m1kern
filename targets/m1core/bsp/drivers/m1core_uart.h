/**
 * @file m1core_uart.h
 * @brief CMSDK uart driver
 */
#ifndef M1CORE_UART_H
#define M1CORE_UART_H

#include "M1CORE.h"

void UART_Init(UART_TypeDef *uart, uint32_t baud);
void UART_SendChar(UART_TypeDef *uart, char c);
int  UART_GetChar(UART_TypeDef *uart);

#endif
