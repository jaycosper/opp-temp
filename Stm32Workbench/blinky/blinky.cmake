# Bootloader CMake file for OppGen3 project

set(APP_DEFINITIONS
    STM32F1
    STM32F103xB
    USE_HAL_DRIVER
)

set(APP_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32f1xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
)

set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_stm32f103xb.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/blinky.c
)

set(APP_LINKER_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103XX_FLASH.ld
)
