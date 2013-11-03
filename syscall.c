#include "inc/syscall.h"
#include "inc/cpu.h"
#include "inc/error.h"
uint32_t syscall_stats[SYSCALL_MAX - SYSCALL_BASE + 1];

uint32_t
syscall(
	syscall_num num,
	cpu_t *cpu)
{
	uint32_t retval = 0;
	switch (num){
	case SYS_exit:
		retval = 0;
		cpu->exec_finished = 1;
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
	default:
		error_process(NO_SYSCALL, num);
	}
	syscall_stats[num - SYSCALL_BASE] ++;
	return retval;
}