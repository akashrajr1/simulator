
CC = clang
CFLAGS = -O0 -g -Werror -Wall -Wno-unused-variable
OBJS = elf.o main.o instruction.o mmu.o cache.o error.o cpu.o syscall.o
SRC = *.c
all: $(OBJS)
	$(CC) $(CFLAGS) -o sim.exe $(OBJS)
clean:
	rm sim.exe *.o