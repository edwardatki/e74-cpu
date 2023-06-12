#bits 16

#subruledef a_src
{
    PC => 0x0
    SP => 0x1
    BP => 0x2
	PC++ => 0x4
	SP-- => 0x5
    BP-- => 0x6
    SP++ => 0xd
    BP++ => 0xe
}

#subruledef d_dest
{
    IR => 0x0
    MEM => 0x1
    ACC => 0x2
	RES => 0x3
	PCL => 0x4
	PCH => 0x5
	SPL => 0x6
	SPH => 0x7
	BPL => 0x8
	BPH => 0x9
	NONE => 0xf
}

#subruledef d_src
{
    MEM => 0x0
    RES => 0x1
	AL => 0x2
	AH => 0x3
	NONE => 0xf
}

#subruledef alu_mode
{
    ADD => 0x0
    SUB => 0x1
	AND => 0x2
	OR => 0x3
    NAND => 0x4
	XOR => 0x5
	SHR => 0x6
	PASS => 0x7

	NONE => 0x0
	CARRY => 0x1
	!CARRY => 0x2
	MORE => 0x3
	LESS => 0x4
	EQUAL => 0x5
	!EQUAL => 0x6
}

#ruledef
{
	mov {d: d_dest} {s: d_src} => 0b00`2 @ 0x7`3 @ s`3 @ d`4 @ 0`4
	mov {d: d_dest} {s: d_src} end => 0b10`2 @ 0x7`3 @ s`3 @ d`4 @ 0`4
	mov {d: d_dest} {s: d_src} {alu: alu_mode} => 0b00`2 @ alu`3 @ s`3 @ d`4 @ 0`4
	mov {d: d_dest} {s: d_src} {alu: alu_mode} end => 0b10`2 @ alu`3 @ s`3 @ d`4 @ 0`4

	mov {d: d_dest} {s: d_src} ({a: a_src}) => 0b00`2 @ 0x7`3 @ s`3 @ d`4 @ a`4
	mov {d: d_dest} {s: d_src} ({a: a_src}) end => 0b10`2 @ 0x7`3 @ s`3 @ d`4 @ a`4
	mov {d: d_dest} {s: d_src} ({a: a_src}) {alu: alu_mode} => 0b00`2 @ alu`3 @ s`3 @ d`4 @ a`4
	mov {d: d_dest} {s: d_src} ({a: a_src}) {alu: alu_mode} end => 0b10`2 @ alu`3 @ s`3 @ d`4 @ a`4

	mov {d: d_dest} [{a: a_src}] => 0b00`2 @ 0x7`3 @ 0x0`3 @ d`4 @ a`4
	mov {d: d_dest} [{a: a_src}] end => 0b10`2 @ 0x7`3 @ 0x0`3 @ d`4 @ a`4
	mov {d: d_dest} [{a: a_src}] {alu: alu_mode} => 0b00`2 @ alu`3 @ 0x0`3 @ d`4 @ a`4
	mov {d: d_dest} [{a: a_src}] {alu: alu_mode} end => 0b10`2 @ alu`3 @ 0x0`3 @ d`4 @ a`4

	mov [{a: a_src}] {s: d_src} => 0b00`2 @ 0x7`3 @ s`3 @ 0x1`4 @ a`4
	mov [{a: a_src}] {s: d_src} end => 0b10`2 @ 0x7`3 @ s`3 @ 0x1`4 @ a`4
	mov [{a: a_src}] {s: d_src} {alu: alu_mode} => 0b00`2 @ alu`3 @ s`3 @ 0x1`4 @ a`4
	mov [{a: a_src}] {s: d_src} {alu: alu_mode} end => 0b10`2 @ alu`3 @ s`3 @ 0x1`4 @ a`4
	
	end => 0x8000
	halt => 0x4000
	halt end => 0xc000
}

; TODO: Decrease SP can be combined with other steps if I figure out a better system of ruledefs

; 0x00 NOP
#addr 0x000
mov IR [PC++]			; Fetch
end

; 0x01 LD SP, i16
#addr 0x010
mov IR [PC++]			; Fetch
mov SPH [PC++]			; Load immediate to stack pointer low
mov SPL [PC++] end		; Load immediate to stack pointer high

