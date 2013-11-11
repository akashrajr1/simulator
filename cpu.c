#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/cpu.h"
#include "inc/error.h"
#include "inc/syscall.h"
#include "inc/instruction.h"
#include "inc/cache.h"
#define REG(num) cpu->reg[num]
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
		/*if (REG(PC_NUM) == 0x2000098){
			// printf("LR(return address):0x%08x, SP:0x%08x\n", REG(LR_NUM), REG(SP_NUM));
			if (REG(SP_NUM) == 0xfff766c + 4){
				int value = cpu->stats.ninst;
				printf("cool, inst:%d, LR:0x%x\n", value, REG(LR_NUM));
			}
		}*/
		/*if (REG(PC_NUM) == 0x2000180 || REG(PC_NUM) == 0x20001a4 || REG(PC_NUM) == 0x20001e8 || REG(PC_NUM) == 0x2000274){
			 if (cpu->stats.ninst == 37798595){
				int value = cpu->stats.ninst;
				printf("oh oh inst:%d PC:0x%x SP:0x%x *SP:0x%x\n", value, REG(PC_NUM), REG(SP_NUM), cache_load(cpu->cache, REG(SP_NUM), MEM_DATA, NULL));
			}
			//printf("hi! PC:0x%08x, *SP=0x%08x, inst:%d\n", REG(PC_NUM), cache_load(cpu->cache, REG(SP_NUM), MEM_DATA, NULL), cpu->stats.ninst);
		}*/
		// if (REG(PC_NUM) == 0x2000234){
		// 	printf("PC:0x%x\n", REG(PC_NUM));	
		// }
		*(uint32_t*)(&inst) = cache_load(cpu->cache, cpu->reg[PC_NUM], MEM_INST, &cycles);
		cpu->reg[PC_NUM] += 4;
		cycles += inst_dispatch(cpu, inst);
		cpu->stats.ncycle += cycles;
		cpu->stats.ninst ++;
	}
	printf("return value: 0x%08x\n", cpu->reg[0]);
	printf("instructions:%d; cycles:%d\n", cpu->stats.ninst, cpu->stats.ncycle);
	return EXEC_FINISH;
}


void
cpu_destroy(
	cpu_t *cpu)
{
	free(cpu);
}