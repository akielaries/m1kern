# m1kern

A small preemptive kernel for ARM Cortex-M. Static threads, no heap, strict
priority scheduling with round robin among equals. A 1 ms tick pends PendSV
every tick, so threads are preempted whether or not they yield.

Three layers: `kernel/` is portable, `arch/armv6m/` is the context switch, and
`target/gowin_m1/` is the chip. Only the Gowin EMPU-M1 softcore is supported
today; adding an MCU means a new directory under `target/`.

Docs: https://akielaries.github.io/m1kern

## Usage

```c
#include "m1kern.h"

THREAD_STACK(blink, 256);
THREAD_FUNCTION(blink_fn, arg) {
  while (1) {
    led_toggle(LED0);
    thread_sleep_ms(250);
  }
}

int main(void) {
  board_init();
  kernel_init();
  mkthd_static(blink, blink_fn, sizeof(blink), PRIO_NORMAL, NULL);
  kernel_start();   // does not return
}
```

Entry points must not return. See `examples/` for five runnable programs.

## Building

```
cmake -S . -B build
cmake --build build
```

Needs `arm-none-eabi-gcc`. Binaries land in `build/bin/`. Pick a target with
`-DM1KERN_TARGET=<name>`, defaulting to `gowin_m1`.