; 0x02 LD BP, i16
#addr 0x020
mov IR [PC++]			; Fetch
mov BPH [PC++]			; Load immediate to address register low
mov BPL [PC++] end		; Load immediate to address register high

; 0x03 PUSH i8
#addr 0x030
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES end		; Store result to stack

; 0x04 ADD
#addr 0x040
mov IR [PC++]			; Fetch
mov ACC [SP++]			; Load accumulator from stack and increase stack pointer
mov RES [SP] ADD		; Operate on value from stack and store in result
mov [SP] RES end		; Store result to stack

; 0x05 SUB
#addr 0x050
mov IR [PC++]			; Fetch
mov ACC [SP++]			; Load accumulator from stack and increase stack pointer
mov RES [SP] SUB		; Operate on value from stack and store in result
mov [SP] RES end		; Store result to stack

; 0x06 NAND
#addr 0x060
mov IR [PC++]			; Fetch
mov ACC [SP++]			; Load accumulator from stack
mov RES [SP] NAND		; Operate on value from stack and store in result
mov [SP] RES end		; Store result to stack

; 0x07 PUSH [BP]
#addr 0x070
mov IR [PC++]			; Fetch
mov RES [BP]			; Load [BP] to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES end		; Store result to stack

; 0x08 POP [BP]
#addr 0x080
mov IR [PC++]			; Fetch
mov RES [SP++]			; Load result from stack and increase stack pointer
mov [BP] RES end		; Store to [BP]

; 0x09 PUSH BP
#addr 0x090
mov IR [PC++]			; Fetch
mov RES AL (BP)			; Load BP low to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES			; Store result to stack
mov RES AH (BP)			; Load BP high to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES end		; Store result to stack

; 0x0A POP BP
#addr 0x0a0
mov IR [PC++]			; Fetch
mov BPH [SP++]			; Load BP high from stack
mov BPL [SP++] end		; Load BP low from stack

; 0x0B LD SP, BP
#addr 0x0b0
mov IR [PC++]			; Fetch
mov BPH AH (SP)			; Load SP high to BP high		
mov BPL AL (SP)	end		; Load SP low to BP low

; 0x0C LD BP, SP
#addr 0x0c0
mov IR [PC++]			; Fetch
mov SPH AH (BP)			; Load BP high to SP high		
mov SPL AL (BP)	end		; Load BP low to SP low

; 0x0D JMP i16
#addr 0x0d0
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] NONE		; Load immediate to PC low
mov PCH RES NONE end	; Store result to PC high

; 0x0E CALL i16
#addr 0x0e0
mov IR [PC++]			; Fetch
mov RES AL (PC)			; Load PC low to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES			; Store result to stack
mov RES AH (PC)			; Load PC high to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES			; Store result to stack
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] NONE		; Load immediate to PC low
mov PCH RES	NONE end		; Store result to PC high

; 0x0F RET
#addr 0x0f0
mov IR [PC++]			; Fetch
mov PCH [SP++]			; Load result from stack and increase stack pointer
mov PCL [SP++]			; Load result from stack and increase stack pointer
mov NONE [PC++]			; Increase PC
mov NONE [PC++] end		; Increase PC

; 0x10 PUSH [BP+i8]
#addr 0x100
mov IR [PC++]		; Fetch

; Add offset
mov ACC [PC]		; Load accumulator from immediate
mov RES AL (BP) ADD	; Add immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) ADD	; Add nothing to BP high apply carry and store in result
mov BPH RES 		; Store result back to BP high

; Push value at offset
mov RES [BP]			; Load [BP] to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES			; Store result to stack

; Undo offset
mov ACC [PC++]		; Load accumulator from immediate
mov RES AL (BP) SUB	; Sub immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accululator
mov RES AH (BP) SUB	; Sub nothing to BP high apply carry and store in result
mov BPH RES end		; Store result back to BP high

; 0x11 PUSH [BP-i8]
#addr 0x110
mov IR [PC++]		; Fetch

