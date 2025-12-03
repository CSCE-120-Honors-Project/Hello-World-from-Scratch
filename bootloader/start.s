.section ".text.boot" // tell linkerscript to put this at the start of the output binary
.global _start

_start:
    // setup stack pointer
    ldr x0, =stack_top
    mov sp, x0
    // clear the stack
    ldr x0, =__bss_start
    ldr x1, =__bss_end
clear_bss_loop:
    cmp x0, x1
    beq bss_cleared
    str xzr, [x0]
    add x0, x0, #8
    b clear_bss_loop
bss_cleared:
    // jump to c code that will then do the handoff to os
    bl main 

.section ".bss"
.align 4
stack:
    .space //TODO: HOW MUCH SPACE DOES THE BOOTLOADER STACK NEED???
stack_top: