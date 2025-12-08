/*
 * main.c - ARM64 Bootloader Main Logic (Phase 2)
 * 
 * This is the core bootloader that:
 * 1. Initializes the UART for debug output
 * 2. Initializes the VIO block device
 * 3. Initializes the FAT32 filesystem
 * 4. Searches for the kernel file
 * 5. Loads the kernel into memory
 * 6. Jumps to the kernel entry point
 */

#include "uart.h"
#include "vio.h"
#include "fat.h"
#include <stdint.h>
#include <stdbool.h>

// Configuration
#define KERNEL_FILENAME "KERNEL  BIN"
#define KERNEL_LOAD_ADDR 0x40080000    // Where to load kernel in memory
#define MAX_KERNEL_SIZE (16 * 1024 * 1024)  // 16MB max kernel size

// Simple string functions (no libc available)
void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

/**
 * boot_main - Main bootloader entry point
 * Called from start.s after basic setup
 */
void boot_main(void) {
    uart_init();
    
    // Banner
    uart_puts("\n\r");
    uart_puts("===========================================\n\r");
    uart_puts("ARM64 Bootloader v2.0\n\r");
    uart_puts("===========================================\n\r");
    uart_puts("\n\r");
    
    // ============================================================================
    // PHASE 1: Initialize Block Device
    // ============================================================================
    uart_puts("[1] Initializing VIO block device...\n\r");
    
    if (vio_init() < 0) {
        uart_puts("FATAL: VIO initialization failed!\n\r");
        uart_puts("The bootloader cannot access the disk.\n\r");
        goto fatal_error;
    }
    
    uart_puts("     SUCCESS: VIO block device ready\n\r");
    uart_puts("\n\r");
    
    // ============================================================================
    // PHASE 2: Initialize Filesystem
    // ============================================================================
    uart_puts("[2] Initializing FAT32 filesystem...\n\r");
    
    if (fat_init() < 0) {
        uart_puts("FATAL: FAT initialization failed!\n\r");
        uart_puts("Could not read master boot record.\n\r");
        goto fatal_error;
    }
    
    uart_puts("     MBR read successfully\n\r");
    
    // Mount the first partition
    uart_puts("[2.1] Mounting partition 0...\n\r");
    if (fat_mount(0) < 0) {
        uart_puts("FATAL: FAT mount failed!\n\r");
        uart_puts("Could not mount partition or invalid FAT32.\n\r");
        goto fatal_error;
    }
    
    uart_puts("      SUCCESS: FAT32 mounted\n\r");
    uart_puts("\n\r");
     
    // ============================================================================
    // PHASE 3: Search for Kernel File
    // ============================================================================
    uart_puts("[3] Searching for kernel file...\n\r");
    uart_puts("    Looking for: ");
    uart_puts(KERNEL_FILENAME);
    uart_puts("\n\r");
    
    fat_file kernel_file = {0};

    uart_puts("Opening kernel file...\n\r");
    if (fat_open(KERNEL_FILENAME, &kernel_file) < 0) {
        uart_puts("FATAL: Kernel file not found!\n\r");
        uart_puts("Make sure KERNEL.BIN exists on the disk.\n\r");
        goto fatal_error;
    }
    
    uart_puts("    SUCCESS: Kernel file found\n\r");
    uart_puts("    File size: 0x");
    uart_print_hex(kernel_file.file_size);
    uart_puts(" (");
    uart_print_dec(kernel_file.file_size);
    uart_puts(" bytes)\n\r");
    uart_puts("\n\r");
    
    // Sanity check: kernel size
    if (kernel_file.file_size > MAX_KERNEL_SIZE) {
        uart_puts("FATAL: Kernel too large!\n\r");
        uart_puts("Maximum size: 0x");
        uart_print_hex(MAX_KERNEL_SIZE);
        uart_puts("\n\r");
        goto fatal_error;
    }
    
    if (kernel_file.file_size < 100) {
        uart_puts("WARNING: Kernel suspiciously small (< 100 bytes)\n\r");
    }
    
    // ============================================================================
    // PHASE 4: Load Kernel into Memory
    // ============================================================================
    uart_puts("[4] Loading kernel into memory...\n\r");
    uart_puts("    Load address: 0x");
    uart_print_hex(KERNEL_LOAD_ADDR);
    uart_puts("\n\r");
    uart_puts("    Reading from disk...\n\r");
    
    //uart_puts("DEBUG main: About to call fat_read()...\n\r");
    
    uint64_t sp_main;
    asm volatile("mov %0, sp" : "=r"(sp_main));
    //uart_puts("DEBUG main: SP before call: 0x");
    //uart_print_hex(sp_main);
    //uart_puts("\n\r");
    
    if (fat_read(&kernel_file, (uint8_t*)KERNEL_LOAD_ADDR) < 0) {
        uart_puts("FATAL: Kernel load failed!\n\r");
        uart_puts("Could not read kernel from disk.\n\r");
        goto fatal_error;
    }
    
    //uart_puts("DEBUG main: fat_read() returned successfully\n\r");

    // Force a small delay
    for (volatile int delay = 0; delay < 100000; delay++);

   // uart_puts("DEBUG main: Delay done\n\r");

    uart_puts("[5] Boot Information:\n\r");

    uart_puts("    SUCCESS: Kernel loaded\n\r");
    uart_puts("\n\r");
    
    // ============================================================================
    // PHASE 5: Boot Information
    // ============================================================================
    //uart_puts("DEBUG main: Starting Phase 5\n\r");
    uart_puts("[5] Boot Information:\n\r");
    uart_puts("    Kernel Entry Point: 0x");
    uart_print_hex(KERNEL_LOAD_ADDR);
    uart_puts("\n\r");
    uart_puts("    Kernel Size:        0x");
    uart_print_hex(kernel_file.file_size);
    uart_puts(" bytes\n\r");
    uart_puts("\n\r");
    //uart_puts("DEBUG main: Phase 5 complete\n\r");
    
    // ============================================================================
    // PHASE 6: Transfer Control to Kernel
    // ============================================================================
   // uart_puts("DEBUG main: Starting Phase 6\n\r");
    uart_puts("[6] Transferring control to kernel...\n\r");
    uart_puts("    Jumping to 0x");
    uart_print_hex(KERNEL_LOAD_ADDR);
    uart_puts("\n\r");
    uart_puts("\n\r");
    uart_puts("===========================================\n\r");
    uart_puts("Bootloader complete. Starting kernel.\n\r");
    uart_puts("===========================================\n\r");
    uart_puts("\n\r");
    
   // uart_puts("DEBUG main: Creating kernel_entry function pointer\n\r");
    
    // Jump to kernel
    // Cast the address as a function pointer with void return and no arguments
    typedef void (*kernel_entry_t)(void);
    kernel_entry_t kernel_entry = (kernel_entry_t)KERNEL_LOAD_ADDR;
    
    /*uart_puts("DEBUG main: About to call kernel entry point at 0x");
    uart_print_hex(KERNEL_LOAD_ADDR);
    uart_puts("\n\r");
    */
    
    // Call the kernel
    kernel_entry();
    
    // ============================================================================
    // ERROR HANDLING: Should never reach here
    // ============================================================================
    uart_puts("FATAL: Kernel returned to bootloader!\n\r");
    
fatal_error:
    uart_puts("\n\r");
    uart_puts("SYSTEM HALTED\n\r");
    uart_puts("Bootloader cannot continue.\n\r");
    
    // Hang forever
    while (1) {
        asm volatile("wfe");  // Wait for event/interrupt
    }
}