; Add offset
mov ACC [PC]		; Load accumulator from immediate
mov RES AL (BP) SUB	; Sub immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) SUB	; Sub nothing to BP high apply carry and store in result
mov BPH RES 		; Store result back to BP high

; Push value at offset
mov RES [BP]			; Load [BP] to result
mov NONE [SP--]			; Decrease stack pointer
mov [SP] RES			; Store result to stack

; Undo offset
mov ACC [PC++]		; Load accumulator from immediate
mov RES AL (BP) ADD	; Add immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) ADD	; Add nothing to BP high apply carry and store in result
mov BPH RES end		; Store result back to BP high

; 0x12 POP [BP+i8]
#addr 0x120
mov IR [PC++]		; Fetch

; Add offset
mov ACC [PC]		; Load accumulator from immediate
mov RES AL (BP) ADD	; Add immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) ADD	; Add nothing to BP high apply carry and store in result
mov BPH RES 		; Store result back to BP high

; Pop value to offset
mov RES [SP++]			; Load result from stack and increase stack pointer
mov [BP] RES			; Store to [BP]

; Undo offset
mov ACC [PC++]		; Load accumulator from immediate
mov RES AL (BP) SUB	; Sub immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accululator
mov RES AH (BP) SUB	; Sub nothing to BP high apply carry and store in result
mov BPH RES end		; Store result back to BP high

; 0x13 POP [BP-i8]
#addr 0x130
mov IR [PC++]		; Fetch

; Add offset
mov ACC [PC]		; Load accumulator from immediate
mov RES AL (BP) SUB	; Sub immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) SUB	; Sub nothing to BP high apply carry and store in result
mov BPH RES 		; Store result back to BP high

; Pop value to offset
mov RES [SP++]			; Load result from stack and increase stack pointer
mov [BP] RES			; Store to [BP]

; Undo offset
mov ACC [PC++]		; Load accumulator from immediate
mov RES AL (BP) ADD	; Add immediate from BP low
mov BPL RES			; Store result back to BP low
mov ACC NONE		; Clear accumulator
mov RES AH (BP) ADD	; Add nothing to BP high apply carry and store in result
mov BPH RES end		; Store result back to BP high

; 0x14 POP
#addr 0x140
mov IR [PC++]		; Fetch
mov NONE [SP++] end

; 0x15 SHR
#addr 0x150
mov IR [PC++]			; Fetch
mov ACC [SP++]			; Load accumulator from stack and increase stack pointer
mov RES [SP] SHR		; Operate on value from stack and store in result
mov [SP] RES end		; Store result to stack

; 0x16 HLT
#addr 0x160
mov IR [PC++]			; Fetch
halt end

; 0x17 INC BP
#addr 0x170
mov IR [PC++]			; Fetch
mov NONE [BP++] end		; Increase BP

; 0x18 DEC BP
#addr 0x180
mov IR [PC++]			; Fetch
mov NONE [BP--] end		; Decrease BP

; 0x19 CMP
#addr 0x190
mov IR [PC++]			; Fetch
mov ACC [SP++]			; Load accumulator from stack
mov RES [SP] NONE end	; Operate on value from stack and store in result

; 0x20 JL i16
#addr 0x200
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] LESS		; Load immediate to PC low
mov PCH RES LESS end	; Store result to PC high

; 0x21 JM i16
#addr 0x210
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] MORE		; Load immediate to PC low
mov PCH RES MORE end	; Store result to PC high

; 0x22 JE i16
#addr 0x220
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] EQUAL	; Load immediate to PC low
mov PCH RES EQUAL end	; Store result to PC high

; 0x23 JNE i16
#addr 0x230
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] !EQUAL	; Load immediate to PC low
mov PCH RES !EQUAL end	; Store result to PC high

; 0x24 JC i16
#addr 0x240
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] CARRY	; Load immediate to PC low
mov PCH RES CARRY end	; Store result to PC high

; 0x25 JNC i16
#addr 0x250
mov IR [PC++]			; Fetch
mov RES [PC++]			; Load immediate to result
mov PCL [PC++] !CARRY	; Load immediate to PC low
mov PCH RES !CARRY end	; Store result to PC high