# to use GuardMalloc
# export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
CC = gcc
#x86_64-apple-darwin13.0.0-gcc
CFLAGS = -O3 -Werror -Wall -Wno-unused-variable -Wno-strict-aliasing
OBJS = elf.o main.o instruction.o mmu.o cache.o error.o cpu.o syscall.o unittest.o
SRC = *.c
all: $(OBJS)
	$(CC) $(CFLAGS) -o sim.exe $(OBJS)
clean:
	rm sim.exe *.o