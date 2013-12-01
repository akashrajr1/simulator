#ifndef _UC32SIM_DECLARATIONS_H
#define _UC32SIM_DECLARATIONS_H
#include <setjmp.h>
#include <stdint.h>
#include <stddef.h>
typedef struct _cpu cpu_t;
// typedef struct _general_inst general_inst;
typedef struct {
	uint32_t N:		1;
	uint32_t Z:		1;
	uint32_t C:		1;
	uint32_t V:		1;
	uint32_t unused:20;
	uint32_t I:		1;
	uint32_t F:		1;
	uint32_t T:		1;
	uint32_t mode:	5;
} flags_reg;
typedef enum {
	LEFT = 0,
	LOGIC_RIGHT,
	ARITH_RIGHT,
	ROTATE,
	IMM_ROTATE,
	LDST_LR, 
	LDST_AR
} shift_type;
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
typedef struct {
	uint32_t rm:	5;
	uint32_t func:	4;
	uint32_t rs:	5;
	uint32_t rd:	5;
	uint32_t rn:	5;
	uint32_t flag:	1;
	uint32_t opcode:4;
	uint32_t id:	3;
} general_inst;
typedef uint32_t reg_num;
typedef enum{
	MM_WORD = 0,
	MM_BYTE,
	MM_HALFWORD,
} mem_size;
typedef struct _cache cache_t;
typedef struct _mmu mmu_t;
typedef enum mem_type{
	MEM_INST = 0,
	MEM_DATA
}mem_type;
#endif