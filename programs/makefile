.DEFAULT_GOAL := default

default: main.bin main.lbi

clean:
	rm -f *.lbi *.bin *.e74

converter.bin: converter.c
	gcc converter.c -o converter.bin
	customasm $< architecture.asm -f annotated

%.lbi: %.asm *.asm
	customasm $< architecture.asm -o $@ -f logisim8

%.bin: %.asm *.asm
	customasm $< architecture.asm -o $@ -f binary
	
%.txt: %.asm *.asm
	customasm $< architecture.asm -o $@ -f annotated

%.e74: %.asm *.asm converter.bin
	customasm $< architecture.asm -o tmp.txt -f hexstr
	./converter.bin tmp.txt -o $@
	rm tmp.txt