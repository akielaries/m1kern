#include "m1core_uart.h"

void UART_Init(UART_TypeDef *uart, uint32_t baud) {
  uint32_t div = SystemCoreClock / baud;

  /* the cmsdk block will not divide below 16 */
  if (div < 16U) {
    div = 16U;
  }

  uart->BAUDDIV = div;
  uart->CTRL    = UART_CTRL_TXEN | UART_CTRL_RXEN;
}

void UART_SendChar(UART_TypeDef *uart, char c) {
  while (uart->STATE & UART_STATE_TXBF) {
  }

  uart->DATA = (uint32_t)(unsigned char)c;
}

int UART_GetChar(UART_TypeDef *uart) {
  if (!(uart->STATE & UART_STATE_RXBF)) {
    return -1;
  }

  return (int)(uart->DATA & 0xFFU);
}
