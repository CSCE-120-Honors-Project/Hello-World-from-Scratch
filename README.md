# DIY Bootloader
Our honors project for CSCE 120

## Table of Contents
* [What is our Bootloader?](#what-is-our-bootloader)
* [Building the Program](#building-the-program)
* [The Linker Script](#the-linker-script)

## What is our Bootloader?

## Building the Program
The provided [Makefile](Makefile) provides a platform-agnostic way of building the program from source.
> [!IMPORTANT]
> To build the program, the following build tools must be installed, depending on the operating system:
> * Windows: [Windows Aarch64 Bare Metal Target ARM Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads#:~:text=x86_64%2Darm%2Dnone%2Dlinux%2Dgnueabihf.exe.sha256asc-,AArch64%20bare%2Dmetal%20target%20(aarch64%2Dnone%2Delf),-arm%2Dgnu%2Dtoolchain%2D14.3.rel1%2Dmingw%2Dw64)
> * macOS: Install [Homebrew](https://brew.sh/) and run `brew install aarch64-elf-gcc`

```sh
# To assemble the program
make assemble SCRIPT=bootloader.s
# To link the program
make link SCRIPT=bootloader.o
# To flatten the executable
make flatten SCRIPT=bootloader.elf OUTPUT=bootloader.bin
# To delete the intermediate object and elf files
make clean
# To delete all object, elf, and binary
make delete
# To build the entire program in one command
make build SCRIPT=bootloader.s OUTPUT=bootloader.bin
# To run the program in QEMU
make virtualize BINARY=bootloader.bin 
```

## The Linker Script
The provided [linker script](linker.ld) is used to control the memory layout of the bootloader during the linking process.
It specifies the starting address of the bootloader in RAM and defines sections:
* `.text.boot`: Contains the bootloader's startup code, such as the code that zero-initializes the .bss section.
* `.text`: Contains the main code of the bootloader.
* `.rodata`: Contains read-only data, such as string literals.
* `.data`: Contains initialized global and static variables.
* `.bss`: Reserves space for uninitialized global and static variables. These variables may be initialized at runtime.

The linker script also includes symbols to track the start and end of the various sections, which can be used in the bootloader's code for memory management tasks.
Refer to **The Location Counter and Linker Script Symbols** section of [Kedar's Notes](notes/kedar/notes.md) for more information on how to use these symbols in `C/C++` code.

