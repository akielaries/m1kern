/*
 * compile-time tunables for m1kern
 */
#pragma once

/* hard cap on thread_create calls. each slot is one pointer in .bss */
#ifndef MAX_THREADS
#define MAX_THREADS 8
#endif

/*
 * scheduler tick rate. this sets the preemption quantum: no thread can hold
 * the CPU longer than 1/M1KERN_TICK_HZ seconds regardless of whether it
 * yields. system_time_ms assumes this is 1000.
 */
#ifndef M1KERN_TICK_HZ
#define M1KERN_TICK_HZ 1000
#endif

/*
 * optional trace hook. define M1KERN_LOG to a printf-alike before including
 * m1kern.h (or set it here) to get kernel state dumps on stdout. left empty
 * by default so the kernel has no I/O dependency at all.
 */
#ifndef M1KERN_LOG
#define M1KERN_LOG(...) ((void)0)
#endif
