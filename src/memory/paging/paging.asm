[BITS 32]

section .asm

global paging_load_directory
global paging_enable

; Load the page dictionary entry into the CR3
paging_load_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    pop ebp
    ret

; Enable paging https://wiki.osdev.org/Paging#32-bit_Paging
paging_enable:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000 ;; This writes on the PG bit, thus enabling paging
    mov cr0, eax
    pop ebp
    ret
