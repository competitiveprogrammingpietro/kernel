; Set the start address, ORIGIN. We're in real mode
ORG 0x0
BITS 16

_start:
	jmp short start
	nop

times 33 db 0 ; BIOS parameters block

start:
	jmp 0x7c0:step2 ; set CS


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
        
	; The bootloader file is made of two sectors, one contains the code the other
	; contains the message we want to print. Hence we need to read the message
	; into memory and print it out.	
        ; https://www.ctyme.com/intr/rb-0607.htm

	; Read from disk, 1 sector
	mov ah, 2 ; read ..
	mov al, 1 ; ... one sector ...
	mov ch, 0x0 ; ... from cylinder zero ..
	mov cl, 2 ;  ... start from the second sector ...
	mov bx, buffer ; ... write data into &buffer ...
	int 0x13 ; start the show
	jc error ; the BIOS routine sets the CF flag on error
	jmp $


error:
	mov si, error_message
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

error_message: db 'Something went wrong', 0

message: db 'Hello world!', 0
times 510- ($ - $$) db 0 ; fill 510 bytes padding with zeros after code
dw 0xAA55 ; 55AA little indian intel

; We use it to point at the end of our bootloader
buffer: 
