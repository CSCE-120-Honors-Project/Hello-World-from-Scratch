".bss" block started by symbol
memory for uninitialized dat
auto assigned to zero when program start
doesnt take space in bin file, only in real memory at runtime

.align 3
aligns next piece of data on 2^3=8 byte boundary
so starting addy of next data is multiple of 8

stack:
given a name for address for the actual stack
lowest address (smallest number)
but the data stored in the stack starts at the highest point and stacks towards the lowest point

.space 4096
reserves 4 kibibytes of zeroed memory when the program runs

stack_top:
highest number of stack
this is where you initialize stack pointer (sp)
calculated automatically bc bss maps out a real block 