cmake_minimum_required(VERSION 3.15)
project(OppGen3 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Toolchain file (must be created in project root as arm-none-eabi-gcc.cmake)
#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake")

# === CMSIS and system sources ===
set(CMSIS_SOURCES
    ${CMAKE_SOURCE_DIR}/Src/system_stm32f1xx.c
    ${CMAKE_SOURCE_DIR}/Src/stm32f1xx_it.c
    ${CMAKE_SOURCE_DIR}/Src/stm32f1xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/startup/startup_stm32f103xb.S
)

# === HAL driver sources (only include the ones actually used in your project) ===
# Start with the common ones that almost every project needs, then add peripherals you use.
set(HAL_SOURCES
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    #${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c

    # Add only the peripheral drivers you actually use:
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
    # ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c
    # etc.
)

# === Middleware sources ===
set(MIDDLEWARE_SOURCES
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
)

# === Application sources (list every .c file you have in Src/) ===
set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/Src/main.c
    #${CMAKE_SOURCE_DIR}/Src/syscalls.c
    #${CMAKE_SOURCE_DIR}/Src/usb_device.c
    #${CMAKE_SOURCE_DIR}/Src/usbd_cdc_if.c
    #${CMAKE_SOURCE_DIR}/Src/usbd_conf.c
    #${CMAKE_SOURCE_DIR}/Src/usbd_desc.c
    #${CMAKE_SOURCE_DIR}/Src/Common/debug.c
    #${CMAKE_SOURCE_DIR}/Src/Common/digital.c
    #${CMAKE_SOURCE_DIR}/Src/Common/fade.c
    #${CMAKE_SOURCE_DIR}/Src/Common/incand.c
    #${CMAKE_SOURCE_DIR}/Src/Common/lampMtrx.c
    #${CMAKE_SOURCE_DIR}/Src/Common/neopxl.c
    #${CMAKE_SOURCE_DIR}/Src/Common/rs232proc.c
    #${CMAKE_SOURCE_DIR}/Src/Common/servo.c
    #${CMAKE_SOURCE_DIR}/Src/Common/spiwing.c
    #${CMAKE_SOURCE_DIR}/Src/Common/stdldigio.c
    #${CMAKE_SOURCE_DIR}/Src/Common/stdlflash.c
    #${CMAKE_SOURCE_DIR}/Src/Common/stdlser.c
    #${CMAKE_SOURCE_DIR}/Src/Common/timer.c
)

# === Combine all sources ===
set(ALL_SOURCES
    ${APP_SOURCES}
    #${HAL_SOURCES}
    #${CMSIS_SOURCES}
)

add_executable(${PROJECT_NAME}.elf ${ALL_SOURCES})

# === Compile definitions ===
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    USE_HAL_DRIVER
    # STM32F103C8Tx
    STM32F103xB
)

# === Include directories ===
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Src/Common

    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include

    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
)

# Compiler options
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -march=armv7-m
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-Os>
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${CMAKE_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld
    -march=armv7-m
    -specs=nosys.specs
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${PROJECT_NAME}.map
)

# === Post-build: generate .hex, .bin and show size ===
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
)
