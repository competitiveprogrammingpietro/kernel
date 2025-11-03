section .asm

global idt_load
global int21h
global no_interrupt

extern int21_handler
extern no_interrupt_handler

idt_load:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8] ;; first param, recall base pointer, return address, param
	lidt [ebx]

	pop ebp
	ret


; Generic routing for no mapped interrupt
no_interrupt:
	cli
	pushad
	call no_interrupt_handler ; C function
	popad
	sti
	iret

; Keyboard interrupt 0x21
int21h:
	cli
	pushad
	call int21_handler ; C function
	popad
	sti
	iret
