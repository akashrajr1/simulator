
CC = clang
CFLAGS = -O0 -g -Werror -Wall -Wno-unused-variable
SRC = elf.c main.c instruction.c mmu.c
all: $(SRC)
	$(CC) $(CFLAGS) -o sim $(SRC)
clean:
	rm *.o