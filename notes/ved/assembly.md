.section:
directive that tells assembler that everything after that point belongs in a "section" which is just a named block of code for assembler to use
**Standard sections** that exist by default:

- `.text` - executable code
    
- `.data` - initialized variables
    
- `.rodata` - read-only data (constants, strings)
    
- `.bss` - uninitialized variables (zeroed at startup)

but you can make custom ones
".text.boot"
.text shows that its code, then the .boot will say that its for boot stuff

WHY WE DO THIS
so that linkerscript can differentiate the boot stuff from the regular code


bl: branch and link
* call a function and return back once done
* example: bl func
br: branch register
* jump to an address stored in a register
	* example: br x1
ldr: load register
* reads from memory address and puts to a register
* eg: ldr x0, \[x1]
* eg: ldr x0, =0x40000000
mov: move
* copy form one reg to another
* mov x0, x0   same as x0 = x1