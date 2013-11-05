#include <stdlib.h>
#include <string.h>
#include "inc/cpu.h"
#include "inc/error.h"
#include "inc/syscall.h"
#include "inc/instruction.h"
#include "inc/cache.h"
jmp_buf cpu_exec_buf;

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
	if (setjmp(cpu_exec_buf)){
		// caused by non-local jump from error_process
		return EXEC_ABORT;
	}
	/* ... 
	 * execute
	 */
	while (cpu->exec_finished == 0){
		uint32_t cycles;
		general_inst inst;
		*(uint32_t*)&inst = cache_load(cpu->cache, cpu->reg[PC_NUM], MEM_INST, &cycles);
		cpu->reg[PC_NUM] += 4;
		cycles += inst_dispatch(cpu, inst);
		cpu->stats.ncycle += cycles;
		cpu->stats.ninst ++;
	}
	return EXEC_FINISH;
}


void
cpu_destroy(
	cpu_t *cpu)
{
	free(cpu);
}