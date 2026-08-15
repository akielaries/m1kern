#include "console.h"
#include "m1core_uart.h"

#include <stdbool.h>

#define CONSOLE_UART     UART0
#define CONSOLE_BAUDRATE 115200

static void putch(char c) {
  UART_SendChar(CONSOLE_UART, c);
}

static void print_hex(unsigned int value, int padding) {
  int num_nibbles       = 0;
  unsigned int temp_val = value;

  if (temp_val == 0) {
    num_nibbles = 1;
  } else {
    while (temp_val > 0) {
      temp_val >>= 4;
      num_nibbles++;
    }
  }

  for (int i = 0; i < padding - num_nibbles; i++) {
    putch('0');
  }

  for (int i = (num_nibbles - 1) * 4; i >= 0; i -= 4) {
    unsigned int nibble = (value >> i) & 0xF;
    putch(nibble < 10 ? (char)(nibble + '0') : (char)(nibble - 10 + 'A'));
  }
}

static void print_number_u(uint32_t n) {
  if (n == 0) {
    putch('0');
    return;
  }

  char buf[10]; // max uint32 is 4294967295, 10 digits
  int i = 0;

  while (n > 0) {
    buf[i++] = (char)('0' + (n % 10));
    n /= 10;
  }
  while (i > 0) {
    putch(buf[--i]);
  }
}

static void print_number(int n) {
  if (n < 0) {
    putch('-');
    n = -n;
  }
  if (n / 10) {
    print_number(n / 10);
  }
  putch((char)((n % 10) + '0'));
}

static void print_float(double val, int precision) {
  if (val < 0.0) {
    putch('-');
    val = -val;
  }

  uint32_t int_part = (uint32_t)val;
  print_number_u(int_part);
  putch('.');

  double frac = val - (double)int_part;
  for (int i = 0; i < precision; i++) {
    frac *= 10.0;
    int digit = (int)frac;
    putch((char)('0' + digit));
    frac -= (double)digit;
  }
}

void console_init(void) {
  /* the m1core uart takes a baud rate directly rather than a struct of
     enables, so this is simpler than the gowin driver's console_init */
  UART_Init(CONSOLE_UART, CONSOLE_BAUDRATE);
}

/* 16 bytes per row: offset | hex bytes | ascii */
void console_hexdump(const uint8_t *buf, uint32_t len) {
  for (uint32_t i = 0; i < len; i += 16) {
    print_hex(i, 4);
    putch(' ');
    putch(' ');

    for (uint32_t j = 0; j < 16; j++) {
      if (i + j < len) {
        print_hex(buf[i + j], 2);
      } else {
        putch(' ');
        putch(' ');
      }
      putch(' ');
      if (j == 7) {
        putch(' ');
      }
    }

    putch(' ');
    putch('|');
    for (uint32_t j = 0; j < 16 && i + j < len; j++) {
      uint8_t c = buf[i + j];
      putch((c >= 0x20 && c < 0x7f) ? (char)c : '.');
    }
    putch('|');
    putch('\r');
    putch('\n');
  }
}

/*
 * supports %d %u %s %c %x %X %f %% with optional zero padding (%08X).
 * hex is printed bare, so callers write the "0x" themselves.
 */
void console_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (*format != '%') {
      putch(*format++);
      continue;
    }

    format++;
    int padding = 0;

    if (*format == '0') {
      format++;
      if (*format >= '0' && *format <= '9') {
        padding = *format - '0';
        format++;
      }
    }

    switch (*format) {
    case 'd':
      print_number(va_arg(args, int));
      break;
    case 'u':
      print_number_u(va_arg(args, uint32_t));
      break;
    case 's': {
      char *s = va_arg(args, char *);
      while (*s) {
        putch(*s++);
      }
      break;
    }
    case 'c':
      putch((char)va_arg(args, int));
      break;
    case 'x':
    case 'X':
      print_hex(va_arg(args, unsigned int), padding);
      break;
    case 'f':
      print_float(va_arg(args, double), 3);
      break;
    case '%':
      putch('%');
      break;
    default:
      putch('%');
      putch(*format);
      break;
    }
    format++;
  }

  va_end(args);
}
