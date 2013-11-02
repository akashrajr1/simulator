
CC = clang
CFLAGS = -O0 -g -Werror -Wall -Wno-unused-variable
#SRC = elf.c main.c instruction.c mmu.c cache.c error.c cpu.c syscall.c
SRC = *.c
all: $(SRC)
	$(CC) $(CFLAGS) -o sim.exe $(SRC)
clean:
	rm sim.exe