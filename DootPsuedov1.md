DootPsuedov1.md


//ldr x1, =stack_top // the 101s
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
Phase 2: Second Stage (C++)
✅ UART initialization

✅ Print "Bootloader Started" message 
    // "hello world "
✅ Hang in infinite loop








