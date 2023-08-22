.DEFAULT_GOAL := microcode.lbi

CC=gcc
CFLAGS=

clean:
	rm -f *.out *.bin

microcode.lbi: main.bin
	./main.bin
	rm main.bin

%.bin: %.c
	$(CC) $< -o $@ $(CFLAGS)