[BITS 32]

section .asm

global paging_load_directory
global enable_paging

; Load the page dictionary entry into the CR3
paging_load_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    pop ebp
    ret

; Enable paging https://wiki.osdev.org/Paging#32-bit_Paging
enable_paging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    pop ebp
    ret
