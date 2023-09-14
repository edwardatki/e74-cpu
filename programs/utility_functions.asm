TERMINAL = 0x7FFF

#bank ROM

rand_u8:
	push de
	push bc
	mov de, rand_seed
	mov a, [de]
	mov b, a
	mov c, a
	add c
	mov c, a
	add c
	add b
	add 7
	mov [de], a
	pop bc
	pop de
	ret

; A divide by B
; Result in C
; Remainder in A
divide_u8:
	mov c, 0
.loop:
	cmp b
	je .skip
	jnc .exit ; Less than or equal
.skip:
	sub b
	inc c
	jmp .loop
.exit:
	ret

; Print character in A
put_char:
	push de
	mov de, TERMINAL
	mov [de], a
	pop de
	ret

; Print A in decimal
print_u8_dec:
	push a
	push bc
	mov b, 100
	call divide_u8
	
	; x--
	push a
	mov a, c
	cmp 0
	je .skip1
	add "0"
	call put_char
	pop a

	mov b, 10
	call divide_u8

	; -x-
	push a
	mov a, c
	add "0"
	call put_char
	pop a
	jmp .skip3

.skip1:
	pop a

	mov b, 10
	call divide_u8

	; -x-
	push a
	mov a, c
	cmp 0
	je .skip2
	add "0"
	call put_char
.skip2:
	pop a

.skip3:
	; --x
	add "0"
	call put_char

	pop bc
	pop a
	ret

; Print A in hex
print_u8_hex:
	push a
	rol
	rol
	rol
	rol
	and 0x0f
	cmp 9
	jnc .skip1 ; Less than or equal
	add "A"-"9"-1
.skip1:
	add "0"
	call put_char

	pop a
	push a
	and 0x0f
	cmp 9
	jnc .skip2 ; Less than or equal
	add "A"-"9"-1
.skip2:
	add "0"
	call put_char

	pop a
	ret

; Print string pointed to by BC
print_string:
	push a
	push bc
	push de
	mov de, TERMINAL
.loop:
	mov a, [bc]
	mov [de], a
	inc bc
	cmp 0
	jne .loop
.exit:
	pop de
	pop bc
	pop a
	ret

#bank RAM
rand_seed:
#res 1