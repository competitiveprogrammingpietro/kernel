
; You may customize this and other start-up templates; 
; The location of this template is c:\emu8086\inc\0_com_template.txt

org 100h
; DS starts at 0x0700
; indirect move move DS * 16 + 0xff. (0x0700 * 16) + 0xff = 7000h = ffh = 70ff
mov [0xff], 0x1234
mov bx, 0xff
mov si, bx
lodsb   ; AL contains 34
mov bx, 0xff
mov si, bx
lodsw   ; AX contains 1234
; switch data segment
mov ax, 0800h
mov ds, ax

; indirect move move DS * 16 + 0xff.(0x0800 * 16) + 0xff = 8000h = ffh = 80ff
mov [0xff], 0x30

; Stack
push 0xaaaa
push 0xbbbb
push 0xcccc
push 0x1234 ; watch out endianess obviously stored as 0x3412
pop ax ; ax stores 0x1234



; change CS and jump, in effect jump to 7cffh. Nowhere
jmp 0x7c0:0xff  
ret





