/**
 * @file M1CORE.h
 * @brief device header for the m1core soft core
 *
 * CMSIS shaped, so core_cm1.h supplies SCB, SysTick and the NVIC helpers
 * unchanged. that works because m1core implements the architectural SCS at the
 * addresses ARM specifies, not because anything here is clever.
 *
 * the peripheral layout matches ARM's CMSDK blocks, which is also what Gowin's
 * eMPU M1 uses, so the register structs are the same shape as GOWIN_M1.h and
 * drivers port between the two with base address changes only.
 */
#ifndef M1CORE_H
#define M1CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * interrupt numbers
 *
 * deliberately the same numbering as Gowin's eMPU M1 so a target layer written
 * for one works on the other. entries m1core does not implement are still
 * listed, they simply never fire
 * ------------------------------------------------------------------------- */
typedef enum IRQn {
  /* Cortex-M1 processor exceptions */
  NonMaskableInt_IRQn = -14,
  HardFault_IRQn      = -13,
  SVCall_IRQn         = -5,
  PendSV_IRQn         = -2,
  SysTick_IRQn        = -1,

  /* m1core interrupts */
  UART0_IRQn   = 0,
  UART1_IRQn   = 1,
  TIMER0_IRQn  = 2,
  TIMER1_IRQn  = 3,
  GPIO0_IRQn   = 4,
  UARTOVF_IRQn = 5,
  RTC_IRQn     = 6,
  I2C_IRQn     = 7
} IRQn_Type;

/* ---------------------------------------------------------------------------
 * processor and core peripheral configuration
 * ------------------------------------------------------------------------- */
#define __CM1_REV              0x0100U
#define __NVIC_PRIO_BITS       2U        /* armv6-m implements two priority bits */
#define __Vendor_SysTickConfig 0U        /* use the CMSIS SysTick_Config */
#define __MPU_PRESENT          0U
#define __FPU_PRESENT          0U

#include "core_cm1.h"
#include <stdint.h>

/* ---------------------------------------------------------------------------
 * peripherals
 * ------------------------------------------------------------------------- */
typedef struct {
  __IO uint32_t DATA;       /*!< 0x000 tx on write, rx on read              */
  __IO uint32_t STATE;      /*!< 0x004 [0]TXBF [1]RXBF [2]TXOR [3]RXOR      */
  __IO uint32_t CTRL;       /*!< 0x008 [0]TXEN [1]RXEN [2]TXIRQEN [3]RXIRQEN */
  union {
    __I  uint32_t INTSTATUS; /*!< 0x00c pending interrupts                   */
    __O  uint32_t INTCLEAR;  /*!< 0x00c write one to clear                   */
  };
  __IO uint32_t BAUDDIV;    /*!< 0x010 system clocks per bit, minimum 16    */
} UART_TypeDef;

/* note: m1core's gpio is not yet CMSDK shaped, see docs/memory-map.md in the
   m1core tree. it is data/dir/set/clr rather than data/dataout/outenset */
typedef struct {
  __IO uint32_t DATA;       /*!< 0x000 output value                         */
  __IO uint32_t DIR;        /*!< 0x004 1 = drive the pin                    */
  __O  uint32_t SET;        /*!< 0x008 write ones to set                    */
  __O  uint32_t CLR;        /*!< 0x00c write ones to clear                  */
} GPIO_TypeDef;

/* ---------------------------------------------------------------------------
 * memory map, matching docs/memory-map.md in the m1core tree
 * ------------------------------------------------------------------------- */
#define ITCM_BASE       (0x00000000UL)
#define DTCM_BASE       (0x20000000UL)
#define AHB1PERIPH_BASE (0x40000000UL)
#define APB1PERIPH_BASE (0x50000000UL)

#define GPIO0_BASE      (AHB1PERIPH_BASE + 0x0000000UL)
#define UART0_BASE      (APB1PERIPH_BASE + 0x4000UL)
#define UART1_BASE      (APB1PERIPH_BASE + 0x5000UL)

#define GPIO0 ((GPIO_TypeDef *)GPIO0_BASE)
#define UART0 ((UART_TypeDef *)UART0_BASE)
#define UART1 ((UART_TypeDef *)UART1_BASE)

/* UART STATE bits */
#define UART_STATE_TXBF (1UL << 0)
#define UART_STATE_RXBF (1UL << 1)
#define UART_STATE_TXOR (1UL << 2)
#define UART_STATE_RXOR (1UL << 3)

/* UART CTRL bits */
#define UART_CTRL_TXEN      (1UL << 0)
#define UART_CTRL_RXEN      (1UL << 1)
#define UART_CTRL_TXIRQEN   (1UL << 2)
#define UART_CTRL_RXIRQEN   (1UL << 3)

#include "system_M1CORE.h"

#ifdef __cplusplus
}
#endif

#endif /* M1CORE_H */
