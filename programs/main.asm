#include "architecture.asm"

#bank ROM

#addr 0x0000

; Read terminal input into circular buffer
interrupt_handler:
        push a
        push bc
        push de
        pushf

        mov de, TERMINAL                ; Get character
        mov a, [de]
        
        call put_char                   ; Echo character
        
        mov de, input_pointer           ; Store character in buffer
        mov bc, [de]
        mov [bc], a

        cmp "\n"                        ; Test for newline and set flag
        lde
        mov de, line_ready_flag
        mov [de], a
        jne .skip_1
        popf
        pop de
        pop bc
        pop a
        ret                             ; Use regular return so interrupts remain disabled
.skip_1:
        
        inc bc                          ; Increment pointer

        mov a, c                        ; For now only testing low byte
        cmp input_buffer_end[7:0]       ; So buffer must be properly aligned
        jne .skip_2
        mov bc, input_buffer_start      ; Wrap buffer back to start
.skip_2:
        mov de, input_pointer
        mov [de], bc

        popf
        pop de
        pop bc
        pop a
        reti

#addr 0xf4
jmp fibonacci_demo
#addr 0xf7
jmp maze_demo
#addr 0xfa
jmp slow_primes_demo
#addr 0xfd
jmp fast_primes_demo

#addr 0x0100

reset:
        mov sp, 0xffff                  ; Set up stack

        mov bc, welcome_message
        call print_string

monitor:
        mov bc, input_buffer_start      ; Reset input buffer pointer
        mov de, input_pointer
        mov [de], bc
.put_prompt:
        mov a, ">"
        call put_char
        tei                             ; Enable interrupts
.wait_for_line:
        mov de, line_ready_flag         ; Check if line is ready
        mov a, [de]                     ; Could go do other things while we wait
        cmp 0
        je .wait_for_line               ; Interrupts will already be disabled by the handler if EOL

        mov a, 0                        ; Reset line ready flag
        mov de, line_ready_flag
        mov [de], a

        mov de, input_buffer_start
        mov a, [de]                     ; First character determines command

        cmp "H"                         ; Help command
        je .help_command

        cmp "R"                         ; Read command
        je .read_command

        cmp "W"                         ; Write command
        je .write_command

        cmp "D"                         ; Dump command
        je .dump_command

        cmp "X"                         ; Execute command
        je .execute_command

        cmp "?"                         ; Return code command
        je .return_code_command

        jmp .error                      ; Not a vaild command so exit

.parse_hex_byte:
        push bc
        mov b, 0x80
        dec de
.next_char:
        inc de                          ; Skip characters until valid hex char
        mov a, [de]

        cmp "\n"                        ; If eol then exit
        jne .not_eol
        mov sp, sp+4                    ; Remove return address and bc from stack
        jmp .error
.not_eol:
        cmp 0x2F                        ; If not between 0 and F
        jnc .next_char
        cmp 0x46
        jc .next_char

        sub 0x30

        cmp 0x09                        ; If 0-9
        jnc .skip

        cmp 0x10                        ; If beween 9 and A
        jnc .next_char

        sub 0x07                        ; A-F
.skip:
        mov c, a                        ; Current digit in C
        mov a, b                        ; Test if already a digit in B
        cmp 0x80
        jne .low_byte                   ; If have both digits the done
        mov b, c                        ; Otherwise this is the high digit, put in B
        jmp .next_char                  ; Continue to get low digit
.low_byte:
        mov a, b                        ; Combine high and low digits
        rol
        rol
        rol
        rol
        or c

        pop bc
        ret

.help_command:
        mov bc, help_message
        call print_string
        jmp .end_of_line

.read_command:
        inc de                          ; Parse address high
        call .parse_hex_byte
        mov b, a

        inc de                          ; Parse address low
        call .parse_hex_byte
        mov c, a

        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex
        
        mov a, ":"
        call put_char
        
        mov a, [bc]                     ; Read and print
        call print_u8_hex
        
        mov a, "\n"
        call put_char
        
        jmp .end_of_line

