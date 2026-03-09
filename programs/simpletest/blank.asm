; This silly test program does nothing more than jumping
; to the same label again and again.
[BITS 32]
section .asm

global _start

_start:

label:
    jmp label