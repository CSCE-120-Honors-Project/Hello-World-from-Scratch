ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 16M
}

SECTIONS {
    .text.boot : {
        __text_boot_start = .;
        KEEP(*(.text.boot))
        . = ALIGN(8);
        __text_boot_end = .;
    } > RAM

    .text : {
        __text_start = .;
        *(.text*)
        . = ALIGN(8);
        __text_end = .;
    } > RAM

    .rodata : { *(.rodata*) } > RAM
    .data : { *(.data*) } > RAM

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;

        /* small stack area */
        . = ALIGN(8);
        . = . + 4096;
        boot_stack_top = .;
    } > RAM
}
