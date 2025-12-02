.section .text.boot
.global _start

_start:
    // Set up stack
    ldr x1, =stack_top
    mov sp, x1 // sp contain
    
    // Zero BSS
    ldr x1, =__bss_start
    ldr x2, =__bss_end
loop:
    cmp x1, x2
    b.ge done
    str xzr, [x1], #8
    b loop
done:
    // Infinite loop for now
    wfe
    b done
