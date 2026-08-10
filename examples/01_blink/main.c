/*
 * 01_blink - the smallest useful m1kern program
 *
 * two threads, two LEDs, two different periods. neither thread knows the
 * other exists; the only reason both make progress is that thread_sleep_ms
 * puts the caller to sleep and hands the CPU back to the scheduler.
 *
 * expected: LED0 at 2 Hz, LED1 at 1 Hz, both steady and independent.
 */
#include "m1kern.h"
#include "board.h"

THREAD_STACK(blink_fast, 256);
THREAD_FUNCTION(blink_fast_fn, arg) {
  while (1) {
    led_toggle(LED0);
    thread_sleep_ms(250);
  }
}

THREAD_STACK(blink_slow, 256);
THREAD_FUNCTION(blink_slow_fn, arg) {
  while (1) {
    led_toggle(LED1);
    thread_sleep_ms(500);
  }
}

int main(void) {
  board_init();
  kernel_init();

  mkthd_static(blink_fast, blink_fast_fn, sizeof(blink_fast), PRIO_NORMAL, NULL);
  mkthd_static(blink_slow, blink_slow_fn, sizeof(blink_slow), PRIO_NORMAL, NULL);

  console_printf("01_blink: starting\r\n");
  kernel_start(); // does not return
  return 0;
}
