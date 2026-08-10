cmake_minimum_required(VERSION 3.16)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_SIZE         arm-none-eabi-size)

# skip the compiler check, there is no libc to link a test binary against yet
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(m1kern C ASM)

# --------------------------------------------------
# Options
# --------------------------------------------------
set(BOARD_VARIANT "" CACHE STRING "Board variant: MEGA_138K or MEGA_60K")
set(BOOT_MODE "flash_burn" CACHE STRING "Linker script: flash_burn or flash_xip")

# the original firmware built with -ffixed-r8..r11, reserving the high
# registers from the compiler entirely. pendsv.S saves and restores r8-r11
# correctly on every switch, so this should not be load bearing, but it is
# left ON to match the known-good configuration. turn it OFF and run
# examples/05_preemption for a while to confirm before relying on it.
option(M1KERN_FIXED_HIGH_REGS "Reserve r8-r11 from the compiler" ON)

if(BOARD_VARIANT)
  add_compile_definitions(${BOARD_VARIANT})
endif()

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/bsp/startup/linker/GOWIN_M1_${BOOT_MODE}.ld)
if(NOT EXISTS ${LINKER_SCRIPT})
  message(FATAL_ERROR "no linker script for BOOT_MODE=${BOOT_MODE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --------------------------------------------------
# Shared CPU flags
# --------------------------------------------------
set(M1_CPU_FLAGS -mcpu=cortex-m1 -mthumb)

add_compile_options(
  ${M1_CPU_FLAGS}
  -O0
  -g3
  -Wall
  -Wextra
  -fmessage-length=0
  -fsigned-char
  -ffunction-sections
  -fdata-sections
  -std=gnu11
)

if(M1KERN_FIXED_HIGH_REGS)
  add_compile_options(-ffixed-r8 -ffixed-r9 -ffixed-r10 -ffixed-r11)
endif()

# --------------------------------------------------
# bsp: vendor supplied, built as-is
# --------------------------------------------------
add_library(bsp STATIC
  bsp/system/system_GOWIN_M1.c
  bsp/drivers/src/GOWIN_M1_uart.c
  bsp/drivers/src/GOWIN_M1_gpio.c
  bsp/drivers/src/GOWIN_M1_misc.c
)

target_include_directories(bsp PUBLIC
  bsp/core
  bsp/system
  bsp/drivers/inc
)

# vendor code is not warning clean and is not ours to fix
target_compile_options(bsp PRIVATE -w)

# --------------------------------------------------
# m1kern: the kernel plus its gowin port
#
# OBJECT, not STATIC, and that matters. PendSV_Handler is only ever referenced
# by the weak vector table in startup_GOWIN_M1.S, so as an archive member
# pendsv.S is never pulled in and PendSV silently resolves to Default_Handler.
# an object library puts every object on the link line unconditionally, so the
# strong handler wins.
# --------------------------------------------------
add_library(m1kern OBJECT
  m1kern/src/kernel.c
  m1kern/src/pendsv.S
  port/gowin_m1/src/port.c
  port/gowin_m1/src/crt0.c
)

target_include_directories(m1kern PUBLIC
  m1kern/include
  port/gowin_m1/include
)

target_link_libraries(m1kern PUBLIC bsp)

# --------------------------------------------------
# example support code
# --------------------------------------------------
add_library(example_common OBJECT
  examples/common/board.c
  examples/common/console.c
  examples/common/syscalls.c
)

target_include_directories(example_common PUBLIC examples/common)
target_link_libraries(example_common PUBLIC m1kern)

# --------------------------------------------------
# one elf + bin per example
# --------------------------------------------------
function(add_m1kern_example name)
  set(elf ${name}.elf)

  add_executable(${elf}
    examples/${name}/main.c
    bsp/startup/startup_GOWIN_M1.S
  )

  # without this the startup leaves .bss uninitialized and relies on the
  # toolchain crt0 to zero it, which -nostartfiles has removed. every piece of
  # kernel state lives in .bss, so this is not optional
  set_source_files_properties(bsp/startup/startup_GOWIN_M1.S
    TARGET_DIRECTORY ${elf}
    PROPERTIES COMPILE_DEFINITIONS __STARTUP_CLEAR_BSS
  )

  # both are OBJECT libraries and object files do not propagate transitively,
  # so each has to be named on the executable directly
  target_link_libraries(${elf} PRIVATE m1kern example_common)

  target_link_options(${elf} PRIVATE
    ${M1_CPU_FLAGS}
    -T ${LINKER_SCRIPT}
    # startup_GOWIN_M1.S is the vector table and Reset_Handler, so we do not
    # want the toolchain's crt0 as well. newer arm-none-eabi ships picocrt,
    # which expects linker symbols the Gowin script does not define
    -nostartfiles
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.map
  )

  set_target_properties(${elf} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

  add_custom_command(TARGET ${elf} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${elf}>
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.bin
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/size_summary.sh $<TARGET_FILE:${elf}>
  )
endfunction()

add_m1kern_example(01_blink)
add_m1kern_example(02_sleep)
add_m1kern_example(03_priority)
add_m1kern_example(04_compute)
add_m1kern_example(05_preemption)
