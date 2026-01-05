# Bootloader CMake file for OppGen3 project

set(BL_DEFINITIONS
    STM32F1
    STM32F103x6
    USBD_SOF_DISABLED
)

# Minimal HAL set for bootloader (e.g., flash, GPIO, UART for update)
set(BL_HAL_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
    # # Add CRC if doing integrity check, etc.
)

set(BL_CMSIS_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s
)

set(BL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usbd_devfs_asm.S
    ${CMAKE_CURRENT_SOURCE_DIR}/src/booty.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdc_loop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlflash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usbd_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usbd_devfs.c
)

set(BL_LINKER_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103x6.ld
)
