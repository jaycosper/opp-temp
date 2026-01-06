# Automatically derive target name from directory
get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(TARGET_ELF  ${TARGET_NAME}.elf)
set(TARGET_HEX  ${TARGET_NAME}.hex)
set(TARGET_BIN  ${TARGET_NAME}.bin)
set(TARGET_MAP  ${TARGET_NAME}.map)

# Include the target-specific config (must be named <dirname>.cmake)
include(${TARGET_NAME}.cmake)

add_executable(${TARGET_ELF}
    ${BL_SOURCES}
    ${BL_HAL_SOURCES}
    ${BL_CMSIS_SOURCES}
)

# === Target-specific compile definitions ===
target_compile_definitions(${TARGET_ELF} PRIVATE
    ${BL_DEFINITIONS}   # Expands to APP_DEFINITIONS or BOOT_DEFINITIONS
)

target_compile_options(${TARGET_ELF} PRIVATE
    -march=armv7-m
    -fdata-sections
    -ffunction-sections
    -Os
    -g3
    -flto
    -Wall
    -Wextra
    -Werror
)

# === Target-specific include directories ===
target_include_directories(${TARGET_ELF} PRIVATE
    inc
    src
)

target_link_options(${TARGET_ELF} PRIVATE
    -T${BL_LINKER_SCRIPT}
    -L${CMAKE_CURRENT_SOURCE_DIR} # to find sections.ld linker script
    -nostartfiles
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${TARGET_MAP}
)

add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   ${TARGET_ELF} ${TARGET_HEX}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_ELF} ${TARGET_BIN}
    COMMAND ${CMAKE_SIZE} ${TARGET_ELF}
    COMMENT "Generating ${TARGET_HEX} and ${TARGET_BIN}"
)

# Copy artifacts to known location
add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_BIN}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Gen3Images/${TARGET_NAME}-TEMP.bin   # Fixed known location
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
    ${BL_SOURCES}
    #${BL_HAL_SOURCES}
    #${BL_CMSIS_SOURCES}
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
            -I ${CMAKE_CURRENT_SOURCE_DIR}/inc
            -I ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${LINT_SOURCE_FILES} 2> ${CMAKE_BINARY_DIR}/../_logs/lint-${TARGET_NAME}.log
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck static analysis..."
        VERBATIM
    )
else()
    message(WARNING "cppcheck not found — lint target will be unavailable. Install with: sudo apt install cppcheck")
endif()
