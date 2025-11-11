; Set the start address, ORIGIN. This bootloader enables the protected mode
; db byte
; dw 2 bytes
; dd 4 bytes
ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; Those are the BIOS parameter (BPB), they were emty but not they contain the
; FAT16 header, that is, our FS is a FAT16 fs although the first 512 bytes are
; reserved for the boot sector, that is, this file.
; https://wiki.osdev.org/FAT#FAT_16
; Mind the db sequences as they need to be conform to the standard hence every
; whitespace matters.
OEMIdentifier           db 'PEACHOSP'
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'PEACHOS VID'
SystemIDString          db 'FAT16   '

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
	; Load kernel into memory
	mov eax, 1 ; starting sector
	mov ecx, 100 ; number of sectors to load
	mov edi, 0x100000 ; where to store it
	call ata_lba_read
	jmp CODE_SEG:0x100000


ata_lba_read:
	mov ebx, eax ; backup LBA
	
	; send higher 8 bits of the LBA to HR controller
	shr eax, 24
	or eax, 0xE0 ; Master drive select
	mov dx, 0x1F6
	out dx, al
	; Finished sending the highest 8 bits of the LBA
	
	; Send the total sectors to read
	mov eax, ecx
	mov dx, 0x1F2
	out dx, al
	; Finished sending sectors to read

	; Unclear to me TBH
	mov eax, ebx
	mov dx, 0x1F3
	out dx, al
	
	mov dx, 0x1F4
	mov eax, ebx
	shr eax, 8
	out dx, al
	; end


	; Send upper 16 bits
	mov dx, 0x1F5
	mov eax, ebx
	shr eax, 16
	out dx, al
	; end

	mov dx, 0x1F7
	mov al, 0x20
	out dx, al

	; Read all sectors into memory
.next_sector:
	push ecx

; Do we need to read any other data
.try_again:
	mov dx, 0x1F7
	in al, dx
	test al, 8 
	jz .try_again
	
	; We need to read 256 words at the time
	mov ecx, 256
	mov dx, 0x1F0
	rep insw 
	pop ecx
	loop .next_sector
	ret


	

times 510-($ - $$) db 0 ; fill 510 bytes padding with zeros after code
dw 0xAA55 ; 55AA little indian intel
