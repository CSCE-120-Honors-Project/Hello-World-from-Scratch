#ifndef UART_H
#define UART_H

#include <stdint.h>

// function prototypes
void uart_init(void);
void uart_putc(char c); // to put a single character
void uart_puts(const char* s); // to put a single c-style string (ptr to bytes ending with '\0')
void uart_print_hex(uint64_t value);
void uart_print_dec(uint32_t value);

#endif