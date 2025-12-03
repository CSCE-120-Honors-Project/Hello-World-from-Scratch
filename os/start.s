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
.global _start // this makes it so that the linker script can see _start

//defining start <- this is where bootloader sends us
_start:
    //now do the stack pointer
    ldr x0, =boot_stack_top // loads value of stack_top to register x0
    mov sp, x0 // put that value into the real stack pointer register

    // then clear bss (for uninitialized variables)
    // the size of this section is calculated by the linker
    // so we have to get the size as labels provided by linker
    ldr x0, =__bss_start // start address
    ldr x1, =__bss_end // end address
    // now zero out the start and increment until start = end
clear_bss_loop:
    cmp x0, x1 // this will set an invisible Z flag to 1 if theyre equal
    beq bss_cleared // beq checks if z flag is 1, if so, itll go to the bss_cleared symbol and stop the loop
    str xzr, [x0] // zeros the memory by storing zero from zero register at the location of x0
    add x0, x0, #8 // read as add <destination> <operand 1> <operand 2>, so this is x0 = x0 + #8
    b clear_bss_loop // continue the loop
bss_cleared:
    bl main // branch with link to main.c
    // if main somehow returns, processor will go to hang because its the next line, then itll get stuck there
hang:
    wfi
    b hang
    