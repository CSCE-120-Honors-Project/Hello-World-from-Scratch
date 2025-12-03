# Test suite for FAT, VIO, and UART drivers
# Targets QEMU virt board with Cortex-A53 CPU

# Compiler and tools
CC = aarch64-elf-gcc
AS = aarch64-elf-as
LD = aarch64-elf-ld
OBJCOPY = aarch64-elf-objcopy

# Directories
UART_DIR = ../uart
VIO_DIR = ../filesystem/vio
FAT_DIR = ../filesystem/fat

# Compiler flags
CFLAGS = -Wall -Wextra -O0 -ffreestanding -nostdlib -nostartfiles \
         -mcpu=cortex-a53 -I$(UART_DIR) -I$(VIO_DIR) -I$(FAT_DIR)

ASFLAGS = -mcpu=cortex-a53

# Linker flags
LDFLAGS = -T test.ld -nostdlib

# QEMU settings
QEMU = qemu-system-aarch64
QEMU_FLAGS = -M virt -cpu cortex-a53 -m 128M -nographic -serial mon:stdio

# Disk image settings
DISK_IMG = test_disk.img
DISK_SIZE = 100M

# Source files
UART_SRC = $(UART_DIR)/uart.c
VIO_SRC = $(VIO_DIR)/vio.c
FAT_SRC = $(FAT_DIR)/fat.c
STARTUP_SRC = start.s

# Object files
UART_OBJ = uart.o
VIO_OBJ = vio.o
FAT_OBJ = fat.o
STARTUP_OBJ = start.o

# Test executables
TEST_UART = test_uart.elf
TEST_VIO = test_vio.elf
TEST_FAT = test_fat.elf

.PHONY: all clean test-uart test-vio test-fat disk help

# Default target
all: $(TEST_UART) $(TEST_VIO) $(TEST_FAT)

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build all test executables"
	@echo "  test-uart   - Build and run UART test"
	@echo "  test-vio    - Build and run VIO test (requires disk image)"
	@echo "  test-fat    - Build and run FAT test (requires disk image)"
	@echo "  disk        - Create a test disk image with FAT32 partition"
	@echo "  clean       - Remove all build artifacts"
	@echo ""
	@echo "Disk image commands:"
	@echo "  make disk   - Creates $(DISK_IMG) with FAT32 filesystem"

# Startup code
$(STARTUP_OBJ): $(STARTUP_SRC)
	$(AS) $(ASFLAGS) $< -o $@

# UART driver
$(UART_OBJ): $(UART_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# VIO driver
$(VIO_OBJ): $(VIO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# FAT driver
$(FAT_OBJ): $(FAT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# UART test
test_uart.o: test_uart.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_UART): test_uart.o $(UART_OBJ) $(STARTUP_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# VIO test
test_vio.o: test_vio.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_VIO): test_vio.o $(VIO_OBJ) $(UART_OBJ) $(STARTUP_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# FAT test
test_fat.o: test_fat.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_FAT): test_fat.o $(FAT_OBJ) $(VIO_OBJ) $(UART_OBJ) $(STARTUP_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Create test disk image with FAT32 partition
disk: $(DISK_IMG)

$(DISK_IMG):
	@echo "Creating $(DISK_SIZE) disk image..."
	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=$(shell echo $(DISK_SIZE) | sed 's/M//') 2>/dev/null
	@echo "Writing MBR and partition table..."
	@bash -c 'fn="$(DISK_IMG)"; total_bytes=$$(stat -c%s "$$fn"); total_sectors=$$((total_bytes / 512)); start_lba=2048; sectors_count=$$((total_sectors - start_lba)); tmp=$$(mktemp); printf %b \\x80\\x00\\x00\\x00 > $$tmp; printf %b \\x0C\\x00\\x00\\x00 >> $$tmp; b0=$$((start_lba & 0xFF)); b1=$$(((start_lba >> 8) & 0xFF)); b2=$$(((start_lba >> 16) & 0xFF)); b3=$$(((start_lba >> 24) & 0xFF)); printf %b "$(printf "\\\\x%02x\\\\x%02x\\\\x%02x\\\\x%02x" $$b0 $$b1 $$b2 $$b3)" >> $$tmp; sc0=$$((sectors_count & 0xFF)); sc1=$$(((sectors_count >> 8) & 0xFF)); sc2=$$(((sectors_count >> 16) & 0xFF)); sc3=$$(((sectors_count >> 24) & 0xFF)); printf %b "$(printf "\\\\x%02x\\\\x%02x\\\\x%02x\\\\x%02x" $$sc0 $$sc1 $$sc2 $$sc3)" >> $$tmp; dd if=$$tmp of="$$fn" bs=1 seek=446 conv=notrunc >/dev/null 2>&1; printf %b \\x55\\xAA | dd of="$$fn" bs=1 seek=510 conv=notrunc >/dev/null 2>&1; rm -f $$tmp; echo "MBR/partition written (shell)"'
	@echo "Formatting as FAT32..."
	# Create a loopback device (may require sudo on some systems)
	# Alternative: use mformat from mtools package
	@if command -v mformat > /dev/null; then \
		mformat -i $(DISK_IMG)@@1M -F -v TESTDISK :: ; \
		echo "Creating test file TEST.TXT..."; \
		echo "Hello from FAT32 filesystem!\nThis is a test file.\nLine 3 of test data." | mcopy -i $(DISK_IMG)@@1M - ::TEST.TXT 2>/dev/null || true; \
		echo "Disk image created successfully with mtools"; \
	else \
		echo "WARNING: mtools not found. Creating empty disk image."; \
		echo "Install mtools (brew install mtools) to create a proper FAT32 filesystem."; \
	fi

# Run UART test
test-uart: $(TEST_UART)
	@echo "Running UART test..."
	$(QEMU) $(QEMU_FLAGS) -kernel $(TEST_UART)

# Run VIO test (requires disk image)
test-vio: $(TEST_VIO) $(DISK_IMG)
	@echo "Running VIO test..."
	$(QEMU) $(QEMU_FLAGS) -kernel $(TEST_VIO) -drive file=$(DISK_IMG),if=none,format=raw,id=hd -device virtio-blk-device,drive=hd

# Run FAT test (requires disk image)
test-fat: $(TEST_FAT) $(DISK_IMG)
	@echo "Running FAT test..."
	$(QEMU) $(QEMU_FLAGS) -kernel $(TEST_FAT) -drive file=$(DISK_IMG),if=none,format=raw,id=hd -device virtio-blk-device,drive=hd

# Clean build artifacts
clean:
	rm -f *.o *.elf $(DISK_IMG)
	@echo "Clean complete"
