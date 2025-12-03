#include "uart.h"

// uart registers
#define UART_BASE   0x09000000
#define UART_DR     (UART_BASE + 0x000)
#define UART_FR     (UART_BASE + 0x018)
// #define UART_CR     UART_BASE + 0x030

// flag register bits
#define FR_TXFF     (1 << 5) // if this bit is set, the transmit FIFO is full
// this is a 00100000
#define FR_RXFE     (1 << 4) // if this bit is set, the receive FIFO is empty
// this is a 00010000

// init is handled by qemu, so we dont have to do anything
// but it is good practice to have an init function (even if its empty) for completeness
void uart_init(void) {
    // nothing to do
}

// put a char to transmit FIFO
void uart_putc(char c) {
    while (*((volatile unsigned int*) UART_FR) & FR_TXFF) {
        // this loop waits until FR_TXFF is clear
        // the expression above converts the number definition of UART_FR, converts to pointer, derefs the pointer to get the data, and then bitwise and operator with FR_TXFF to get a 0 or nonzero value, which is taken by the while
    }

    // now write the character
    *((volatile unsigned int*) UART_DR) = c;
}

// put a string to transmit FIFO
// calls the putc function for the length of the string
void uart_puts(const char* s) {
    // go for the length of the c style string, so end at '\0' character
    while (*s != '\0') {
        // Convert newline to carriage return + newline
        if (*s == '\n') {
            uart_putc('\r');
        }
        // put the character we are currently on while incrementing
        uart_putc(*s++);
    }
}

// print a hexadecimal value
void uart_print_hex(unsigned long value) {
    uart_puts("0x");
    
    if (value == 0) {
        uart_putc('0');
        return;
    }
    
    char buffer[16];
    int len = 0;
    unsigned long temp = value;
    
    while (temp > 0) {
        buffer[len++] = (char)((temp % 16 < 10) ? ('0' + temp % 16) : ('a' + temp % 16 - 10));
        temp /= 16;
    }
    
    for (int i = len - 1; i >= 0; i--) {
        uart_putc(buffer[i]);
    }
}

// print a decimal value
void uart_print_dec(unsigned long value) {
    if (value == 0) {
        uart_putc('0');
        return;
    }
    
    char buffer[20];
    int len = 0;
    unsigned long temp = value;
    
    while (temp > 0) {
        buffer[len++] = (char)('0' + temp % 10);
        temp /= 10;
    }
    
    for (int i = len - 1; i >= 0; i--) {
        uart_putc(buffer[i]);
    }
}
