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

# Create FAT32 disk image with kernel
disk: os
	@echo "==> Creating FAT32 disk image ($(DISK_IMG))"
	@if [ ! -f $(OS_BIN) ]; then \
		echo "Error: $(OS_BIN) not found. Build the OS first with 'make os'"; exit 1; \
	fi
	@echo "Creating $(DISK_SIZE_MB)MB zero file..."
	@$(DD) if=/dev/zero of=$(DISK_IMG) bs=1M count=$(DISK_SIZE_MB) status=none
	@echo "Partitioning image (single FAT32 partition)..."
	@printf "o\nn\np\n1\n2048\n\n\nt\nc\nw\n" | $(FDISK) $(DISK_IMG) > /dev/null 2>&1
	@LOOP_DEV=$$($(LOSETUP) -fP --show $(DISK_IMG)); \
	 echo "Using loop device: $$LOOP_DEV"; \
	 $(MKFS_VFAT) -F 32 $${LOOP_DEV}p1 > /dev/null 2>&1; \
	 sudo mkdir -p /mnt/disk_temp; \
	 sudo mount $${LOOP_DEV}p1 /mnt/disk_temp; \
	 sudo cp $(OS_BIN) /mnt/disk_temp/KERNEL.BIN; \
	 sudo umount /mnt/disk_temp; \
	 sudo rmdir /mnt/disk_temp; \
	 sudo $(LOSETUP) -d $$LOOP_DEV; \
	 echo "Disk image created: $(DISK_IMG)"

# Run the OS directly in QEMU (bypass bootloader)
run-os: os
	@echo "==> Running OS directly in QEMU"
	@echo "Press Ctrl+A then X to exit QEMU"
	@$(QEMU) $(QEMU_FLAGS) -kernel $(OS_ELF)

# Run bootloader with disk image attached (firmware should load OS from disk)
run: build
	@echo "==> Running firmware in QEMU (with disk if available)"
	@echo "Press Ctrl+A then X to exit QEMU"
	@if [ -f $(DISK_IMG) ]; then \
	  $(QEMU) $(QEMU_FLAGS) -kernel $(BOOTLOADER_ELF) -drive file=$(DISK_IMG),format=raw; \
	else \
	  $(QEMU) $(QEMU_FLAGS) -kernel $(BOOTLOADER_ELF); \
	fi

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
