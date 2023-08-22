.DEFAULT_GOAL := all

all: programs microcode

programs: ./programs/*
	make -C ./programs

microcode: ./microcode/*
	make -C ./microcode