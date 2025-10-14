; This file maps low level x86 I/O instructions into functions callable from
; C code

section .asm

global insb
global insw
global outb
global outw


insb:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in al, dx ;; Write the byte read from port into DX into AL (8 bits)
	
	pop ebp
	ret



insw:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in ax, dx ;; Write the byte read from port into DX into AX (16 bits one word wide register)
	
	pop ebp
	ret


outb:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov eax, [ebp+8]
	mov edx, [ebp+12]
	out dx, al
	
	pop ebp
	ret



outw:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov eax, [ebp+8]
	mov edx, [ebp+12]
	out dx, ax
	
	pop ebp
	ret
