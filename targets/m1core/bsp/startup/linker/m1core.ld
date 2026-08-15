/*
 * m1core memory map. there is no flash: the debugger loads code straight into
 * ITCM, so .data is stored there and copied to DTCM by the reset handler
 */
MEMORY
{
  ITCM (rx)  : ORIGIN = 0x00000000, LENGTH = 32K
  DTCM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
}

ENTRY(Reset_Handler)

SECTIONS
{
  .vectors : {
    KEEP(*(.vectors))
  } > ITCM

  .text : {
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
    __etext = .;
  } > ITCM

  .data : AT (__etext) {
    . = ALIGN(4);
    __data_start__ = .;
    *(.data*)
    . = ALIGN(4);
    __data_end__ = .;
  } > DTCM

  .bss : {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > DTCM

  .heap (COPY) : {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapLimit = .;
  } > DTCM

  /* the stack occupies whatever is left at the top of DTCM */
  __StackTop   = ORIGIN(DTCM) + LENGTH(DTCM);
  __StackLimit = __HeapLimit;
  PROVIDE(__stack = __StackTop);

  ASSERT(__StackTop > __StackLimit, "DTCM overflowed: no room left for a stack")
}
