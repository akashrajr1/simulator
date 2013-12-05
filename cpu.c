#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "inc/cpu.h"
#include "inc/error.h"
#include "inc/syscall.h"
#include "inc/instruction.h"
#include "inc/cache.h"
#define REG(num) (exec_cpu->reg[num])
#define FLAGS 	(exec_cpu->flags)
#define IFID  	(exec_cpu->ifid)
#define IDEX 	(exec_cpu->idex)
#define EXMM  	(exec_cpu->exmm)
#define MMWB 	(exec_cpu->mmwb)

jmp_buf cpu_exec_buf;
cpu_t* exec_cpu;

cpu_t *
cpu_init(
	cache_t *cache,
	mmu_t *mmu)
{
	cpu_t *cpu = (cpu_t *)malloc(sizeof(cpu_t));
	memset(cpu, 0, sizeof(cpu_t));
	cpu->cache = cache;
	cpu->mmu = mmu;
	return cpu;
}

static char *inst_desc[] = {
	"bubble", "alu(reg-shift)", "alu(imm-shift)", "count-bits", "jump", "multiply(32)", "multiply(64)",
	"alu(imm-rotate)", "load/store(reg-offset)", "load/store(half-sign)", "load/store(imm-offset)",
	"branch", "syscall"
};

exec_result
cpu_exec(
	cpu_t *cpu,
	reg_t stack_ptr,
	reg_t prog_cntr)
{

	cpu->reg[SP_NUM] = stack_ptr;
	cpu->reg[PC_NUM] = prog_cntr;
	if (setjmp(cpu_exec_buf)){
		int i;
		for (i = 0; i < 16; i++) printf("r%02d:     ", i);
		printf("\n");
		for (i = 0; i < 16; i++) printf("%08x ", REG(i));
		printf("\n");
		for (i = 16; i < 32; i++) printf("r%02d:     ", i);
		printf("\n");
		for (i = 16; i < 31; i++) printf("%08x ", REG(i));
		printf("%08x \n", MMWB.pc);

		printf("cycles:%d\nforwards:%d\nuse-after-load:%d\nbranch-stall1:%d\nbranch-stall12:%d\nsystem-calls:%d\n\n",
			exec_cpu->stats.ncycle, exec_cpu->stats.nforward, exec_cpu->stats.nload_use, exec_cpu->stats.nctrl1,
			exec_cpu->stats.nctrl2, exec_cpu->stats.nsyscall);
		for (i = 0; i < NINST_TYPE; i++)
			printf("%s:%d\n", inst_desc[i], exec_cpu->stats.ninst[i]);
		// caused by non-local jump from error_process
		return cpu->exec_finished? EXEC_FINISH: EXEC_ABORT;
	}
	exec_cpu = cpu;
	IFID.nop = 1;
	/* ... 
	 * execute
	 */
	/*
	while (cpu->exec_finished == 0){
		int32_t cycles;
		general_inst inst;
		*(uint32_t*)(&inst) = cache_load(cpu->cache, cpu->reg[PC_NUM], MEM_INST, &cycles);
		cpu->reg[PC_NUM] += 4;
		cycles += inst_dispatch(cpu, inst);
		cpu->stats.ncycle += cycles;
		cpu->stats.ninst ++;
	}*/
	
	while (1){
		cpu_pipe_if();
		if (exec_cpu->fault) exec_cpu->fault++;
		exec_cpu->stats.ncycle ++;
	}
	// printf("return value: 0x%08x\n", cpu->reg[0]);
	// printf("instructions:%d; cycles:%d\n", cpu->stats.ninst, cpu->stats.ncycle);
	return EXEC_FINISH;
}

uint32_t
get_reg(reg_num r)
{
	if (r == PC_NUM) return IFID.pc + 4;
	uint32_t value = REG(r);
	// bypass result from MM
	if (r == MMWB.rd1 && (MMWB.do_write & 1)){
		exec_cpu->stats.nforward ++;
		value = MMWB.val1;
	}
	else if (r == MMWB.rd2 && (MMWB.do_write & 2)){
		exec_cpu->stats.nforward ++;
		value = MMWB.val2;
	}

	// bypass result from EX
	if (r == EXMM.to_wb.rd1 && (EXMM.to_wb.do_write & 1)){
		if (EXMM.do_mem) exec_cpu->load_use = 1; // use after load harzard
		else {
			value = EXMM.to_wb.val1;
			exec_cpu->stats.nforward ++;
		}
	}
	else if (r == EXMM.to_wb.rd2 && (EXMM.to_wb.do_write & 2)){
		value = EXMM.to_wb.val2;
		exec_cpu->stats.nforward ++;
	}
	return value;
}

