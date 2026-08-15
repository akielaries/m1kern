#include "M1CORE.h"

#ifndef SYSTEM_CLOCK_HZ
#define SYSTEM_CLOCK_HZ 25000000U
#endif

uint32_t SystemCoreClock = SYSTEM_CLOCK_HZ;

void SystemInit(void) {
  /*
   * nothing to do. there is no PLL, no flash wait state configuration and no
   * vector table relocation: armv6-m has no VTOR and m1core's table is fixed
   * at address zero
   */
}

void SystemCoreClockUpdate(void) {
  SystemCoreClock = SYSTEM_CLOCK_HZ;
}
