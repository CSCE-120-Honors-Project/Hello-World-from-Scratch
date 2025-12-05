read only data: string literals and const variables ONLY
hello world string variable -> compiler and linker takes it and puts it in .rodata section in elf

then elf goes into img, 

then bootloader grabs all the sections, including read only data with "hello world" and puts it in RAM

code can't modify it

