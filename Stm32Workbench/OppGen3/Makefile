
# Makefile for STM32 CMake project with timestamped build logs

BUILD_DIR := _build
GENERATOR := Ninja
BUILD_TYPE := Debug
TOOLCHAIN_FILE := arm-none-eabi-gcc.cmake

# Log directory and timestamp format
LOG_DIR := _logs
TIMESTAMP := $(shell date +%Y-%m-%d_%H-%M-%S)
LOG_FILE := $(LOG_DIR)/build-log-$(TIMESTAMP).log

# Create log directory if it doesn't exist
$(shell mkdir -p $(LOG_DIR))

# Default target: build the project with timestamped log
all: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building project... (log: $(LOG_FILE))"
	echo @cmake --build $(BUILD_DIR) --verbose 2>&1 | tee $(LOG_FILE)
# @cmake --build $(BUILD_DIR) --verbose > $(LOG_FILE) 2>&1 || (echo "Build failed. See build.log for details." && exit 1)
	@cmake --build $(BUILD_DIR) --verbose 2>&1 | tee $(LOG_FILE)

# Set up CMake
cmake-setup: $(BUILD_DIR)/CMakeCache.txt
	@echo "CMake setup complete (Debug mode). Run 'make' to build."
	@echo "Build logs are saved in $(LOG_DIR)/"

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt $(TOOLCHAIN_FILE)
	@echo "Configuring CMake in $(BUILD_DIR)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-G "$(GENERATOR)" \
		-DCMAKE_TOOLCHAIN_FILE=../$(TOOLCHAIN_FILE) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@echo "Configuration done."

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Optional: clean logs too
clean-logs:
	@echo "Removing all build logs..."
	@rm -rf $(LOG_DIR)
	@echo "Logs cleaned."

# Full rebuild
rebuild: clean cmake-setup all

.PHONY: all cmake-setup clean clean-logs rebuild