void
cpu_pipe_if(){
	cpu_pipe_id();
	if (exec_cpu->load_use){
		exec_cpu->load_use = 0; // use after load stalls 1 cycle
		return;
	}
	IFID.nop = 1;
	if (exec_cpu->use_new_pc){
		REG(PC_NUM) = exec_cpu->new_pc;
		exec_cpu->use_new_pc = 0;
		if (exec_cpu->fault < 4) exec_cpu->fault = 0;
		// cache contamination of speculative execution is not considered (wrong instruction load does not occur)
		return;
	}
	if (exec_cpu->fault >= 1) return;
	int32_t latency = 0;
	*(uint32_t *)(&(IFID.inst)) = cache_load(exec_cpu->cache, REG(PC_NUM), MEM_INST, &latency);
	IFID.pc = REG(PC_NUM);
	IFID.nop = 0;
	if (latency < 0) exec_cpu->fault = 1;
	REG(PC_NUM) += 4;
}

void
cpu_pipe_id(){
	cpu_pipe_ex();
	EXMM.to_wb.flags = FLAGS;
	memset(&(IDEX), 0, sizeof(pipe_idex_t));
	if (IFID.nop || exec_cpu->fault >= 2) return;
	general_inst inst = IFID.inst;
	IDEX.to_mm.to_wb.inst = inst;
	IDEX.to_mm.to_wb.pc = IFID.pc;
	switch ((inst_ident)inst.id){
		case REG_ALU:
			if ((inst.func & FUNC_NOT_ALU) == 0){
				if (inst.opcode == ALU_COND_MOV || inst.opcode == ALU_COND_MVN)
					IDEX.val1 = (inst.rn & 0x10)? (inst.rn & 0xf): COND_AL;
				else
					IDEX.val1 = get_reg(inst.rn);
				IDEX.val2 = get_reg(inst.rm);
				IDEX.sa = (inst.func & FUNC_REG_IMM)? get_reg(inst.rs): inst.rs;
				IDEX.to_mm.to_wb.type = (inst.func & FUNC_REG_IMM)? ALU_REG_SHIFT: ALU_IMM_SHIFT;
				IDEX.shift = FUNC_GET_SHIFT(inst.func);
				IDEX.opcode = inst.opcode;
				IDEX.to_mm.to_wb.rd1 = inst.rd;
				IDEX.to_mm.to_wb.do_write = 1;
				IDEX.flag = inst.flag;
			}
			else if (inst.func == FUNC_MUL_BR_CNT){
				if (inst.opcode & OP_BRANCH_CNT){
					if (inst.opcode & OP_CNT){
						IDEX.val1 = get_reg(inst.rm);
						IDEX.to_mm.to_wb.rd1 = inst.rd;
						IDEX.to_mm.to_wb.do_write = 1;
						IDEX.opcode = inst.opcode;
						IDEX.to_mm.to_wb.type = CNT_BITS;
					}else{
						// is this correct? seems to be...
						exec_cpu->new_pc = get_reg(inst.rm);
						if (!exec_cpu->load_use){
							exec_cpu->use_new_pc = 1;
							if (inst.flag){
								IDEX.to_mm.to_wb.rd1 = LR_NUM;
								IDEX.to_mm.to_wb.val1 = REG(PC_NUM);
								IDEX.to_mm.to_wb.do_write = 1;
							}
							exec_cpu->stats.nctrl1 ++;
						}
						IDEX.to_mm.to_wb.type = JUMP;
					}
				}
				else if (inst.opcode & OP_MUL_LONG){
					IDEX.mul = MULTIPLY | inst.opcode;
					IDEX.val1 = get_reg(inst.rn);
					IDEX.val2 = get_reg(inst.rm);
					IDEX.sa = get_reg(inst.rd);
					IDEX.addhi = get_reg(inst.rs);
					IDEX.flag = inst.flag;
					IDEX.to_mm.to_wb.rd1 = inst.rd;
					IDEX.to_mm.to_wb.rd2 = inst.rs;
					IDEX.to_mm.to_wb.do_write = 3;
					IDEX.to_mm.to_wb.type = MUL64;
				}
				else{
					IDEX.mul = MULTIPLY | inst.opcode;
					IDEX.val1 = get_reg(inst.rn);
					IDEX.val2 = get_reg(inst.rm);
					IDEX.sa = get_reg(inst.rs);
					IDEX.flag = inst.flag;
					IDEX.to_mm.to_wb.rd1 = inst.rd;
					IDEX.to_mm.to_wb.do_write = 1;
					IDEX.to_mm.to_wb.type = MUL32;
				}
			}
			else{
				error_log(INST_INVAL, *(uint32_t *)(&inst));
				exec_cpu->fault = 2;
			}
			break;
		case IMM_ALU:
		{
			imm9_inst *i_ptr = (imm9_inst*)(&inst);
			if (inst.opcode == ALU_COND_MOV || inst.opcode == ALU_COND_MVN)
				IDEX.val1 = (inst.rn & 0x10)? (inst.rn & 0xf): COND_AL;
			else
				IDEX.val1 = get_reg(inst.rn);
			IDEX.val2 = i_ptr->imm;
			IDEX.sa = i_ptr->rotate;
			IDEX.opcode = i_ptr->opcode;
			IDEX.shift = IMM_ROTATE;
			IDEX.flag = i_ptr->flag;
			IDEX.to_mm.to_wb.rd1 = i_ptr->rd;
			IDEX.to_mm.to_wb.do_write = 1;
			IDEX.to_mm.to_wb.type = ALU_IMM_ROTATE;
			break;
		}
		case REG_LDST:
		{
			reg_ldst_inst *i_ptr = (reg_ldst_inst*)(&inst);
			if ((i_ptr->func & (~FUNC_SHIFT_MASK)) == 0){
				IDEX.val1 = get_reg(i_ptr->rn);
				IDEX.val2 = get_reg(i_ptr->rm);
				IDEX.sa = i_ptr->rs;
				IDEX.shift = FUNC_GET_SHIFT(inst.func);
				IDEX.opcode = i_ptr->addsub? ALU_ADD: ALU_SUB;
				IDEX.flag = 0;
				IDEX.ldst = 1 | (i_ptr->prepost? 2 : 0);

				if (! i_ptr->loadstore) IDEX.to_mm.st_val = get_reg(i_ptr->rd);
				IDEX.to_mm.loadstore = i_ptr->loadstore;
				IDEX.to_mm.size = i_ptr->byteword? MM_BYTE: MM_WORD;
				IDEX.to_mm.signzero = 0;
				IDEX.to_mm.do_mem = 1;

				IDEX.to_mm.to_wb.rd1 = i_ptr->rd;
				IDEX.to_mm.to_wb.rd2 = i_ptr->rn;
				if (! i_ptr->prepost) i_ptr->writeback = 1;
				IDEX.to_mm.to_wb.do_write = (i_ptr->loadstore? 1: 0) | (i_ptr->writeback? 2: 0);
				IDEX.to_mm.to_wb.type = LDST_REG;
			}else{
				IDEX.val1 = get_reg(i_ptr->rn);
				IDEX.val2 = i_ptr->byteword? (i_ptr->rs << 5) + i_ptr->rm: get_reg(i_ptr->rm);
				IDEX.sa = 0;
				IDEX.shift = LEFT;
				IDEX.opcode = i_ptr->addsub? ALU_ADD: ALU_SUB;
				IDEX.flag = 0;
				IDEX.ldst = 1 | (i_ptr->prepost? 2 : 0);

				if (! i_ptr->loadstore) IDEX.to_mm.st_val = get_reg(i_ptr->rd);
				IDEX.to_mm.loadstore = i_ptr->loadstore;
				IDEX.to_mm.size = i_ptr->func & FUNC_HALF_BYTE? MM_HALFWORD: MM_BYTE;
				IDEX.to_mm.signzero = i_ptr->func & FUNC_SIGN_EXT;
				IDEX.to_mm.do_mem = 1;

				IDEX.to_mm.to_wb.rd1 = i_ptr->rd;
				IDEX.to_mm.to_wb.rd2 = i_ptr->rn;
				if (! i_ptr->prepost) i_ptr->writeback = 1;
				IDEX.to_mm.to_wb.do_write = (i_ptr->loadstore? 1: 0) | (i_ptr->writeback? 2: 0);
				IDEX.to_mm.to_wb.type = LDST_HALFSIGN;
			}
			break;
		}
		case IMM_LDST:
		{
			imm14_inst *i_ptr = (imm14_inst*)(&inst);
			IDEX.val1 = get_reg(i_ptr->rn);
			IDEX.val2 = i_ptr->imm;
			IDEX.sa = 0;
			IDEX.shift = LEFT;
			IDEX.opcode = i_ptr->addsub? ALU_ADD: ALU_SUB;
			IDEX.flag = 0;
			IDEX.ldst = 1 | (i_ptr->prepost? 2 : 0);

			if (! i_ptr->loadstore) IDEX.to_mm.st_val = get_reg(i_ptr->rd);
			IDEX.to_mm.loadstore = i_ptr->loadstore;
			IDEX.to_mm.size = i_ptr->byteword? MM_BYTE: MM_WORD;
			IDEX.to_mm.signzero = 0;
			IDEX.to_mm.do_mem = 1;

			IDEX.to_mm.to_wb.rd1 = i_ptr->rd;
			IDEX.to_mm.to_wb.rd2 = i_ptr->rn;
			if (! i_ptr->prepost) i_ptr->writeback = 1;
			IDEX.to_mm.to_wb.do_write = (i_ptr->loadstore? 1: 0) | (i_ptr->writeback? 2: 0);
			IDEX.to_mm.to_wb.type = LDST_IMM;
			break;
		}
		case COND_BRANCH:
		{
			cond_branch_inst *i_ptr = (cond_branch_inst*)(&inst);
			IDEX.val1 = REG(PC_NUM);
			IDEX.val2 = i_ptr->offset << 2;
			IDEX.sa = 0;
			IDEX.addhi = i_ptr->cond;
			IDEX.shift = LEFT;
			IDEX.opcode = ALU_ADD;
			IDEX.flag = 0;
			IDEX.br = 1;
			if (i_ptr->link){
				IDEX.to_mm.to_wb.val1 = REG(PC_NUM);
				IDEX.to_mm.to_wb.rd1 = LR_NUM;
				IDEX.to_mm.to_wb.do_write = 1;
			}
			exec_cpu->stats.nctrl2 ++;
			IDEX.to_mm.to_wb.type = BRANCH;
			// latency = exec_cond_branch(i_ptr->cond, i_ptr->offset, i_ptr->link);
			break;
		}
		case SOFT_TRAP:
		{
			soft_trap_inst *i_ptr = (soft_trap_inst*)(&inst);
			if (i_ptr->opcode != 0xff) error_log(INST_INVAL, *(uint32_t *)(&inst));
			else REG(0) = perform_syscall((syscall_num)i_ptr->trapno, exec_cpu);
			exec_cpu->fault = 2;
			exec_cpu->in_syscall = 1;
			IDEX.to_mm.to_wb.type = JEPRIV;
			break;
		}
		default:
			error_log(INST_INVAL, *(uint32_t *)(&inst));
			exec_cpu->fault = 2;
			break;
	}
	if (exec_cpu->load_use){ // use after load harzard
		exec_cpu->stats.nload_use ++;
		memset(&IDEX, 0, sizeof(pipe_idex_t)); // buble in EX stage
	}
}


