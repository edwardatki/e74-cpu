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
        
        inc bc                          ; Increment pointer

        mov a, c                        ; For now only testing low byte
        cmp input_buffer_end[7:0]       ; So buffer must be properly aligned
        jne .skip
        mov bc, input_buffer_start      ; Wrap buffer back to start
.skip:
        mov de, input_pointer
        mov [de], bc

        popf
        pop de
        pop bc
        pop a
        reti

#addr 0x0100

reset:
        
        mov sp, 0x7fff                  ; Set up stack

        mov bc, welcome_message
        call print_string

        mov bc, input_buffer_start      ; Reset input buffer pointer
        mov de, input_pointer
        mov [de], bc

.put_prompt:
        mov a, ">"
        call put_char
        tei                             ; Enable interrupts
.wait_for_line:
        mov de, line_ready_flag         ; Check if line is ready
        mov a, [de]
        cmp 0
        je .wait_for_line

        tei                             ; Disable interrupts

        mov a, 0                        ; Reset line ready flag
        mov de, line_ready_flag
        mov [de], a

        mov de, input_buffer_start
        mov a, [de]                     ; First character determines command

        cmp "r"                         ; If read command
        je .read_command

        cmp "w"                         ; If write command
        je .write_command

        cmp "x"                         ; If execute command
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

        mov c, a                        ; Current digit in c
        mov a, b                        ; Test if already a digit in b
        cmp 0x80
        jne .low_byte                   ; If have both digits the done
        mov b, c                        ; Otherwise this is the high digit, put in b
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
        mov [de], bc                    ; Enable interrupts

        jmp .put_prompt

#include "print_functions.asm"

welcome_message:
#d "--- E74 Minicomputer ---\n\0"
error_message:
#d "ERROR!\n\0"

#bank RAM

#align 0x100 * 8
input_pointer:
#res 2
input_buffer_start:
#res 32
input_buffer_end:

line_ready_flag:
#res 1

return_code:
#res 1