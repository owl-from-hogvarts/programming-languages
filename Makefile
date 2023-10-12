.PHONY: link build clean

ASM=nasm
ASMFLAGS=-felf64 -g
LD=ld

clean: 
	rm *.o main

build: link

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

link:
	$(LD) *.o -o ./main


