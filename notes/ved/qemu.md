![[Pasted image 20251001104402.png]]
qemu doesnt emulate a particular chip, it has its own SoC design for clean standardized arm development environment
so theres no hardware counterpart

`0x00000000 - 0x08000000: Flash/ROM space (128MB) 0x08000000 - 0x08010000: GIC Distributor 0x08010000 - 0x08020000: GIC CPU Interface   0x09000000 - 0x09001000: PL011 UART 0x0a000000 - 0x0a000200: Real-time Clock (PL031) 0x0c000000 - 0x0e000000: GPIO controller 0x40000000 - variable:    RAM (starts at 1GB mark)`

that is just typical, may not be the actual implementation in the qemu
