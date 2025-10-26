# Notes for CSCE 120 Honors Project

## Table of Contents
* [Linker Scripts](#linker-scripts)
    * [The ENTRY Command](#the-entry-command)
    * [The MEMORY Command](#the-memory-command)

## Linker Scripts
Linker scripts are text files which explain how different sections of object (`.o`) files should be merged to create an executable (`.elf`) by assigning them different addresses. Linker scripts also include the various memory locations of the target board, as well as where code will be stored in that memory.

Linker scripts have an extension of `.ld` and are written in the GNU linker command language. Linker scripts are case sensitive, except for their commands, which are case-insensitive.

Linker scripts have several important commands:
* [ENTRY](#the-entry-command)
* [MEMORY](#the-memory-command)
* SECTIONS
* KEEP
* ALIGN
* AT>

### The ENTRY Command
The `ENTRY` command sents the "entry point address" information in the header of the output `.elf` file. This "entry point address" is the first function the program will execute. If the assembly code being linked has `global _start` or `global start` in it, this command supercedes it. 

**IMPORTANT NOTE:** bare metal Aarch64 architectures, on powerup, the reset vector (the memory address specified in the `RVBAR_EL3` read-only register) supercedes this command's specification. 

The `ENTRY` command isn't required to use, but is required when debugging the output `.elf` file using GDB or any other debugger.

The syntax for using the `ENTRY` command is as follows:
```
ENTRY(<function name>)
ENTRY(main)
```

> A note about `Reset_Handler`:  
> `Reset_Handler` is a piece of software executed after the system resets and sets up the environment C/C++ code requires. BIOS typically initializes this, so we don't have to worry about rolling out our own `Reset_Handler`.

### The MEMORY Command
The `MEMORY` command defines the locations of different memories present in the target device, as well as where those memory locations start addresses are and how large they are. Using this information, the linker can calculate the total code and memory used so far and throw errors if data, code, stack, or heap memory doesn't have enough space to fit into the available size.

This command allows fine-tuning the amount of memory available in the target device and allow different parts/sections of the program to occupy different regions in memory. Typically, linker scripts have a singular `MEMORY` command.

The syntax for the `MEMORY` command is as follows:
```
MEMORY {
    <NAME>(<ATTRS>): ORIGIN = <origin>, LENGTH = <len>
    FLASH(RX): ORIGIN=0x08000000, LENGTH=1024K
}
```
Where:
*  `<NAME>` is the name of the memory region. This name can be anything, as they don't have any specific meaning to the linker and are instead meant for later reference in the `SECTIONS` command.
* `<ATTRS>` is an optional list of attributes to give to the memory section. These attributes are descriptive and help the linker place sections in memory but don't actually enforce any of their properties. The attributes are:
    * `r`: Read-only sections
    * `w`: Read-write sections
    * `x`: Sections with executable code
    * `a`: Allocated sections that occupy memory when the program is run
    * `i` or `l`: Initialized sections that contain variables with explicit initial values that are loaded from the executable
    * `!`: Exclude any of the subsequent attributes
* `ORIGIN` is the memory address (in hexadecimal and prefixed with a `0x`, octal prefixed with a `0`, or decimal with no prefix) of the start of the memory region
* `LENGTH` is how much memory the memory region occupies. This can be suffixed with `K` or `M` to indicate kilobytes or megabytes.

The `ORIGIN` parameter supports calculations by adding/subtracting length (e.g. `0x20000000+116K`) to find memory address start positions.