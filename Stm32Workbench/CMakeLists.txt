cmake_minimum_required(VERSION 3.15)

# Main project (name is just for IDE/display)
project(OPP LANGUAGES C ASM)

# Load toolchain (passed from command line)
# Note: Do NOT set CMAKE_TOOLCHAIN_FILE here — it's set via -DCMAKE_TOOLCHAIN_FILE

# Common settings shared across all targets
add_compile_definitions(
    USE_HAL_DRIVER
    STM32F103xB                  # Adjust to your exact MCU
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
add_subdirectory(OppGen3)
add_subdirectory(Booty)