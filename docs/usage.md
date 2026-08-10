@page usage usage

## build

```
cmake -S . -B build
cmake --build build
```

needs `arm-none-eabi-gcc`. binaries land in `build/bin/`. `BOOT_MODE` picks the
linker script (`flash_burn` or `flash_xip`) and `M1KERN_FIXED_HIGH_REGS`
controls whether r8-r11 are reserved from the compiler.

## writing threads

declare the stack and control block at file scope, define the entry point, then
register it before `kernel_start`:

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
  board_init();     // SystemInit, console, m1kern_target_tick_init
  kernel_init();

  mkthd_static(blink, blink_fn, sizeof(blink), PRIO_NORMAL, NULL);

  kernel_start();   // does not return
}
```

entry points must not return. everything is static, so the sum of your
`THREAD_STACK` sizes plus `.bss` has to fit RAM.

## order of operations

`m1kern_target_tick_init()` has to run before `kernel_init()`. `board_init()`
does it for you. `kernel_start()` sets the exception priorities, then drops to
PSP and fires `svc #0` to enter the first thread.

## scheduling

`scheduler_next()` picks the highest priority `THREAD_READY` thread, round
robin among ties. `thread_sleep_ms()` marks the caller `THREAD_SLEEPING` with a
`wake_time` and pends a switch; `thread_yield()` pends a switch without
blocking. when nothing is ready, PendSV parks in `wfi` until a tick changes
that, so no idle thread is required.

## integrating into your own firmware

three things fail silently:

1. **link the kernel objects directly, not as a static archive.**
   `PendSV_Handler` is only referenced by the weak vector table in the startup
   file, so as an archive member `pendsv.S` never gets pulled in and PendSV
   resolves to `Default_Handler` - an infinite loop, no diagnostic. this repo
   uses a CMake `OBJECT` library. check it with:
   ```
   arm-none-eabi-nm build/bin/01_blink.elf | grep PendSV_Handler
   ```
   it must be `T`, never `W`.

2. **nothing but the kernel may touch SysTick.** the vendor `delay_us`
   and `delay_ms` reprogram `SysTick->LOAD`, `VAL` and `CTRL` directly, which
   corrupts the tick `system_time_ms` and every sleeping thread depend on.
   that is why `delay.c` is not in this repo.

3. **`.bss` must actually be cleared.** all kernel state lives there. recent
   arm-none-eabi ships picocrt, whose `crt0` wants linker symbols the Gowin
   scripts do not define, so this repo builds `-nostartfiles`, supplies
   `_start` in `target/src/crt0.c`, and defines `__STARTUP_CLEAR_BSS`
   so the vendor startup zeroes `.bss` instead of leaving it to `crt0`.

`target.c` defines `SVC_Handler`, `SysTick_Handler` and `HardFault_Handler`, so
remove those from your own interrupt file. set `M1KERN_LOG` to your printf for
the boot dump and fault reports.

## a real integration

[gowin_cortexm1_fw](https://github.com/akielaries/gowin_cortexm1_fw) is where
this kernel came from and runs it alongside a UART console, a GPIO driver, an
ethernet stack and a set of cheby-generated register maps. `src/os_demo.c`
there is the equivalent of these examples in a firmware that does real work.
