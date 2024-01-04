.DEFAULT_GOAL := main.bin

CC=gcc
CFLAGS=-lncurses

clean:
	rm -f *.out *.bin

run: main.bin
	./main.bin

rom_image.h: ../programs/main.bin
	xxd -i ../programs/main.bin rom_image.h

microcode_image.h: ../microcode/microcode.bin
	xxd -i ../microcode/microcode.bin microcode_image.h

main.c: rom_image.h microcode_image.h

%.bin: %.c
	$(CC) $< -o $@ $(CFLAGS)