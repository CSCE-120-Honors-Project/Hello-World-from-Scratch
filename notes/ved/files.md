ELF: ezpz way to go for starting off
`qemu-system-aarch64 -M virt -cpu cortex-a53 -kernel kernel.elf -nographic`
kernel.elf will be used for development and testing. it's skipping all the bootloader stuff and going straight into a program (vinux)

img: pretend to be a real computer
this is what well actually do in the end: its the end goal
`qemu-system-aarch64 -M virt -cpu cortex-a53 -drive file=disk.img -nographic`
1. QEMU starts with no kernel loaded
    
2. CPU begins at reset vector (typically 0x0)
    
3. **Your bootloader runs first** (loaded from disk)
    
4. **Bootloader finds your OS** on the simulated drive
    
5. **Bootloader loads OS into memory**
    
6. **Bootloader transfers control to OS**
    
7. **Your OS prints "Hello World"**

