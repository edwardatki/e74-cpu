#include "architecture.asm"

#bank ROM

#addr 0x0000

; Read terminal input into circular buffer
interrupt_handler:
        push a
        push bc
        pushf

        mov bc, TERMINAL                ; Get character
        mov a, [bc]
        mov [bc], a
        
        mov bc, input_pointer           ; Store character in buffer
        mov bc, [bc]
        mov [bc], a

        cmp "\n"                        ; Test for newline and set flag
        jne .skip_1
        mov bc, line_ready_flag
        mov a, 1
        mov [bc], a
        popf
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
        mov a, b
        push c
        mov bc, input_pointer
        mov [bc], a
        inc bc
        pop a
        mov [bc], a

        popf
        pop bc
        pop a
        reti

#addr 0xee
jmp fibonacci_demo
#addr 0xf1
jmp maze_demo
#addr 0xf4
jmp slow_primes_demo
#addr 0xf7
jmp fast_primes_demo
#addr 0xfa
jmp circle_demo
#addr 0xfd
jmp game_of_life

#addr 0x0100

reset:
        mov sp, 0xffff                  ; Set up stack

        mov bc, welcome_message
        mov de, TERMINAL
.loop:
        mov a, [bc]                     ; Print string without functions call so we hopefully still get some indication of life even if something is busted
        mov [de], a
        inc bc
        cmp 0
        jne .loop

        ; call print_string

monitor:
        mov bc, input_buffer_start      ; Reset input buffer pointer
        mov de, input_pointer
        mov [de], bc
        jmp .put_prompt
.skip_whitespace:
        mov a, [de]
        inc de
        cmp " "
        je .skip_whitespace
        dec de
        ret
.put_prompt:
        mov a, 0                        ; Reset line ready flag
        mov de, line_ready_flag
        mov [de], a

        mov a, ">"
        call put_char
        tei                             ; Enable interrupts
.wait_for_line:
        mov de, line_ready_flag         ; Check if line is ready
        mov a, [de]                     ; Could go do other things while we wait
        cmp 0
        je .wait_for_line               ; Interrupts will already be disabled by the handler if EOL

        mov de, input_buffer_start
        mov a, [de]                     ; First character determines command

        cmp 0x60                        ; Shift character to uppercase if needed
        jnc ..upper_case
..lower_case:
        sub "a"-"A"
..upper_case:
        inc de

        cmp "H"                         ; Help command
        je .help_command

        cmp "R"                         ; Read command
        je .read_command

        cmp "W"                         ; Write command
        je .write_command

        cmp "D"                         ; Dump command
        je .dump_command

        cmp "U"                         ; Disassemble command
        je .disassemble_command

        cmp "X"                         ; Execute command
        je .execute_command

        cmp "P"                         ; Upload command
        je .upload_command

        cmp "?"                         ; Return code command
        je .return_code_command

        jmp .error                      ; Not a vaild command so exit

.parse_hex_byte:
        push bc
        mov b, 0x80
..next_char:
        mov a, [de]
        inc de

        cmp "\n"                        ; If eol then exit
        jne ..not_eol
        mov sp, sp+4                    ; Remove return address and bc from stack
        jmp .error
..not_eol:
        cmp 0x60                        ; Shift character to uppercase if needed
        jnc ..upper_case
..lower_case:
        sub "a"-"A"
..upper_case:

        cmp 0x2F                        ; If not between 0 and F
        jnc .error
        cmp 0x46
        jc .error

        sub 0x30

        cmp 0x09                        ; If 0-9
        jnc ..skip

        cmp 0x10                        ; If beween 9 and A
        jnc .error

        sub 0x07                        ; A-F
..skip:
        mov c, a                        ; Current digit in C
        mov a, b                        ; Test if already a digit in B
        cmp 0x80
        jne ..low_byte                  ; If have both digits the done
        mov b, c                        ; Otherwise this is the high digit, put in B
        jmp ..next_char                 ; Continue to get low digit
..low_byte:
        mov a, b                        ; Combine high and low digits
        rol
        rol
        rol
        rol
        or c

        pop bc
        ret

.help_command:
        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        mov bc, help_message
        call print_string
        jmp .end_of_line

.read_command:
        call .skip_whitespace
        
        call .parse_hex_byte            ; Parse address high
        mov b, a
        
        call .parse_hex_byte            ; Parse address low
        mov c, a

        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

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
        call .skip_whitespace
  
        call .parse_hex_byte            ; Parse address high
        mov b, a
         
        call .parse_hex_byte            ; Parse address low
        mov c, a

        call .skip_whitespace
             
        call .parse_hex_byte            ; Parse data to write

        push a
        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        pop a
        jne .error
        push a

        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex

        pop a
        mov [bc], a                     ; Write data to address
        
        mov a, ":"
        call put_char
        
        mov a, [bc]                     ; Read back and print
        call print_u8_hex
        
        mov a, "\n"
        call put_char
        
        jmp .end_of_line

