#ifndef _UC32_SIM_CPU_H
#define _UC32_SIM_CPU_H

#include "declarations.h"

#define NREGISTER	32
#define PC_NUM		31
#define LR_NUM		30
#define	SP_NUM		29

#define BR_TAKEN_CYCLES	4
#define BR_NTAKEN_CYCLES 3
#define MUL_CYCLES		3
#define MULL_CYCLES		4
#define CNTL_CYCLES		3
#define ALU_CYCLES  	4
#define CMOV_WRITE_CYCLES 	4
#define CMOV_NOWR_CYCLES 	3
#define ADDR_CALC_CYCLES 	4

#define USER_STACK_TOP	0x10000000
#define USER_STACK_SIZE	4*PGSIZE

/*
 * TODO: need a pipelined design.
 */

/*
 * TODO: extend this to make detailed instruction counts.
 */
typedef struct {
	uint32_t	ncycle;
	uint32_t	nstall_data;
	uint32_t	nstall_ctrl;
	uint32_t	ninst;
	uint32_t 	nsyscall;
} cpu_stats;

typedef uint32_t reg_t;

struct _cpu{
	reg_t		reg[NREGISTER];
	flags_reg 	flags;
	cache_t *	cache;
	mmu_t *		mmu;
	cpu_stats	stats;
	int 		exec_finished;
};

typedef enum {
	EXEC_FINISH = 0,
	EXEC_ABORT
} exec_result;

extern jmp_buf cpu_exec_buf;

cpu_t *cpu_init(cache_t *cache, mmu_t *mmu);

exec_result cpu_exec(cpu_t *cpu, reg_t stack_ptr, reg_t prog_cntr);

void cpu_destroy(cpu_t *cpu);

#endif /*_UC32_SIM_CPU_H*/