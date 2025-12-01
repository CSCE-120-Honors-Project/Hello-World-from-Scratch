ENTRY(_start)

MEMORY {
    // Initialize flash memory, where bootloaders are typically stored
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128M
    // QEMU typically loads bare metal executables in RAM
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 512K
    // Define a memory region for the kernel load area
    KERNEL (rwx): ORIGIN = 0x40080000, LENGTH = 16M
}

// Each section has doubleword alignment to match Aarch64 alignment standard
SECTIONS = {
    // Boot code for the bootloader should be placed at the start of load memory
    .text.boot : {
        __text_boot_start = .;
        KEEP(*(.text.boot))
        . = ALIGN(8);
        __text_boot_end = .;
    }> RAM 
    // Executable code should be placed next
    .text : {
        __text_start = .;
        *(.text*)
        . = ALIGN(8);
        __text_end = .;
    }> RAM
    // Literals and data are placed last
    .rodata : {
        __rodata_start = .;
        *(.rodata*)
        . = ALIGN(8);
        __rodata_end = .;
    }> RAM
    .data : {
        __data_start = .;
        *(.data*)
        . = ALIGN(8);
        __data_end = .;
    }> RAM 
    // Reserve space for, but don't copy, the .bss section for uninitialized data
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }> RAM
}
