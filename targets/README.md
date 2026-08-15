# Targets

The kernel in `kernel/` is portable. Everything that knows a chip exists lives
in a target directory here, selected at configure time:

```
cmake -S . -B build -DM1KERN_TARGET=m1core
cmake --build build
```

| Target | Device |
| --- | --- |
| `gowin_m1` | Gowin eMPU-M1 hard IP (default) |
| `m1core` | the m1core soft core, `pub/cortexm1_clone` |

`common/core/` holds the CMSIS headers, which are architectural and shared.

## What a target supplies

| Path | Purpose |
| --- | --- |
| `bsp/system/<DEV>.h` | CMSIS style device header: IRQn enum, peripheral structs, base addresses |
| `bsp/system/system_<DEV>.c` | `SystemCoreClock`, `SystemInit` |
| `bsp/drivers/` | uart and gpio |
| `bsp/startup/startup_<DEV>.S` | vector table and `Reset_Handler` |
| `bsp/startup/linker/*.ld` | memory map |
| `include/m1kern_target.h` | tick init, pend switch, exception priorities |
| `src/target.c` | `SVC_Handler`, `SysTick_Handler`, `HardFault_Handler`, tick init |
| `src/board.c`, `src/console.c` | LEDs and console for the examples |

## Why the port was small

The kernel only needs SysTick, SVC, PendSV and NVIC priorities, and all four are
*architectural* — same registers at the same addresses on any ARMv6-M part. So
`kernel/`, `pendsv.S` and most of `target.c` carried over unchanged.

Two decisions on the m1core side made the rest cheap: its interrupt numbering
matches this eMPU-M1 map, and its peripherals are register compatible with ARM's
CMSDK blocks, which is what Gowin used. The UART driver differs only in taking a
baud rate instead of a struct.

## m1core notes

`SystemCoreClock` is a compile time constant because there is no PLL to read
back. It must match the clock the fabric was synthesised with:

```
cmake -S . -B build -DM1KERN_TARGET=m1core -DSYSTEM_CLOCK_HZ=25000000
```

Lower it for a simulation build so SysTick and the UART divider are small enough
to simulate. `sim/Makefile` in the m1core tree has a `make m1kern` target that
does this and runs `01_blink` on the RTL.

Two things worth knowing when picking a tick rate: m1core is multi-cycle at
roughly 5 cycles per instruction, and an exception entry plus exit is about
sixteen bus transactions. A tick period that approaches the cost of a context
switch will livelock PendSV, which never gets to finish before the next tick
re-pends it.
