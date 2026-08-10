/*
 * 02_sleep - sleeping, waking, and what the idle path costs
 *
 * three threads sleep on unrelated periods and print when they wake. the
 * point is the drift column: thread_sleep_ms(n) sets an absolute wake_time of
 * now + n, so the wake is late by however long the thread waited for the CPU,
 * and that error does not accumulate across iterations.
 *
 * when every thread is sleeping, scheduler_next returns NULL and PendSV parks
 * in WFI until the next tick. there is no idle thread in this example and
 * none is needed.
 *
 * expected: three interleaved streams, drift staying within a tick or two.
 */
#include "m1kern.h"
#include "board.h"

static void report(const char *name, uint32_t *expected, uint32_t period) {
  uint32_t now = system_time_ms;

  console_printf("[%s] t=%u ms  drift=%d ms\r\n",
                 name,
                 now,
                 (int)(now - *expected));

  *expected = now + period;
}

THREAD_STACK(sleeper_a, 512);
THREAD_FUNCTION(sleeper_a_fn, arg) {
  uint32_t expected = system_time_ms + 300;
  while (1) {
    thread_sleep_ms(300);
    report("a", &expected, 300);
  }
}

THREAD_STACK(sleeper_b, 512);
THREAD_FUNCTION(sleeper_b_fn, arg) {
  uint32_t expected = system_time_ms + 700;
  while (1) {
    thread_sleep_ms(700);
    report("b", &expected, 700);
  }
}

THREAD_STACK(sleeper_c, 512);
THREAD_FUNCTION(sleeper_c_fn, arg) {
  uint32_t expected = system_time_ms + 1100;
  while (1) {
    thread_sleep_ms(1100);
    report("c", &expected, 1100);
  }
}

int main(void) {
  board_init();
  kernel_init();

  mkthd_static(sleeper_a, sleeper_a_fn, sizeof(sleeper_a), PRIO_NORMAL, NULL);
  mkthd_static(sleeper_b, sleeper_b_fn, sizeof(sleeper_b), PRIO_NORMAL, NULL);
  mkthd_static(sleeper_c, sleeper_c_fn, sizeof(sleeper_c), PRIO_NORMAL, NULL);

  console_printf("02_sleep: starting\r\n");
  kernel_start();
  return 0;
}