void
cpu_pipe_ex(){
	cpu_pipe_mm();
	EXMM = IDEX.to_mm;
	if (exec_cpu->fault >= 3) return;
	if (IDEX.mul & MULTIPLY){
		if (IDEX.mul & OP_MUL_LONG){
			uint64_t value = (IDEX.mul & OP_MUL_ADD)? ((uint64_t)IDEX.addhi<<32) + IDEX.sa:0ull;
			if (IDEX.mul & OP_MUL_UNSIGN)
				value += (uint64_t)IDEX.val1 * IDEX.val2;
			else
				value += (int64_t)(int32_t)IDEX.val1 * (int32_t)IDEX.val2;
			if (IDEX.flag){
				FLAGS.Z = (value == 0ll);
				FLAGS.N = ((int64_t)value < 0ll);
			}
			EXMM.to_wb.val1 = (uint32_t)value;
			EXMM.to_wb.val2 = (uint32_t)(value>>32);
		}else{
			uint32_t value = (IDEX.mul & OP_MUL_ADD)? IDEX.sa:0u;
			value += IDEX.val1 * IDEX.val2;
			if (IDEX.flag){
				FLAGS.Z = (value == 0l);
				FLAGS.N = (value < 0l);
			}
			EXMM.to_wb.val1 = value;
		}
		return;
	}
	if (IDEX.cnt){
		EXMM.to_wb.val1 = exec_cnt_bits(IDEX.val1, IDEX.opcode & OP_CNT_ZERO);
		return;
	}
	if (IDEX.opcode >= ALU_CMP_AND && IDEX.opcode <= ALU_CMP_ADD){
		EXMM.to_wb.do_write = 0;
		IDEX.flag = 1;
	}
	else if (IDEX.opcode == ALU_COND_MOV || IDEX.opcode == ALU_COND_MVN)
		EXMM.to_wb.do_write = exec_test_cond(IDEX.val1)? 1: 0;
	uint32_t shiftcarry;
	uint32_t val2 = exec_shift(IDEX.val2, IDEX.sa, IDEX.shift, &shiftcarry);
	uint32_t res = exec_alu_compute(IDEX.val1, val2, IDEX.opcode);
	if (IDEX.br){
		if (exec_test_cond(IDEX.addhi)){
			IFID.nop = 1; // this actually went a bit back in time
			exec_cpu->use_new_pc = 1;
			exec_cpu->new_pc = res;
		}else EXMM.to_wb.do_write = 0;
		return;
	}
	if (IDEX.flag)
		exec_update_flags(res, IDEX.val1, val2, shiftcarry, IDEX.opcode, &FLAGS);
	if (IDEX.ldst){
		EXMM.to_wb.val2 = res;
		EXMM.addr = (IDEX.ldst & 2)? res:IDEX.val1;
	}else EXMM.to_wb.val1 = res;
}

