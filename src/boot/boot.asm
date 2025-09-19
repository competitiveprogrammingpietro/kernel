; Set the start address, ORIGIN. This bootloader enables the protected mode
; db byte
; dw 2 bytes
; dd 4 bytes
ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start:
	jmp short start
	nop

times 33 db 0 ; BIOS parameters block

start:
	jmp 0:step2 ; set CS. Origin alreay set so CS is set to zero


step2:
	; Set the segments register explicitly as we cannot rely on the BIOS
	cli ; clear interrupts: ignore them for now
	
	; all segments to zero
	mov ax, 0x0
	mov ds, ax
	mov es, ax,
	mov ss, ax
	mov sp, 0x7c00
	sti ; enable interrupts


.load_protected:
	cli;
	lgdt[gdt_descriptor]
	mov eax, cr0 ; the next three lines in fact enable protected mode
	or eax, 0x1
	mov cr0, eax
	
	; This tells the CPU to use the index 1 from the GDT, hence the code	
	jmp CODE_SEG:load32 

; This is the Global Descriptor Table https://wiki.osdev.org/Global_Descriptor_Table
; GDT start
gdt_start:
gdt_null:
	
	; first entry the null entry 
	dd 0x0
	dd 0x0

; Each entry is 8 bytes
gdt_code: ; CS
	dw 0xffff 	; Segment limit 2^16
	dw 0	  	; Base first
	db 0      	; base second chunk
	db 0x9a   	; Access byte 1001 1010, by reading the specs:
			; present, code segment, executable hence code segment, readable
	db 11001111b 	; Limit half byte hence limit becomes 2^20 
			; Flags: granularity 4Kb, hence the maximum addressable mem is 4GB
			;        DB set hence 32 bit protected mode
	db 0		; base last byte

; offset 0x10
gdt_data: ; DS, SS, ES, FS, GS
	dw 0xffff 	; Segment limit 2^16
	dw 0	  	; Base first
	db 0      	; base second chunk
	db 0x92   	; Access byte 1001 0010, by reading the specs:
			; present, data segment, executable, read/write access
	db 11001111b 	; Limit half byte hence limit becomes 2^20 
			; Flags: granularity 4Kb, hence the maximum addressable mem is 4GB
			;        DB set hence 32 bit protected mode
	db 0		; base last byte

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start ; the GDT descriptor offset can be set using this little computatin
		     ; as the correct ORIGIN is set so we're good 
		     ; using the assembler computed offset. 



[BITS 32]
load32:
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
	jmp $

times 510- ($ - $$) db 0 ; fill 510 bytes padding with zeros after code
dw 0xAA55 ; 55AA little indian intel