; 00d0.00df no crash
; 00d0.00e0 no crash
; 00e0.00ef no crash
; 00e0.00f0 crash
.dump_command:
        call .skip_whitespace
          
        call .parse_hex_byte            ; Parse start address high
        mov b, a

        call .parse_hex_byte            ; Parse start address low
        and 0xf0                        ; Align to 16 bytes
        mov c, a

        mov a, [de]                     ; If . then end address to come
        cmp "."
        jne ..no_end_address
        inc de
           
        push bc

        call .parse_hex_byte            ; Parse end address high
        mov b, a
  
        call .parse_hex_byte            ; Parse end address low
        and 0xf0                        ; Align to 16 bytes
        mov c, a

        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        mov de, bc                      ; End address in DE
        inc de
        pop bc                          ; Start address in BC

        jmp ..got_end_address

..no_end_address:
        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        mov de, bc                      ; If no end address given then use start address plus 1
..got_end_address:
        push de
..next_line:
        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex

        mov a, ":"
        call put_char

        mov e, 15
..loop:
        mov a, [bc]
        call print_u8_hex
        mov a, " "
        call put_char
        inc bc
        jnc ..no_wrap                    ; Don't wrap back to start of memory WHY DOES THIS NOT WORK
        pop de
        jmp ..exit
..no_wrap:
        dec e
        jc ..loop

        mov a, "\n"
        call put_char

        pop de
        mov a, d
        cmp b
        je ..high_equal                 ; If end adress high < current address high then exit
        jnc ..exit
..high_equal:
        jc ..low_check_skip             ; If end address high > current address high then skip low check
        mov a, e
        cmp c
        jnc ..exit                      ; If end address low <= current address low
..low_check_skip:
        push de

        jmp ..next_line

..exit:
        jmp .end_of_line

.disassemble_command:
        call .skip_whitespace
          
        call .parse_hex_byte            ; Parse start address high
        mov b, a

        call .parse_hex_byte            ; Parse start address low
        mov c, a

        mov a, [de]                     ; If . then end address to come
        cmp "."
        jne ..no_end_address
        inc de
           
        push bc

        call .parse_hex_byte            ; Parse end address high
        mov b, a
  
        call .parse_hex_byte            ; Parse end address low
        mov c, a

        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        mov de, bc                      ; End address in DE
        inc de
        pop bc                          ; Start address in BC

        jmp ..got_end_address

..no_end_address:
        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        mov de, bc                      ; If no end address given then use start address plus 1
        inc de
..got_end_address:
        push de
..next_opcode:
        pop de
        mov a, d
        cmp b
        jc ..low_check_skip             ; If end address high > current address high
        mov a, e
        cmp c
        jnc ..exit                      ; If end address low <= current address low
..low_check_skip:
        push de

        mov a, b                        ; Print address if parsed correctly
        call print_u8_hex
        mov a, c
        call print_u8_hex

        mov a, ":"
        call put_char
        mov a, " "
        call put_char

        mov de, disassembly_table
        mov a, e
        mov e, [bc]                     ; Get opcode
        add e                           ; Add opcode times 2 to get table entry address
        jnc ..no_inc_1
        inc d
..no_inc_1:
        add e
        jnc ..no_inc_2
        inc d
..no_inc_2:
        mov e, a

        push bc
        mov de, [de]
        pop bc

        inc bc
..print_loop:
        mov a, [de]                     ; Print string, if '@' then print hex opperand

        cmp 0                           ; If zero then break
        je ..next_opcode

        cmp "@"
        je ..print_opperand

        call put_char

        inc de
        jmp ..print_loop

..print_opperand:
        mov a, [bc]
        call print_u8_hex
        inc bc

        inc de
        jmp ..print_loop

..exit:
        jmp .end_of_line

.execute_command:
        call .skip_whitespace
    
        call .parse_hex_byte            ; Parse address high
        mov b, a
              
        call .parse_hex_byte            ; Parse address low
        mov c, a

        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        call bc                         ; Call address

        mov bc, return_code             ; Store return code
        mov [bc], a
        
        jmp .end_of_line

