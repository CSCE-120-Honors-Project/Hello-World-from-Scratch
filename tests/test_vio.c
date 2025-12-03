#include "../uart/uart.h"
#include "../filesystem/vio/vio.h"

// Helper function to print a byte in hex
static void print_hex_byte(uint8_t byte) {
    const char hex[] = "0123456789ABCDEF";
    uart_putc(hex[byte >> 4]);
    uart_putc(hex[byte & 0x0F]);
}

// Helper function to print a 32-bit value in hex
static void print_hex32(uint32_t value) {
    uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        print_hex_byte((value >> (i * 4)) & 0xFF);
    }
}

// Test VirtIO block driver functionality
int main(void) {
    uart_init();

    uart_puts("=== VirtIO Block Driver Test ===\n");

    // Test 1: Initialize VirtIO device
    uart_puts("Test 1: Initializing VirtIO device...\n");
    if (vio_init() < 0) {
        uart_puts("FAIL - VirtIO initialization failed\n");
        return -1;
    }
    uart_puts("PASS - VirtIO initialized successfully\n");

    // Test 2: Read sector 0 (MBR)
    uart_puts("\nTest 2: Reading sector 0 (MBR)...\n");
    uint8_t sector_buffer[512];
    if (vio_read_sector(0, sector_buffer) < 0) {
        uart_puts("FAIL - Could not read sector 0\n");
        return -1;
    }
    uart_puts("PASS - Sector 0 read successfully\n");

    // Display MBR signature
    uart_puts("MBR Signature: ");
    print_hex_byte(sector_buffer[510]);
    uart_putc(' ');
    print_hex_byte(sector_buffer[511]);
    uart_putc('\n');

    // Test 3: Read multiple sectors
    uart_puts("\nTest 3: Reading multiple sectors (0-3)...\n");
    uint8_t multi_sector_buffer[512 * 4];
    if (vio_read_sectors(0, 4, multi_sector_buffer) < 0) {
        uart_puts("FAIL - Could not read multiple sectors\n");
        return -1;
    }
    uart_puts("PASS - Multiple sectors read successfully\n");

    // Test 4: Read a different sector
    uart_puts("\nTest 4: Reading sector 1...\n");
    if (vio_read_sector(1, sector_buffer) < 0) {
        uart_puts("FAIL - Could not read sector 1\n");
        return -1;
    }
    uart_puts("PASS - Sector 1 read successfully\n");

    // Display first 16 bytes of sector 1
    uart_puts("First 16 bytes of sector 1:\n");
    for (int i = 0; i < 16; i++) {
        print_hex_byte(sector_buffer[i]);
        uart_putc(' ');
        if ((i + 1) % 8 == 0) {
            uart_putc('\n');
        }
    }

    uart_puts("\n=== All VirtIO Tests Completed ===\n");
    
    return 0;
}
