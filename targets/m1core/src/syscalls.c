/*
 * newlib stubs. nothing in m1kern needs these, they exist so linking against
 * the default arm-none-eabi libc does not fail. all weak, override freely.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>

#include "M1CORE.h"

/* end of .bss, provided by the linker script. the heap starts here */
extern int _end;

__attribute__((weak)) int _close(int file) {
  (void)file;
  return -1;
}

__attribute__((weak)) int _lseek(int file, int ptr, int dir) {
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

__attribute__((weak)) int _read(int file, char *ptr, int len) {
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}

__attribute__((weak)) int _write(int file, char *ptr, int len) {
  (void)file;
  (void)ptr;
  return len;
}

__attribute__((weak)) void _exit(int status) {
  (void)status;
  __asm("BKPT #0");
  while (1) {
  }
}

__attribute__((weak)) caddr_t _sbrk(int incr) {
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == 0) {
    heap_end = (caddr_t)&_end;
  }

  prev_heap_end = heap_end;
  if (heap_end + incr > (char *)__get_MSP()) {
    errno = ENOMEM;
    return (caddr_t)-1;
  }

  heap_end += incr;
  return (caddr_t)prev_heap_end;
}

__attribute__((weak)) int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

__attribute__((weak)) int _getpid(void) {
  return 1;
}

__attribute__((weak)) int _fstat(int file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

__attribute__((weak)) int _isatty(int file) {
  (void)file;
  return 1;
}