static uint32_t
sign_extend(
	uint32_t word,
	uint32_t addr,
	mem_size sz,
	uint32_t signzero)
{
	if (sz == MM_BYTE){
		word >>= 8 * (addr & 3);
		if (signzero) word = (int32_t)(int8_t)word;
		else word = (uint32_t)(uint8_t)word;
	}else if (sz == MM_HALFWORD){
		word >>= 8 * (addr & 2);
		if (signzero) word = (int32_t)(int16_t)word;
		else word = (uint32_t)(uint16_t)word;
	}
	return word;
}

void
cpu_pipe_mm(){
	cpu_pipe_wb();
	int32_t latency;
	MMWB = EXMM.to_wb;
	if (!EXMM.do_mem || exec_cpu->fault >= 4) return;
	if (EXMM.loadstore){
		uint32_t word = cache_load(exec_cpu->cache, EXMM.addr, MEM_DATA, &latency);
		MMWB.val1 = sign_extend(word, EXMM.addr, EXMM.size, EXMM.signzero);
		assert(MMWB.do_write & 1);
	}else{
		assert((MMWB.do_write & 1) == 0);
		cache_dstore(exec_cpu->cache, EXMM.addr, EXMM.st_val, EXMM.size, &latency);
	}
	if (latency < 0) exec_cpu->fault = 4;
}

void
cpu_pipe_wb(){
	exec_cpu->stats.ninst[MMWB.type] ++;
	if (exec_cpu->fault == 5){
		if (exec_cpu->exec_finished) longjmp(cpu_exec_buf, 100);
		if (exec_cpu->in_syscall){
			exec_cpu->in_syscall = 0;
			exec_cpu->fault = 0;
			exec_cpu->stats.nsyscall ++;
		}else core_dump(exec_cpu); // should not return
	}
	if (MMWB.do_write & 1) REG(MMWB.rd1) = MMWB.val1;
	if (MMWB.do_write & 2) REG(MMWB.rd2) = MMWB.val2;
}

void
cpu_destroy(
	cpu_t *cpu)
{
	free(cpu);
}