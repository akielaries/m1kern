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

# see targets/README.md for what a target supplies and how to add one

# --------------------------------------------------
# Options
# --------------------------------------------------
set(BOOT_MODE "flash_burn" CACHE STRING "Linker script: flash_burn or flash_xip (gowin_m1 only)")

# which target layer to build against. each lives in targets/<name>/ and
# supplies the same small contract: a device header, a startup file, a linker
# script, and the handlers in src/target.c
set(M1KERN_TARGET "gowin_m1" CACHE STRING "Target: gowin_m1 or m1core")

# the core clock the fabric was synthesised with. there is no PLL to read back,
# so this is a compile time constant and must match the SoC build. lowering it
# for a simulation build makes SysTick and the uart baud divider small enough
# to actually simulate
set(SYSTEM_CLOCK_HZ "25000000" CACHE STRING "Core clock in Hz")
set(TARGET_DIR ${CMAKE_SOURCE_DIR}/targets/${M1KERN_TARGET})
if(NOT EXISTS ${TARGET_DIR})
  message(FATAL_ERROR "no such target: ${M1KERN_TARGET}")
endif()
message(STATUS "m1kern target: ${M1KERN_TARGET}")

# the original firmware built with -ffixed-r8..r11, reserving the high
# registers from the compiler entirely. pendsv.S saves and restores r8-r11 on
# every switch, so this should not be load bearing, but it is left ON to match
# the known-good configuration. turn it OFF and run examples/05_preemption for
# a while to confirm before relying on it.
option(M1KERN_FIXED_HIGH_REGS "Reserve r8-r11 from the compiler" ON)

if(M1KERN_TARGET STREQUAL "gowin_m1")
  set(LINKER_SCRIPT ${TARGET_DIR}/bsp/startup/linker/GOWIN_M1_${BOOT_MODE}.ld)
  set(STARTUP ${TARGET_DIR}/bsp/startup/startup_GOWIN_M1.S)
  set(BSP_SRCS
    ${TARGET_DIR}/bsp/system/system_GOWIN_M1.c
    ${TARGET_DIR}/bsp/drivers/src/GOWIN_M1_uart.c
    ${TARGET_DIR}/bsp/drivers/src/GOWIN_M1_gpio.c
    ${TARGET_DIR}/bsp/drivers/src/GOWIN_M1_misc.c
  )
  set(BSP_INCLUDES
    ${TARGET_DIR}/bsp/system
    ${TARGET_DIR}/bsp/drivers/inc
  )
else()
  set(LINKER_SCRIPT ${TARGET_DIR}/bsp/startup/linker/m1core.ld)
  set(STARTUP ${TARGET_DIR}/bsp/startup/startup_M1CORE.S)
  set(BSP_SRCS
    ${TARGET_DIR}/bsp/system/system_M1CORE.c
    ${TARGET_DIR}/bsp/drivers/m1core_uart.c
    ${TARGET_DIR}/bsp/drivers/m1core_gpio.c
  )
  set(BSP_INCLUDES
    ${TARGET_DIR}/bsp/system
    ${TARGET_DIR}/bsp/drivers
  )
endif()

if(NOT EXISTS ${LINKER_SCRIPT})
  message(FATAL_ERROR "no linker script at ${LINKER_SCRIPT}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --------------------------------------------------
# Compile options
# --------------------------------------------------
set(CPU_FLAGS -mcpu=cortex-m1 -mthumb)

add_compile_options(
  ${CPU_FLAGS}
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

add_compile_definitions(SYSTEM_CLOCK_HZ=${SYSTEM_CLOCK_HZ}U)

if(M1KERN_FIXED_HIGH_REGS)
  add_compile_options(-ffixed-r8 -ffixed-r9 -ffixed-r10 -ffixed-r11)
endif()

# --------------------------------------------------
# bsp: vendor supplied, built as-is
# --------------------------------------------------
add_library(bsp STATIC ${BSP_SRCS})

# core_cm1.h and friends are plain CMSIS and shared by every target
target_include_directories(bsp PUBLIC
  targets/common/core
  ${BSP_INCLUDES}
)

# vendor code is not warning clean and is not ours to fix
target_compile_options(bsp PRIVATE -w)

# --------------------------------------------------
# m1kern: the kernel and its gowin M1 target layer
#
# OBJECT, not STATIC, and that matters. PendSV_Handler is only ever referenced
# by the weak vector table in startup_GOWIN_M1.S, so as an archive member
# pendsv.S is never pulled in and PendSV silently resolves to Default_Handler.
# an object library puts every object on the link line unconditionally, so the
# strong handler wins.
# --------------------------------------------------
add_library(m1kern OBJECT
  kernel/src/kernel.c
  kernel/src/pendsv.S
  ${TARGET_DIR}/src/target.c
)

target_include_directories(m1kern PUBLIC
  kernel/include
  ${TARGET_DIR}/include
)

target_link_libraries(m1kern PUBLIC bsp)

# --------------------------------------------------
# board support for the examples. not part of the kernel
# --------------------------------------------------
add_library(board OBJECT
  ${TARGET_DIR}/src/board.c
  ${TARGET_DIR}/src/console.c
  ${TARGET_DIR}/src/syscalls.c
  # supplies _start, which only makes sense under the -nostartfiles link below.
  # a firmware linking against libc's crt0 must not compile this
  ${TARGET_DIR}/src/crt0.c
)

target_link_libraries(board PUBLIC m1kern)

# --------------------------------------------------
# one elf + bin per example
# --------------------------------------------------
function(add_m1kern_example name)
  set(elf ${name}.elf)

  add_executable(${elf} examples/${name}/main.c ${STARTUP})

  # without this the startup leaves .bss uninitialized and relies on the
  # toolchain crt0 to zero it, which -nostartfiles has removed. every piece of
  # kernel state lives in .bss, so this is not optional
  # the gowin startup needs telling to clear .bss; the m1core one always does
  if(M1KERN_TARGET STREQUAL "gowin_m1")
    set_source_files_properties(${STARTUP}
      TARGET_DIRECTORY ${elf}
      PROPERTIES COMPILE_DEFINITIONS __STARTUP_CLEAR_BSS
    )
  endif()

  # both are OBJECT libraries and object files do not propagate transitively,
  # so each has to be named on the executable directly
  target_link_libraries(${elf} PRIVATE m1kern board)

  target_link_options(${elf} PRIVATE
    ${CPU_FLAGS}
    -T ${LINKER_SCRIPT}
    # startup_GOWIN_M1.S is the vector table and Reset_Handler, so we do not
    # want the toolchain's crt0 as well. recent arm-none-eabi ships picocrt,
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
