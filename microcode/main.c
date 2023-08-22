#include <stdio.h>
#include <stdint.h>

#define MICROCODE_STEPS 16

#define nPC_OUT 0x00000001
#define nSP_OUT 0x00000002
#define nBC_OUT 0x00000004
#define nDE_OUT 0x00000008

#define nALU_OUT 0x00000010
#define nAL_OUT 0x00000020
#define nAH_OUT 0x00000040
#define nRAM_OUT 0x00000080

#define PC_H_WRITE 0x00000100
#define PC_L_WRITE 0x00000200
#define SP_H_WRITE 0x00000400
#define SP_L_WRITE 0x00000800
#define B_WRITE 0x00001000
#define C_WRITE 0x00002000
#define D_WRITE 0x00004000
#define E_WRITE 0x00008000
#define IR_WRITE 0x00010000
#define nRAM_WRITE 0x00020000

#define ALU_S0 0x00040000
#define ALU_S1 0x00080000
#define ALU_S2 0x00100000
#define ALU_S3 0x00200000
#define ALU_MODE 0x00400000
#define ALU_CARRY 0x00800000

#define FLAGS_USR_UPDATE 0x01000000
#define FLAGS_SYS_UPDATE 0x02000000

#define nACC_OUT 0x04000000
#define nTMP_OUT 0x08000000

#define ACC_WRITE 0x10000000
#define TMP_WRITE 0x20000000

#define END_INSTRUCTION 0x80000000


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

#define NOP_STEP nPC_OUT | nSP_OUT | nBC_OUT | nDE_OUT | nALU_OUT | nAL_OUT | nAH_OUT | nRAM_OUT | nRAM_WRITE | nACC_OUT | nTMP_OUT

FILE *outputFile;

int opcodeStepCount = 0;

