.DEFAULT_GOAL := default

default: main.bin main.lbi

clean:
	rm -f *.lbi *.bin *.e74

converter.bin: converter.c
	gcc converter.c -o converter.bin

%.lbi: %.asm *.asm
	customasm $< -o $@ -f logisim8

%.bin: %.asm *.asm
	customasm $< -f binary
	customasm $< -f annotated

%.e74: %.asm converter.bin *.asm
	customasm $< -o tmp.txt -f hexstr
	./converter.bin tmp.txt -o $@
	rm tmp.txt