# Test Suite for FAT, VIO, and UART Drivers

This directory contains comprehensive tests for the FAT32 filesystem driver, VirtIO block driver, and UART driver.

## Prerequisites

- `aarch64-elf-gcc` toolchain (for cross-compilation)
- `qemu-system-aarch64` (for running tests)
- `mtools` (optional, for creating FAT32 disk images)

### Installing Prerequisites on macOS

```bash
# Install cross-compiler toolchain
brew install aarch64-elf-gcc

# Install QEMU
brew install qemu

# Install mtools (optional, for disk image creation)
brew install mtools
```

## Building Tests

Build all test executables:
```bash
make all
```

Build individual tests:
```bash
make test_uart.elf
make test_vio.elf
make test_fat.elf
```

## Creating Test Disk Image

Create a 100MB disk image with FAT32 partition:
```bash
make disk
```

This will create `test_disk.img` with:
- MBR partition table
- FAT32 filesystem on partition 0
- A test file `TEST.TXT` (if mtools is installed)

## Running Tests

### UART Test
Tests basic UART driver functionality (character and string output):
```bash
make test-uart
```

Expected output:
- Basic string output test
- Character output test
- Newline conversion test
- Special characters test
- Numbers test

### VIO Test
Tests VirtIO block driver (requires disk image):
```bash
make test-vio
```

Expected output:
- VirtIO device initialization
- Single sector read (MBR)
- Multiple sector reads
- Sector data display in hex

### FAT Test
Tests FAT32 filesystem driver (requires disk image):
```bash
make test-fat
```

Expected output:
- VirtIO and FAT initialization
- Partition mounting
- Filename formatting tests
- File opening and reading (if TEST.TXT exists)

## Test Structure

```
tests/
├── Makefile          # Build and run configuration
├── README.md         # This file
├── test.ld           # Linker script for QEMU virt board
├── start.s           # Minimal startup assembly code
├── test_uart.c       # UART driver tests
├── test_vio.c        # VirtIO driver tests
└── test_fat.c        # FAT32 driver tests
```

## Makefile Targets

- `make all` - Build all test executables
- `make test-uart` - Build and run UART test
- `make test-vio` - Build and run VIO test (requires disk)
- `make test-fat` - Build and run FAT test (requires disk)
- `make disk` - Create test disk image
- `make clean` - Remove all build artifacts
- `make help` - Display available targets

## QEMU Configuration

Tests run on QEMU with the following configuration:
- **Machine**: QEMU virt board
- **CPU**: Cortex-A53
- **Memory**: 128MB
- **Serial**: UART output to stdio
- **Storage**: VirtIO block device (for VIO and FAT tests)

## Exiting QEMU

To exit QEMU during test execution:
- Press `Ctrl+A`, then `X`

Or from another terminal:
```bash
killall qemu-system-aarch64
```

## Troubleshooting

### "aarch64-elf-gcc: command not found"
Install the AArch64 cross-compiler toolchain:
```bash
brew install aarch64-elf-gcc
```

### "mformat: command not found" during disk creation
Install mtools or use a pre-created disk image:
```bash
brew install mtools
```

### Tests hang or don't produce output
- Ensure UART base address (0x09000000) matches QEMU virt board
- Verify VirtIO MMIO address (0x0a000000) is correct
- Check that the test executable was built successfully

### FAT test reports "File not found"
This is expected if the disk image doesn't have TEST.TXT. The disk creation with mtools should create this file automatically.

## Notes

- All tests are compiled with `-ffreestanding -nostdlib` for bare-metal execution
- The linker script places code at 0x40000000 (QEMU default load address)
- Stack grows downward from the end of 128MB RAM
- BSS section is cleared during startup to ensure proper C runtime environment
