SCRIPT ?= start.s
OUTPUT ?= bootloader.elf
BINARY ?= bootloader.bin

# Source files to compile
SRCS_ASM = bootloader/start.s
SRCS_C = bootloader/main.c uart/uart.c filesystem/vio/vio.c filesystem/fat/fat.c

OBJECTS = $(SRCS_ASM:.s=.o) $(SRCS_C:.c=.o)

ifeq ($(OS), WINDOWS_NT)
    COMMAND_PREFIX := aarch64-none-elf
    SCRIPT_PREFIX := $(basename $(SCRIPT))
    OUTPUT_PREFIX := $(basename $(OUTPUT))
    BINARY_PREFIX := $(basename $(BINARY))
else ifeq ($(shell uname), Linux)
    COMMAND_PREFIX := aarch64-linux-gnu
    SCRIPT_PREFIX := $(basename $(SCRIPT))
    OUTPUT_PREFIX := $(basename $(OUTPUT))
    BINARY_PREFIX := $(basename $(BINARY))
else
    COMMAND_PREFIX := aarch64-elf
    SCRIPT_PREFIX := $(basename $(SCRIPT))
    OUTPUT_PREFIX := $(basename $(OUTPUT))
    BINARY_PREFIX := $(basename $(BINARY))
endif

# Include paths
INCLUDES = -Iuart -Ifilesystem/vio -Ifilesystem/fat

# Compiler flags
CFLAGS = -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -O2 $(INCLUDES)

.PHONY: help assemble link flatten clean build delete virtualize
.DEFAULT_GOAL := help

help:
	@echo "Available targets:"
	@echo "  assemble - Assemble all .s files"
	@echo "  link - Link all .o files to ELF"
	@echo "  flatten - Convert ELF to binary"
	@echo "  clean - Delete object and elf files"
	@echo "  build - Build everything"
	@echo "  delete - Delete all generated files"
	@echo "  virtualize - Run in QEMU"
	@echo "  virtualizeVinux - Run in QEMU with disk"

# Compile all assembly and C files
assemble: $(OBJECTS)

%.o: %.s
	$(COMMAND_PREFIX)-as -o $@ $<

%.o: %.c
	$(COMMAND_PREFIX)-gcc $(CFLAGS) -c -o $@ $<

# Link all object files
link: $(OBJECTS)
	$(COMMAND_PREFIX)-ld -T linker.ld $(OBJECTS) -o $(OUTPUT_PREFIX).elf

# Convert ELF to binary
flatten: $(OUTPUT_PREFIX).elf
	$(COMMAND_PREFIX)-objcopy $(OUTPUT_PREFIX).elf -O binary $(BINARY_PREFIX).bin

# Build everything
build: assemble link flatten

# Delete object files
clean:
	rm -f $(OBJECTS) *.elf

# Delete all generated files
delete: clean
	rm -f *.bin

# Run in QEMU (no disk)
virtualize: $(OUTPUT_PREFIX).elf
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(OUTPUT_PREFIX).elf

# Run in QEMU with disk
virtualizeVinux: $(OUTPUT_PREFIX).elf
	qemu-system-aarch64 -M virt -cpu cortex-a53 -m 256M -nographic \
		-kernel $(OUTPUT_PREFIX).elf \
		-drive file=disk.img,if=none,format=raw,id=hd0 \
		-device virtio-blk-device,drive=hd0

# Alias
virtualizen: virtualizeVinux
