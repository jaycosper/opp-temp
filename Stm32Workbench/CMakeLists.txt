cmake_minimum_required(VERSION 3.15)

# Main project (name is just for IDE/display)
project(OPP LANGUAGES C ASM)

# Load toolchain (passed from command line)
# Note: Do NOT set CMAKE_TOOLCHAIN_FILE here — it's set via -DCMAKE_TOOLCHAIN_FILE

# Common settings shared across all targets
add_compile_definitions(
)

add_link_options(
    -march=armv7-m
    -specs=nosys.specs
    -lc -lm -lnosys
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)

# Include common directories
include_directories(
    OppGen3/Drivers/STM32F1xx_HAL_Driver/Inc
    OppGen3/Inc
)

# Add subdirectories (each adds its own executable target)
add_subdirectory(Booty)
add_subdirectory(OppGen3)

# Optional: Clean entire output directories for specific targets
# (useful if you generate many files)
set_property(
    DIRECTORY ${CMAKE_SOURCE_DIR}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES
        ${CMAKE_BINARY_DIR}/OppGen3
        ${CMAKE_BINARY_DIR}/Booty
)
