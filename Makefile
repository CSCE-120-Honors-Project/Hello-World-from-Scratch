# Top-level Makefile: simple shortcuts for CMake build + QEMU testing
# Usage examples:
#   make configure      # configure build directory with CMake
#   make build          # build all targets
#   make os             # build only the OS target
#   make bootloader     # build only the bootloader target
#   make run-os         # run the OS directly in QEMU
#   make run            # run firmware + disk in QEMU (disk must exist)
#   make disk           # create FAT32 disk image and install OS (requires sudo)
#   make clean          - remove build dir and disk image

# Tools (override on command line if needed)
CMAKE ?= cmake
QEMU ?= qemu-system-aarch64
MKFS_VFAT ?= mkfs.vfat
LOSETUP ?= losetup
FDISK ?= fdisk
DD ?= dd

# Build settings
BUILD_DIR ?= build
JOBS ?= $(shell nproc)

# Artifact locations produced by the CMake build
OS_ELF := $(BUILD_DIR)/os/os.elf
OS_BIN := $(BUILD_DIR)/os.bin
BOOTLOADER_ELF := $(BUILD_DIR)/bootloader/bootloader.elf

# Disk image settings
DISK_IMG ?= disk.img
DISK_SIZE_MB ?= 100

# QEMU options
QEMU_FLAGS ?= -M virt -cpu cortex-a53 -nographic

.PHONY: all configure build os bootloader clean distclean disk run run-os run-bootloader help info
.DEFAULT_GOAL := all

# Default: build everything
all: build
	@echo "Built targets; use 'make run' or 'make run-os' to test in QEMU."

# Build with CMake
configure:
	@echo "==> Configure with CMake"
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	@echo "==> Building (parallel=$(JOBS))"
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(JOBS)
	@echo "==> Build finished"

# Create FAT32 disk image by delegating to tests/Makefile (leverages tested mtools workflow)


disk: os
	@echo "==> Creating fresh FAT32 disk image"
	@DISK_SIZE_MB=$(DISK_SIZE_MB) OS_BIN=$(OS_BIN) DISK_IMG=$(DISK_IMG) ./scripts/make_disk.sh

# Run the OS directly in QEMU (bypass bootloader)
run-os: os
	@echo "==> Running OS directly in QEMU"
	@echo "Press Ctrl+A then X to exit QEMU"
	@$(QEMU) $(QEMU_FLAGS) -kernel $(OS_ELF)

# Run bootloader with disk image attached (firmware should load OS from disk)
run: build disk
	@echo "==> Running bootloader in QEMU (with disk)"
	@echo "Press Ctrl+A then X to exit QEMU"
	@$(QEMU) $(QEMU_FLAGS) -kernel $(BOOTLOADER_ELF) \
		-drive file=$(DISK_IMG),if=none,format=raw,id=hd \
		-device virtio-blk-device,drive=hd

# Run bootloader only (no disk)
run-bootloader: bootloader
	@echo "==> Running bootloader (no disk)"
	@$(QEMU) $(QEMU_FLAGS) -kernel $(BOOTLOADER_ELF)

# Clean build artifacts
clean:
	@echo "==> Cleaning build directory"
	@rm -rf $(BUILD_DIR)

# Remove build dir and disk image
distclean: clean
	@echo "==> Removing disk image"
	@rm -f $(DISK_IMG)

# Rebuild everything from scratch
rebuild: distclean all

# Show build information
info:
	@echo "=== Project Build Information ==="
	@echo "  Build directory: $(BUILD_DIR)"
	@echo "  Disk image:      $(DISK_IMG)"
	@echo "  OS ELF:          $(OS_ELF)"
	@echo "  OS BIN:          $(OS_BIN)"
	@echo "  Bootloader ELF:  $(BOOTLOADER_ELF)"
	@echo ""
	@echo "=== Available Targets ==="
	@echo "  make configure       - Configure CMake build directory"
	@echo "  make build           - Configure+Build (default for 'make')"
	@echo "  make os              - Build only OS target"
	@echo "  make bootloader      - Build only bootloader target"
	@echo "  make run-os          - Run OS directly in QEMU (no bootloader)"
	@echo "  make run             - Run bootloader (and disk if present) in QEMU"
	@echo "  make disk            - Create FAT32 disk image and install OS (requires sudo)"
	@echo "  make clean           - Remove build directory"
	@echo "  make distclean       - Remove build dir and disk image"
	@echo "  make rebuild         - Clean and rebuild everything"
	@echo "  make help            - Show this information"

# Help target (alias for info)
help: info
