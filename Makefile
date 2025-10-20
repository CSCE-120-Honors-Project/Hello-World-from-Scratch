SCRIPT ?= "*.s"
SCRIPT_PREFIX := $(shell echo $(SCRIPT) | sed "s/\..+$//")
OUTPUT ?= $(SCRIPT_PREFIX).elf
OUTPUT_PREFIX := $(shell echo $(OUTPUT) | sed "s/\..+$//")

ifeq ($(OS), WINDOWS_NT)
	COMMAND_PREFIX := aarch64-none-elf
else
	COMMAND_PREFIX := aarch64-elf
endif

.PHONY: help assemble link flatten clean build delete
.DEFAULT_GOAL := help

help:
	@echo "Available targets:"
	@echo "  assemble SCRIPT=<file.s>                    - Assemble an assembly file(s) into object file(s) whose names match the inputs"
	@echo "  link SCRIPT=<file.o> OUTPUT=<file.elf>      - Link the object file(s) into an ELF file."
	@echo "  flatten SCRIPT=<file.elf> OUTPUT=<file.bin> - Flatten the ELF file into a binary file"
	@echo "  clean                                       - Delete all object and elf files"
	@echo "  build SCRIPT=<file.s> OUTPUT=<file.bin>     - Build the project and delete all object and elf files"
	@echo "  delete                                      - Delete all object, elf, and binary files"

assemble:
	$(COMMAND_PREFIX)-as $(SCRIPT_PREFIX).s

link:
	$(COMMAND_PREFIX)-ld -T linker.ld $(SCRIPT_PREFIX).o -o $(OUTPUT_PREFIX).elf

flatten:
	$(COMMAND_PREFIX)-objcopy $(SCRIPT_PREFIX).elf -O binary $(OUTPUT_PREFIX).bin

clean:
	rm -f *.o *.elf

build: assemble link flatten clean

delete: clean
	rm -f *.bin
