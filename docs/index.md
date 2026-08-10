# m1kern

a small preemptive kernel for the Gowin Cortex-M1 softcore.

static threads, no heap, strict priority scheduling with round robin among
equals, and a 1 ms SysTick that preempts whether or not a thread cooperates.
the whole kernel is three files:

| file | what it does |
| --- | --- |
| `m1kern/src/kernel.c` | thread table, scheduler, stack frame setup |
| `m1kern/src/pendsv.S` | the ARMv6-M context switch |
| `port/gowin_m1/src/port.c` | SysTick, SVC and HardFault handlers |

this is not portable and does not try to be. it targets the Gowin EMPU-M1
softcore specifically and links against the vendor BSP.

- @ref usage "usage" - wiring it into a project
- @ref examples "examples" - the five programs in `examples/`
- [m1kern.h](m1kern_8h.html) - the whole public API
- [m1kern_port.h](m1kern__port_8h.html) and
  [m1kern_conf.h](m1kern__conf_8h.html) - port layer and tunables

## what it does not have

no mutexes, semaphores, or queues. no priority inheritance and no aging, so a
runnable `PRIO_HIGH` thread starves everything below it for as long as it stays
runnable. no stack overflow detection - there is no MPU on this part. threads
are never removed from the table, and `MAX_THREADS` is 8.
