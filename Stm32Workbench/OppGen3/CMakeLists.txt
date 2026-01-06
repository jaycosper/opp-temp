# Automatically derive target name from directory
get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(TARGET_ELF  ${TARGET_NAME}.elf)
set(TARGET_HEX  ${TARGET_NAME}.hex)
set(TARGET_BIN  ${TARGET_NAME}.bin)
set(TARGET_MAP  ${TARGET_NAME}.map)

# Include the target-specific config (must be named <dirname>.cmake)
include(${TARGET_NAME}.cmake)

add_executable(${TARGET_ELF}
    ${APP_SOURCES}
    ${APP_MIDDLEWARE_SOURCES}
    ${APP_HAL_SOURCES}
    ${APP_CMSIS_SOURCES}
)

# === Target-specific compile definitions ===
target_compile_definitions(${TARGET_ELF} PRIVATE
    ${APP_DEFINITIONS}   # Expands to APP_DEFINITIONS or BOOT_DEFINITIONS
)

target_compile_options(${TARGET_ELF} PRIVATE
    -march=armv7-m
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wextra
    -Werror

    $<$<CONFIG:Debug>:-Og -g3 -gdwarf-4>
    $<$<CONFIG:Release>:-Os>
)

# === Target-specific include directories ===
target_include_directories(${TARGET_ELF} PRIVATE
    Inc
    Drivers/STM32F1xx_HAL_Driver/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Include
)

target_link_options(${TARGET_ELF} PRIVATE
    -T${APP_LINKER_SCRIPT}
    -specs=nosys.specs
    -specs=nano.specs
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${TARGET_MAP}
)

# Make sure the bootloader binary exists before linking the app
add_dependencies(${TARGET_ELF} Booty.elf)

add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   ${TARGET_ELF} ${TARGET_HEX}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_ELF} ${TARGET_BIN}

    # Custom CRC update (requires python2 or python3)
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/updCrc.py
            -hex=${TARGET_HEX}
            -crc=crc32.bin

    # Inject CRC into ELF section
    COMMAND ${CMAKE_OBJCOPY} --update-section .app_crc32_sect=crc32.bin ${TARGET_ELF}

    COMMAND ${CMAKE_SIZE} ${TARGET_ELF}

    COMMENT "Generated hex/bin, updated CRC, and printed size"
)

# Copy artifacts to known location
add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_ELF}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Gen3Images/${TARGET_NAME}-TEMP.elf   # Fixed known location
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_HEX}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Gen3Images/${TARGET_NAME}-TEMP.hex   # Fixed known location
)

# =============================================================================
# Linting target using cppcheck
# =============================================================================

find_program(CPPCHECK cppcheck)

set(LINT_SOURCE_FILES
    ${APP_SOURCES}
    #${APP_MIDDLEWARE_SOURCES}
    #${APP_HAL_SOURCES}
    #${APP_CMSIS_SOURCES}
)

if(CPPCHECK)
    add_custom_target(lint-${TARGET_NAME}
        COMMAND ${CPPCHECK}
            --enable=all
            --suppress=missingIncludeSystem     # Suppress warnings about system headers
            --suppress=unmatchedSuppression     # Clean output
            --suppress=knownConditionTrueFalse  # Optional: reduce noise
            --inconclusive                      # Allow inconclusive checks
            --quiet                             # Only show errors/warnings
            --error-exitcode=1                  # Fail build if issues found
            --force                             # Check all configurations
            #--check-config                      # Check for configuration issues
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Inc
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
            -I ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
            ${LINT_SOURCE_FILES} 2> ${CMAKE_BINARY_DIR}/../_logs/lint-${TARGET_NAME}.log
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck static analysis..."
        VERBATIM
    )
else()
    message(WARNING "cppcheck not found — lint target will be unavailable. Install with: sudo apt install cppcheck")
endif()
# find_program(CLANG_TIDY clang-tidy)
#
# if(CLANG_TIDY)
#     add_custom_target(lint
#         COMMAND ${CLANG_TIDY}
#             -checks='*,-llvm-*,-google-*,-cert-*'  # Customize checks
#             -header-filter=.*
#             --quiet
#             ${ALL_SOURCE_FILES}
#             --
#             -I${CMAKE_SOURCE_DIR}/Inc
#             -I${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
#             # ... other includes
#         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#         COMMENT "Running clang-tidy..."
#     )
# endif()