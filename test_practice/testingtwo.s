/* testingtwo.s - AArch64 PL011 UART on QEMU virt */

.section .text.boot
.global _start

.equ UART0_BASE, 0x09000000
.equ UART_DR,    0x00
.equ UART_FR,    0x18
.equ FR_TXFF,    0x20

_start:
    /* Set up stack using linker-provided symbol */
    ldr x1, =stack_top
    mov sp, x1
    
    /* Initialize UART base address */
    ldr x10, =UART0_BASE
    
    /* Print "HOWDY\r\n" */
    mov w0, #'H'
    bl uart_putc
    
    mov w0, #'O'
    bl uart_putc
    
    mov w0, #'W'
    bl uart_putc
    
    mov w0, #'D'
    bl uart_putc
    
    mov w0, #'Y'
    bl uart_putc
    
    mov w0, #'\r'
    bl uart_putc
    
    mov w0, #'\n'
    bl uart_putc
    
hang:
    wfe
    b hang

/* UART putc: w0 = character, x10 = UART base */
uart_putc:
1:  ldr w1, [x10, #UART_FR]
    tst w1, #FR_TXFF
    b.ne 1b
    
    str w0, [x10, #UART_DR]
    ret
