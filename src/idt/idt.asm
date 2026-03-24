section .asm

global idt_load
global int21h
global int80h
global no_interrupt
global enable_interrupts
global disable_interrupts

extern int21_handler
extern int80_handler
extern no_interrupt_handler

idt_load:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8] ;; first param, recall base pointer, return address, param
	lidt [ebx]

	pop ebp
	ret
; Keyboard interrupt 0x21
int21h:
	pushad
	call int21_handler ; C function
	popad
	iret

; Generic routing for no mapped interrupt
no_interrupt:
	pushad
	call no_interrupt_handler ; C function
	popad
	iret

enable_interrupts:
	sti
	ret

disable_interrupts:
	cli
	ret

int80h:
    ; Interrupt frame start
	; The CPU has already pushed to us upon entering this routine: 
    ; uint32_t ip
    ; uint32_t cs;
    ; uint32_t flags
    ; uint32_t sp;
    ; uint32_t ss;
    ; We not complete the interrupt frame by pushing all the general purpose registers
	; to the stack
    pushad
    ; INTERRUPT FRAME END

    ; Push the stack pointer so that we are pointing to the interrupt frame
    push esp

    ; EAX holds our command lets push it to the stack for int80_handler
    push eax
    call int80_handler
    mov dword[isr80h_handler_res], eax
    add esp, 8

    ; Restore general purpose registers for user land
    popad
    mov eax, [isr80h_handler_res]
    iretd

section .data

; Inside here is stored the return result from isr80h_handler
; https://www.nasm.us/doc/nasm03.html#section-3.2
isr80h_handler_res: dd 0