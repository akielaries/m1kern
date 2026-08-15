# m1kern

A small preemptive kernel for the Gowin Cortex-M1 softcore. Static threads, no
heap, strict priority scheduling with round robin among equals. A 1 ms SysTick
pends PendSV every tick, so threads are preempted whether or not they yield.

The project takes a great deal of inspiration from chibiOS. You will find some
syntax is similar.

`kernel/` is the scheduler and context switch, `target/` is the chip: tick,
exception handlers, vendor BSP and linker scripts.

Docs: https://akielaries.github.io/m1kern

## Footprint

The kernel, its context switch and the target's tick and exception handlers,
with nothing else linked in:

| | `.text` | `.bss` |
| --- | --- | --- |
| `-O0` (this repo's default) | 1602 B | 49 B |
| `-Os` | 660 B | 49 B |

The 49 bytes of `.bss` is the whole scheduler: an 8-entry thread table, the
current thread pointer, the tick counter and the round robin cursor. It does
not scale with thread count.

Each thread costs 20 bytes for its control block plus whatever you give it in
`THREAD_STACK`. No heap, so that is the entire runtime cost.

The example binaries are around 14 KB, but that is almost all UART console and
libc, not the kernel.

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

Needs `arm-none-eabi-gcc`. Binaries land in `build/bin/`.
