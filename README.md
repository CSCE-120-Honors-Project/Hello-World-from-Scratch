# DIY Bootloader
Our honors project for CSCE 120

## Table of Contents
* [What is our bootloader?](#what-is-our-bootloader)
* [Building the program](#building-the-program)

## What is our bootloader?

## Building the program
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
