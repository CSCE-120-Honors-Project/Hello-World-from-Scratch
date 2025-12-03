**Where it lives:**

- **On disk**: Yes - stored in kernel.bin as compiled machine code
- **In RAM**: Yes - loaded to 0x40080000 when bootloader copies it from disk
- **In ELF**: Yes - in .text section with full metadata

**Where it comes from:**

- **Compiler**: Generates machine code from your C/assembly source
- **Linker**: Organizes all .text sections from different .o files, places at 0x40080000
- **Bootloader**: Physically copies from disk sector 64 to RAM at 0x40080000

**Lifecycle:**

1. Compile time: Your code (start.s, main.c, uart.c) → machine instructions
2. Link time: Linker combines all code, places starting at 0x40080000
3. Build time: objcopy creates kernel.bin with raw code
4. Deploy time: dd writes to disk.img at sector 64
5. Boot time: Bootloader reads from disk, copies to 0x40080000
6. Runtime: CPU executes instructions directly from this RAM location
7. Shutdown: Remains in RAM until power off or overwritten