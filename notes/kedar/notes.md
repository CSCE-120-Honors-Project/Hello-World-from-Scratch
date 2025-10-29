# Notes for CSCE 120 Honors Project

## Table of Contents
* [Linker Scripts](#linker-scripts)
    * [The ENTRY Command](#the-entry-command)
    * [The MEMORY Command](#the-memory-command)
    * [The SECTIONS Command](#the-sections-command)
    * [The Location Counter](#the-location-counter-and-linker-script-symbols)

## Linker Scripts
Linker scripts are text files which explain how different sections of object (`.o`) files should be merged to create an executable (`.elf`) by assigning them different addresses. Linker scripts also include the various memory locations of the target board, as well as where code will be stored in that memory.

Linker scripts have an extension of `.ld` and are written in the GNU linker command language. Linker scripts are case sensitive, except for their commands, which are case-insensitive.

Linker scripts have several important commands:
* [ENTRY](#the-entry-command)
* [MEMORY](#the-memory-command)
* [SECTIONS](#the-sections-command)
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
 
> **Note:** `Reset_Handler` is a piece of software executed after the system resets and sets up the environment C/C++ code requires. BIOS typically initializes this, so we don't have to worry about rolling out our own `Reset_Handler`.  
> The `Reset_Handler`'s address is stored in something called a vector table, which also stores information on where instructions are located to handle interrupts or exceptions. On ARM Cortex-A processors, the vector table contains the actual instructions used to handle exceptions or interrupts.

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

### The SECTIONS Command
The `SECTIONS` command creates different output sections in the final `.elf` file. This command tells the linker how to merge the different object files' sections and controls the order of the different sections of the final executable.

The syntax of the `SECTIONS` command is as follows:
```
SECTIONS {
    <SECTION_LABEL>: 

    }<ADDRESS_INFORMATION>

    .text: {

    }> <VMA> AT> <LMA>
}
```
Where: 
* `<SECTION_LABEL>` is the name of the section in the final `.elf` file, which can be any name given it follows certain naming conventions.
* `<ADDRESS_INFORMATION>` is the information of how that section should be stored and loaded in memory. The names specified here are the names present in the `MEMORY` command.
    * `<VMA>` is the virtual memory address of the section and comes first in address information. The virtual memory address of a section is where it resides in memory during code execution.
    * `<LMA>` is the load memory address of the section and comes second in address information. The load memory address of a section is where it resides when the system loads the program into memory initially.
    > **Note:** Typically, VMA and LMA are the same. They would differ in bootloader development, however there might be certain sections that reside in VMA instead of LMA for performance reasons or to reprogram flash memory.
    * If `<VMA>` and `<LMA>` are the same, only `<VMA>` needs to be specified in the `SECTION` command.

The order in which sections are linked matters based on the hardware specification and safety constraints, but aren't enforced by the linker itself. Typically, the order looks like this:
* The vector table containing instructions on how to handle exceptions or interrupts
* The `.text` section containing actual executable code
* The `.rodata` section containing read-only data like string literals and constants
* The `.data` section containing global and static variables with initial values.
* The `.bss` section containing uninitialized or zero-initialized variables and data.
* Any unused code memory.

For example, given two object files `main.o` and `led.o`, each with sections `.text`, `.data`, `.bss`, and `.rodata`, the `SECTIONS` portion of the linker script would look like this:
```
SECTIONS {
    .text: {
        *(.isr_vector) // * is wildcard notation
        *(.text)
        *(.rodata)
    }> FLASH

    .data: {
        *(.data)
    }> SRAM AT> FLASH

    .bss: {
        *(.bss)
    }> SRAM
}
```

### The Location Counter and Linker Script Symbols
The location counter is denoted by a `.` and tracks the boundaries of different sections during the mapping process. The location counter can also be explicitly set to certain values during the script, but can only appear in the `SECTIONS` command. 
The location counter tracks the virtual memory address location.

The location counter is used in conjunction with other linker script symbols. Symbols are the names of addresses. For example, if in a C/C++ program there's some code `int x = 67;`, the resultant `.o` file will match the symbol name `x` to the memory address at which the variable with name `x` is stored in the C/C++ program in something called a symbol table. Function names are also mapped to addresses as symbol names in the resultant `.o` file as well.
> **Note:** The C/C++ compiler looks at the variable definitions present in the program and creates symbols based on those variable names.  

When developing a linker script, we create our own symbols to identify the addresses of the boundaries between the different sections present in the program. These symbols are not variables and are local to the linker script. When the linker sees symbol definitions, it adds these definitions to the symbol table of the final `.elf` file.

The syntax for defining a symbol is as follows:
```
max_heap_size = 0x400;
```
The syntax and naming of symbol names are just like those of C/C++ variable names. 
Symbols defined in the `SECTIONS` command can make use of the location counter, e.g.:
```
SECTIONS {
    .text: {
        // At the start, the location counter = virtual memory address (FLASH)
        *(.text) // The location counter is incremented by the size of the .text sections
        // Remaining sections...
         // Now the location counter is at the end of the text section
        end_of_text = .; // Store its value in a new symbol
    } > FLASH
}
```
