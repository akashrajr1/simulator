# to use GuardMalloc
# set DYLD_INSERT_LIBRARIES to /usr/lib/libgmalloc.dylib
CC = clang
CFLAGS = -O0 -fno-omit-frame-pointer -g -Werror -Wall
OBJS = elf.o main.o instruction.o mmu.o cache.o error.o cpu.o syscall.o unittest.o
SRC = *.c
all: $(OBJS)
	$(CC) $(CFLAGS) -o sim.exe $(OBJS)
clean:
	rm sim.exe *.o