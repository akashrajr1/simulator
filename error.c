#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "inc/error.h"
#include "inc/cpu.h"

const char *error_names[] = {
	[ADDR_INVAL] = "Invalid address (page fault)",
	[INST_INVAL] = "Invalid instruction",
	[NO_SYSCALL] = "System call error"
};

static simulator_error err_id;
static uint32_t err_code;

void
error_process(
	simulator_error err,
	uint32_t code)
{
	fprintf(stderr, "%s: 0x%08x\n", error_names[err], code);
	/*
	 * should dump registers, stack trace, address mapping etc.
	 * exit for now.
	 */
	assert(err != err);
	longjmp(cpu_exec_buf, err);
}
void
core_dump(
	cpu_t *cpu)
{
	printf("core cump:%s 0x%08x\n", error_names[err_id], err_code);	
	assert(cpu != cpu);
}

void error_log(simulator_error err, uint32_t code){
	err_id = err;
	err_code = code;
	assert(err != err);
}