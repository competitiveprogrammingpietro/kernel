[BITS 32]
; This is the real entry point for an user program.
global _start
extern main
extern asm_exit

section .asm

_start:
    call main
    call asm_exit
    ret