.write_command:
        inc de                          ; Parse address high
        call .parse_hex_byte
        mov b, a

        inc de                          ; Parse address low
        call .parse_hex_byte
        mov c, a

        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex

        inc de                          ; Parse data to write
        call .parse_hex_byte

        mov [bc], a                     ; Write data to address
        
        mov a, ":"
        call put_char
        
        mov a, [bc]                     ; Read back and print
        call print_u8_hex
        
        mov a, "\n"
        call put_char
        
        jmp .end_of_line

.dump_command:
        inc de                          ; Parse address high
        call .parse_hex_byte
        mov b, a

        inc de                          ; Parse address low
        call .parse_hex_byte
        mov c, a

        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex

        mov a, ":"
        call put_char

        push de
        mov e, 15
.dump_loop:
        mov a, [bc]
        call print_u8_hex
        mov a, " "
        call put_char
        inc bc
        dec e
        jc .dump_loop

        mov a, "\n"
        call put_char

        pop de
      
        jmp .end_of_line

.execute_command:
        inc de                          ; Parse address high
        call .parse_hex_byte
        mov b, a

        inc de                          ; Parse address low
        call .parse_hex_byte
        mov c, a

        call bc                         ; Call address

        mov bc, return_code             ; Store return code
        mov [bc], a
        
        jmp .end_of_line

.return_code_command:
        mov bc, return_code             ; Read return code
        mov a, [bc]
        call print_u8_dec
        mov a, "\n"
        call put_char
        jmp .end_of_line

.error:
        mov bc, error_message           ; Print error message
        call print_string
.end_of_line:

        mov bc, input_buffer_start      ; Set pointer back to start of buffer
        mov de, input_pointer
        mov [de], bc

        jmp .put_prompt

#include "utility_functions.asm"

welcome_message:
#d "--- E74 MINICOMPUTER ---\n"
#d " By Ed Atkinson 2023\n"
#d " Type 'H' for help\n\n\0"

help_message:
#d "Commands:\n"
#d " R aaaa      read data from address\n"
#d " W aaaa dd   write data to address\n"
#d " D aaaa      dump memory at address\n"
#d " X aaaa      call address\n"
#d " ?           print return code\n"
#d "Built-in programs:\n"
#d " Fibonacci    @ 00F4\n"
#d " Maze         @ 00F7\n"
#d " Slow primes  @ 00FA\n"
#d " Fast primes  @ 00FD\n\0"

not_implemented_message:
#d "Not yet implemented...\n\0"

error_message:
#d "Error!\n\0"

fibonacci_demo:
	mov a, 1
	mov b, 1
.loop:
	call print_u8_dec
	push a
	mov a, "\n"
	call put_char
	pop a

	add b
	push a
	mov a, b
	pop b

	jnc .loop

        mov a, 0
	ret

maze_demo:
.loop:
        call rand_u8
        mov b, "\\"
        cmp 0x80
        jc .skip
        mov b, "/"
.skip:
        mov a, b
        call put_char
        jmp .loop
	ret

slow_primes_demo:
        mov a, 2
        mov b, 2
.loop:
        mov c, a
.div_loop:                              ; A = A % B
        cmp b
        je .div_skip                    ; If A < B .div_skip else .div_exit
        jnc .div_exit
.div_skip:
        sub b
        jmp .div_loop
.div_exit:
        cmp 0                           ; If no remainder then not prime
        mov a, c
        jne .skip                       ; Skip if not prime
        add 1                           ; Increment number under test
        jc .exit                        ; If hit max value then exit
        mov b, 2                        ; Reset divisor
.skip:
        inc b
        cmp b                           ; If A > B then not done testing this number
        jc .loop
        mov b, 2
        call print_u8_dec
        push a
        mov a, "\n"
        call put_char
        pop a
        add 1
        jmp .loop
.exit:
        ret

fast_primes_demo:
        mov bc, not_implemented_message
        call print_string
        ret


#bank RAM

line_ready_flag:
#res 1
return_code:
#res 1

#align 0x100 * 8
input_pointer:
#res 2
input_buffer_start:
#res 32
input_buffer_end: