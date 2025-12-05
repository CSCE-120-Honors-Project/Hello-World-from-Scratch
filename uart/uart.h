#ifndef UART_H
#define UART_H

#include <stdint.h>

// function prototypes
void uart_init(void);
void uart_putc(char c);                      // put a single character
void uart_puts(const char* s);               // put a c-style string (null-terminated)
void uart_print_hex(uint64_t value);         // print a value in hexadecimal
void uart_print_dec(uint32_t value);         // print a value in decimal

#endif