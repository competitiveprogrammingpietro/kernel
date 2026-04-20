section .asm


global idt_load
global int80h
global no_interrupt
global enable_interrupts
global disable_interrupts
global asm_interrupt_pointer_table

extern idt_interrupt_handler
extern int80_handler
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

; Interrupts' handlers are called by the processor - the interrupt number itself
; is not explicitly given. This forces us to declare an interrupt handler for
; every interrupt number, even if they are all calling the same C function to
; handle it, regardless of its interrupt number.
; We automate the genration of this assembly routing using NASM macros.
%macro interrupt 1
    global asm_interrupt_handler_%1
    asm_interrupt_handler_%1:

    ; Interrupt frame start
	; The CPU has already pushed to us upon entering this routine: 
    ; uint32_t ip
    ; uint32_t cs;
    ; uint32_t flags
    ; uint32_t sp;
    ; uint32_t ss;
    ; We not complete the interrupt frame by pushing all the general purpose
    ; registers to the stack
    pushad
    ; Interrupt frame end

    ; Push the stack pointer so that we are pointing to the interrupt frame
    push esp

    ; push interrupt number
    push dword %1
    call idt_interrupt_handler
    add esp, 8

    ; Restore general purpose registers for user land
    popad
    mov eax, [isr80h_handler_res]
    iret
%endmacro

; Call the macro in a loop, the number of cycles must coincide with the defined
; number of interrupt handlers defined in the C world. PEACOS_INTERRUPT_NUMBER.
%assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep

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

; Generate the table of pointers to be exposed in the C language
%macro asm_interrupt_table_entry 1
    dd asm_interrupt_handler_%1
%endmacro

asm_interrupt_pointer_table:
%assign i 0
%rep 512
    asm_interrupt_table_entry i
%assign i i+1
%endrep

; Inside here is stored the return result from isr80h_handler
; https://www.nasm.us/doc/nasm03.html#section-3.2
isr80h_handler_res: dd 0