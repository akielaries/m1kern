/**
 * @file m1kern_port.h
 * @brief gowin cortex-m1 port layer
 *
 * the only file in the kernel build that knows the chip exists. everything it
 * pulls in from GOWIN_M1.h is ARM CMSIS (core_cm1.h) apart from
 * SystemCoreClock and the linker-provided stack symbols.
 */
#pragma once

#include <stdint.h>

#include "GOWIN_M1.h"

/** @brief provided by the linker script, reported by kernel_init */
extern uint32_t __StackLimit;
extern uint32_t __StackTop;

/**
 * @brief start the periodic tick that drives system_time_ms and preemption
 *
 * call before kernel_init. after this returns SysTick belongs to the kernel
 */
void m1kern_port_tick_init(void);

/**
 * @brief request a context switch by pending PendSV
 *
 * safe from thread context and from any handler
 */
static inline void m1kern_port_pend_switch(void) {
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief set the exception priorities the switch depends on
 *
 * SysTick highest so the tick is never delayed, PendSV lowest so the switch
 * only happens once every other handler has drained
 */
static inline void m1kern_port_set_exc_prio(void) {
  NVIC_SetPriority(PendSV_IRQn, 0xFF);
  NVIC_SetPriority(SysTick_IRQn, 0x00);
}
