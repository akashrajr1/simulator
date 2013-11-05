#ifndef _UC32SIM_ERROR_H
#define _UC32SIM_ERROR_H

typedef enum{
	ADDR_INVAL = 1,
	INST_INVAL,
	NO_SYSCALL,
} simulator_error;

extern const char *error_names[];
/*
 * process an (fatal) error.
 * TODO: long jump to cpu module.
 */
void error_process(simulator_error err, uint32_t code);
#endif /*!_UC32SIM_ERROR_H*/