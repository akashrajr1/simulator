#ifndef _UC32SIM_DECLARATIONS_H
#define _UC32SIM_DECLARATIONS_H
#include <setjmp.h>
#include <stdint.h>
#include <stddef.h>
typedef struct _cpu cpu_t;
typedef struct _general_inst general_inst;
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
typedef struct _cache cache_t;
typedef struct _mmu mmu_t;
typedef enum mem_type{
	MEM_INST = 0,
	MEM_DATA
}mem_type;
#endif