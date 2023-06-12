#bits 8

#ruledef {
	NOP => 0x00

	LD SP, {a: i16} => 0x01 @ a
	LD BP, {a: i16} => 0x02 @ a

	PUSH {i: i8} => 0x03 @ i

	ADD => 0x04
	SUB => 0x05
	NAND => 0x06

	PUSH [BP] => 0x07
	POP [BP] => 0x08

	PUSH BP => 0x09
	POP BP => 0x0A

	LD BP, SP => 0x0B
	LD SP, BP => 0x0C


	JMP {a: i16} => 0x0D @ a

	CALL {a: i16} => 0x0E @ a
	RET => 0x0F

	PUSH [BP+{i: u8}] => 0x10 @ i
	PUSH [BP-{i: u8}] => 0x11 @ i
	POP [BP+{i: u8}] => 0x12 @ i
	POP [BP-{i: u8}] => 0x13 @ i

	POP => 0x14

	SHR => 0x15

	HALT => 0x16

	INC BP => 0x17
	DEC BP => 0x18

	CMP => 0x19

	JL {a: i16} => 0x20 @ a
	JM {a: i16} => 0x21 @ a
	JE {a: i16} => 0x22 @ a
	JNE {a: i16} => 0x23 @ a
	JC {a: i16} => 0x24 @ a
	JNC {a: i16} => 0x25 @ a
}

start:
LD SP, 0x7fff

; Hex dump
LD BP, 0x0000
PUSH 0
.loop:
	PUSH BP
	PUSH [BP]
	CALL print_hex
	POP
	POP BP

	PUSH 1
	ADD
	PUSH 8
	CMP
	JL .less
	POP
	PUSH 0
	PUSH BP
	PUSH "\n"
	jmp .endif
	.less:
	PUSH BP
	PUSH " "
	.endif:
	CALL putc
	POP
	POP BP

	INC BP

	JMP .loop

JMP $

print_hex:
	LD BP, SP

	PUSH 0		; Local variable [BP-1]

	PUSH [BP+2]	; Load parameter
	PUSH 0xf0  	; Get upper 4 bits
	NAND        
	PUSH 0xff
	NAND
	PUSH 4		; Shift right
	SHR
	PUSH 0x0A	; Compare
	CMP
	JL .skip1	; If 0-9
	PUSH 0x07	; ASCII A-F
	ADD
	.skip1:
	PUSH 0x30   ; ASCII 0-9
	ADD
	POP [BP-1]	; Store in local var
	
	PUSH BP
	PUSH [BP-1]	; Load local var
	CALL putc   ; Print char
	POP
	POP BP

	PUSH [BP+2] ; Load parameter
	PUSH 0x0f   ; Get lower 4 bits
	NAND        
	PUSH 0xff
	NAND
	PUSH 0x0A	; Compare
	CMP
	JL .skip2	; If 0-9
	PUSH 0x07	; ASCII A-F
	ADD
	.skip2:
	PUSH 0x30   ; ASCII 0-9
	ADD
	POP [BP-1]	; Store in local var

	PUSH BP
	PUSH [BP-1]	; Load local var
	CALL putc   ; Print char
	POP
	POP BP

	LD SP, BP
	RET

putc:
	LD BP, SP

	PUSH [BP+2]		; Load parameter
	LD BP, 0x8000
	POP [BP]      	; Store parameter to output

	RET