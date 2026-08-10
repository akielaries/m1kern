/**
 * @file m1kern.h
 * @brief public API. include this and nothing else from application code
 *
 * m1kern is a preemptive kernel for the Gowin Cortex-M1 softcore. threads are
 * statically allocated, the scheduler is strict priority with round robin
 * among equals, and a 1 ms SysTick drives preemption.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#include "m1kern_conf.h"

/** @brief thread priority. higher value wins, always */
typedef enum {
  PRIO_LOW    = 1,
  PRIO_NORMAL = 2,
  PRIO_HIGH   = 3, // if a thread is HIGH priority it will ALWAYS run first
} thread_prio_e;

/** @brief scheduler state of a thread */
typedef enum {
  THREAD_READY = 0,  /**< runnable, eligible to be picked */
  THREAD_SLEEPING,   /**< blocked until wake_time */
  THREAD_UNUSED      /**< slot not in play */
} thread_state_e;

/**
 * @brief thread control block
 *
 * @c sp must stay at offset 0, pendsv.S dereferences the thread pointer
 * directly to load and store it
 */
typedef struct thread {
  uint32_t *sp;                /**< saved stack pointer */
  volatile uint32_t wake_time; /**< system_time_ms value to wake at */
  thread_state_e state;
  uint8_t priority;
  uint8_t *stack_mem;
  size_t stack_size;
} thread_t;

/** @brief global system time, incremented in the systick interrupt handler */
extern volatile uint32_t system_time_ms;

/**
 * @name PendSV interface
 * these are needed by the PendSV handler
 * @{
 */
extern thread_t *volatile current_thread;

/** @brief pick the next thread to run, or NULL if everything is asleep */
thread_t *scheduler_next(void);

extern volatile uint8_t kernel_running;
/** @} */

/**
 * @name kernel lifecycle
 * @{
 */

/** @brief reset the thread table. call once, before creating any threads */
void kernel_init(void);

/**
 * @brief set exception priorities and switch into the first thread
 *
 * does not return. returns immediately without starting if no threads exist
 */
void kernel_start(void);
/** @} */

/**
 * @name thread API
 * @{
 */

/**
 * @brief register a thread and build its initial stack frame
 *
 * prefer @ref mkthd_static, which pairs this with @ref THREAD_STACK
 *
 * @param t          control block, typically from THREAD_STACK
 * @param func       entry point. must not return
 * @param stack      stack memory
 * @param stack_size size of @p stack in bytes
 * @param prio       one of @ref thread_prio_e
 * @param arg        unused, reserved
 * @return @p t, or NULL if MAX_THREADS is already reached
 */
thread_t *thread_create(thread_t *t,
                        void (*func)(void),
                        uint8_t *stack,
                        size_t stack_size,
                        uint8_t prio,
                        void *arg);

/** @brief pend a context switch. the caller stays runnable */
void thread_yield(void);

/**
 * @brief sleep the calling thread for at least @p ms milliseconds
 *
 * only valid once the kernel is running
 */
void thread_sleep_ms(uint32_t ms);
/** @} */

/**
 * @name static declaration macros
 * @{
 */

/** @brief declare the stack and control block for a thread, both in .bss */
#define THREAD_STACK(name, size)                                               \
  static uint8_t __stack_##name[size];                                         \
  static thread_t __thread_##name

/** @brief define a thread entry point */
#define THREAD_FUNCTION(name, arg) void name(void)

/** @brief create a thread declared with @ref THREAD_STACK */
#define mkthd_static(name, func, size, prio, arg)                              \
  thread_create(&__thread_##name,                                              \
                func,                                                          \
                __stack_##name,                                                \
                sizeof(__stack_##name),                                        \
                prio,                                                          \
                arg)
/** @} */
