
; You may customize this and other start-up templates; 
; The location of this template is c:\emu8086\inc\0_com_template.txt
; www.falstad.com
; www.gabrielececchetti.it 8086 instruction set
org 100h

jmp main

message:
    db 'Hello World!', 0

print:
    mov ah, 0eh
._loop:
    lodsb
    cmp al, 0
    je .done
	int 0x10 ; Calling BIOS routine https://www.ctyme.com/rbrown.htm    
    jmp ._loop
    
.done:
    ret

main:
    mov si, message
    call print
ret




                                                 
