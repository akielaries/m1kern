/*
 * m1core target
 *
 * supplies the tick and the three exception handlers the switch depends on.
 * the vector table in startup_M1CORE.S declares all of these weak, so defining
 * them here is all the wiring that is needed.
 */
#include "m1kern.h"
#include "m1kern_target.h"

void m1kern_target_tick_init(void) {
  /*
   * m1core's SysTick has no external reference clock, so CLKSOURCE is fixed at
   * the core clock and there is nothing to select. SystemCoreClock is a compile
   * time constant: there is no PLL to read back, so it must match the clock the
   * fabric was synthesised with
   */
  SysTick_Config(SystemCoreClock / M1KERN_TICK_HZ);
}

void m1kern_target_dump_state(void) {
  M1KERN_LOG("SysTick LOAD: 0x%X\r\n", SysTick->LOAD);
  M1KERN_LOG("SysTick CTRL: 0x%X\r\n", SysTick->CTRL);
  M1KERN_LOG("__StackLimit: 0x%X\r\n", (uint32_t)&__StackLimit);
  M1KERN_LOG("__StackTop:   0x%X\r\n", (uint32_t)&__StackTop);
}

/*
 * kernel_start issues svc #0 to get the very first switch going. all this has
 * to do is pend PendSV; the scheduler runs from there.
 */
void SVC_Handler(void) {
  m1kern_target_pend_switch();
}

/*
 * fires every tick at the highest priority, preempting whatever thread is
 * running. it increments time and pends PendSV. since PendSV is lowest
 * priority the switch does not happen until every other handler has drained,
 * but the pending bit stays set, which is what makes the scheduler preemptive
 * rather than cooperative
 */
void SysTick_Handler(void) {
  system_time_ms++;

  if (kernel_running) {
    m1kern_target_pend_switch();
  }
}

void HardFault_Handler(void) {
  register uint32_t msp __asm("r0");
  __asm volatile("mrs r0, msp" : "=r"(msp));
  M1KERN_LOG("FAULT msp=0x%08X\r\n", msp);
  __asm volatile("bkpt #0");

  while (1) {
  }
}
