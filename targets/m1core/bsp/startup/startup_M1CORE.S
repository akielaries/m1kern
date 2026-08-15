/*
 * m1core vector table and reset entry
 *
 * armv6-m has no VTOR, so the table must live at address zero. every handler
 * is weak and aliased to Default_Handler, which is how the kernel's target
 * layer can simply define SVC_Handler, PendSV_Handler and SysTick_Handler and
 * have them win at link time.
 */
    .syntax unified
    .cpu cortex-m1
    .thumb

    .section .vectors, "a"
    .align 2
    .globl __Vectors
__Vectors:
    .long   __StackTop
    .long   Reset_Handler
    .long   NMI_Handler
    .long   HardFault_Handler
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   SVC_Handler
    .long   0
    .long   0
    .long   PendSV_Handler
    .long   SysTick_Handler

    /* external interrupts, numbering matches gowin's empu m1 */
    .long   UART0_Handler
    .long   UART1_Handler
    .long   TIMER0_Handler
    .long   TIMER1_Handler
    .long   GPIO0_Handler
    .long   UARTOVF_Handler
    .long   RTC_Handler
    .long   I2C_Handler
    .globl  __Vectors_End
__Vectors_End:

    .text
    .thumb_func
    .align 2
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /*
     * set sp explicitly. hardware loads it from vector word 0 on a real reset,
     * but gdb's load sets pc only, so load-then-continue would otherwise run
     * with a stale stack pointer
     */
    ldr     r0, =__StackTop
    mov     sp, r0

    /* copy .data from its load address in itcm to dtcm */
    ldr     r1, =__etext
    ldr     r2, =__data_start__
    ldr     r3, =__data_end__
1:  cmp     r2, r3
    bhs     2f
    ldr     r0, [r1]
    str     r0, [r2]
    adds    r1, #4
    adds    r2, #4
    b       1b
2:

    /* zero .bss. every piece of kernel state lives there */
    ldr     r1, =__bss_start__
    ldr     r2, =__bss_end__
    movs    r0, #0
3:  cmp     r1, r2
    bhs     4f
    str     r0, [r1]
    adds    r1, #4
    b       3b
4:

    bl      SystemInit
    bl      _start
    b       .

    .align 2
    .thumb_func
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b       .

    .macro WEAK_HANDLER name
    .weak   \name
    .thumb_set \name, Default_Handler
    .endm

    WEAK_HANDLER NMI_Handler
    WEAK_HANDLER HardFault_Handler
    WEAK_HANDLER SVC_Handler
    WEAK_HANDLER PendSV_Handler
    WEAK_HANDLER SysTick_Handler
    WEAK_HANDLER UART0_Handler
    WEAK_HANDLER UART1_Handler
    WEAK_HANDLER TIMER0_Handler
    WEAK_HANDLER TIMER1_Handler
    WEAK_HANDLER GPIO0_Handler
    WEAK_HANDLER UARTOVF_Handler
    WEAK_HANDLER RTC_Handler
    WEAK_HANDLER I2C_Handler

    .end
