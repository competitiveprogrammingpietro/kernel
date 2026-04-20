; This silly test program does nothing more than jumping
; to the same label again and again.
[BITS 32]
section .asm

global _start

_start:
    push 10h
    push 10h
    mov eax, 0
    int 0x80
    add esp, 8 ; restore stack, shortend for push eax, push eax
    jmp $