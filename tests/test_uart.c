#include "../uart/uart.h"

// Test UART driver functionality
int main(void) {
    uart_init();

    // Test 1: Basic string output
    uart_puts("=== UART Driver Test ===\n");
    uart_puts("Test 1: Basic string output - PASS\n");

    // Test 2: Character output
    uart_puts("Test 2: Character output - ");
    uart_putc('P');
    uart_putc('A');
    uart_putc('S');
    uart_putc('S');
    uart_putc('\n');

    // Test 3: Newline conversion
    uart_puts("Test 3: Newline conversion\n");
    uart_puts("Line 1\nLine 2\nLine 3\n");
    uart_puts("PASS\n");

    // Test 4: Special characters
    uart_puts("Test 4: Special characters - !@#$%^&*() - PASS\n");

    // Test 5: Numbers
    uart_puts("Test 5: Numbers - 0123456789 - PASS\n");

    uart_puts("\n=== All UART Tests Completed ===\n");
    
    return 0;
}
