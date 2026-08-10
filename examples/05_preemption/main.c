/*
 * 05_preemption - proof that the switch is preemptive, not cooperative
 *
 * three PRIO_LOW threads run tight loops and never call thread_yield or
 * thread_sleep_ms. under a cooperative scheduler the first one to start would
 * own the CPU forever and the other two counters would stay at zero.
 *
 * they don't, because SysTick_Handler pends PendSV every tick regardless of
 * what the running thread wants. the three counters should stay within a few
 * percent of each other, which is round robin among equals doing its job.
 *
 * the monitor thread is PRIO_NORMAL, so it beats all three whenever its sleep
 * expires and always gets to print.
 *
 * expected: three counters climbing together, roughly balanced.
 */
#include "m1kern.h"
#include "board.h"

static volatile uint32_t spin0_count = 0;
static volatile uint32_t spin1_count = 0;
static volatile uint32_t spin2_count = 0;

THREAD_STACK(spin0, 256);
THREAD_FUNCTION(spin0_fn, arg) {
  while (1) {
    spin0_count++;
  }
}

THREAD_STACK(spin1, 256);
THREAD_FUNCTION(spin1_fn, arg) {
  while (1) {
    spin1_count++;
  }
}

THREAD_STACK(spin2, 256);
THREAD_FUNCTION(spin2_fn, arg) {
  while (1) {
    spin2_count++;
  }
}

THREAD_STACK(monitor, 512);
THREAD_FUNCTION(monitor_fn, arg) {
  while (1) {
    thread_sleep_ms(2000);

    uint32_t a = spin0_count, b = spin1_count, c = spin2_count;
    uint32_t total = a + b + c;

    console_printf("t=%u  spin0=%u spin1=%u spin2=%u\r\n",
                   system_time_ms,
                   a,
                   b,
                   c);

    if (total > 0) {
      console_printf("       share %u%% / %u%% / %u%%\r\n",
                     (a * 100) / total,
                     (b * 100) / total,
                     (c * 100) / total);
    }

    led_toggle(LED0);
  }
}

int main(void) {
  board_init();
  kernel_init();

  mkthd_static(spin0, spin0_fn, sizeof(spin0), PRIO_LOW, NULL);
  mkthd_static(spin1, spin1_fn, sizeof(spin1), PRIO_LOW, NULL);
  mkthd_static(spin2, spin2_fn, sizeof(spin2), PRIO_LOW, NULL);
  mkthd_static(monitor, monitor_fn, sizeof(monitor), PRIO_NORMAL, NULL);

  console_printf("05_preemption: starting\r\n");
  kernel_start();
  return 0;
}
