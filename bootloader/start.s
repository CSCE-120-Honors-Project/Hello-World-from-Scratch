.section ".text.boot"
.global _start

_start:
    // Disable all interrupts
    msr DAIFSet, #0xF
    
    // Set up stack pointer
    ldr x0, =stack_top
    mov sp, x0
    
    // Clear BSS section (required for C runtime)
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    
clear_bss_loop:
    cmp x0, x1
    beq bss_cleared
    str xzr, [x0], #8      // Write zero and increment by 8
    b clear_bss_loop
    
bss_cleared:
    // Call C main bootloader function
    bl boot_main
    
    // If boot_main returns (shouldn't happen), hang forever
hang:
    wfe                     // Wait for event/interrupt
    b hang
    
// Stack space (stored in BSS to save ELF file size)
.section ".bss"
.align 8
stack:
    .space 4096
stack_top:
