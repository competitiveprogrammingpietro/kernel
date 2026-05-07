[BITS 32]

global print:function
global getkey:function
global malloc:function
global free:function

print:
    push ebp
    mov ebp, esp
    push dword[ebp+8] ; Our push plus the return address
    mov eax, 1 ; Kernel command print
    int 0x80
    add esp, 4
    pop ebp
    ret

getkey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Kernel command get_key()
    int 0x80
    pop ebp
    ret

malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Kernel command malloc
    push dword[ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

free:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 free (Frees the allocated memory for this process)
    push dword[ebp+8] ; Variable "ptr"
    int 0x80
    add esp, 4
    pop ebp
    ret