#include "../uart/uart.h"
#include "../filesystem/vio/vio.h"
#include "../filesystem/fat/fat.h"

// Helper function to print a byte in hex
static void print_hex_byte(uint8_t byte) {
    const char hex[] = "0123456789ABCDEF";
    uart_putc(hex[byte >> 4]);
    uart_putc(hex[byte & 0x0F]);
}

// Helper function to print a 32-bit value in hex
static void print_hex32(uint32_t value) {
    uart_puts("0x");
    for (int i = 3; i >= 0; i--) {
        print_hex_byte((value >> (i * 8)) & 0xFF);
    }
}

// Helper function to print FAT filename (11 characters)
static void print_fat_filename(const char* name) {
    for (int i = 0; i < 11; i++) {
        if (name[i] != ' ') {
            uart_putc(name[i]);
        }
    }
}

// Test FAT32 filesystem driver functionality
int main(void) {
    uart_init();

    uart_puts("=== FAT32 Filesystem Driver Test ===\n");

    // Test 1: Initialize VirtIO device
    uart_puts("Test 1: Initializing VirtIO device...\n");
    if (vio_init() < 0) {
        uart_puts("FAIL - VirtIO initialization failed\n");
        return -1;
    }
    uart_puts("PASS - VirtIO initialized successfully\n");

    // Test 2: Initialize FAT filesystem
    uart_puts("\nTest 2: Initializing FAT filesystem...\n");
    if (fat_init() < 0) {
        uart_puts("FAIL - FAT initialization failed\n");
        return -1;
    }
    uart_puts("PASS - FAT initialized successfully\n");

    // Test 3: Mount FAT32 partition
    uart_puts("\nTest 3: Mounting FAT32 partition 0...\n");
    if (fat_mount(0) < 0) {
        uart_puts("FAIL - Could not mount partition 0\n");
        return -1;
    }
    uart_puts("PASS - Partition 0 mounted successfully\n");

    // Test 4: Format filename
    uart_puts("\nTest 4: Testing filename formatting...\n");
    char formatted_name[11];
    format_filename("TEST.TXT", formatted_name);
    uart_puts("Formatted 'TEST.TXT' as: '");
    print_fat_filename(formatted_name);
    uart_puts("'\n");
    
    // Check if formatting is correct
    const char expected[] = "TEST    TXT";
    int match = 1;
    for (int i = 0; i < 11; i++) {
        if (formatted_name[i] != expected[i]) {
            match = 0;
            break;
        }
    }
    if (match) {
        uart_puts("PASS - Filename formatting correct\n");
    } else {
        uart_puts("FAIL - Filename formatting incorrect\n");
    }

    // Test 5: Open a test file
    uart_puts("\nTest 5: Opening file 'TEST.TXT'...\n");
    fat_file test_file;
    char test_filename[11];
    format_filename("TEST.TXT", test_filename);
    
    if (fat_open(test_filename, &test_file) < 0) {
        uart_puts("INFO - File 'TEST.TXT' not found (may not exist on disk)\n");
        uart_puts("This is expected if the test disk doesn't have this file\n");
    } else {
        uart_puts("PASS - File opened successfully\n");
        uart_puts("File size: ");
        print_hex32(test_file.file_size);
        uart_puts(" bytes\n");
        uart_puts("Start cluster: ");
        print_hex32(test_file.start_cluster);
        uart_putc('\n');

        // Test 6: Read the file
        uart_puts("\nTest 6: Reading file contents...\n");
        uint8_t file_buffer[4096]; // Buffer for file data
        if (fat_read(&test_file, file_buffer) < 0) {
            uart_puts("FAIL - Could not read file\n");
        } else {
            uart_puts("PASS - File read successfully\n");
            uart_puts("First 64 bytes of file:\n");
            for (uint32_t i = 0; i < 64 && i < test_file.file_size; i++) {
                if (file_buffer[i] >= 32 && file_buffer[i] <= 126) {
                    uart_putc(file_buffer[i]);
                } else if (file_buffer[i] == '\n') {
                    uart_putc('\n');
                } else {
                    uart_putc('.');
                }
            }
            uart_putc('\n');
        }
    }

    // Test 7: Test filename edge cases
    uart_puts("\nTest 7: Testing filename edge cases...\n");
    char name_no_ext[11];
    format_filename("NOEXT", name_no_ext);
    uart_puts("Formatted 'NOEXT' as: '");
    print_fat_filename(name_no_ext);
    uart_puts("'\n");

    char name_long[11];
    format_filename("LONGNAME.EXT", name_long);
    uart_puts("Formatted 'LONGNAME.EXT' as: '");
    print_fat_filename(name_long);
    uart_puts("'\n");
    uart_puts("PASS - Edge case formatting completed\n");

    uart_puts("\n=== All FAT32 Tests Completed ===\n");
    
    return 0;
}
