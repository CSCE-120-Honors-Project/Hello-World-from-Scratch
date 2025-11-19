/* testingtwo.s - Simple test to prove bootloader works */
/* Prints "HELLO" to UART */

.section .text.boot
.global _start

/* UART0 base address for QEMU virt */
.equ UART0_BASE, 0x09000000
.equ UART_DR,    0x00
.equ UART_FR,    0x18
.equ FR_TXFF,    0x20

_start:
    /* Set up stack */
    ldr x1, =stack_top
    mov sp, x1
    
    /* Print "HELLO\n" */
    ldr x10, =UART0_BASE
    
    /* Print 'H' */
    mov w0, #'H'
    bl uart_putc
    
    /* Print 'E' */
    mov w0, #'O'
    bl uart_putc
    
    /* Print 'L' */
    mov w0, #'W'
    bl uart_putc
    
    /* Print 'L' */
    mov w0, #'D'
    bl uart_putc
    
    /* Print 'O' */
    mov w0, #'Y'
    bl uart_putc
    
    /* Print newline */
    mov w0, #'\n'
    bl uart_putc
    
    /* Print carriage return */
    mov w0, #'\r'
    bl uart_putc
    
    /* Infinite loop */
hang:
    wfe
    b hang

/* Simple UART putc function */
/* Input: w0 = character to print */
/* Uses: x10 = UART base address */
uart_putc:
    /* Wait for TX FIFO to have space */
1:  ldr w1, [x10, #UART_FR]
    tst w1, #FR_TXFF
    b.ne 1b
    
    /* Write character to data register */
    str w0, [x10, #UART_DR]
    ret
    