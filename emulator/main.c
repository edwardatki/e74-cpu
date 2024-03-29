#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

#include "rom_image.h"
#include "microcode_image.h"

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

// 64KB of microcode
uint32_t* microcode = (uint32_t*)___microcode_microcode_bin;

// 32KB of ROM
uint8_t* rom = ___programs_main_bin;

// 32KB of RAM, although for simplicities sake the array is 64KB and we only use half
uint8_t ram[0x10000];

uint8_t pc_h_reg = 0;
uint8_t pc_l_reg = 0;
uint8_t sp_h_reg = 0;
uint8_t sp_l_reg = 0;
uint8_t b_reg = 0;
uint8_t c_reg = 0;
uint8_t d_reg = 0;
uint8_t e_reg = 0;
uint8_t a_reg = 0;
uint8_t t_reg = 0;

uint8_t user_carry_flag = 0;
uint8_t user_equal_flag = 0;
uint8_t sys_carry_flag = 0;
uint8_t interrupt_flag = 0;
uint8_t interrupt_enable = 0;

uint8_t instr_reg = 0;
uint8_t micro_counter = 0;

uint8_t waiting_char = 0;
uint8_t char_available = 0;

int use_ncurses = 1;

uint8_t read_memory(uint16_t address) {
    if (address == 0x7000) {                        // TERMINAL
        uint8_t data = waiting_char;
        waiting_char = 0;
        char_available = 0;
        return data;
    } else if (address == 0x7001) {
        return char_available;
    }

    if (address < 0x8000) return rom[address];      // ROM
    else return ram[address];                       // RAM
}

void write_memory(uint16_t address, uint8_t data) {
    if (address == 0x7000) {                        // TERMINAL
        if (use_ncurses) printw("%c", data);
        else printf("%c", data);
    }      

    if (address < 0x8000);                          // ROM
    else ram[address] = data;                       // RAM
}

