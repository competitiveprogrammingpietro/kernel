; This silly test program does nothing more than jumping
; to the same label again and again.
[BITS 32]
section .asm

global _start

; This program waits for a key to be pressed and print a message right
; after
_start:
_loop:
    call getkey
    push eax
    mov eax, 3 ; put char command
    int 0x80
    add esp, 4 ; restore stack, shortend for push eax, push eax
    jmp _loop

getkey:
    mov eax, 2 ; command getkey
    int 0x80
    cmp eax, 0x00
    je getkey
    ret

section .data:
message: db 'Kiss you babe', 0