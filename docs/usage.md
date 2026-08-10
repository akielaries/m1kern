@page usage usage

## build

```
cmake -S . -B build
cmake --build build
```

`M1KERN_TARGET` selects a directory under `target/`, defaulting to `gowin_m1`.
everything else is target-owned: on `gowin_m1`, `BOOT_MODE` picks the linker
script (`flash_burn` or `flash_xip`) and `M1KERN_FIXED_HIGH_REGS` controls
whether r8-r11 are reserved from the compiler. binaries land in `build/bin/`.

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
does it for you. `kernel_start()` sets the exception priorities, then
`m1kern_arch_start_first_thread()` drops to PSP and fires `svc #0` to enter the
first thread.

## scheduling

`scheduler_next()` picks the highest priority `THREAD_READY` thread, round
robin among ties. `thread_sleep_ms()` marks the caller `THREAD_SLEEPING` with a
`wake_time` and pends a switch; `thread_yield()` pends a switch without
blocking. when nothing is ready, PendSV parks in `wfi` until a tick changes
that, so no idle thread is required.

## adding a target

a target is a directory under `target/` with a `target.cmake` and two headers.
copy `target/gowin_m1` and replace:

| file | what it must provide |
| --- | --- |
| `include/m1kern_device.h` | the CMSIS device header, plus the linker's stack symbols |
| `include/m1kern_target.h` | declarations for the two functions below |
| `include/m1kern_conf.h` | `MAX_THREADS`, `M1KERN_TICK_HZ`, `M1KERN_LOG` |
| `src/target.c` | `m1kern_target_tick_init`, `m1kern_target_dump_state`, and the `SysTick_Handler` / `SVC_Handler` that pend PendSV |
| `target.cmake` | CPU flags, linker script, startup file, BSP sources, and `M1KERN_ARCH` |

`M1KERN_ARCH` names a directory under `arch/`. `armv6m` covers Cortex-M0, M0+
and M1. anything ARMv7-M needs a new arch: the switch in `pendsv.S` is written
around Thumb-1 not being able to `stm` the high registers directly, which an
M3 or M4 has no reason to do.

nothing in `kernel/` should need to change.

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

2. **nothing but the kernel may touch the tick timer.** the vendor `delay_us`
   and `delay_ms` reprogram `SysTick->LOAD`, `VAL` and `CTRL` directly, which
   corrupts the tick `system_time_ms` and every sleeping thread depend on.
   that is why `delay.c` is not in this repo.

3. **`.bss` must actually be cleared.** all kernel state lives there. recent
   arm-none-eabi ships picocrt, whose `crt0` wants linker symbols the Gowin
   scripts do not define, so this target builds `-nostartfiles`, supplies
   `_start` in `target/gowin_m1/src/crt0.c`, and defines `__STARTUP_CLEAR_BSS`
   so the vendor startup zeroes `.bss` instead of leaving it to `crt0`.

`target.c` defines `SVC_Handler`, `SysTick_Handler` and `HardFault_Handler`, so
remove those from your own interrupt file. set `M1KERN_LOG` to your printf for
the boot dump and fault reports.

## a real integration

[gowin_cortexm1_fw](https://github.com/akielaries/gowin_cortexm1_fw) is where
this kernel came from and runs it alongside a UART console, a GPIO driver, an
ethernet stack and a set of cheby-generated register maps. `src/os_demo.c`
there is the equivalent of these examples in a firmware that does real work.
