#ifndef UART_H
#define UART_H

// function prototypes
void uart_init(void);
void uart_putc(char c);                      // put a single character
void uart_puts(const char* s);               // put a c-style string (null-terminated)
void uart_print_hex(unsigned long value);    // print a value in hexadecimal
void uart_print_dec(unsigned long value);    // print a value in decimal

#endif