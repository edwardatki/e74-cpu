#ruledef {
	nop => 0x00

	mov a, {i: i8} => 0x01 @ i
	mov b, {i: i8} => 0x02 @ i
	mov c, {i: i8} => 0x03 @ i
    mov d, {i: i8} => 0x04 @ i
	mov e, {i: i8} => 0x05 @ i

    mov bc, {i: i16} => 0x06 @ i
    mov de, {i: i16} => 0x07 @ i
    mov sp, {i: i16} => 0x08 @ i

	add {i: i8} => 0x09 @ i
    sub {i: i8} =>0x0A @ i
    and {i: i8} => 0x0B @ i
    nand {i: i8} => 0x0C @ i
    or {i: i8} => 0x0D @ i
    nor {i: i8} => 0x0E @ i
    xor {i: i8} => 0x0F @ i

    add b => 0x10
    sub b => 0x11
    and b => 0x12
    nand b => 0x13
    or b => 0x14
    nor b => 0x15
    xor b => 0x16

    add c => 0x17
    sub c => 0x18
    and c => 0x19
    nand c => 0x1A
    or c => 0x1B
    nor c => 0x1C
    xor c => 0x1D

    add d => 0x1E
    sub d => 0x1F
    and d => 0x20
    nand d => 0x21
    or d => 0x22
    nor d => 0x23
    xor d => 0x24

    add e => 0x25
    sub e => 0x26
    and e => 0x27
    nand e => 0x28
    or e => 0x29
    nor e => 0x2A
    xor e => 0x2B

    add [bc] => 0x2C
    sub [bc] => 0x2D
    and [bc] => 0x2E
    nand [bc] => 0x2F
    or [bc] => 0x30
    nor [bc] => 0x31
    xor [bc] => 0x32

    add [de] => 0x33
    sub [de] => 0x34
    and [de] => 0x35
    nand [de] => 0x36
    or [de] => 0x37
    nor [de] => 0x38
    xor [de] => 0x39

    not => 0x3A

    push a => 0x3B
    push b => 0x3C
    push c => 0x3D
    push d => 0x3E
    push e => 0x3F

    pop a => 0x40
    pop b => 0x41
    pop c => 0x42
    pop d => 0x43
    pop e => 0x44

    push bc => 0x45
    push de => 0x46

    pop bc => 0x47
    pop de => 0x48

    inc bc => 0x49
	inc de => 0x4A

    dec bc => 0x4B
	dec de => 0x4C

	jmp {i: i16} => 0x4D @ i
    jmp bc => 0x4E
    jmp de => 0x4F
    jmp [bc] => 0x50
    jmp [de] => 0x51

    jc {i: i16} => 0x52 @ i
    jc bc => 0x53
    jc de => 0x54
    jc [bc] => 0x55
    jc [de] => 0x56

    jnc {i: i16} => 0x57 @ i
    jnc bc => 0x58
    jnc de => 0x59
    jnc [bc] => 0x5A
    jnc [de] => 0x5B

    je {i: i16} => 0x5C @ i
    je bc => 0x5D
    je de => 0x5E
    je [bc] => 0x5F
    je [de] => 0x60

    jne {i: i16} => 0x61 @ i
    jne bc => 0x62
    jne de => 0x63
    jne [bc] => 0x64
    jne [de] => 0x65

    cmp {i: i8} => 0x66 @ i
    cmp b => 0x67
    cmp c => 0x68
    cmp d => 0x69
    cmp e => 0x6A
    cmp [bc] => 0x6B
    cmp [de] => 0x6C

    mov a, [bc] => 0x6D
    mov d, [bc] => 0x6E
    mov e, [bc] => 0x6F

	mov a, [de] => 0x70
    mov b, [de] => 0x71
    mov c, [de] => 0x72

    mov [bc], a => 0x73
    mov [bc], d => 0x74
    mov [bc], e => 0x75

    mov [de], a => 0x76
    mov [de], b => 0x77
    mov [de], c => 0x78

    mov bc, [de] => 0x79
    mov de, [bc] => 0x7A

    mov [bc], de => 0x7B
    mov [de], bc => 0x7C

    call {i: i16} => 0x7D @ i
    call bc => 0x7E
    call de => 0x7F
    call [bc] => 0x80
    call [de] => 0x81

    ret => 0x82

    mov b, a => 0x83
    mov b, c => 0x84
    mov b, d => 0x85
    mov b, e => 0x86

    mov c, a => 0x87
    mov c, b => 0x88
    mov c, d => 0x89
    mov c, e => 0x8A

    mov d, a => 0x8B
    mov d, b => 0x8C
    mov d, c => 0x8D
    mov d, e => 0x8E

    mov e, a => 0x8F
    mov e, b => 0x90
    mov e, c => 0x91
    mov e, d => 0x92

    mov bc, de => 0x93
    mov bc, sp => 0x94

    mov de, bc => 0x95
    mov de, sp => 0x96

    mov sp, bc => 0x97
    mov sp, de => 0x98

    rol => 0x99

    mov bc, sp+{i: i8} => 0x9A @ i
    mov de, sp+{i: i8} => 0x9B @ i

    inc b => 0x9C
    inc c => 0x9D
    inc d => 0x9E
    inc e => 0x9F

    dec b => 0xA0
    dec c => 0xA1
    dec d => 0xA2
    dec e => 0xA3

    mov a, b => 0xA4
    mov a, c => 0xA5
    mov a, d => 0xA6
    mov a, e => 0xA7

    mov bc, sp-{i: i8} => 0xA8 @ i
    mov de, sp-{i: i8} => 0xA9 @ i

    mov sp, sp+{i: i8} => 0xAA @ i
    mov sp, sp-{i: i8} => 0xAB @ i

    tei => 0xAC
    reti => 0xAD

    ldc => 0xAE
    lde => 0xAF

    pushf => 0xB0
    popf => 0xB1
}

; cmp i8
; je        ; a == i8
; jne       ; a != i8
; jc i16    ; a > i8
; jnc i16   ; a <= i8

#bankdef RAM
{
    #addr 0x0000
    #size 0x8000
    #outp 0
	#bits 8
}