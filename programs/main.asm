#bank RAM

mov sp, 0x7fff
call main
call print_u8_dec
jmp $

#include "architecture.asm"
#include "print_functions.asm"

main:
        mov a, 0
        ret