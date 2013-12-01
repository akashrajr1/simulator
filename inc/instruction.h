#ifndef _UC32SIM_INSTRUCTION_H
#define _UC32SIM_INSTRUCTION_H
#include "declarations.h"
// #define INST_OP(inst) ((uint32_t)(inst)>>24)
#define REG_LIST(inst) (((uint32_t)inst & 0x1f) + (((uint32_t)inst>>4) & 0x7fe0))
#define FUNC_GET_SHIFT(func) ((func & FUNC_SHIFT_MASK)>>1)
#define OP_BRANCH_CNT 	8
#define OP_MUL_LONG 	4
#define OP_MUL_UNSIGN	2
#define OP_MUL_ADD		1
#define OP_CNT_ZERO		2
#define OP_CNT 			1
#define FUNC_MUL_BR_CNT 9
#define FUNC_NOT_ALU	8
#define FUNC_SHIFT_MASK	6
#define FUNC_SIGN_EXT	4
#define FUNC_HALF_BYTE	2
#define FUNC_REG_IMM	1

typedef enum {
	COND_EQ	= 0,// equal, Z=1
	COND_NEQ,	// not-equal, Z=0
	COND_UGE,	// unsigned-greater-equal, C=1
	COND_ULT,	// unsigned-less-than C=0
	COND_NEG,	// negative, N=1
	COND_NN,	// not-negative, N=0
	COND_OV,	// overflow, V=1
	COND_NV,	// not-overflow, V=0
	COND_UGT,	// unsigned-greater-than, C=1 and Z=0
	COND_ULE,	// unsigned-less-equal, C=0 OR Z=1
	COND_SGE,	// signed-greater-equel, N=V
	COND_SLT,	// signed-less-than, N!=V
	COND_SGT, 	// signed-greater-than, N=V and Z=0
	COND_SLE, 	// signed-less-equal, Z=1 or N!=V
	COND_AL,	// always.
} cond_code;

extern const char* cond_str[];

typedef enum {
	REG_ALU = 0,
	IMM_ALU,
	REG_LDST,
	IMM_LDST,
	MUL_LDST,
	COND_BRANCH,
	SOFT_TRAP = 7 
} inst_ident;


extern const char *shift_str[];

extern const char *alu_op_str[];

typedef struct{
	uint32_t imm:	9;
	uint32_t rotate:5;
	uint32_t rd:	5;
	uint32_t rn:	5;
	uint32_t flag:	1;
	uint32_t opcode:4;
	uint32_t id:	3;
} imm9_inst;

typedef struct {
	uint32_t imm:		14;
	uint32_t rd:		5;
	uint32_t rn:		5;
	uint32_t loadstore:	1;
	uint32_t writeback:	1;
	uint32_t byteword:	1;
	uint32_t addsub:	1;
	uint32_t prepost:	1;
	uint32_t id:		3;
} imm14_inst;

typedef struct {
	uint32_t rm:		5;
	uint32_t func:		4;
	uint32_t rs:		5;
	uint32_t rd:		5;
	uint32_t rn:		5;
	uint32_t loadstore:	1;
	uint32_t writeback:	1;
	uint32_t byteword:	1;
	uint32_t addsub:	1;
	uint32_t prepost:	1;
	uint32_t id:		3;
} reg_ldst_inst;

typedef struct {
	uint32_t rlistlow:	6;
	uint32_t func:		3;
	uint32_t rlisthigh:	10;
	uint32_t rn:		5;
	uint32_t loadstore:	1;
	uint32_t writeback:	1;
	uint32_t byteword:	1;
	uint32_t addsub:	1;
	uint32_t prepost:	1;
	uint32_t id:		3;
} multi_ldst_inst;

typedef struct {
	int32_t	 offset:	24;
	uint32_t link:		1;
	uint32_t cond:		4;
	uint32_t id:		3;
} cond_branch_inst;

typedef struct {
	uint32_t trapno:	24;
	uint32_t opcode:	8;
} soft_trap_inst;

uint32_t exec_alu_compute(uint32_t value1, uint32_t value2, alu_opcode opcode);
uint32_t exec_cnt_bits(uint32_t value, uint32_t zero_one);
uint32_t exec_test_cond(cond_code cond);
uint32_t exec_shift(uint32_t val, uint32_t s, shift_type shift, uint32_t *carry_ptr);
void exec_update_flags(uint32_t res, uint32_t value1, uint32_t value2, uint32_t shift_c, alu_opcode opcode, flags_reg *flags);
uint32_t inst_dispatch(cpu_t *cpu, general_inst inst);
#endif /* !_UC32SIM_INSTRUCTION_H */