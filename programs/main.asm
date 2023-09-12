#bank RAM

#addr 0x0000

; Read terminal input into circular buffer
interrupt_handler:
        push a
        push bc
        push de
        pushf

        mov de, TERMINAL
        mov a, [de]
        
        mov de, input_pointer
        mov bc, [de]
        mov [bc], a
        inc bc
        mov a, c ; TODO how do we handle 16-bit comaparisons?
        cmp input_buffer_end[7:0]
        jne .skip
        mov bc, input_buffer_start
.skip:
        mov [de], bc

        popf
        pop de
        pop bc
        pop a
        reti

#addr 0x0100

mov sp, 0x7fff
call main
call print_u8_dec
jmp $

#include "architecture.asm"
#include "print_functions.asm"

main:
        mov de, input_buffer_start
        mov bc, input_pointer
        mov [bc], de
        tei
        mov a, 0
.loop:
        call print_u8_dec
        add 1
        push a
        mov a, " "
        call put_char
        pop a
        jmp .loop
        ret

#align 0x1000
input_pointer:
#d16 input_buffer_start
input_buffer_start:
#res 32
input_buffer_end: