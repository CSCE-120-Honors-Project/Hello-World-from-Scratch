DootPsuedov1.md


//ldr x1, =stack_top // the 101s
//    mov sp, x1 // copies x1 to sp

//assem ing it 
```
.section .text.boot
.global _start
```

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








