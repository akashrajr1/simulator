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
#define FUNC_BYTE_HALF	2
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

const static char* cond_str[] = {
	"equal",
	"not-equal",
	"unsigned-greater-equal",
	"unsigned-less-than",
	"negative",
	"not-negative",
	"overflow",
	"not-overflow",
	"unsigned-greater-than",
	"unsigned-less-equal",
	"signed-greater-equel",
	"signed-less-than",
	"signed-greater-than",
	"signed-less-equal",
	"always"
};

typedef enum {
	REG_ALU = 0,
	IMM_ALU,
	REG_LDST,
	IMM_LDST,
	MUL_LDST,
	COND_BRANCH,
	SOFT_TRAP = 7 
} inst_ident;

typedef enum {
	LEFT = 0,
	LOGIC_RIGHT,
	ARITH_RIGHT,
	ROTATE
} shift_type;

const static char *shift_str[] = {
	"<<",
	"l>>",
	"a>>",
	"<>"
};

typedef enum {
	ALU_AND = 0,
	ALU_XOR,
	ALU_SUB,
	ALU_RSUB,
	ALU_ADD,
	ALU_ADC,
	ALU_SBC,
	ALU_RSBC,
	ALU_CMP_AND,
	ALU_CMP_XOR,
	ALU_CMP_SUB,
	ALU_CMP_ADD,
	ALU_OR,
	ALU_COND_MOV,
	ALU_AND_NOT,
	ALU_COND_MVN
} alu_opcode;

const static char *alu_op_str[] = {
	"&",
	"^",
	"-",
	"R-",
	"+",
	"+C",
	"-C",
	"R-C",
	"&",
	"^",
	"-",
	"+",
	"|",
	"",
	"& ~",
	"~"
};

typedef uint32_t reg_num;

struct _general_inst{
	uint32_t rm:	5;
	uint32_t func:	4;
	uint32_t rs:	5;
	uint32_t rd:	5;
	uint32_t rn:	5;
	uint32_t flag:	1;
	uint32_t opcode:4;
	uint32_t id:	3;
};

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


uint32_t inst_dispatch(cpu_t *cpu, general_inst inst);
#endif /* !_UC32SIM_INSTRUCTION_H */