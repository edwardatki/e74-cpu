.DEFAULT_GOAL := default

default: programs microcode
all: programs microcode emulator

programs: $(wildcard ./programs/*.asm)
	make -C ./programs

microcode: $(wildcard ./microcode/*.c ./microcode/*.h)
	make -C ./microcode

emulator: $(wildcard ./emulator/*.c ./emulator/*.h)
	make -C ./emulator

run: programs microcode
	make -C ./emulator run
