/**
 * @file m1kern_conf.h
 * @brief compile-time tunables
 */
#pragma once

/** @brief hard cap on thread_create calls. each slot is one pointer in .bss */
#ifndef MAX_THREADS
#define MAX_THREADS 8
#endif

/**
 * @brief scheduler tick rate in Hz
 *
 * sets the preemption quantum: no thread holds the CPU longer than one tick
 * regardless of whether it yields. system_time_ms assumes this is 1000
 */
#ifndef M1KERN_TICK_HZ
#define M1KERN_TICK_HZ 1000
#endif

/**
 * @brief optional trace hook, a printf-alike
 *
 * no-op by default so the kernel has no I/O dependency. define it to
 * console_printf for the boot dump and fault reports
 */
#ifndef M1KERN_LOG
#define M1KERN_LOG(...) ((void)0)
#endif
