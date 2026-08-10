/*
 * 03_priority - strict priority, and the starvation that comes with it
 *
 * m1kern's scheduler is strict-priority with round robin only among equals.
 * there is no aging and no priority inheritance. a runnable PRIO_HIGH thread
 * means nothing below it ever runs.
 *
 * the hog thread demonstrates this the blunt way: it spins without sleeping
 * for two seconds at PRIO_HIGH, and during that window the two PRIO_NORMAL
 * counters stop dead even though SysTick is still preempting on schedule.
 * then it sleeps, and they resume.
 *
 * expected: normal counters climb, freeze for ~2 s while the hog runs, resume.
 */
#include "m1kern.h"
#include "board.h"

static volatile uint32_t count_a = 0;
static volatile uint32_t count_b = 0;

/* equal priority: these two should advance at roughly the same rate */
THREAD_STACK(counter_a, 384);
THREAD_FUNCTION(counter_a_fn, arg) {
  while (1) {
    count_a++;
    thread_sleep_ms(100);
  }
}

THREAD_STACK(counter_b, 384);
THREAD_FUNCTION(counter_b_fn, arg) {
  while (1) {
    count_b++;
    thread_sleep_ms(100);
  }
}

THREAD_STACK(hog, 512);
THREAD_FUNCTION(hog_fn, arg) {
  while (1) {
    thread_sleep_ms(4000);

    console_printf("hog: taking over, counters a=%u b=%u\r\n", count_a, count_b);

    // spin at PRIO_HIGH without yielding. SysTick still fires and PendSV
    // still runs, but the scheduler keeps handing the CPU right back
    uint32_t until = system_time_ms + 2000;
    while ((int32_t)(system_time_ms - until) < 0) {
      led_toggle(LED2);
    }

    console_printf("hog: releasing,   counters a=%u b=%u (unchanged)\r\n",
                   count_a,
                   count_b);
  }
}

THREAD_STACK(reporter, 512);
THREAD_FUNCTION(reporter_fn, arg) {
  while (1) {
    console_printf("t=%u  a=%u  b=%u\r\n", system_time_ms, count_a, count_b);
    thread_sleep_ms(500);
  }
}

int main(void) {
  board_init();
  kernel_init();

  mkthd_static(counter_a, counter_a_fn, sizeof(counter_a), PRIO_NORMAL, NULL);
  mkthd_static(counter_b, counter_b_fn, sizeof(counter_b), PRIO_NORMAL, NULL);
  mkthd_static(reporter, reporter_fn, sizeof(reporter), PRIO_NORMAL, NULL);
  mkthd_static(hog, hog_fn, sizeof(hog), PRIO_HIGH, NULL);

  console_printf("03_priority: starting\r\n");
  kernel_start();
  return 0;
}
