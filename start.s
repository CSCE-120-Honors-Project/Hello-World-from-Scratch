/* 
This is the os starter code
so the bootloader jumps straight into this,
then this will do some setup
then it will go to main which has hello world or whatver

so this file needs:
- _start label for entry point
- setup stack pointer
- 
- call to c main func
- hang in case of main returning



*/ 

.section ".text.boot"
.global _start // this makes it so that kinkerscript can see _start

//defining start <- this is where bootloader sends us
_start:
    //now do the stack pointer
    ldr x0, =stack_top // loads value of stack_top to register x0, TODO:who defines stack_top?
    mov sp, x0 // put that value into the real stack pointer register
    // WHY THIS 2 STEP PROCESS? its probably safer or smth

