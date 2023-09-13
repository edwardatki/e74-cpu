#include <stdio.h>
#include <stdint.h>

#define MICROCODE_STEPS 16

#define nSP_OUT 0x00000001
#define nALU_OUT 0x00000002
#define nPC_OUT 0x00000004
#define nA_OUT 0x00000008
#define nT_OUT 0x00000010
#define nBC_OUT 0x00000020
#define nDE_OUT 0x00000040
#define nRAM_OUT 0x00000080

#define PC_H_WRITE 0x00000100
#define SP_H_WRITE 0x00000200
#define B_WRITE 0x00000400
#define PC_L_WRITE 0x00000800
#define C_WRITE 0x00001000
#define SP_L_WRITE 0x00002000
#define E_WRITE 0x00004000
#define D_WRITE 0x00008000

#define nAH_OUT 0x00010000
#define ALU_MODE 0x00020000
#define nAL_OUT 0x00040000
#define ALU_CARRY 0x00080000
#define ALU_S0 0x00100000
#define ALU_S1 0x00200000
#define ALU_S2 0x00400000
#define ALU_S3 0x00800000

#define A_WRITE 0x01000000
#define T_WRITE 0x02000000
#define nRAM_WRITE 0x04000000
#define END_INSTRUCTION 0x08000000
#define IE_TOGGLE 0x10000000
#define IR_WRITE 0x20000000
#define FLAGS_USR_UPDATE 0x40000000
#define FLAGS_SYS_UPDATE 0x80000000


#define ALU_OP_ZERO                 ALU_MODE | ALU_S1 | ALU_S0
#define ALU_OP_ONE                  ALU_MODE | ALU_S3 | ALU_S2 // Broken in logisim???
#define ALU_OP_FFFF                 ALU_CARRY | ALU_S1 | ALU_S0

#define ALU_OP_A                    ALU_CARRY
#define ALU_OP_B                    ALU_MODE | ALU_S3 | ALU_S1

#define ALU_OP_A_PLUS_1             0
#define ALU_OP_A_PLUS_B             ALU_S3 | ALU_S0 | ALU_CARRY
#define ALU_OP_A_PLUS_B_PLUS_1      ALU_S3 | ALU_S0

#define ALU_OP_A_MINUS_1            ALU_S3 | ALU_S2 | ALU_S1 | ALU_S0 | ALU_CARRY
#define ALU_OP_A_MINUS_B            ALU_S2 | ALU_S1
#define ALU_OP_A_MINUS_B_MINUS_1    ALU_S2 | ALU_S1 | ALU_CARRY

#define ALU_OP_A_PLUS_A             ALU_S3 | ALU_S2 | ALU_CARRY
#define ALU_OP_A_PLUS_A_PLUS_1      ALU_S3 | ALU_S2

#define ALU_OP_NOT_A                ALU_MODE
#define ALU_OP_NOT_B                ALU_MODE | ALU_S2 | ALU_S0

#define ALU_OP_A_AND_B              ALU_MODE | ALU_S3 | ALU_S1 | ALU_S0
#define ALU_OP_A_OR_B               ALU_MODE | ALU_S3 | ALU_S2 | ALU_S1
#define ALU_OP_A_NAND_B             ALU_MODE | ALU_S2
#define ALU_OP_A_XOR_B              ALU_MODE | ALU_S2 | ALU_S1

#define NOP_STEP nPC_OUT | nSP_OUT | nBC_OUT | nDE_OUT | nALU_OUT | nAL_OUT | nAH_OUT | nRAM_OUT | nRAM_WRITE | nA_OUT | nT_OUT

FILE *outputFile;

int opcodeStepCount = 0;

void addStep(uint32_t stepMask) {
    static int count = 0;
    
    uint32_t controlWord = ((uint32_t)NOP_STEP) ^ stepMask;
    fprintf(outputFile, "%08x", controlWord); 
    
    count += 1;
    if (count >= 8) {
        fprintf(outputFile, "\n");
        count = 0;
    }
    else fprintf(outputFile, " ");

    opcodeStepCount += 1;
}