void addStep(uint32_t stepMAsk) {
    static int count = 0;
    
    uint32_t controlWord = ((uint32_t)NOP_STEP) ^ stepMAsk;
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

    for (int usrCarry = 0; usrCarry <= 1; usrCarry++) {
        for (int usrEqual = 0; usrEqual <= 1; usrEqual++) {
            for (int sysCarry = 0; sysCarry <= 1; sysCarry++) {
                for (int sysEqual = 0; sysEqual <= 1; sysEqual++) {
                    for (int opcode = 0; opcode <= 0xff; opcode++) {
                        // IR = RAM[PC++]
                        addStep(nPC_OUT | nRAM_OUT | IR_WRITE | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                        addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        
                        // --- MOV A, i8 ---
                        if (opcode == 0x01) {
                            // A = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | ACC_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }
                        
                        // --- MOV B, i8 ---
                        if (opcode == 0x02) {
                            // B = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }
                        
                        // --- MOV C, i8 ---
                        if (opcode == 0x03) {
                            // C = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }
                        
                        // --- MOV D, i8 ---
                        if (opcode == 0x04) {
                            // D = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }
                        
                        // --- MOV E, i8 ---
                        if (opcode == 0x05) {
                            // E = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
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
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
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
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
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
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- ADD i8 ---
                        if (opcode == 0x09) {        
                            // A = A plus RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- SUB i8 ---
                        if (opcode == 0x0A) {        
                            // A = A sub RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- AND i8 ---
                        if (opcode == 0x0B) {        
                            // A = A and RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_AND_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- NAND i8 ---
                        // --- OR i8 ---
                        // --- NOR i8 ---
                        // --- XOR i8 ---

                        // --- ADD B ---
                        if (opcode == 0x10) {
                            // TMP = B
                            addStep(nBC_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // A minus TMP
                            addStep(nTMP_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE);
                        }

                        // --- SUB B ---
                        if (opcode == 0x11) {
                            // TMP = B
                            addStep(nBC_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // A minus TMP
                            addStep(nTMP_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_MINUS_B | FLAGS_USR_UPDATE);
                        }
                        // --- AND B ---
                        // --- NAND B ---
                        // --- OR B ---
                        // --- NOR B ---
                        // --- XOR B ---

                        // --- ADD C ---
                        // --- SUB C ---
                        // --- AND C ---
                        // --- NAND C ---
                        // --- OR C ---
                        // --- NOR C ---
                        // --- XOR C ---

                        // --- ADD B ---
                        // --- SUB D ---
                        // --- AND D ---
                        // --- NAND D ---
                        // --- OR D ---
                        // --- NOR D ---
                        // --- XOR D ---

                        // --- ADD E ---
                        // --- SUB E ---
                        // --- AND E ---
                        // --- NAND E ---
                        // --- OR E ---
                        // --- NOR E ---
                        // --- XOR E ---
                        
                        // --- ADD [BC] ---
                        if (opcode == 0x2C) {
                            // A = A plus RAM[BC]
                            addStep(nBC_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_A_PLUS_B | FLAGS_USR_UPDATE);
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
                            addStep(nSP_OUT | nRAM_WRITE | nACC_OUT | nALU_OUT | ALU_OP_A);
                        }

                        // --- PUSH B ---
                        if (opcode == 0x3C) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = B
                            addStep(nBC_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
                        }

                        // --- PUSH C ---
                        if (opcode == 0x3D) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = C
                            addStep(nBC_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
                        }

                        // --- PUSH D ---
                        if (opcode == 0x3E) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = D
                            addStep(nDE_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
                        }

                        // --- PUSH E ---
                        if (opcode == 0x3F) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = E
                            addStep(nDE_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
                        }

                        // --- POP A ---
                        if (opcode == 0x40) {
                            // A = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | ACC_WRITE | ALU_OP_B);

                            // SP++
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- POP B ---
                        if (opcode == 0x41) {
                            // B = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | B_WRITE | ALU_OP_B);

                            // SP++
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- POP C ---
                        if (opcode == 0x42) {
                            // B = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | C_WRITE | ALU_OP_B);

                            // SP++
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- POP D ---
                        if (opcode == 0x43) {
                            // D = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | D_WRITE | ALU_OP_B);

                            // SP++
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- POP E ---
                        if (opcode == 0x44) {
                            // E = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | E_WRITE | ALU_OP_B);

                            // SP++
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- PUSH BC ---
                        if (opcode == 0x45) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = C
                            addStep(nBC_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);

                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = B
                            addStep(nBC_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
                        }

                        // --- PUSH DE ---
                        if (opcode == 0x46) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = E
                            addStep(nDE_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);

                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = D
                            addStep(nDE_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);
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
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
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
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- INC BC ---
                        if (opcode == 0x49) {
                            // BC++
                            addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nBC_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- INC DE ---
                        if (opcode == 0x4A) {
                            // DE++
                            addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nDE_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- DEC BC ---
                        if (opcode == 0x4B) {
                            // BC--
                            addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nBC_OUT | nAH_OUT | B_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));
                        }

                        // --- DEC DE ---
                        if (opcode == 0x4C) {
                            // DE--
                            addStep(nBC_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nBC_OUT | nAH_OUT | D_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));
                        }

                        // --- JMP i16 ---
                        if (opcode == 0x4D) {
                            // TMP = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | TMP_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            // PC_L = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                            // PC_H = TMP
                            addStep(nTMP_OUT | PC_H_WRITE | ALU_OP_B);
                        }

                        // --- JMP BC ---
                        // --- JMP DE ---
                        // --- JMP [BC] ---
                        // --- JMP [DE] ---

                        // --- JC i16 ---
                        if (opcode == 0x52) {
                            // TMP = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | TMP_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            if (!usrCarry) {
                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = TMP
                                addStep(nTMP_OUT | PC_H_WRITE | ALU_OP_B);
                            } else {
                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                            }
                        }

                        // --- JC BC ---
                        // --- JC DE ---
                        // --- JC [BC] ---
                        // --- JC [DE] ---

                        // --- JNC i16 ---
                        if (opcode == 0x57) {
                            // TMP = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | TMP_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            if (usrCarry) {
                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = TMP
                                addStep(nTMP_OUT | PC_H_WRITE | ALU_OP_B);
                            } else {
                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                            }
                        }

                        // --- JNC BC ---
                        // --- JNC DE ---
                        // --- JNC [BC] ---
                        // --- JNC [DE] ---

                        // --- JE i16 ---
                        if (opcode == 0x5C) {
                            // TMP = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | TMP_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            if (usrEqual) {
                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = TMP
                                addStep(nTMP_OUT | PC_H_WRITE | ALU_OP_B);
                            } else {
                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                            }
                        }

                        // --- JE BC ---
                        // --- JE DE ---
                        // --- JE [BC] ---
                        // --- JE [DE] ---

                        // --- JNE i16 ---
                        if (opcode == 0x61) {
                            // TMP = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | TMP_WRITE | ALU_OP_B);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            if (!usrEqual) {
                                // PC_L = RAM[PC]
                                addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                                // PC_H = TMP
                                addStep(nTMP_OUT | PC_H_WRITE | ALU_OP_B);
                            } else {
                                // PC++
                                addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                                addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                            }
                        }

                        // --- JNE BC ---
                        // --- JNE DE ---
                        // --- JNE [BC] ---
                        // --- JNE [DE] ---

                        // --- CMP i8 ---
                        if (opcode == 0x66) {
                            // A minus RAM[PC] minus 1
                            addStep(nPC_OUT | nRAM_OUT | nACC_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE);

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- CMP B ---
                        if (opcode == 0x67) {
                            // TMP = B
                            addStep(nBC_OUT | nAH_OUT | TMP_WRITE | ALU_OP_A);

                            // A minus TMP minus 1
                            addStep(nTMP_OUT | nACC_OUT | ALU_OP_A_MINUS_B_MINUS_1 | FLAGS_USR_UPDATE);
                        }
                        
                        // --- CMP C ---
                        // --- CMP D ---
                        // --- CMP E ---
                        // --- CMP [BC] ---
                        // --- CMP [DE] ---

                        // --- MOV A, [BC] ---
                        if (opcode == 0x6D) {
                            // A = RAM[BC]
                            addStep(nBC_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_B | FLAGS_USR_UPDATE);
                        }

                        // --- MOV D, [BC] ---
                        // --- MOV E, [BC] ---

                        // --- MOV A, [DE] ---
                        if (opcode == 0x70) {
                            // A = RAM[DE]
                            addStep(nDE_OUT | nRAM_OUT | nACC_OUT | ACC_WRITE | ALU_OP_B | FLAGS_USR_UPDATE);
                        }

                        // --- MOV B, [DE] ---
                        // --- MOV C, [DE] ---

                        // --- MOV [BC], A ---
                        if (opcode == 0x73) {
                            // RAM[BC] = A
                            addStep(nBC_OUT | nRAM_WRITE | nACC_OUT | nALU_OUT | ALU_OP_B);
                        }

                        // --- MOV [BC], D ---
                        // --- MOV [BC], E ---

                        // --- MOV [DE], A ---
                        if (opcode == 0x76) {
                            // RAM[DE] = A
                            addStep(nDE_OUT | nRAM_WRITE | nACC_OUT | nALU_OUT | ALU_OP_A);
                        }

                        // --- MOV [DE], B ---
                        // --- MOV [DE], C ---

                        // --- MOV BC, [DE] ---
                        // --- MOV DE, [BC] ---
                        
                        // --- MOV [BC], DE ---
                        // --- MOV [DE], BC ---

                        // --- CALL i16 ---
                        if (opcode == 0x7D) {
                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // TMP = PC_L plus 1
                            addStep(nPC_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);

                            // TMP = TMP plus 1
                            addStep(nTMP_OUT | TMP_WRITE | ALU_OP_A_PLUS_B_PLUS_1 | (!sysCarry ? 0 : FLAGS_SYS_UPDATE));

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);

                            // TMP = PC_H
                            addStep(nPC_OUT | nAH_OUT | TMP_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            // SP--
                            addStep(nSP_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A_MINUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (sysCarry ? ALU_OP_A_MINUS_1 : ALU_OP_A));

                            // RAM[SP] = TMP
                            addStep(nSP_OUT | nRAM_WRITE | nTMP_OUT);

                            // ---

                            // PC++
                            addStep(nPC_OUT | nAL_OUT | PC_L_WRITE | ALU_OP_A_PLUS_1 | FLAGS_SYS_UPDATE);
                            addStep(nPC_OUT | nAH_OUT | PC_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));

                            // PC_L = RAM[PC]
                            addStep(nPC_OUT | nRAM_OUT | PC_L_WRITE | ALU_OP_B);

                            // PC_H = RAM[SP]
                            addStep(nSP_OUT | nRAM_OUT | PC_H_WRITE | ALU_OP_B);
                        }

                        // --- CALL BC ---a
                        // --- CALL DE ---
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
                            addStep(nSP_OUT | nAH_OUT | SP_H_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // --- MOV B, A ---
                        // --- MOV B, C ---
                        // --- MOV B, D ---
                        // --- MOV B, E ---

                        // --- MOV C, A ---
                        // --- MOV C, B ---
                        // --- MOV C, D ---
                        // --- MOV C, E ---

                        // --- MOV D, A ---
                        // --- MOV D, B ---
                        // --- MOV D, C ---
                        // --- MOV D, E ---

                        // --- MOV E, A ---
                        // --- MOV E, B ---
                        // --- MOV E, C ---
                        // --- MOV E, D ---

                        // --- MOV BC, DE ---
                        if (opcode == 0x93) {
                            addStep(nDE_OUT | nAH_OUT | B_WRITE | ALU_OP_A);
                            addStep(nDE_OUT | nAL_OUT | C_WRITE | ALU_OP_A);
                        }

                        // --- MOV BC, SP ---
                        if (opcode == 0x94) {
                            addStep(nSP_OUT | nAH_OUT | B_WRITE | ALU_OP_A);
                            addStep(nSP_OUT | nAL_OUT | C_WRITE | ALU_OP_A);
                        }


                        // --- MOV DE, BC ---
                        if (opcode == 0x95) {
                            addStep(nBC_OUT | nAH_OUT | D_WRITE | ALU_OP_A);
                            addStep(nBC_OUT | nAL_OUT | E_WRITE | ALU_OP_A);
                        }

                        // --- MOV DE, SP ---
                        if (opcode == 0x96) {
                            addStep(nSP_OUT | nAH_OUT | D_WRITE | ALU_OP_A);
                            addStep(nSP_OUT | nAL_OUT | E_WRITE | ALU_OP_A);
                        }

                        // --- MOV SP, BC ---
                        if (opcode == 0x96) {
                            addStep(nBC_OUT | nAH_OUT | SP_H_WRITE | ALU_OP_A);
                            addStep(nBC_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A);
                        }

                        // --- MOV SP, DE ---
                        if (opcode == 0x96) {
                            addStep(nDE_OUT | nAH_OUT | SP_H_WRITE | ALU_OP_A);
                            addStep(nDE_OUT | nAL_OUT | SP_L_WRITE | ALU_OP_A);
                        }

                        // --- ROL ---
                        if (opcode == 0x99) {        
                            // A = A plus A
                            addStep(nACC_OUT | ACC_WRITE | ALU_OP_A_PLUS_A | FLAGS_SYS_UPDATE);
                            addStep(nACC_OUT | ACC_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        }

                        // // --- MOV BC, SP+i8 ---
                        // if (opcode == 0x9A) {
                        //     // TMP = SP_L
                        //     addStep(nSP_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                        //     // C = TMP plus RAM[PC]
                        //     addStep(nPC_OUT | nRAM_OUT | C_WRITE | nTMP_OUT | ALU_OP_A_PLUS_B | FLAGS_SYS_UPDATE);
                            
                        //     // B = SP_H
                        //     addStep(nSP_OUT | nAH_OUT | B_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        // }

                        // // --- MOV DE, SP+i8 ---
                        // if (opcode == 0x9B) {
                        //     // TMP = SP_L
                        //     addStep(nSP_OUT | nAL_OUT | TMP_WRITE | ALU_OP_A);

                        //     // E = TMP plus RAM[PC]
                        //     addStep(nPC_OUT | nRAM_OUT | E_WRITE | nTMP_OUT | ALU_OP_A_PLUS_B | FLAGS_SYS_UPDATE);
                            
                        //     // D = SP_H
                        //     addStep(nSP_OUT | nAH_OUT | D_WRITE | (!sysCarry ? ALU_OP_A_PLUS_1 : ALU_OP_A));
                        // }

                        // --- INC B ---
                        if (opcode == 0x9C) {
                            addStep(nBC_OUT | nAH_OUT | B_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- INC C ---
                        if (opcode == 0x9D) {
                            addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- INC D ---
                        if (opcode == 0x9E) {
                            addStep(nDE_OUT | nAH_OUT | D_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- INC E ---
                        if (opcode == 0x9F) {
                            addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_PLUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- DEC B ---
                        if (opcode == 0xA0) {
                            addStep(nBC_OUT | nAH_OUT | B_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- DEC C ---
                        if (opcode == 0xA1) {
                            addStep(nBC_OUT | nAL_OUT | C_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- DEC D ---
                        if (opcode == 0xA2) {
                            addStep(nDE_OUT | nAH_OUT | D_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE);
                        }

                        // --- DEC E ---
                        if (opcode == 0xA3) {
                            addStep(nDE_OUT | nAL_OUT | E_WRITE | ALU_OP_A_MINUS_1 | FLAGS_USR_UPDATE);
                        }

                        // -- MOV A, B ---
                        if (opcode == 0xA4) { 
                            addStep(nBC_OUT | nAH_OUT | ACC_WRITE | ALU_OP_A);
                        }

                        // -- MOV A, C ---
                        if (opcode == 0xA5) {
                            addStep(nBC_OUT | nAL_OUT | ACC_WRITE | ALU_OP_A);
                        }

                        // -- MOV A, D ---
                        if (opcode == 0xA6) {
                            addStep(nDE_OUT | nAH_OUT | ACC_WRITE | ALU_OP_A);
                        }

                        // -- MOV A, E ---
                        if (opcode == 0xA7) {
                            addStep(nDE_OUT | nAL_OUT | ACC_WRITE | ALU_OP_A);
                        }


                        addStep(END_INSTRUCTION);
                        finishOpcode();
                    }
                }
            }
        }
    }

    fclose(outputFile);

    return 0;
}