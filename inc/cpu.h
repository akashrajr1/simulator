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
#define USER_STACK_SIZE	1000*PGSIZE

#define MULTIPLY 8

typedef enum {
	BUBBLE = 0,
	ALU_REG_SHIFT,
	ALU_IMM_SHIFT,
	CNT_BITS,
	JUMP,
	MUL32,
	MUL64,
	ALU_IMM_ROTATE,
	LDST_REG,
	LDST_HALFSIGN,
	LDST_IMM,
	BRANCH,
	JEPRIV,
	NINST_TYPE
}inst_type;

typedef struct {
	// nop if do_write = 0
	general_inst inst;
	inst_type type;
	uint32_t pc;
	uint32_t istall;
	uint32_t dstall;
	flags_reg flags;
	// load/alu register (and address writeback)
	reg_num rd1, rd2;
	uint32_t val1, val2;
	uint32_t do_write;

} pipe_mmwb_t;
typedef struct {
	// nop if do_mem = 0
	uint32_t do_mem;

	uint32_t addr;
	uint32_t st_val;
	uint32_t loadstore;
	mem_size size;
	uint32_t signzero;
	// wb
	pipe_mmwb_t to_wb;
}pipe_exmm_t;
typedef struct {
	// nop if all zero
	// s1, s2, addlo, addhi for long mul
	uint32_t val1, val2, sa, addhi;
	shift_type shift;
	alu_opcode opcode;
	uint32_t flag;

	uint32_t mul; // integer multiply
	uint32_t cnt; // count bits
	uint32_t ldst;// bit0:ldst, bit1:pre/post
	uint32_t br;
	pipe_exmm_t to_mm;
}pipe_idex_t;
typedef struct {
	general_inst inst;
	uint32_t nop;
	uint32_t pc;
	uint32_t wait;
	// pipe_idex_t idex;
} pipe_ifid_t;

typedef struct {
	uint32_t	ncycle;
	uint32_t	nforward;
	uint32_t	nload_use;
	uint32_t	nctrl1;
	uint32_t	nctrl2;
	uint32_t	nicache_stall;
	uint32_t	ndcache_stall;
	uint32_t 	nsyscall;
	uint32_t	ninst[NINST_TYPE];
} cpu_stats;

typedef uint32_t reg_t;

 /*
 Pipeline registers in between stages: ifid, idex, exmm, mmwb
 forwarding datapath from EX to ID, MM to ID, EX to IF (for instructions
 modifying PC register).
 
 When to forward: 
 1. ID depends on *current* result of EX (2 forward paths since long multiply
 modifies two registers)

 2. previous result of EX (currently latched in exmm, to be transfered to mmwb)

 3. data loaded in MM stage. If use-after-load occurs, which means the load
 instruction is in EX stage calculating the memory address while the next
 instruction is in ID stage waiting for an operand, IF and ID will lock them-
 selves and a nop is inserted into EX stage. The stall in IF/ID and bubble in
 EX will persist until MM stage returns data or throws an exception.

 No need to forward from WB stage. WB happends before register read in ID even
 if they occur in the same cycle.

 4. If result of EX goes to PC register (conditional or unconditional), current
 ID and IF stage becomes a bubble (nop), then in the next cycle, EX becomes nop
 (from ID), ID becomes nop (from IF), IF fetches the new instruction. The 
 previous IF stage may stall for cache miss etc, which must be cancelled upon
 new fetch (even if i-cache cause an exception).

 Treating PC just like any other GPR effectively solves control harzards. The
 special thing is that WB will not modify PC, and PC<-PC+4 uses independent
 hardware from ALU in EX stage.

 On flags register: flags may only be modified at the end  of EX stage, and may
 only be used at the beginning of the EX stage. Forwarding is required to pass
 the new flags to the next EX stage. However, there is a register named CMSR
 which holds the contents of flags, and must be updated in WB stage.

 In the current implementation of pipeline registers, idex, exmm contains info
 on next stage (to_mm, to_wb) so they are aware of dependencies. Unlike any
 reasonable hardware implementation, the software simulator must perform the
 pipeline operations in reverse order to avoid priorer stages overwrite the
 pipeline registers. IF and ID stage will enquire later stages for forwarding.

 */


struct _cpu{
	reg_t		reg[NREGISTER];
	flags_reg 	flags;
	pipe_ifid_t ifid;
	pipe_idex_t idex;
	pipe_exmm_t exmm;
	pipe_mmwb_t mmwb;
	uint32_t 	new_pc;
	uint32_t 	use_new_pc;
	uint32_t 	load_use; // lock IF & ID, bubbles for later stages
	uint32_t 	fault; // the stage that generated a fault
	uint32_t	in_syscall;
	cache_t *	cache;
	mmu_t *		mmu;
	cpu_stats	stats;
	int 		exec_finished;
};

typedef enum {
	EXEC_FINISH = 0,
	EXEC_ABORT
} exec_result;

extern cpu_t* exec_cpu;
extern jmp_buf cpu_exec_buf;

cpu_t *cpu_init(cache_t *cache, mmu_t *mmu);

exec_result cpu_exec(cpu_t *cpu, reg_t stack_ptr, reg_t prog_cntr);

void cpu_pipe_if();
void cpu_pipe_id();
void cpu_pipe_ex();
void cpu_pipe_mm();
void cpu_pipe_wb();

void cpu_destroy(cpu_t *cpu);

#endif /*_UC32_SIM_CPU_H*/