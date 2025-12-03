/* ///asem of the phase 1 bootloader 
.section ".text.boot"
.global _start // this makes it so that the linker script can see _start

//defining start cause that how asem works, 
_start:
    // Disable all interrupts
    msr daif, #0xf
  
    //now do the stack pointer
    ldr x0, =stack_top // loads value of stack_top to register x0
    mov sp, x0 // put that value into the real stack pointer register
    // you got to do this in 2 steps cause you can't load a value into a register and move it at the same time, you need to use mov on sp, and ldr is whats needed to actually grab the top of the stack. 



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
    wfe //wait for event/interupt
    b hang  
// stack is just a block of memory we want at runtime, so we can just put it in bss
// this way at runtime we get a nice zeroed out stack while not wasting 4kb of space in the elf file
.section ".bss"
.align 4
stack:
    .space 4096
stack_top: 
*/ 

/*
 * start.s - ARM64 Bootloader First Stage
 * 
 * This is the entry point for the bootloader.
 * It handles:
 * - Stack pointer setup
 * - BSS section clearing (for uninitialized variables)
 * - Calling boot_main()
 * - Hanging if boot_main returns
 */

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