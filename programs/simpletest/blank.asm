; This silly test program does nothing more than jumping
; to the same label again and again.
[BITS 32]
section .asm

global _start

_start:
    push message
    mov eax, 1
    int 0x80
    add esp, 4 ; restore stack, shortend for push eax, push eax
    jmp $

section .data:
message: db 'Kiss you babe', 0