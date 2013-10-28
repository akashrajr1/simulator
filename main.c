#include <stdio.h>
#include "inc/elf.h"
#include "inc/misc.h"
#include "inc/instruction.h"
#include "inc/mmu.h"
int main(int argc, char *argv[])
{
	// static_assert(sizeof(general_inst) == 4);
	// static_assert(sizeof(imm9_inst) == 4);
	// static_assert(sizeof(imm14_inst) == 4);
	// static_assert(sizeof(cond_branch_inst) == 4);
	// static_assert(sizeof(soft_trap_inst) == 4);
	// static_assert(sizeof(status_reg) == 4);
	printf("Hello! This is Unicore32 Simulator\n");
	Elf *elfhdr;
	FILE *elf = elf_check(argv[1], &elfhdr);
	if (!elf)
		return 0;
	mmu_t *mmu = mmu_init();
	elf_load(elf, elfhdr, mmu);

	mmu_destroy(mmu);
	return 0;
}