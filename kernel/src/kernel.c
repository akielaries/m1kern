#include "m1kern.h"
#include "m1kern_target.h"

static thread_t *threads[MAX_THREADS];
static uint32_t thread_count = 0;

/* read and written by pendsv.S, so neither of these can be static */
thread_t *volatile current_thread = 0;
volatile uint8_t kernel_running   = 0;

volatile uint32_t system_time_ms = 0;

/* -------------------------------------------------- */
/* internal helpers */
/* -------------------------------------------------- */

/**
 * this creates a fake exception frame on the thread's stack so that when
 * PendSV restores it for the first time the core thinks it's returning from
 * an exception that was already in progress...
 */
static void init_stack(thread_t *new_thread, void (*entry)(void)) {
  uint32_t *sp = (uint32_t *)(new_thread->stack_mem + new_thread->stack_size);
  sp           = (uint32_t *)((uintptr_t)sp & ~7); // 8-byte align

  // hw exception frame (CPU unstacks these on EXC_RETURN)
  *(--sp) = 0x01000000;      // xPSR with the thumb bit set
  *(--sp) = (uint32_t)entry; // PC
  *(--sp) = 0xFFFFFFFD;      // LR (EXC_RETURN: Thread mode, PSP)
  *(--sp) = 0;               // R12
  *(--sp) = 0;               // R3
  *(--sp) = 0;               // R2
  *(--sp) = 0;               // R1
  *(--sp) = 0;               // R0

  // sw frame (PendSV restore pops these before EXC_RETURN)

  *(--sp) = 0; // R7
  *(--sp) = 0; // R6
  *(--sp) = 0; // R5
  *(--sp) = 0; // R4

  *(--sp)        = 0; // R11
  *(--sp)        = 0; // R10
  *(--sp)        = 0; // R9
  *(--sp)        = 0; // R8
  // here is the bottom of this stack frame. we point to R8
  // the thread has never run but looks to the CPU exactly like a thread that
  // was interrupted mid execution
  new_thread->sp = sp;
}

void kernel_init(void) {
  thread_count = 0;
  m1kern_target_dump_state();
}

thread_t *thread_create(thread_t *t,
                        void (*func)(void),
                        uint8_t *stack,
                        size_t stack_size,
                        uint8_t prio,
                        void *arg) {
  (void)arg;

  if (thread_count >= MAX_THREADS) {
    return NULL;
  }

  t->stack_mem  = stack;
  t->stack_size = stack_size;
  t->wake_time  = 0;
  t->state      = THREAD_READY;
  t->priority   = prio;

  init_stack(t, func);

  threads[thread_count++] = t;
  return t;
}

/* -------------------------------------------------- */

thread_t *scheduler_next(void) {
  // last thread that was serviced?
  static uint32_t last_thd_idx = 0;

  // wake any sleeping threads whose time has come
  for (uint32_t i = 0; i < thread_count; i++) {
    thread_t *t = threads[i];
    if (t->state == THREAD_SLEEPING && system_time_ms >= t->wake_time) {
      t->state = THREAD_READY;
    }
  }

  // find highest prio out of all the ready threads
  uint8_t highest_prio = 0;
  for (uint32_t i = 0; i < thread_count; i++) {
    if (threads[i]->state == THREAD_READY &&
        threads[i]->priority > highest_prio) {
      // set the highest priority found... i guess if there's multiple of the
      // highest prio then we use the first found?... or "round robin" all
      // threads found at that same priority?
      highest_prio = threads[i]->priority;
    }
  }

  for (uint32_t i = 1; i <= thread_count; i++) {
    uint32_t current_thd_idx = (last_thd_idx + i) % thread_count;
    thread_t *t              = threads[current_thd_idx];

    if (t->state == THREAD_READY && t->priority == highest_prio) {
      last_thd_idx = current_thd_idx;
      return t;
    }
  }

  return NULL;
}

void thread_yield(void) {
  // system control block->interrupt control and state reg
  // this bit is going to pend the pendSV exception. not directly invoking a
  // context switch but tells the processor to handle pendSV soon which DOES
  // handle the context switch (pendsv.S)
  m1kern_target_pend_switch();
}

void thread_sleep_ms(uint32_t ms) {
  if (current_thread == NULL) {
    M1KERN_LOG("ERROR: current_thread is NULL!\r\n");
    while (1) {
    }
  }

  current_thread->wake_time = system_time_ms + ms;
  current_thread->state     = THREAD_SLEEPING;
  thread_yield();
}

void kernel_start(void) {
  if (thread_count == 0) {
    return;
  }

  m1kern_target_set_exc_prio();

  uint32_t first_sp = (uint32_t)threads[0]->sp;

  kernel_running = 1;

  // svc #0 fires SVC_Handler which just pends PendSV. PendSV then runs the
  // scheduler for the first time with `current_thread == NULL`
  // SVC_Handler will set the PENDSVSET mask in the Interrupt Control and
  // State register
  __asm volatile("msr psp, %0             \n" // PSP = first thread stack
                 "ldr r0, =current_thread \n"
                 "movs r1, #0             \n"
                 "str r1, [r0]            \n" // current_thread = NULL
                 "movs r0, #2             \n"
                 "msr CONTROL, r0         \n"
                 "isb                     \n"
                 "svc #0                  \n" ::"r"(first_sp)
                 : "r0", "r1");
  while (1) {
  }
}