.upload_command:
        call .skip_whitespace
        
        call .parse_hex_byte            ; Parse start address high
        mov b, a
              
        call .parse_hex_byte            ; Parse start address low
        mov c, a

        push bc

        call .skip_whitespace

        call .parse_hex_byte            ; Parse length high
        mov b, a
              
        call .parse_hex_byte            ; Parse length low
        mov c, a

        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

        pop de

..loop:
        dec bc
        je ..exit
        push bc
..wait:
        mov bc, TERMINAL+1
        mov a, [bc]
        cmp 0
        je ..wait

        mov bc, TERMINAL
        mov a, "."
        mov [bc], a
        mov a, [bc]
        pop bc

        mov [de], a
        inc de

        jmp ..loop

..exit:
        mov bc, TERMINAL
        mov a, "\n"
        mov [bc], a

        jmp .end_of_line

.return_code_command:
        call .skip_whitespace           ; If not end of line then error
        mov a, [de]
        cmp "\n"
        jne .error

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
#bank ROM

welcome_message:
#d "\n--- E74 MINICOMPUTER ---\n"
#d " By Ed Atkinson 2023\n"
#d " Type 'H' for help\n\n\0"

help_message:
#d "Commands:\n"
#d " R aaaa        read data from address\n"
#d " W aaaa dd     write data to address\n"
#d " D aaaa        dump memory at address\n"
#d " D aaaa.aaaa   dump memory in range\n"
#d " U aaaa        disassemble memory at address\n"
#d " U aaaa.aaaa   disassemble memory in range\n"
#d " X aaaa        call address\n"
#d " P aaaa llll   upload data to memory address\n"
#d " ?             print return code\n"
#d "Built-in programs:\n"
#d " Fibonacci    @ 00EE\n"
#d " Maze         @ 00F1\n"
#d " Slow primes  @ 00F4\n"
#d " Fast primes  @ 00F7\n"
#d " Circle       @ 00FA\n"
#d " Game of life @ 00FD\n\0"

not_implemented_message:
#d "Not yet implemented...\n\0"

error_message:
#d "Error!\n\0"

#include "disassembly_table.asm"

fibonacci_demo:
	mov a, 1
	mov b, 1
        mov c, 0
.loop:
        inc c
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

        mov a, c
	ret

maze_demo:
        mov de, TERMINAL
.loop:
        call rand_u8
        mov b, "\\"
        cmp 0x80
        jc .skip
        mov b, "/"
.skip:
        mov a, b
        call put_char
        mov a, [de]
        cmp 3                           ; If ctrl-c exit
        jne .loop

        mov a, "\n"
        call put_char
        mov a, 0
	ret

slow_primes_demo:
        mov a, 1
        mov b, 1
        mov e, 0
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
        inc e
        mov b, 2
        call print_u8_dec
        push a
        mov a, "\n"
        call put_char
        pop a
        add 1
        jmp .loop
.exit:
        mov a, e
        ret

fast_primes_demo:
        mov bc, prime_sieve
.clear_loop:                            ; Set sieve as all prime
        mov a, 1
        mov [bc], a
        inc c
        jnc .clear_loop

        mov b, prime_sieve[15:8]
        mov d, prime_sieve[15:8]
        mov c, 2                        ; Number under test in C
.test_loop:
        mov a, [bc]                     ; Check entry to see if prime
        cmp 0
        je .skip                        ; If not prime then skip
        mov e, c
.multiples_loop:
        mov a, e                        ; Mark all multiples as not prime
        add c                           ; Can speed up by starting at C*C instead of C
        jc .skip
        mov e, a
        mov a, 0
        mov [de], a
        jmp .multiples_loop
.skip:
        inc c
        mov a, c
        cmp 16                          ; Only need to test up to square root of max value
        jnc .test_loop

        mov bc, prime_sieve+2
        mov e, 0
.print_loop:
        mov a, [bc]                     ; Print out all entries marked prime
        cmp 0
        je .print_skip
        inc e
        mov a, c
        call print_u8_dec
        mov a, "\n"
        call put_char
.print_skip:
        inc c
        jnc .print_loop

        mov a, e
        ret

circle_demo:
        mov bc, 0                       ; B is y, C is x
.loop:
        push b
        mov a, b
        sub 8                           ; Shift down
        mov b, a
        call multiply_u8                ; Y*Y in D
        mov d, a
        pop b

        push b
        mov b, c
        mov a, c
        sub 8                           ; Shift right
        mov b, a
        call multiply_u8                ; X*X in E
        mov e, a
        pop b

        mov a, d
        add e
        cmp 63                          ; R*R = 16
        mov a, " "
        jc .no_output                   ; If X*X + Y*Y > R*R then no output
        mov a, "X"
