#include <stdio.h>
#include "inc/elf.h"
#include "inc/misc.h"
#include "inc/instruction.h"

int main(int argc, char *argv[])
{
	// static_assert(sizeof(general_inst) == 4);
	// static_assert(sizeof(imm9_inst) == 4);
	// static_assert(sizeof(imm14_inst) == 4);
	// static_assert(sizeof(cond_branch_inst) == 4);
	// static_assert(sizeof(soft_trap_inst) == 4);
	// static_assert(sizeof(status_reg) == 4);
	printf("Hello!\n");
	read_elf(argv[1]);
	return 0;
}