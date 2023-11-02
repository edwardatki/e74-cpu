#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

unsigned int start_address = 0x8000; // Only bother generating commands for data in RAM

int main(int argc, char **argv) {
    char* input_filename = NULL;
    char* output_filename = NULL;
    
    // Process arguments
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-o") == 0) {
            if (argc <= (i+1)) {
                printf("ERROR: flag given with no value\n");
                return EXIT_FAILURE;
            }
            output_filename = argv[i+1];
            i += 2;
        } else {
            if (input_filename != NULL) {
                printf("ERROR: more than one input file supplied\n");
                return EXIT_FAILURE;
            }
            input_filename = argv[i];
            i += 1;
        }
    }
    
    if (input_filename == NULL) {
        printf("ERROR: no input file supplied\n");
        return EXIT_FAILURE;
    }
    FILE *input_file;
    input_file = fopen(input_filename, "r");

    if (output_filename == NULL) output_filename = "out.txt";
    FILE *output_file;
    output_file = fopen(output_filename, "w");

    unsigned char data;
    unsigned int address = 0;
    unsigned int entry_address;
    int reached_actual_data = 0;
    fprintf(output_file, "P 0000 0000\n");
    while (fscanf(input_file, "%02x", (unsigned int*)&data) == 1) {
        if (!reached_actual_data) {  // Skip over blank data at start of image
            if (data != 0x00) {
                entry_address = address;
                reached_actual_data = 1;
            }
        }
        // if (reached_actual_data && (address >= start_address)) fprintf(output_file, "w%04x%02x\n", address, data);
        if (reached_actual_data && (address >= start_address)) fprintf(output_file, "%c", data);
        address++;
    }

    // fprintf(output_file, "x%04x\n", entry_address);
    
    fseek(output_file, 0, SEEK_SET);
    fprintf(output_file, "P %04x %04x\n", entry_address, address-entry_address);

    fclose(input_file);
    fclose(output_file);

    return EXIT_SUCCESS;
}