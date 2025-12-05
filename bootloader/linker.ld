ENTRY(_start)

MEMORY {
    /* Bootloader loads at 0x40000000 for ARM64 QEMU virt machine */
    BOOTLOADER_RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

/* Each section has doubleword alignment to match Aarch64 alignment standard */
SECTIONS {
    /* Boot code for the bootloader should be placed at the start of load memory */
    .text.boot : {
        __text_boot_start = .;
        KEEP(*(.text.boot))
        . = ALIGN(8);
        __text_boot_end = .;
    } > BOOTLOADER_RAM
    
    /* Executable code should be placed next */
    .text : {
        __text_start = .;
        *(.text*)
        . = ALIGN(8);
        __text_end = .;
    } > BOOTLOADER_RAM
    
    /* Read-only data (strings, constants) */
    .rodata : {
        __rodata_start = .;
        *(.rodata*)
        . = ALIGN(8);
        __rodata_end = .;
    } > BOOTLOADER_RAM
    
    /* Initialized data */
    .data : {
        __data_start = .;
        *(.data*)
        . = ALIGN(8);
        __data_end = .;
    } > BOOTLOADER_RAM
    
    /* Reserve space for, but don't copy, the .bss section for uninitialized data */
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
        
        /* Allocate stack space for bootloader */
        . = ALIGN(16);
        . = . + 4096;
        boot_stack_top = .;
    } > BOOTLOADER_RAM
}
