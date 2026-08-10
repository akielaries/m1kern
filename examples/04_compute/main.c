/*
 * 04_compute - a long CPU-bound job that does not wreck responsiveness
 *
 * the compute thread runs a multi-second integer workload at PRIO_LOW while a
 * heartbeat thread keeps blinking at PRIO_NORMAL. no cooperative yield is
 * needed anywhere: SysTick preempts the compute thread every tick.
 *
 * the elapsed time it reports is wall clock, not CPU time, so it includes
 * every tick spent in the heartbeat thread. that gap is the scheduler
 * overhead plus whatever else was runnable, and it is the number worth
 * watching if you are sizing a workload.
 *
 * expected: heartbeat stays at a steady 1 Hz throughout each compute run.
 */
#include "m1kern.h"
#include "board.h"

static uint32_t lcg_state = 12345;

static uint32_t lcg_rand(void) {
  lcg_state = lcg_state * 1664525 + 1013904223;
  return lcg_state;
}

THREAD_STACK(heartbeat, 384);
THREAD_FUNCTION(heartbeat_fn, arg) {
  while (1) {
    led_toggle(LED0);
    console_printf("  heartbeat t=%u\r\n", system_time_ms);
    thread_sleep_ms(1000);
  }
}

THREAD_STACK(compute, 512);
THREAD_FUNCTION(compute_fn, arg) {
  while (1) {
    uint32_t iters  = 500000 + (lcg_rand() % 5000000);
    uint32_t result = 0;

    uint32_t t_start = system_time_ms;

    for (uint32_t i = 0; i < iters; i++) {
      result += i * i;
    }

    console_printf("compute: %u iters -> 0x%08X in %u ms\r\n",
                   iters,
                   result,
                   system_time_ms - t_start);

    thread_sleep_ms(500);
  }
}

int main(void) {
  board_init();
  kernel_init();

  mkthd_static(heartbeat, heartbeat_fn, sizeof(heartbeat), PRIO_NORMAL, NULL);
  mkthd_static(compute, compute_fn, sizeof(compute), PRIO_LOW, NULL);

  console_printf("04_compute: starting\r\n");
  kernel_start();
  return 0;
}
