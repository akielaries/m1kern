@page examples examples

each one isolates a single kernel behavior. all print at 230400 baud on UART1
and blink `GPIO0` pins 0-3.

| example | shows |
| --- | --- |
| `01_blink` | two threads, two periods, nothing shared |
| `02_sleep` | absolute wake times, and that drift does not accumulate |
| `03_priority` | strict priority, and the starvation it implies |
| `04_compute` | a long CPU-bound job that stays responsive |
| `05_preemption` | round robin among threads that never yield |

build all five with `cmake --build build`, flash `build/bin/<name>.bin`.

## 01_blink

@include 01_blink/main.c

## 03_priority

the hog runs at `PRIO_HIGH` and spins for two seconds without sleeping. the
`PRIO_NORMAL` counters stop dead for that whole window even though SysTick is
still firing on schedule. that is strict priority working as designed, not a
bug - there is no aging to rescue them.

@include 03_priority/main.c

## 05_preemption

three `PRIO_LOW` threads run tight loops and never call `thread_yield` or
`thread_sleep_ms`. under a cooperative scheduler the first would own the CPU
forever. the counters staying balanced is the proof that `SysTick_Handler`
pending PendSV every tick is what drives the switch.

@include 05_preemption/main.c