uint8_t emulate_alu(uint8_t a, uint8_t b, uint32_t control_word) {
    // 6 mode bits are M Cn S3 S2 S1 S0
    uint8_t mode = 0;
    if (control_word & ALU_MODE) mode |= 0b100000;
    if (control_word & ALU_CARRY) mode |= 0b010000;
    if (control_word & ALU_S3) mode |= 0b001000;
    if (control_word & ALU_S2) mode |= 0b000100;
    if (control_word & ALU_S1) mode |= 0b000010;
    if (control_word & ALU_S0) mode |= 0b000001;

    // Wipe carry bit if a logic operation
    if (mode & 0b010000) mode ^=  0xb010000;

    uint8_t carry_out = 0;

    int16_t temp_result;
    switch(mode) {
        // Arithmetic
        case 0x00:
            temp_result = a + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x01:
            temp_result = (a | b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x02:
            temp_result = (a | ~b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x03:
            temp_result = 0;
            carry_out = 1;
            break;
        case 0x04:
            temp_result = a + (a & ~b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x05:
            temp_result = (a | b) + (a & ~b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x06:
            temp_result = a - b;
            carry_out = !(temp_result >= 0);
            break;
        case 0x07:
            temp_result = a & ~b;
            carry_out = !(temp_result >= 0);
            break;
        case 0x08:
            temp_result = a + (a & b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x09:
            temp_result = a + b + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x0A:
            temp_result = (a | ~b) + (a & b) + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x0B:
            temp_result = a & b;
            carry_out = !(temp_result >= 0);
            break;
        case 0x0C:
            temp_result = a + a + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x0D:
            temp_result = (a | b) + a + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x0E:
            temp_result = (a | ~b) + a + 1;
            carry_out = !(temp_result > 255);
            break;
        case 0x0F:
            temp_result = a;
            carry_out = !(temp_result >= 0);
            break;
        case 0x10:
            temp_result = a;
            carry_out = !(temp_result > 255);
            break;
        case 0x11:
            temp_result = a | b;
            carry_out = !(temp_result > 255);
            break;
        case 0x12:
            temp_result = a | ~b;
            carry_out = !(temp_result > 255);
            break;
        case 0x13:
            temp_result = 0xff;
            carry_out = !(temp_result >= 0);
            break;
        case 0x14:
            temp_result = a + (a & ~b);
            carry_out = !(temp_result > 255);
            break;
        case 0x15:
            temp_result = (a | b) + (a & ~b);
            carry_out = !(temp_result > 255);
            break;
        case 0x16:
            temp_result = a - b - 1;
            carry_out = !(temp_result >= 0);
            break;
        case 0x17:
            temp_result = (a & ~b) - 1;
            carry_out = !(temp_result >= 0);
            break;
        case 0x18:
            temp_result = a + (a & b);
            carry_out = !(temp_result > 255);
            break;
        case 0x19:
            temp_result = a + b;
            carry_out = !(temp_result > 255);
            break;
        case 0x1A:
            temp_result = (a | ~b) + (a & b);
            carry_out = !(temp_result > 255);
            break;
        case 0x1B:
            temp_result = (a & b) - 1;
            carry_out = !(temp_result >= 0);
            break;
        case 0x1C:
            temp_result = a + a;
            carry_out = !(temp_result > 255);
            break;
        case 0x1D:
            temp_result = (a | b) + a;
            carry_out = !(temp_result > 255);
            break;
        case 0x1E:
            temp_result =  (a | ~b) + a;
            carry_out = !(temp_result > 255);
            break;
        case 0x1F:
            temp_result = a - 1;
            carry_out = !(temp_result >= 0);
            break;
        
        // Logic
        case 0x20:
            temp_result = ~a;
            break;
        case 0x21:
            temp_result = ~a  | ~b;
            break;
        case 0x22:
            temp_result = ~a & b;
            break;
        case 0x23:
            temp_result = 0;
            break;
        case 0x24:
            temp_result = ~(a & b);
            break;
        case 0x25:
            temp_result = ~b;
            break;
        case 0x26:
            temp_result = a ^ b;
            break;
        case 0x27:
            temp_result = a & ~b;
            break;
        case 0x28:
            temp_result = ~a | b;
            break;
        case 0x29:
            temp_result = ~(a ^ b);
            break;
        case 0x2A:
            temp_result = b;
            break;
        case 0x2B:
            temp_result = a & b;
            break;
        case 0x2C:
            temp_result = 1;
            break;
        case 0x2D:
            temp_result = a | ~b;
            break;
        case 0x2E:
            temp_result = a | b;
            break;
        case 0x2F:
            temp_result = a;
            break;
    }

    uint8_t result = (uint8_t)temp_result;
    uint8_t equal_out = result == 0xff;

    if (control_word & FLAGS_USR_UPDATE) {
        user_carry_flag = carry_out;
        user_equal_flag = equal_out;
    }

    if (control_word & FLAGS_SYS_UPDATE) {
        sys_carry_flag = carry_out;
    }

    return (uint8_t) result;
}

int main(int argc, char **argv) {
    char* input_filename = NULL;
    char* output_filename = NULL;
    
    // Process arguments
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: ./emulator.bin [options] [file]\n");
            printf(" [file] optionally specify a binary file to be loaded to 0x8100 and executed\n");
            printf("Options:\n");
            printf(" -h   print help message\n");
            printf(" -n   don't use ncurses, useful for automated tests\n");
            return EXIT_SUCCESS;
            i += 1;
        } else if (strcmp(argv[i], "-n") == 0) {
            use_ncurses = 0;
            i += 1;
        } else {
            if (input_filename != NULL) {
                printf("ERROR: more than one input file supplied\n");
                return EXIT_FAILURE;
            }
            input_filename = argv[i];
            i += 1;
        }
    }

    // Fill RAM with random data
    for (int i = 0x8000; i < 0x10000; i++) {
        ram[i] = rand();
    }

    // Input files loaded to 0x8100
    if (input_filename != NULL) {
        FILE* f0 = fopen(input_filename, "rb");
        if (!f0) {
            printf("ERROR: couldn't open \"%s\"\n", input_filename);
            return EXIT_FAILURE;
        }
        if (fseek(f0, sizeof(uint8_t) * 0x8100, SEEK_SET) != 0) {
            printf("ERROR: couldn't seek to 0x8100 in \"%s\"\n", input_filename);
            return EXIT_FAILURE;
        }
        int input_size = fread(&ram[0x8100], sizeof(uint8_t), (0x10000-0x8100), f0);
        fclose(f0);

        pc_h_reg = 0x81;
        pc_l_reg = 0x00;
        sp_h_reg = 0xff;
        sp_l_reg = 0xff;
        instr_reg = ram[0x8100];
    }

    printf("*** Emulator start ***\n");

    if (use_ncurses) {
        initscr();
        noecho();
        raw();
        timeout(0);
        scrollok(stdscr, TRUE);
        curs_set(0);
        move(1, 0);
    }

    while (1) {
        uint8_t c;
        if (use_ncurses) c = getch();
        else c = 0;
        if (c == 27) break;

        if (waiting_char == 0) {
            if (c != 0xff) {
                waiting_char = c;
                char_available = 1;
            } else {
                waiting_char = 0;
                char_available = 0;
            }
        }

        uint16_t control_address = (instr_reg << 8) | (interrupt_flag << 7) | (sys_carry_flag << 6) | (user_equal_flag << 5) | (user_carry_flag << 4) | micro_counter;
        uint32_t control_word = microcode[control_address];

        if (control_word & IE_TOGGLE) interrupt_enable = !interrupt_enable;
        if (!interrupt_enable) interrupt_flag = 0;

        // Determine value on address bus
        uint16_t address_bus = 0;
        if (~control_word & nPC_OUT) address_bus = (pc_h_reg << 8) | pc_l_reg;
        if (~control_word & nSP_OUT) address_bus = (sp_h_reg << 8) | sp_l_reg;
        if (~control_word & nBC_OUT) address_bus = (b_reg << 8) | c_reg;
        if (~control_word & nDE_OUT) address_bus = (d_reg << 8) | e_reg;

        // Determine value on left bus
        uint8_t left_bus = 0;
        if (~control_word & nAH_OUT) left_bus = (address_bus >> 8) & 0xff;
        if (~control_word & nAL_OUT) left_bus = address_bus & 0xff;
        if (~control_word & nA_OUT) left_bus = a_reg;

        // Determine value on right bus
        uint8_t right_bus = 0;
        if (~control_word & nRAM_OUT) right_bus = read_memory(address_bus);
        if (~control_word & nT_OUT) right_bus = t_reg;

        // Determine value on alu bus
        uint8_t alu_bus = 0;
        alu_bus = emulate_alu(left_bus, right_bus, control_word);

        // Put alu bus value onto right bus, this must happen after alu bus value computed
        if (~control_word & nALU_OUT) right_bus = alu_bus;

        if (use_ncurses) {
            int y, x;
            getyx(stdscr, y, x);
            move(0, 0);
            printw("%04x %08x", control_address, control_word);
            printw(" A: %02x T: %02x PC: %02x%02x SP: %02x%02x BC: %02x%02x DE: %02x%02x", a_reg, t_reg, pc_h_reg, pc_l_reg, sp_h_reg, sp_l_reg, b_reg, c_reg, d_reg, e_reg);
            printw(" mem[%04x]: %02x left: %02x right: %02x alu: %02x ie:%d c:%02x\n", address_bus, read_memory(address_bus), left_bus, right_bus, alu_bus, interrupt_enable, waiting_char);
            move(y, x);
            refresh();
        }

        // Write new values to registers
        if (control_word & PC_H_WRITE) pc_h_reg = alu_bus;
        if (control_word & PC_L_WRITE) pc_l_reg = alu_bus;
        if (control_word & SP_H_WRITE) sp_h_reg = alu_bus;
        if (control_word & SP_L_WRITE) sp_l_reg = alu_bus;
        if (control_word & B_WRITE) b_reg = alu_bus;
        if (control_word & C_WRITE) c_reg = alu_bus;
        if (control_word & D_WRITE) d_reg = alu_bus;
        if (control_word & E_WRITE) e_reg = alu_bus;
        if (control_word & A_WRITE) a_reg = alu_bus;
        if (control_word & T_WRITE) t_reg = alu_bus;

        if (~control_word & nRAM_WRITE) write_memory(address_bus, right_bus);

        static int call_depth = 0;
        if (control_word & IR_WRITE) {
            instr_reg = right_bus;
            if ((instr_reg >= 0x7D) && (instr_reg <= 0x81)) { // If call
                call_depth++;
            } else if (instr_reg == 0x82 && (pc_h_reg != 0x00)) { // If return and not from interrupt handler
                if (call_depth <= 0) {
                    if (use_ncurses) endwin();
                    printf("*** Emulator exit with %d ***\n", a_reg);
                    return a_reg;
                }
                call_depth--;
            }
        }

        micro_counter += 1;

        // End of instruction
        if ((micro_counter > 15) | (control_word & END_INSTRUCTION)) {
            micro_counter = 0;

            if (interrupt_enable && (waiting_char != 0)) {
                interrupt_flag = 1;
            }
        }

        // usleep(1000000); // 1Hz
        // usleep(100000); // 10Hz
        // usleep(10000); // 100Hz
        // usleep(1000); // 1KHz
        // usleep(100); // 10KHz
        // usleep(10); // 100KHz
        // usleep(1); // 1MHz
    }

    if (use_ncurses) endwin();
    printf("*** Emulator crash ***");
    return EXIT_FAILURE;
}