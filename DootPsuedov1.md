DootPsuedov1.md

/*

so the bootloader jumps straight into this,
then this will do some setup
then it will go to main which has hello world or whatever

LINKER MUST PROVIDE __bss_start, __bss_end

so this file needs:
- _start label for entry point
- setup stack pointer
- clear bss because c requires uninitialized variables to be cleared
- call to c main func
- hang in case of main returning
*/ 

.section ".text.boot"
.global _start // this makes it so that the linker script can see _start

//ldr x1, =stack_top // the 101s
//x1 ro x0 general purpose register that can be used as scratch, but otherwise its like first arg or second argument for a function call 
//    mov sp, x1 // copies x1 to sp

//assem ing it 
```
.section .text.boot
.global _start
```

//c++ stuff so far

// code will start by finding the size of boot  
// Declare the symbols (they're not variables, just addresses)
extern char __text_boot_start;
extern char __text_boot_end;

// Get their addresses
char *boot_code_start = &__text_boot_start;
char *boot_code_end = &__text_boot_end;

// Calculate bootloader size
size_t boot_size = (size_t)boot_code_end - (size_t)boot_code_start;



// what perplex told me 

MUST HAVE (Minimum Viable Bootloader)

Phase 1: First Stage (Assembly)
✅ Entry point (_start)
    // so you do the .global _start thing
✅ Set up stack pointer
    // you do the 101s
✅ Zero BSS section
    // you set up bss
    // simple and necesary 

✅ Call C++ main function
    // hand it over to s

✅ Hang loop if main returns
    // what to do if it actually finishes the call (kill itself)


Phase 2 : Kernel Loading (The "Real" Work)
[ ] Initialize Vio Driver
    // vio_init()
    // check for success

[ ] Initialize FAT Filesystem
    // fat_init()
    // fat_mount(0) - mount partition 0

[ ] Find Kernel File
    // fat_open("KERNEL  BIN", &file)
    // Note: filename must be in 8.3 format (spaces for padding)

[ ] Load Kernel
    // Define KERNEL_LOAD_ADDR (e.g., 0x40080000)
    // fat_read(&file, (uint8_t*)KERNEL_LOAD_ADDR)

[ ] Execute Kernel
    // Cast address to function pointer: void (*kernel_entry)(void) = (void*)KERNEL_LOAD_ADDR
    // kernel_entry()



what vinux needs (reminder checklist):
so this file needs:
- _start label for entry point
- setup stack pointer
- clear bss because c requires uninitialized variables to be cleared
- call to c main func
- hang in case of main returning
*/ 



////asem of the phase 1 bootloader 
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




