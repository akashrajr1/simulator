#include <stdio.h>
#include "inc/elf.h"
#include "inc/misc.h"
#include "inc/cpu.h"
#include "inc/cache.h"
#include "inc/mmu.h"
#include "inc/instruction.h"
#include "inc/unittest.h"

static void
stack_allocate(mmu_t *mmu){
	uint32_t va = USER_STACK_TOP - USER_STACK_SIZE;
	while (va < USER_STACK_TOP){
		mmu_get_page(mmu, va, 1);
		va += PGSIZE;
	}
}

int main(int argc, char *argv[])
{
	// run_unit_tests();

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
	printf(res == EXEC_ABORT? "execution aborted.\n": "execution finished.\n");
	mmu_destroy(mmu);
	cache_destroy(cache);
	cpu_destroy(cpu);
	return 0;
}