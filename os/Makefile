# Makefile for OS (alternative to CMake)

# Toolchain
CROSS_PREFIX := aarch64-linux-gnu
CC := $(CROSS_PREFIX)-gcc
AS := $(CROSS_PREFIX)-as
LD := $(CROSS_PREFIX)-ld
OBJCOPY := $(CROSS_PREFIX)-objcopy

# Directories
OS_DIR := .
UART_DIR := ../uart

# Source files
ASM_SRC := start.s
C_SRC := main.c
UART_SRC := $(UART_DIR)/uart.c

# Object files
ASM_OBJ := $(ASM_SRC:.s=.o)
C_OBJ := $(C_SRC:.c=.o)
UART_OBJ := $(UART_DIR)/uart.o

# Output files
ELF := os.elf
BIN := os.bin

# Compiler flags
CFLAGS := -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -O2 -I$(UART_DIR)
LDFLAGS := -T linker.ld -nostdlib

# Default target
.PHONY: all
all: $(BIN)

# Assemble start.s
$(ASM_OBJ): $(ASM_SRC)
	$(AS) -o $@ $<

# Compile main.c
$(C_OBJ): $(C_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile uart.c
$(UART_OBJ): $(UART_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link to create ELF
$(ELF): $(ASM_OBJ) $(C_OBJ) $(UART_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Convert ELF to binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "✓ OS binary created: $(BIN)"

# Clean
.PHONY: clean
clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(UART_OBJ) $(ELF) $(BIN)

# Test OS standalone
.PHONY: test
test: $(BIN)
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(BIN)
