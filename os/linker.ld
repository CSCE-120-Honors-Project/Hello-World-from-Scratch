ENTRY(_start)

MEMORY {
    /* os code loads at QEMU default kernel load address for independent testing */
    /* correct location for final product */
    /* KERNEL_RAM (rwx) : ORIGIN = 0x40080000, LENGTH = 128M */
    /* move location to start of ram just for testing */
    KERNEL_RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

/* Each section has doubleword alignment to match Aarch64 alignment standard */
SECTIONS { 
    /* assembly code for os has to be put at the start because it sets the rest up for the c (main) code */
    .text.boot : {
        __text_boot_start = . ;
        KEEP(*(.text.boot))
        . = ALIGN(8);
        __text_boot_end = . ;
    } > KERNEL_RAM

    /* executable, compiled C code goes next */
    .text : {
        __text_start = . ;
        *(.text*)
        . = ALIGN(8);
        __text_end = . ;
    } > KERNEL_RAM
 
    /* Literals and data are placed last */
    .rodata : {
        __rodata_start = .;
        *(.rodata*)
        . = ALIGN(8);
        __rodata_end = .;
    } > KERNEL_RAM
    
    .data : {
        __data_start = .;
        *(.data*)
        . = ALIGN(8);
        __data_end = .;
    } > KERNEL_RAM

    /* reserve space for bss section, but dont copy bc ts is empty */
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = . ;
        *(.bss*)
        *(COMMON)
        __bss_end = . ;

        /* put the stack here too */
        /* this way, the linker defines the stack top instead of defining it in the assembly */
        . = ALIGN(16);
        . = . + 4096;
        boot_stack_top = . ; /* use this symbol in assembly (start.s) */
    }
}