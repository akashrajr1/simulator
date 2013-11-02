#ifndef _UC32SIM_SYSCALL_H
#define _UC32SIM_SYSCALL_H

#include "cpu.h"

#define O_RDONLY	0
#define O_WRONLY	1
#define O_RDWR		2
#define O_APPEND	8
#define O_CREAT	  512
#define O_TRUNC	 1024
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define SYSCALL_BASE	0x900000

typedef enum {
	SYS_exit  = 0x900001,
	SYS_read  = 0x900003,
	SYS_write = 0x900004,
	SYS_open  = 0x900005,
	SYS_close = 0x900006,
	SYS_lseek = 0x900013
} syscall_num;

void syscall(syscall_num num, cpu_t *cpu);

#endif