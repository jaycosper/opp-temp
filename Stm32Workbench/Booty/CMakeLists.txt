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
    -Wall
    -Werror
    -Wextra

    $<$<CONFIG:Debug>:-Og -g3 -gdwarf-4>
    $<$<CONFIG:Release>:-Os>
)

# === Target-specific include directories ===
target_include_directories(${TARGET_ELF} PRIVATE
    inc
    src
)

target_link_options(${TARGET_ELF} PRIVATE
    -T${BL_LINKER_SCRIPT}
    -Wl,-Map=${TARGET_MAP}
)

add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   ${TARGET_ELF} ${TARGET_HEX}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_ELF} ${TARGET_BIN}
    COMMAND ${CMAKE_SIZE} ${TARGET_ELF}
    COMMENT "Generating ${TARGET_HEX} and ${TARGET_BIN}"
)

# Copy artifacts to known location
add_custom_command(TARGET Booty.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/Booty.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/../Gen3Images/Booty-TBD.bin   # Fixed known location
)
