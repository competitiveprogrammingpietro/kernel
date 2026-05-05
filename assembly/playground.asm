; Set the start address, ORIGIN. We're in real mode
ORG 0x0
BITS 16

_start:
	jmp short start
	nop

times 33 db 0 ; BIOS parameters block

start:
	jmp 0x7c0:step2 ; set CS


; We handle the zeo interrupt here
handle_zero:
	mov ah, 0eh
	mov al, 'A'
	mov bx, 0x00
	int 0x10
	iret

step2:
	; Set the segments register explicitly as we cannot rely on the BIOS
	cli ; clear interrupts
	mov ax, 0x7c0
	mov ds, ax
	mov es, ax,
	mov ax, 0x00
	mov ss, ax
	mov sp, 0x7c00
	sti ; enable interrupts

	; Set the IVT for interrupt zero OFFSET:SEGMENT
	mov word [ss:0x00], handle_zero
	mov word [ss:0x02], 0x7c0
	
	; Cause the divide by zero interrupt
	mov ax, 0x0
	div ax
	
		
	mov si, message
	call print
	jmp $


print:
	mov bx, 0
.loop:
	lodsb ; DS:SI into AL
	cmp al, 0 ; NULL string ? 
	je .done
	call print_char
	jmp .loop

.done:
	ret

print_char:
	mov ah, 0eh ; BIOS routine number for printing a char on the screen
	int 0x10 ; Calling BIOS routine https://www.ctyme.com/rbrown.htm
	ret


message: db 'Hello world!', 0
times 510- ($ - $$) db 0 ; fill 510 bytes padding with zeros after code
dw 0xAA55 ; 55AA little indian intel 
