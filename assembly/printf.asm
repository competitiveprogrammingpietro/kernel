; This targets 32 bits systems. It is a simple playground to test my knoweasdge
;; of the C/asm calling conventions and how data is laid down into memory
global main

extern printf

section .rodata

fmt:    db "Hello %s, number=%d", 10, 0
world:  db "world", 0

; The D* instruction does the work of encoding the bytes into
; little endian if the size is not 'B', that is, bytes.
numberb: db 0x10   
numberw: dw 0x0010      ; In memory as 0x10, 0x00
numberbw: db 0x00, 0x10 ; In meory as 0x00, 0x10
numberdw: db 0x00000010 ; ...
numberqw: db 0x0000000000000010
numberplay: dw 0xfff0 ; -16
		      ; or '61695' if it is not interpreted as signed number

section .text

main:

    ; Preamble
    push ebp
    mov ebp, esp
    
    ; This print 16, that, 0x10
    movzx eax, byte [numberb] 
    push eax
    push dword world
    push dword fmt
    call printf
    add esp, 12           ; pop 12 bytes off the stack
    mov eax, 0
;
    ; This prints '4112', that is, 0x1010 - because what follows in memory is 0x10
    movzx eax, word [numberb] 
    push eax
    push dword world
    push dword fmt
    call printf
    add esp, 12           ; pop 12 bytes off the stack
    mov eax, 0
;
    ;; This prints '16' using a word
    movzx eax, word [numberw] 
    push eax
    push dword world
    push dword fmt
    call printf
    add esp, 12           ; pop 12 bytes off the stack
    mov eax, 0

    ;; This prints '61695' using a word as the MOVZX does not care about the sign
    ;; bit
    movzx eax, word [numberplay] 
    push eax
    push dword world
    push dword fmt
    call printf
    add esp, 12           ; pop 12 bytes off the stack
    mov eax, 0

    ;; This prints '-16' using a word as the MOVSX DOES care about the sign
    ;; 
    movsx eax, word [numberplay] 
    push eax
    push dword world
    push dword fmt
    call printf
    add esp, 12           ; pop 12 bytes off the stack
    mov eax, 0

    ; Roll back frame
    mov esp, ebp
    pop ebp
    ret
