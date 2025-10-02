section .asm

global idt_load
idt_load:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8] ;; first param, recall base pointer, return address, param
	lidt [ebx]

	pop ebp
	ret
