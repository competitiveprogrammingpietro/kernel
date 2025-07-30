; Set the start address, ORIGIN
ORG 0x7c00
BITS 16

start:
	mov ah, 0eh
	mov al, 'A'
	int 0x10 ; Calling BIOS routine https://www.ctyme.com/rbrown.htm
	
	jmp $ ; Loop it


times 510- ($ - $$) db 0 ; fill 510 bytes padding with zeros after code
dw 0xAA55 ; 55AA little indian intel 
