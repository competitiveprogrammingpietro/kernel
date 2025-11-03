[BITS 32]
global _start ; export that to the external world, otherwise it is not visible
global divide_by_zero_error
extern kernel_main

CODE_SEG equ 0x8
DATA_SEG equ 0x10
_start:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ebp, 0x00200000
	mov esp, ebp
	
	; Enable A20 line
	in al, 0x92
	or al, 2
	out 0x92, al

	; We need to remap the master PIC to allow for the first 0x20 interrupts number
	; to be used as exceptions. The x86 addresses for the master PIC are 0x20 for
	; commands and 0x21 for data.
	
	; Initialise command
	mov al, 00010001b 
	out 0x20, al

	; That is where IVD starts
	mov al, 0x20

	; Send data
	out 0x21, al	   	

	; Additional configuration data
	mov al, 00000001b 
	out 0x21, al


        ; Enable interrupts
	sti

	call kernel_main
	jmp $


divide_by_zero_error:
	mov eax, 0
	div eax

times 512-($ - $$) db 0 ; fill 512 bytes padding with zeros after code.
			; this is important to maintain the alignment
