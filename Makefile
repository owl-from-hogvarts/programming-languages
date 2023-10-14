.PHONY: link build clean test

ASM=nasm
ASMFLAGS=-felf64 -g
LD=ld

SRC_DIR := src
BIN_DIR := target

EXEC_NAME := main
OBJS := $(addprefix $(BIN_DIR)/, lib.o dict.o main.o)

build: $(BIN_DIR) link

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

test:
	npm run test

clean: 
	rm -rd $(BIN_DIR)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	$(ASM) -I$(SRC_DIR)/ $(ASMFLAGS) $< -o $@

link: $(OBJS)
	$(LD) $(OBJS) -o $(BIN_DIR)/main

