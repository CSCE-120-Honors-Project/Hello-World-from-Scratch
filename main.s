// Basic assembly program that just exits on a Linux system
.global _start

.section .text
_start:
    mov x8, #0x50
    mov x0, #0
    svc 0
