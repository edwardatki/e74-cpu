#bank RAM

#addr 0x0000

interrupt_handler:
        push a
        push bc
        mov bc, TERMINAL
        mov a, [bc]
        call put_char
        pop bc
        pop a
        reti

#addr 0x0100

mov sp, 0x7fff
tei
call main
call print_u8_dec
jmp $

#include "architecture.asm"
#include "print_functions.asm"

main:
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