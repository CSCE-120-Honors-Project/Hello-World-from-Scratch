/* 
This is the os starter code
so the bootloader jumps straight into this,
then this will do some setup
then it will go to main which has hello world or whatever

LINKER MUST PROVIDE boot_stack_top, __bss_start, __bss_end

so this file needs:
- _start label for entry point
- setup stack pointer
- clear bss because c requires uninitialized variables to be cleared
- call to c main func
- hang in case of main returning
*/ 

.section ".text.boot"
.global _start

_start:
    # Set up stack pointer
    ldr x0, =boot_stack_top
    mov sp, x0

    # Clear BSS section (required for C runtime)
    ldr x0, =__bss_start
    ldr x1, =__bss_end

clear_bss_loop:
    cmp x0, x1
    beq bss_cleared
    str xzr, [x0]
    add x0, x0, #8
    b clear_bss_loop

bss_cleared:
    bl main
    # If main returns, hang forever
hang:
    wfi
    b hang
    
    