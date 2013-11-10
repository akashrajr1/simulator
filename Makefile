# to use GuardMalloc
# export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
CC = clang
CFLAGS = -O0 -fno-omit-frame-pointer -g -Werror -Wall -Wno-unused-variable
OBJS = elf.o main.o instruction.o mmu.o cache.o error.o cpu.o syscall.o unittest.o
SRC = *.c
all: $(OBJS)
	$(CC) $(CFLAGS) -o sim.exe $(OBJS)
clean:
	rm sim.exe *.o