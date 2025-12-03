Memory Layout:
0x40080000: [.text section](text%20section.md)     (your code)
0x40082000: [.rodata section](rodata%20section.md)   (strings like "Hello World")
0x40083000: [.data section](data%20section.md)     (initialized variables)
0x40084000: [.bss section](bss%20section.md)      (uninitialized variables)
0x40088000: [heap](heap)              (malloc/free)
0x40100000: [stack](stack)             (function calls)