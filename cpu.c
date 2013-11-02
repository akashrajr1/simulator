#include <stdlib.h>
#include <string.h>
#include "inc/cpu.h"
#include "inc/error.h"
#include "inc/syscall.h"
cpu_t *
cpu_init(
	cache_t *cache,
	mmu_t *mmu)
{
	cpu_t *cpu = (cpu_t *)malloc(sizeof(cpu_t));
	memset(cpu, 0, sizeof(cpu_t));
	cpu->cache = cache;
	cpu->mmu = mmu;
	return cpu;
}

exec_result
cpu_exec(
	cpu_t *cpu,
	reg_t stack_ptr,
	reg_t prog_cntr)
{
	cpu->reg[SP_NUM] = stack_ptr;
	cpu->reg[PC_NUM] = prog_cntr;
	return EXEC_FINISH;
}


void
cpu_destroy(
	cpu_t *cpu)
{
	free(cpu);
}