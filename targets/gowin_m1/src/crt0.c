/*
 * minimal C runtime entry.
 *
 * we build with -nostartfiles because startup_GOWIN_M1.S already supplies the
 * vector table and Reset_Handler, and recent arm-none-eabi toolchains ship
 * picocrt, whose crt0 wants linker symbols (__data_size, __tls_base, ...) that
 * the Gowin linker scripts do not define.
 *
 * Reset_Handler copies .data, clears .bss (we define __STARTUP_CLEAR_BSS in
 * CMakeLists.txt so that path is compiled in), calls SystemInit, then branches
 * to _start. by that point the C environment is ready and there is nothing
 * left to do but call main.
 *
 * note there is no __libc_init_array call here. this repo is C only and has no
 * static constructors; add it if you pull in C++.
 */

extern int main(void);

__attribute__((noreturn)) void _start(void) {
  main();

  // main returning is a bug in the application, not something to recover from
  while (1) {
  }
}
