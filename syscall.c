#include "inc/syscall.h"
#include "inc/error.h"
void
syscall(
	syscall_num num,
	cpu_t *cpu)
{
	switch (num){
	case SYS_exit:
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
}