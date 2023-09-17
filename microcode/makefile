.DEFAULT_GOAL := microcode

CC=gcc
CFLAGS=

clean:
	rm -f *.out *.bin

microcode: main.bin
	./main.bin
	rm main.bin

	srec_cat -o rom_a.bin -binary microcode.bin -binary -split 4 0
	srec_cat -o rom_b.bin -binary microcode.bin -binary -split 4 1
	srec_cat -o rom_c.bin -binary microcode.bin -binary -split 4 2
	srec_cat -o rom_d.bin -binary microcode.bin -binary -split 4 3

%.bin: %.c
	$(CC) $< -o $@ $(CFLAGS)