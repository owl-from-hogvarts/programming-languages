.PHONY: link build clean

ASM=nasm
ASMFLAGS=-felf64 -g
LD=ld

build: link

clean: 
	rm *.o main


lib.o: lib.asm
	$(ASM) $(ASMFLAGS) $< -o $@

dict.o: dict.asm
	$(ASM) $(ASMFLAGS) $< -o $@

main.o: main.asm
	$(ASM) $(ASMFLAGS) $< -o $@


link: lib.o dict.o main.o
	$(LD) *.o -o ./main


