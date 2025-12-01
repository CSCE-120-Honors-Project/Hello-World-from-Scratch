SCRIPT ?= start.s
OUTPUT ?= bootloader.elf
BINARY ?= bootloader.bin

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

.PHONY: help assemble link flatten clean build delete virtualize
.DEFAULT_GOAL := help

help:
	@echo "Available targets:"
	@echo "  assemble SCRIPT=<file.s>                    - Assemble an assembly file"
	@echo "  link SCRIPT=<file.o> OUTPUT=<file.elf>      - Link object file to ELF"
	@echo "  flatten OUTPUT=<file.elf> BINARY=<file.bin> - Convert ELF to binary"
	@echo "  clean                                       - Delete object and elf files"
	@echo "  build SCRIPT=<file.s>                       - Build everything"
	@echo "  delete                                      - Delete all generated files"
	@echo "  virtualize BINARY=<file.bin>                - Run in QEMU"

assemble:
	$(COMMAND_PREFIX)-as $(SCRIPT_PREFIX).s -o $(SCRIPT_PREFIX).o

link:
	$(COMMAND_PREFIX)-ld -T linker.ld $(SCRIPT_PREFIX).o -o $(OUTPUT_PREFIX).elf

flatten:
	$(COMMAND_PREFIX)-objcopy $(OUTPUT_PREFIX).elf -O binary $(BINARY_PREFIX).bin

clean:
	rm -f *.o *.elf

build: assemble link flatten clean

buildn: assemble link flatten clean virtualizen

delete: clean
	rm -f *.bin

virtualize:
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(BINARY_PREFIX).bin
	
virtualizeVinux:
	#qemu-system-aarch64 -M virt -cpu cortex-a53 
	#
	qemu-system-aarch64 -M virt -cpu cortex-a53 -m 128M -nographic \
  	-kernel $(BINARY_PREFIX).bin \
	-drive file=disk.img,if=none,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0
	-nographic \
	
    

	

virtualizen:
	qemu-system-aarch64 -M virt -cpu cortex-a53 -kernel $(BINARY_PREFIX).bin

