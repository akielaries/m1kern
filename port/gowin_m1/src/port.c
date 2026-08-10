/*
 * gowin cortex-m1 port
 *
 * supplies the tick and the three exception handlers the switch depends on.
 * the vector table in startup_GOWIN_M1.S declares all of these weak, so
 * defining them here is all the wiring that is needed.
 */
#include "m1kern.h"
#include "m1kern_port.h"

#include "GOWIN_M1_misc.h"

void m1kern_port_tick_init(void) {
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  /*
   * SystemCoreClock is a compile-time constant on this part (see
   * system_GOWIN_M1.c). it is not read back from any PLL, it is whatever
   * clock the fabric was synthesized with, so it must match the SoC build.
   */
  SysTick_Config(SystemCoreClock / M1KERN_TICK_HZ);
}

/*
 * kernel_start issues svc #0 to get the very first switch going. all this has
 * to do is pend PendSV; the scheduler runs from there.
 */
void SVC_Handler(void) {
  m1kern_port_pend_switch();
}

/*
 * the kernel might not be running! this is only initially though on boot i
 * think?
 *
 * send us to pendSV to service anything that's "pending"? this is essentially
 * the "preemption" part of this system w/ a quantum of 1ms....
 * this fires every 1ms at highest priority, preempting whatever thread is
 * running. It increments time and pends PendSV. Since PendSV is lowest
 * priority it won't actually run until SysTick returns, but that's fine
 * the pending bit stays set. This is what makes the scheduler preemptive
 * rather than cooperative. A thread can't hold the CPU longer than 1ms
 * regardless of whether it yields.
 */
void SysTick_Handler(void) {
  system_time_ms++;

  if (kernel_running) {
    m1kern_port_pend_switch();
  }
}

void HardFault_Handler(void) {
  register uint32_t msp __asm("r0");
  __asm volatile("mrs r0, msp" : "=r"(msp));
  M1KERN_LOG("FAULT msp=0x%08X\r\n", msp);
  __asm volatile("bkpt #0"); // breakpoint for gdb!

  /* Go to infinite loop when Hard Fault exception occurs */
  while (1) {
  }
}
