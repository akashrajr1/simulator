#ifndef _UC32SIM_ERROR_H
#define _UC32SIM_ERROR_H
#include "declarations.h"
typedef enum{
	ADDR_INVAL = 1,
	INST_INVAL,
	NO_SYSCALL,
} simulator_error;

extern const char *error_names[];
/*
 * process a (fatal) error.
 */
void error_process(simulator_error err, uint32_t code);
void error_log(simulator_error err, uint32_t code);
void core_dump(cpu_t *cpu);
#endif /*!_UC32SIM_ERROR_H*/