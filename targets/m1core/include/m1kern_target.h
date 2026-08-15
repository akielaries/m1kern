/**
 * @file m1kern_target.h
 * @brief m1core target layer
 *
 * the only part of the kernel build that knows the chip exists. everything it
 * pulls in from M1CORE.h is ARM CMSIS (core_cm1.h) apart from SystemCoreClock
 * and the linker provided stack symbols.
 *
 * identical in shape to the gowin_m1 target, because the pieces the kernel
 * depends on (SCB->ICSR, NVIC_SetPriority, SysTick) are architectural rather
 * than vendor specific
 */
#pragma once

#include <stdint.h>

#include "M1CORE.h"

/** @brief provided by the linker script */
extern uint32_t __StackLimit;
extern uint32_t __StackTop;

/**
 * @brief start the periodic tick that drives system_time_ms and preemption
 *
 * call before kernel_init. after this returns SysTick belongs to the kernel
 * and nothing else may reprogram it
 */
void m1kern_target_tick_init(void);

/** @brief report tick and stack configuration through M1KERN_LOG */
void m1kern_target_dump_state(void);

/**
 * @brief request a context switch by pending PendSV
 *
 * safe from thread context and from any handler
 */
static inline void m1kern_target_pend_switch(void) {
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief set the exception priorities the switch depends on
 *
 * SysTick highest so the tick is never delayed, PendSV lowest so the switch
 * only happens once every other handler has drained
 */
static inline void m1kern_target_set_exc_prio(void) {
  NVIC_SetPriority(PendSV_IRQn, 0xFF);
  NVIC_SetPriority(SysTick_IRQn, 0x00);
}
