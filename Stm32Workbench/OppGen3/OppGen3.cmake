# Application CMake file for OppGen3 project

set(APP_DEFINITIONS
    USE_HAL_DRIVER
    STM32F103xB
    "__weak=__attribute__((weak))"
    "__packed=__attribute__((__packed__))"
)

# === CMSIS and system sources ===
set(APP_CMSIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup/startup_stm32f103xb.S
)

# === HAL driver sources (only include the ones actually used in your project) ===
# Start with the common ones that almost every project needs, then add peripherals you use.
set(APP_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c

    # Additional peripheral drivers
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c
    # etc.
)

# === Application sources (list every .c file you have in Src/) ===
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/main.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/syscalls.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/usb_device.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_cdc_if.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_conf.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_desc.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/debug.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/digital.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/fade.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/incand.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/lampMtrx.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/neopxl.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/rs232proc.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/servo.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/spiwing.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/stdldigio.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/stdlflash.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/stdlser.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Src/Common/timer.c
)

# Linker script
set(APP_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld)
