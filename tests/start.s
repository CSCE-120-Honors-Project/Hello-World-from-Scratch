/* 
 * Minimal startup code for test programs
 * Sets up stack and clears BSS before jumping to main
 */

.section ".text.boot"
.global _start

_start:
    // Set up stack pointer
    ldr x0, =__stack_top
    mov sp, x0

    // Clear BSS section
    ldr x0, =__bss_start
    ldr x1, =__bss_end
clear_bss:
    cmp x0, x1
    beq bss_done
    str xzr, [x0], #8
    b clear_bss
bss_done:

    // Call main function
    bl main

    // Just loop forever - tests will output results
hang:
    b hang
