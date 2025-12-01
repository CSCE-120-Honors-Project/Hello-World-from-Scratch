ENTRY(_start)

MEMORY {
    /* QEMU virt board loads executables at 0x40000000 */
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    . = 0x40000000;
    
    /* Boot code section */
    .text.boot : {
        KEEP(*(.text.boot))
        . = ALIGN(8);
    } > RAM
    
    /* Code section */
    .text : {
        *(.text*)
        . = ALIGN(8);
    } > RAM
    
    /* Read-only data */
    .rodata : {
        *(.rodata*)
        . = ALIGN(8);
    } > RAM
    
    /* Initialized data */
    .data : {
        *(.data*)
        . = ALIGN(8);
    } > RAM
    
    /* Uninitialized data */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > RAM
    
    /* Stack grows downward from end of RAM */
    . = ORIGIN(RAM) + LENGTH(RAM);
    __stack_top = .;
}