void finishOpcode() {
    while (opcodeStepCount < MICROCODE_STEPS) {
        addStep(END_INSTRUCTION);
    }
    opcodeStepCount = 0;
}

int main() {
    outputFile = fopen("microcode.lbi", "w");
    fprintf(outputFile, "v3.0 hex words plain\n");

    for (int opcode = 0; opcode <= 0xff; opcode++) {
        for (int interrupt = 0; interrupt <= 1; interrupt++) {
            for (int sysCarry = 0; sysCarry <= 1; sysCarry++) {
                for (int usrEqual = 0; usrEqual <= 1; usrEqual++) {
                    for (int usrCarry = 0; usrCarry <= 1; usrCarry++) {

                        if (interrupt) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = PC_L
                            addStep(nPC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = PC_H
                            addStep(nPC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                            // ---

                            // PC_L = 0x00
                            addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_ZERO);

                            // PC_H = 0x00 and disable interrupts
                            addStep(nSP_OUT | nRAM_OUT | PC_H_WRITE | IE_TOGGLE | ALU_OP_ZERO | END_INSTRUCTION);
                        } else if (opcode == 0x00) {
                            // Opcode 0x00 is a special case loaded by reset signal so load 0x0100 into PC
                            // Maybe I should implement vectors for reset and interrupt like a 6502

                            // PC_L = 0x00
                            addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_ZERO);

                            // PC_H = 0x10
                            addStep(nPC_OUT | nRAM_OUT | PC_H_WRITE | ALU_OP_A_PLUS_1);

                            // Load literally anything else into IR
                            addStep(nPC_OUT | nRAM_OUT | IR_WRITE | END_INSTRUCTION);
                        } else {
                            // IR = RAM[PC++]
                            addStep(nPC_OUT | nRAM_OUT | IR_WRITE | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                            
                            // --- MOV A, i8 ---
                            if (opcode == 0x01) {
                                // A = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | A_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV B, i8 ---
                            if (opcode == 0x02) {
                                // B = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV C, i8 ---
                            if (opcode == 0x03) {
                                // C = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV D, i8 ---
                            if (opcode == 0x04) {
                                // D = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV E, i8 ---
                            if (opcode == 0x05) {
                                // E = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV BC, i16 ---
                            if (opcode == 0x06) {
                                // B = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // C = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV DE, i16 ---
                            if (opcode == 0x07) {
                                // D = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // E = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV SP, i16 ---
                            if (opcode == 0x08) {
                                // SP_H = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | SP_H_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // SP_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | SP_L_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- ADD i8 ---
                            if (opcode == 0x09) {        
                                // A = A plus RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- SUB i8 ---
                            if (opcode == 0x0A) {        
                                // A = A sub RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | nA_OUT | A_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- AND i8 ---
                            if (opcode == 0x0B) {        
                                // A = A and RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | nA_OUT | A_WRITE | ALU_OP_A_AND_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- NAND i8 ---
                            // --- OR i8 ---
                            // --- NOR i8 ---
                            // --- XOR i8 ---

                            // --- ADD B ---
                            if (opcode == 0x10) {
                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- SUB B ---
                            if (opcode == 0x11) {
                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- AND B ---
                            // --- NAND B ---

                            // --- OR B ---
                            if (opcode == 0x14) {
                                // T = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A OR T
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_OR_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }
                            
                            
                            // --- NOR B ---
                            // --- XOR B ---

                            // --- ADD C ---
                            if (opcode == 0x17) {
                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- SUB C ---
                            if (opcode == 0x18) {
                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- AND C ---
                            // --- NAND C ---

                            // --- OR C ---
                            if (opcode == 0x1B) {
                                // T = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A OR T
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_OR_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }
                            
                            // --- NOR C ---
                            // --- XOR C ---

                            // --- ADD D ---
                            if (opcode == 0x1E) {
                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- SUB D ---
                            if (opcode == 0x1F) {
                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- AND D ---
                            // --- NAND D ---
                            // --- OR D ---
                            // --- NOR D ---
                            // --- XOR D ---

                            // --- ADD E ---
                            if (opcode == 0x25) {
                                // TMP = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- SUB E ---
                            if (opcode == 0x26) {
                                // TMP = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP
                                addStep(nT_OUT | nA_OUT | A_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- AND E ---
                            // --- NAND E ---
                            // --- OR E ---
                            // --- NOR E ---
                            // --- XOR E ---
                            
                            // --- ADD [BC] ---
                            if (opcode == 0x2C) {
                                // A = A plus RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | nA_OUT | A_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- SUB [BC] ---
                            // --- NAND [BC] ---
                            // --- OR [BC] ---
                            // --- NOR [BC] ---
                            // --- XOR [BC] ---

                            // --- ADD [DE] ---
                            // --- SUB [DE] ---
                            // --- AND [DE] ---
                            // --- NAND [DE] ---
                            // --- OR [DE] ---
                            // --- NOR [DE] ---
                            // --- XOR [DE] ---

                            // --- NOT ---

                            // --- PUSH A ---
                            if (opcode == 0x3B) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // RAM[SP] = A
                                addStep(nSP_OUT | nRAM_WRITE | nA_OUT | nALU_OUT | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- PUSH B ---
                            if (opcode == 0x3C) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- PUSH C ---
                            if (opcode == 0x3D) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- PUSH D ---
                            if (opcode == 0x3E) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- PUSH E ---
                            if (opcode == 0x3F) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- POP A ---
                            if (opcode == 0x40) {
                                // A = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | A_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- POP B ---
                            if (opcode == 0x41) {
                                // B = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- POP C ---
                            if (opcode == 0x42) {
                                // B = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- POP D ---
                            if (opcode == 0x43) {
                                // D = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- POP E ---
                            if (opcode == 0x44) {
                                // E = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- PUSH BC ---
                            if (opcode == 0x45) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- PUSH DE ---
                            if (opcode == 0x46) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = TMP
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- POP BC ---
                            if (opcode == 0x47) {
                                // B = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // C = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- POP DE ---
                            if (opcode == 0x48) {
                                // D = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // E = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- INC BC ---
                            if (opcode == 0x49) {
                                // BC++
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- INC DE ---
                            if (opcode == 0x4A) {
                                // DE++
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- DEC BC ---
                            if (opcode == 0x4B) {
                                // BC--
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- DEC DE ---
                            if (opcode == 0x4C) {
                                // DE--
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- JMP i16 ---
                            if (opcode == 0x4D) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = TMP
                                addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- JMP BC ---
                            if (opcode == 0x4E) {
                                // PC_L = C
                                addStep(nBC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A);

                                // PC_H = B
                                addStep(nBC_OUT | nAH_OUT | PC_H_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- JMP DE ---
                            if (opcode == 0x4F) {
                                // PC_L = E
                                addStep(nDE_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A);

                                // PC_H = D
                                addStep(nDE_OUT | nAH_OUT | PC_H_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- JMP [BC] ---
                            // --- JMP [DE] ---

                            // --- JC i16 ---
                            if (opcode == 0x52) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                if (!usrCarry) {
                                    // PC_L = RAM[PC]
                                    addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                    // PC_H = TMP
                                    addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);
                                } else {
                                    // PC++
                                    addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                    addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                                }
                            }

                            // --- JC BC ---
                            // --- JC DE ---
                            // --- JC [BC] ---
                            // --- JC [DE] ---

                            // --- JNC i16 ---
                            if (opcode == 0x57) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                if (usrCarry) {
                                    // PC_L = RAM[PC]
                                    addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                    // PC_H = TMP
                                    addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);
                                } else {
                                    // PC++
                                    addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                    addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                                }
                            }

                            // --- JNC BC ---
                            // --- JNC DE ---
                            // --- JNC [BC] ---
                            // --- JNC [DE] ---

                            // --- JE i16 ---
                            if (opcode == 0x5C) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                if (usrEqual) {
                                    // PC_L = RAM[PC]
                                    addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                    // PC_H = TMP
                                    addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);
                                } else {
                                    // PC++
                                    addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                    addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                                }
                            }

                            // --- JE BC ---
                            // --- JE DE ---
                            // --- JE [BC] ---
                            // --- JE [DE] ---

                            // --- JNE i16 ---
                            if (opcode == 0x61) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                if (!usrEqual) {
                                    // PC_L = RAM[PC]
                                    addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                    // PC_H = TMP
                                    addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);
                                } else {
                                    // PC++
                                    addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                    addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                                }
                            }

                            // --- JNE BC ---
                            // --- JNE DE ---
                            // --- JNE [BC] ---
                            // --- JNE [DE] ---

                            // --- CMP i8 ---
                            if (opcode == 0x66) {
                                // A minus RAM[PC] minus 1
                                addStep(nPC_OUT | nRAM_OUT | nA_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- CMP B ---
                            if (opcode == 0x67) {
                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP minus 1
                                addStep(nT_OUT | nA_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }
                            
                            // --- CMP C ---
                            if (opcode == 0x68) {
                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP minus 1
                                addStep(nT_OUT | nA_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- CMP D ---
                            if (opcode == 0x69) {
                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP minus 1
                                addStep(nT_OUT | nA_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- CMP E ---
                            if (opcode == 0x6A) {
                                // TMP = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // A minus TMP minus 1
                                addStep(nT_OUT | nA_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- CMP [BC] ---
                            // --- CMP [DE] ---

                            // --- MOV A, [BC] ---
                            if (opcode == 0x6D) {
                                // A = RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | A_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV D, [BC] ---
                            if (opcode == 0x6E) {
                                // D = RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | D_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV E, [BC] ---
                            if (opcode == 0x6F) {
                                // E = RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | E_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV A, [DE] ---
                            if (opcode == 0x70) {
                                // A = RAM[DE]
                                addStep(nDE_OUT | nRAM_OUT | A_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV B, [DE] ---
                            if (opcode == 0x71) {
                                // B = RAM[DE]
                                addStep(nDE_OUT | nRAM_OUT | B_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV C, [DE] ---
                            if (opcode == 0x72) {
                                // C = RAM[DE]
                                addStep(nDE_OUT | nRAM_OUT | C_WRITE | ALU_OP_B | END_INSTRUCTION);
                            }

                            // --- MOV [BC], A ---
                            if (opcode == 0x73) {
                                // RAM[BC] = A
                                addStep(nBC_OUT | nRAM_WRITE | nA_OUT | nALU_OUT | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV [BC], D ---
                            if (opcode == 0x74) {
                                // TMP = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[BC] = TMP
                                addStep(nBC_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- MOV [BC], E ---
                            if (opcode == 0x75) {
                                // TMP = L
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);
                                
                                // RAM[BC] = TMP
                                addStep(nBC_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- MOV [DE], A ---
                            if (opcode == 0x76) {
                                // RAM[DE] = A
                                addStep(nDE_OUT | nRAM_WRITE | nA_OUT | nALU_OUT | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV [DE], B ---
                            if (opcode == 0x77) {
                                // TMP = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);
                                
                                // RAM[DE] = TMP
                                addStep(nDE_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- MOV [DE], C ---
                            if (opcode == 0x78) {
                                // TMP = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);
                                
                                // RAM[DE] = TMP
                                addStep(nDE_OUT | nRAM_WRITE | nT_OUT | END_INSTRUCTION);
                            }

                            // --- MOV BC, [DE] ---
                            if (opcode == 0x79) {
                                // B = RAM[DE]
                                addStep(nDE_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                                // DE++
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // C = RAM[DE]
                                addStep(nDE_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                                // DE--
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV DE, [BC] ---
                            if (opcode == 0x7A) {
                                // D = RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                                // DE++
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // E = RAM[BC]
                                addStep(nBC_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                                // DE--
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                            
                            // --- MOV [BC], DE ---
                            if (opcode == 0x7B) {
                                // T = D
                                addStep(nDE_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[BC] = T
                                addStep(nBC_OUT | nT_OUT | nRAM_WRITE);

                                // BC++
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // T = E
                                addStep(nDE_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[BC] = T
                                addStep(nBC_OUT | nT_OUT | nRAM_WRITE);

                                // BC--
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV [DE], BC ---
                            if (opcode == 0x7C) {
                                // T = B
                                addStep(nBC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // RAM[DE] = T
                                addStep(nDE_OUT | nT_OUT | nRAM_WRITE);

                                // DE++
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // T = C
                                addStep(nBC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[DE] = T
                                addStep(nDE_OUT | nT_OUT | nRAM_WRITE);

                                // DE--
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- CALL i16 ---
                            if (opcode == 0x7D) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // TMP = PC_L plus 1
                                addStep(nPC_OUT | nAL_OUT | T_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);

                                // TMP = T plus 1
                                addStep(nT_OUT | T_WRITE | ALU_OP_A_PLUS_B_PLUS_1 | (!sysCarry ? 0 : FLAGS_SYS_UPDATE));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // T = PC_H
                                addStep(nPC_OUT | nAH_OUT | T_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // ---

                                // T = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = T
                                addStep(nT_OUT | PC_H_WRITE | ALU_OP_B | END_INSTRUCTION);

                                // This can probably be optimised
                            }

                            // --- CALL BC ---
                            if (opcode == 0x7E) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // T = PC_L
                                addStep(nPC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // T = PC_H
                                addStep(nPC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // ---

                                // PC_L = C
                                addStep(nBC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A);

                                // PC_H = B
                                addStep(nBC_OUT | nAH_OUT | PC_H_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- CALL DE ---
                            if (opcode == 0x7F) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // T = PC_L
                                addStep(nPC_OUT | nAL_OUT | T_WRITE | ALU_OP_A);

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // T = PC_H
                                addStep(nPC_OUT | nAH_OUT | T_WRITE | ALU_OP_A);

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nRAM_WRITE | nT_OUT);

                                // ---

                                // PC_L = E
                                addStep(nDE_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A);

                                // PC_H = D
                                addStep(nDE_OUT | nAH_OUT | PC_H_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- CALL [BC] ---
                            // --- CALL [DE] ---

                            // --- RET ---
                            if (opcode == 0x82) {
                                // PC_H = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | PC_H_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC_L = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV B, A ---
                            if (opcode == 0x83) {
                                addStep(nA_OUT | B_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV B, C ---
                            if (opcode == 0x84) {
                                addStep(nBC_OUT | nAL_OUT | B_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV B, D ---
                            if (opcode == 0x85) {
                                addStep(nDE_OUT | nAH_OUT | B_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV B, E ---
                            if (opcode == 0x86) {
                                addStep(nDE_OUT | nAL_OUT | B_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV C, A ---
                            if (opcode == 0x87) {
                                addStep(nA_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV C, B ---
                            if (opcode == 0x88) {
                                addStep(nBC_OUT | nAH_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV C, D ---
                            if (opcode == 0x89) {
                                addStep(nDE_OUT | nAH_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV C, E ---
                            if (opcode == 0x8A) {
                                addStep(nDE_OUT | nAH_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV D, A ---
                            if (opcode == 0x8B) {
                                addStep(nA_OUT | D_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV D, B ---
                            if (opcode == 0x8C) {
                                addStep(nBC_OUT | nAH_OUT | D_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV D, C ---
                            if (opcode == 0x8D) {
                                addStep(nBC_OUT | nAL_OUT | D_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV D, E ---
                            if (opcode == 0x8E) {
                                addStep(nDE_OUT | nAL_OUT | D_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV E, A ---
                            if (opcode == 0x8F) {
                                addStep(nA_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV E, B ---
                            if (opcode == 0x90) {
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV E, C ---
                            if (opcode == 0x91) {
                                addStep(nBC_OUT | nAL_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV E, D ---
                            if (opcode == 0x92) {
                                addStep(nDE_OUT | nAH_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV BC, DE ---
                            if (opcode == 0x93) {
                                addStep(nDE_OUT | nAH_OUT | B_WRITE | ALU_OP_A);
                                addStep(nDE_OUT | nAL_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV BC, SP ---
                            if (opcode == 0x94) {
                                addStep(nSP_OUT | nAH_OUT | B_WRITE | ALU_OP_A);
                                addStep(nSP_OUT | nAL_OUT | C_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }


                            // --- MOV DE, BC ---
                            if (opcode == 0x95) {
                                addStep(nBC_OUT | nAH_OUT | D_WRITE | ALU_OP_A);
                                addStep(nBC_OUT | nAL_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV DE, SP ---
                            if (opcode == 0x96) {
                                addStep(nSP_OUT | nAH_OUT | D_WRITE | ALU_OP_A);
                                addStep(nSP_OUT | nAL_OUT | E_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV SP, BC ---
                            if (opcode == 0x97) {
                                addStep(nBC_OUT | nAH_OUT | SP_H_WRITE | ALU_OP_A);
                                addStep(nBC_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV SP, DE ---
                            if (opcode == 0x98) {
                                addStep(nDE_OUT | nAH_OUT | SP_H_WRITE | ALU_OP_A);
                                addStep(nDE_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- ROL ---
                            if (opcode == 0x99) {        
                                // A = A plus A
                                addStep(nA_OUT | A_WRITE | ALU_OP_A_PLUS_A | FLAGS_SYS_UPDATE);
                                addStep(nA_OUT | A_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV BC, SP+i8 ---
                            if (opcode == 0x9A) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // C = SP_L plus TMP
                                addStep(nSP_OUT | nAL_OUT| C_WRITE | nT_OUT | ALU_OP_A_PLUS_B | FLAGS_SYS_UPDATE);
                                
                                // B = SP_H
                                addStep(nSP_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV DE, SP+i8 ---
                            if (opcode == 0x9B) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // E = SP_L plus TMP
                                addStep(nSP_OUT | nAL_OUT| E_WRITE | nT_OUT | ALU_OP_A_PLUS_B | FLAGS_SYS_UPDATE);
                                
                                // D = SP_H
                                addStep(nSP_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- INC B ---
                            if (opcode == 0x9C) {
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- INC C ---
                            if (opcode == 0x9D) {
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- INC D ---
                            if (opcode == 0x9E) {
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- INC E ---
                            if (opcode == 0x9F) {
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- DEC B ---
                            if (opcode == 0xA0) {
                                addStep(nBC_OUT | nAH_OUT | B_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- DEC C ---
                            if (opcode == 0xA1) {
                                addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- DEC D ---
                            if (opcode == 0xA2) {
                                addStep(nDE_OUT | nAH_OUT | D_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // --- DEC E ---
                            if (opcode == 0xA3) {
                                addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE | END_INSTRUCTION);
                            }

                            // -- MOV A, B ---
                            if (opcode == 0xA4) { 
                                addStep(nBC_OUT | nAH_OUT | A_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // -- MOV A, C ---
                            if (opcode == 0xA5) {
                                addStep(nBC_OUT | nAL_OUT | A_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // -- MOV A, D ---
                            if (opcode == 0xA6) {
                                addStep(nDE_OUT | nAH_OUT | A_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // -- MOV A, E ---
                            if (opcode == 0xA7) {
                                addStep(nDE_OUT | nAL_OUT | A_WRITE | ALU_OP_A | END_INSTRUCTION);
                            }

                            // --- MOV BC, SP-i8 ---
                            if (opcode == 0xA8) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // C = SP_L minus TMP
                                addStep(nSP_OUT | nAL_OUT| C_WRITE | nT_OUT | ALU_OP_A_MINUS_B | FLAGS_SYS_UPDATE);
                                
                                // B = SP_H
                                addStep(nSP_OUT | nAH_OUT | B_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV DE, SP-i8 ---
                            if (opcode == 0xA9) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // E = SP_L minus TMP
                                addStep(nSP_OUT | nAL_OUT| E_WRITE | nT_OUT | ALU_OP_A_MINUS_B | FLAGS_SYS_UPDATE);
                                
                                // D = SP_H
                                addStep(nSP_OUT | nAH_OUT | D_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV SP, SP+i8 ---
                            if (opcode == 0xAA) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // SP_L = SP_L plus TMP
                                addStep(nSP_OUT | nAL_OUT| SP_L_WRITE | nT_OUT | ALU_OP_A_PLUS_B | FLAGS_SYS_UPDATE);
                                
                                // SP_H = SP_H
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- MOV SP, SP-i8 ---
                            if (opcode == 0xAB) {
                                // TMP = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | T_WRITE | ALU_OP_B);

                                // SP_L = SP_L minus TMP
                                addStep(nSP_OUT | nAL_OUT| SP_L_WRITE | nT_OUT | ALU_OP_A_MINUS_B | FLAGS_SYS_UPDATE);
                                
                                // SP_H = SP_H
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- TEI ---
                            if (opcode == 0xAC) {
                                addStep(IE_TOGGLE | END_INSTRUCTION);
                            }

                            // --- RETI ---
                            if (opcode == 0xAD) {
                                // PC_H = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | PC_H_WRITE | ALU_OP_B);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // PC_L = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // SP++ and enable interrupts
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | IE_TOGGLE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }

                            // --- LDC ---
                            if (opcode == 0xAE) {
                                addStep(A_WRITE | (!usrCarry ? ALU_OP_A_PLUS_1 : ALU_OP_ZERO) | END_INSTRUCTION);
                            }

                            // --- LDE ---
                            if (opcode == 0xAF) {
                                addStep(A_WRITE | (usrEqual ? ALU_OP_A_PLUS_1 : ALU_OP_ZERO) | END_INSTRUCTION);
                            }

                            // --- PUSHF ---
                            if (opcode == 0xB0) {
                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // T = Equal
                                addStep(T_WRITE | (usrEqual ? ALU_OP_A_PLUS_1 : ALU_OP_ZERO));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nT_OUT | nRAM_WRITE);

                                // SP--
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                                // T = Carry
                                addStep(T_WRITE | (!usrCarry ? ALU_OP_A_PLUS_1 : ALU_OP_ZERO));

                                // RAM[SP] = T
                                addStep(nSP_OUT | nT_OUT | nRAM_WRITE | END_INSTRUCTION);
                            }

                            // --- POPF ---
                            if (opcode == 0xB1) {
                                // RAM[SP] - 1
                                addStep(nSP_OUT | nRAM_OUT | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE);

                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                                // RAM[SP] - 1
                                addStep(nSP_OUT | nRAM_OUT | ALU_OP_A_MINUS_B | FLAGS_SYS_UPDATE);

                                // RAM[SP] = A
                                addStep(nSP_OUT | nA_OUT | nALU_OUT | nRAM_WRITE | ALU_OP_A);
                                
                                // A = 0xFF
                                addStep(A_WRITE | ALU_OP_FFFF);

                                if (sysCarry) {        // Equal
                                    if (usrCarry) {    // Carry
                                        // E C
                                        addStep(nA_OUT | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE);
                                    } else {            // No carry
                                        // E -
                                        addStep(ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE);
                                    }
                                } else {                // No equal
                                    if (usrCarry) {    // Carry
                                        // - C
                                        addStep(nA_OUT | ALU_OP_A_PLUS_B_PLUS_1 | FLAGS_USR_UPDATE);
                                    } else {            // No carry
                                        // - -
                                        addStep(ALU_OP_A | FLAGS_USR_UPDATE); // Always end up here ???
                                    }
                                }

                                // A = RAM[SP]
                                addStep(nSP_OUT | nRAM_OUT | A_WRITE | ALU_OP_B);


                                // SP++
                                addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A) | END_INSTRUCTION);
                            }
                        }

                        finishOpcode();
                    }
                }
            }
        }
    }

    fclose(outputFile);

    return 0;
}