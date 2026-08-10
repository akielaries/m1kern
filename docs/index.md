# m1kern

a small preemptive kernel for ARM Cortex-M.

static threads, no heap, strict priority scheduling with round robin among
equals, and a 1 ms tick that preempts whether or not a thread cooperates.

three layers, each replaceable independently:

| layer | what it holds |
| --- | --- |
| `kernel/` | thread table, scheduler, initial stack frames. no CPU or chip code |
| `arch/armv6m/` | the context switch (`pendsv.S`) and first-thread entry |
| `target/gowin_m1/` | tick source, exception handlers, vendor BSP, linker scripts |

only the Gowin EMPU-M1 softcore is supported today. see @ref usage "usage" for
what adding another target involves.

- @ref usage "usage" - wiring it into a project, and adding a target
- @ref examples "examples" - the five programs in `examples/`
- [m1kern.h](m1kern_8h.html) - the whole public API
- [m1kern_arch.h](m1kern__arch_8h.html) and
  [m1kern_target.h](m1kern__target_8h.html) - the two porting seams

## what it does not have

no mutexes, semaphores, or queues. no priority inheritance and no aging, so a
runnable `PRIO_HIGH` thread starves everything below it for as long as it stays
runnable. no stack overflow detection - there is no MPU on this part. threads
are never removed from the table, and `MAX_THREADS` is 8.
