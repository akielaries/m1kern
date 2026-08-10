# m1kern

a small preemptive kernel for the Gowin Cortex-M1 softcore.

static threads, no heap, strict priority scheduling with round robin among
equals, and a 1 ms SysTick that preempts whether or not a thread cooperates.

| directory | what it holds |
| --- | --- |
| `kernel/` | thread table, scheduler, initial stack frames, the context switch |
| `target/` | tick source, exception handlers, vendor BSP, linker scripts, board support |
| `examples/` | five runnable programs |

this targets the Gowin EMPU-M1 specifically and links against the vendor BSP.
it is not portable and does not try to be.

- @ref usage "usage" - building and wiring it into a project
- @ref examples "examples" - the five programs in `examples/`
- [m1kern.h](m1kern_8h.html) - the whole public API
- [m1kern_target.h](m1kern__target_8h.html) - tick and exception plumbing

## what it does not have

no mutexes, semaphores, or queues. no priority inheritance and no aging, so a
runnable `PRIO_HIGH` thread starves everything below it for as long as it stays
runnable. no stack overflow detection - there is no MPU on this part. threads
are never removed from the table, and `MAX_THREADS` is 8.
