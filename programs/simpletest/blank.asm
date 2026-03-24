; This silly test program does nothing more than jumping
; to the same label again and again.
[BITS 32]
section .asm

global _start

_start:
    mov eax, 0
    int 0x80
    jmp $