.no_output:
        call put_char
        mov a, " "
        call put_char

        inc bc
        mov a, c
        cmp 15
        jnc .x_wrap_skip
        mov c, 0
        inc b
        mov a, "\n"
        call put_char
.x_wrap_skip:
        mov a, b
        cmp 15
        jnc .loop

        mov a, "\n"
        call put_char

        mov a, 0
        ret

; 16x16 grid
game_of_life:
        mov bc, .start_message
        call print_string
        
        mov a, 0
        mov de, life_grid
.clear_loop:
        call rand_u8
        cmp 128
        ldc
        mov [de], a
        inc e
        jnc .clear_loop
        
        ; mov a, 1
        ; mov de, life_grid+118
        ; mov [de], a
        ; mov de, life_grid+119
        ; mov [de], a
        ; mov de, life_grid+120
        ; mov [de], a

.next_frame:
        mov bc, 0
        mov de, life_grid
.draw_loop:
        mov a, [de]
        inc de
        cmp 0
        mov a, " "
        je .no_output
        mov a, "X"
.no_output:
        call put_char
        mov a, " "
        call put_char

        inc bc
        mov a, c
        cmp 15
        jnc .x_wrap_skip
        mov c, 0
        inc b
        mov a, "\n"
        call put_char
.x_wrap_skip:
        mov a, b
        cmp 15
        jnc .draw_loop

        mov bc, .next_message
        call print_string

        mov de, TERMINAL
.wait_loop:
        mov a, [de]                     ; Wait for key pressed
        cmp 0
        je .wait_loop
        cmp 3                           ; If ctrl-c exit
        je .exit

        mov de, life_grid_buffer        ; Proccess new grid into buffer
.update_loop:
        mov bc, life_grid
        mov a, 0

        mov c, e
        inc c                           ; Get right neighbour
        jc .skip_right
        add [bc]
.skip_right:

        mov c, e
        dec c                           ; Get left neighbour
        jnc .skip_left
        add [bc]
.skip_left:

        push a
        mov a, e
        add 16                          ; Get down neighbour
        mov c, a
        pop a
        jc .skip_down
        add [bc]
.skip_down:

        push a
        mov a, e
        sub 16                          ; Get up neighbour
        mov c, a
        pop a
        jnc .skip_up
        add [bc]
.skip_up:

        push a
        mov a, e
        add 15                          ; Get down left neighbour
        mov c, a
        pop a
        jc .skip_down_left
        add [bc]
.skip_down_left:

        push a
        mov a, e
        add 17                          ; Get down right neighbour
        mov c, a
        pop a
        jc .skip_down_right
        add [bc]
.skip_down_right:

        push a
        mov a, e
        sub 17                          ; Get up left neighbour
        mov c, a
        pop a
        jnc .skip_up_left
        add [bc]
.skip_up_left:

        push a
        mov a, e
        sub 15                          ; Get up right neighbour
        mov c, a
        pop a
        jnc .skip_up_right
        add [bc]
.skip_up_right:

        push a                          ; Test if current cell alive or dead
        mov c, e
        mov a, [bc]
        cmp 0
        pop a
        je .dead_checks

.alive_checks:
        cmp 2                           ; If 2 neighbours then leave alive
        je .set_one
        cmp 3                           ; If 3 neighbours then leave alive
        je .set_one
        jmp .set_zero                   ; Otherwise set dead

.dead_checks:

        cmp 3                           ; If three neighbours then set alive
        je .set_one
        jmp .set_zero                   ; Otherwise leave dead

.set_one:
        mov a, 1
        jmp .set_done
.set_zero:
        mov a, 0
.set_done:
        mov [de], a

        inc e
        jnc .update_loop        

        mov bc, life_grid_buffer
        mov de, life_grid
.copy_loop:                             ; Copy buffer
        mov a, [bc]
        mov [de], a
        inc c
        inc e
        jnc .copy_loop

        jmp .next_frame

.exit:
        call rand_u8
        mov a, 0
        ret

.next_message:
#d "-------------------------------\n\0"
.start_message:
#d "Press any key for next iteration\n"
#d "Ctrl-C to exit\n"
#d "-------------------------------\n\0"

#bank RAM

; Monitor variables
line_ready_flag:
#res 1
return_code:
#res 1

input_pointer:
#res 2
input_buffer_start:
#res 32
input_buffer_end:

; Demo program variables
#addr 0x8100
prime_sieve:

#addr 0x8100
life_grid:
#res 0x100
life_grid_buffer:
#res 0x100