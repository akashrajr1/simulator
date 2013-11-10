#include "inc/syscall.h"
#include "inc/cpu.h"
#include "inc/error.h"
#include <unistd.h>
#include <stdio.h>
#define REG(num) cpu->reg[num]
uint32_t syscall_stats[SYSCALL_MAX - SYSCALL_BASE + 1];
uint32_t
perform_syscall(
	syscall_num num,
	cpu_t *cpu)
{
	uint32_t retval = 0;
	switch (num){
	case SYS_exit:
		cpu->exec_finished = 1;
		retval = REG(0);
		break;
	case SYS_read:
		break;
	case SYS_write:
		break;
	case SYS_open:
		break;
	case SYS_close:
		break;
	case SYS_lseek:
		break;
	case SYS_putint:
		// printf("%d\n", REG(0));
		break;
	default:
		error_process(NO_SYSCALL, num);
	}
	syscall_stats[num - SYSCALL_BASE] ++;
	return retval;
}