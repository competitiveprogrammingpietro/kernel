[BITS 32]

global asm_print:function
global asm_getkey:function
global asm_malloc:function
global asm_free:function
global asm_putchar:function
global asm_exec:function
global asm_exit:function

asm_print:
    push ebp
    mov ebp, esp
    push dword[ebp+8] ; Our push plus the return address
    mov eax, 1 ; Kernel command print
    int 0x80
    add esp, 4
    pop ebp
    ret

asm_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Kernel command get_key()
    int 0x80
    pop ebp
    ret

asm_putchar:
    push ebp
    mov ebp, esp
    mov eax, 3 ; Command putchar
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

asm_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Kernel command malloc
    push dword[ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

asm_free:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 free (Frees the allocated memory for this process)
    push dword[ebp+8] ; Variable "ptr"
    int 0x80
    add esp, 4
    pop ebp
    ret


asm_exec:
    push ebp
    mov ebp, esp
    mov eax, 6 ; Command exec
    push dword [ebp+8] ; Filename
    int 0x80
    add esp, 4
    pop ebp
    ret

asm_exit:
    push ebp
    mov ebp, esp
    mov eax, 7 ; Command process exit
    int 0x80
    pop ebp
    ret