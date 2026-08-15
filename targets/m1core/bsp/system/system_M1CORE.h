/**
 * @file system_M1CORE.h
 */
#ifndef SYSTEM_M1CORE_H
#define SYSTEM_M1CORE_H

#include <stdint.h>

/**
 * @brief system clock in Hz
 *
 * a compile time constant. m1core has no PLL to read back, so this is whatever
 * the fabric was synthesised with and it must match the SoC build. the tang
 * primer 25k board divides its 50 MHz oscillator by two
 */
extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#endif
