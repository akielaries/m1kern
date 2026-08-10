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

roughly in order of how much work each one is.

### stack overflow detection

nothing catches a thread running off the end of its `THREAD_STACK`. it walks
into whatever `.bss` sits below and corrupts it silently.

cheapest fix is a guard word: have `init_stack` write a magic value at
`stack_mem[0]` and check it in `scheduler_next` before returning a thread. two
loads and a compare per switch. a fuller version paints the whole stack with a
known pattern at creation and scans for the high-water mark, which also tells
you how much of each `THREAD_STACK` you are actually using.

a real guard region would need an MPU, and there is none on this part.

### thread exit and slot reuse

`thread_count` only ever increments and threads are never removed, so
`MAX_THREADS` is a lifetime budget, not a concurrent one. entry functions must
loop forever - the initial frame's LR slot holds `0xFFFFFFFD`, so returning
branches somewhere that is not code.

to allow exit: point that LR slot at a small trampoline that sets
`THREAD_UNUSED` and pends a switch, then have `thread_create` scan for a free
slot instead of appending. mark in place rather than compacting the array,
because `scheduler_next` indexes round robin off `last_thd_idx % thread_count`
and shifting entries would scramble the rotation. a thread cannot release its
own stack while still running on it, so any reuse has to happen after the
switch away.

### aging, or anything that stops starvation

`scheduler_next` picks the highest priority `THREAD_READY` thread, full stop.
a `PRIO_HIGH` thread that stays runnable means nothing below it ever runs, as
`03_priority` demonstrates deliberately.

adding aging means splitting priority in two: a base value set at creation and
an effective value the scheduler reads. the tick bumps the effective priority
of ready threads that have been passed over, and picking a thread resets it.
that is maybe thirty lines, but it trades away the strict-priority guarantee,
which is the whole point of the current design. worth being sure you want it.

### mutexes and semaphores

there is no way for one thread to wait on another. `thread_sleep_ms` is the
only blocking call and it only waits on time.

the state machine mostly supports it already - `scheduler_next` returns only
`THREAD_READY` threads, so a new `THREAD_BLOCKED` state is skipped for free.
what you have to add is a wait list per object and the release path that moves
a waiter back to `THREAD_READY`.

the sharp edge is atomicity. Cortex-M1 is ARMv6-M, which has no `LDREX` and
`STREX`, so a lock-free compare-and-swap is not available. every acquire and
release has to sit inside a `cpsid i` / `cpsie i` pair, the same thing
`PendSV_Handler` already does. keep those windows to a few instructions or
they eat into interrupt latency.

### priority inheritance

only worth anything once mutexes exist, and then it is close to mandatory: a
low priority thread holding a mutex that a high priority thread wants will
otherwise be preempted by every medium priority thread on the system, and the
high priority one waits behind all of them.

it needs the base/effective priority split described under aging, plus an owner
field on the mutex. on contention, raise the owner's effective priority to the
blocker's; on release, drop it back. nested mutexes make the restore step
harder than it first looks - the owner has to fall back to the highest priority
among everything it still holds, not to its base.

### queues and message passing

nothing exists here. once you have semaphores it is mostly mechanical: a ring
buffer plus a counting semaphore for occupancy and another for free space.
worth doing after mutexes, not before.
