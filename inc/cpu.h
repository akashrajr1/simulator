#ifndef _UC32_SIM_CPU_H
#define _UC32_SIM_CPU_H

#include "instruction.h"
#include "cache.h"
#include "mmu.h"

#define NREGISTER	32
#define PC_NUM		31
#define	SP_NUM		29

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
} cpu_stats;

typedef uint32_t reg_t;

typedef struct {
	reg_t		reg[NREGISTER];
	flags_reg 	flags;
	cache_t *	cache;
	mmu_t *		mmu;
	cpu_stats	stats;
} cpu_t;

typedef enum {
	EXEC_FINISH = 0,
	EXEC_ABORT
} exec_result;

cpu_t *cpu_init(cache_t *cache, mmu_t *mmu);

exec_result cpu_exec(cpu_t *cpu, reg_t stack_ptr, reg_t prog_cntr);

void cpu_destroy(cpu_t *cpu);

#endif /*_UC32_SIM_CPU_H*/