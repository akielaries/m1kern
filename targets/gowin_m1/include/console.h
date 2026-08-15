/*
 * minimal UART console for the examples. not part of the kernel.
 */
#pragma once

#include <stdarg.h>
#include <stdint.h>

void console_init(void);
void console_printf(const char *format, ...);
void console_hexdump(const uint8_t *buf, uint32_t len);
