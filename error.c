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
	longjmp(cpu_exec_buf, err);
}