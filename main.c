#include <stdio.h>
#include "inc/elf.h"
#include "inc/misc.h"
#include "inc/cpu.h"
#include "inc/cache.h"
#include "inc/mmu.h"

static void
stack_allocate(mmu_t *mmu){
	uintptr_t va = USER_STACK_TOP - USER_STACK_SIZE;
	while (va < USER_STACK_TOP){
		mmu_get_page(mmu, va, 1);
		va += PGSIZE;
	}
}

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
	mmu_t *mmu = mmu_init(NULL, TLB_EVICT_RAND);
	elf_load(elf, elfhdr, mmu);
	cache_t *cache = cache_init(mmu, NULL, CACHE_EVICT_LRU);
	cpu_t *cpu = cpu_init(cache, mmu);
	stack_allocate(mmu);
	exec_result res = cpu_exec(cpu, USER_STACK_TOP, elfhdr->e_entry);
	mmu_destroy(mmu);
	cache_destroy(cache);
	cpu_destroy(cpu);
	return 0;
}