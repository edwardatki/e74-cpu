.DEFAULT_GOAL := main.lbi

clean:
	rm -f *.lbi *.bin

%.lbi: %.asm
	customasm $< -o $@ -f logisim8
	customasm $< -f annotated