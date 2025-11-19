# Bootloader Makefile for ARM Cortex-A53
# Works on both Windows and WSL/Linux

SCRIPT ?= start.s
OUTPUT ?= bootloader.elf
BINARY ?= bootloader.bin

# Detect OS and set toolchain prefix
ifeq ($(OS), WINDOWS_NT)
    COMMAND_PREFIX := aarch64-none-elf
    SCRIPT_PREFIX := $(basename $(SCRIPT))
    OUTPUT_PREFIX := $(basename $(OUTPUT))
    BINARY_PREFIX := $(basename $(BINARY))
else
    COMMAND_PREFIX := aarch64-linux-gnu
    SCRIPT_PREFIX := $(basename $(SCRIPT))
    OUTPUT_PREFIX := $(basename $(OUTPUT))
    BINARY_PREFIX := $(basename $(BINARY))
endif

# Tool names
AS := $(COMMAND_PREFIX)-as
LD := $(COMMAND_PREFIX)-ld
OBJCOPY := $(COMMAND_PREFIX)-objcopy

.PHONY: help assemble link flatten clean build delete virtualize
.DEFAULT_GOAL := help

help:
	@echo "Available targets:"
	@echo "  assemble SCRIPT=<file.s>                    - Assemble an assembly file into object file"
	@echo "  link SCRIPT=<file.o> OUTPUT=<file.elf>      - Link the object file into an ELF file"
	@echo "  flatten OUTPUT=<file.elf> BINARY=<file.bin> - Flatten the ELF file into a binary file"
	@echo "  clean                                       - Delete all object and elf files"
	@echo "  build SCRIPT=<file.s>                       - Build the project (assemble + link + flatten)"
	@echo "  delete                                      - Delete all object, elf, and binary files"
	@echo "  virtualize BINARY=<file.bin>                - Run the binary in QEMU"

assemble:
	$(AS) $(SCRIPT_PREFIX).s -o $(SCRIPT_PREFIX).o

link:
	$(COMMAND_PREFIX)-ld $(SCRIPT_PREFIX).o -o $(OUTPUT_PREFIX).elf

flatten:
	$(OBJCOPY) $(OUTPUT_PREFIX).elf -O binary $(BINARY_PREFIX).bin

clean:
	rm -f *.o *.elf

build: assemble link flatten clean

delete: clean
	rm -f *.bin

virtualize:
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(BINARY_PREFIX).bin