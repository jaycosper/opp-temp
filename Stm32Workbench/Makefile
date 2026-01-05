# Makefile for multi-target STM32 CMake project (app + bootloader)
# Build Structure
#   Stm32Workbench/
#   ├── Makefile                           # Build flow entry point (this file)
#   ├── CMakeLists.txt                     # Top-level CMake file
#   ├── arm-none-eabi-gcc.cmake            # Toolchain file
#   ├── Booty/
#   │   ├── CMakeLists.txt                 # Adds bootloader target, target-specific options
#   │   ├── Booty.cmake                    # Bootloader-specific files and settings
#   │   ├── src/
#   │   │   ├── booty.c
#   │   │   └── ...
#   │   ├── inc/
#   │   └── stm32F103x6.ld                 # Bootloader linker script
#   ├── OppGen3/
#   │   ├── CMakeLists.txt                 # Adds app target, target-specific options
#   │   ├── OppGen3.cmake                  # App-specific files and settings
#   │   ├── src/
#   │   │   ├── main.c
#   │   │   └── ...
#   │   ├── Inc/
#   |   ├── Drivers/
#   │   |   ├── STM32F1xx_HAL_Driver/
#   │   |   └── CMSIS/
#   |   ├── Middlewares/ST/STM32+USB_Device_Library/
#   │   |   |   ├── Class/
#   │   |   |   |   └── CDC/
#   │   |   |   └── Core/
#   │   └── STM32F103C8Tx_FLASH.ld.template    # Template for build-generated linker script
#   └── _logs/                             # build-generated logs
#   └── _build/                            # build-generated artifacts

BUILD_DIR     := _build
GENERATOR     := Ninja
BUILD_TYPE    ?= Debug                  # Allow override: make BUILD_TYPE=Release
TOOLCHAIN_FILE := arm-none-eabi-gcc.cmake

# Logging
LOG_DIR       := _logs
TIMESTAMP     := $(shell date +%Y-%m-%d_%H-%M-%S)
APP_LOG_FILE  := $(LOG_DIR)/app-build-log-$(TIMESTAMP).log
BL_LOG_FILE   := $(LOG_DIR)/bl-build-log-$(TIMESTAMP).log

# Create log directory
$(shell mkdir -p $(LOG_DIR))

# Default: build both targets
all: OppGen3 Booty

# Build only the application
OppGen3: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building OppGen3 application... (log: $(APP_LOG_FILE))"
	-@cmake --build $(BUILD_DIR) --target OppGen3.elf --verbose 2>&1 | tee $(APP_LOG_FILE)
	@echo "OppGen3 build complete! Output: _build/OppGen3/OppGen3.elf"
	@echo "Log saved to: $(APP_LOG_FILE)"

# Build only the bootloader
Booty: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building Booty bootloader... (log: $(BL_LOG_FILE))"
	-@cmake --build $(BUILD_DIR) --target Booty.elf --verbose 2>&1 | tee $(BL_LOG_FILE)
	@echo "Booty build complete! Output: _build/Booty/Booty.elf"
	@echo "Log saved to: $(BL_LOG_FILE)"

# Configure CMake (runs only if needed)
cmake-setup: $(BUILD_DIR)/CMakeCache.txt
	@echo "CMake setup complete ($(BUILD_TYPE) mode)."
	@echo "Build logs saved in $(LOG_DIR)/"
	@echo "Available targets: make OppGen3 | make Booty | make all"

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt $(TOOLCHAIN_FILE)
	@echo "Configuring CMake in $(BUILD_DIR)..."
	@rm -rf $(BUILD_DIR)          # Ensure clean configure on changes
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-G "$(GENERATOR)" \
		-DCMAKE_TOOLCHAIN_FILE=../$(TOOLCHAIN_FILE) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@echo "Configuration done."

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Clean build logs
clean-logs:
	@echo "Removing all build logs..."
	@rm -rf $(LOG_DIR)
	@echo "Logs cleaned."

# Full clean rebuild
rebuild: clean clean-logs cmake-setup all

lint:
	@cmake --build $(BUILD_DIR) --target lint

# Help
help:
	@echo "Available targets:"
	@echo "  make cmake-setup     - Configure CMake (or reconfigure)"
	@echo "  make OppGen3         - Build only the application"
	@echo "  make Booty           - Build only the bootloader"
	@echo "  make all             - Build both (default)"
	@echo "  make clean           - Remove build directory"
	@echo "  make clean-logs      - Remove all logs"
	@echo "  make rebuild         - Full clean + rebuild"
	@echo ""
	@echo "Override build type: make OppGen3 BUILD_TYPE=Release"

.PHONY: all OppGen3 Booty cmake-setup clean clean-logs rebuild help