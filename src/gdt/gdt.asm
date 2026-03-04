section .asm
global gdt_load

gdt_load:
    
    ; first args is gdt descriptor address, to be stored into the start address
    ; of the GDT descriptor, which starts at the 3rd byte.
    mov eax, [esp+4]
    mov [gdt_descriptor + 2], eax

    ;; Then goes the size, mind that AX is 16 bits
    mov ax, [esp+8]
    mov [gdt_descriptor], ax
    lgdt [gdt_descriptor]
    ret


section .data
gdt_descriptor: ; First entry must be set to null
    dw 0x00     ; Size, 16 bits
    dd 0x00     ; GDT start address, 32 bits