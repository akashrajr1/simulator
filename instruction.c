#include "inc/instruction.h"
#include "inc/error.h"
#include "inc/cpu.h"
#include "inc/syscall.h"
#include <stdio.h>

#define REG(num) exec_cpu->reg[num]
#define FLAGS 	exec_cpu->flags

static cpu_t *exec_cpu = NULL;
static void
illegal_inst(
	general_inst *inst)
{
	error_process(INST_INVAL, *(uint32_t*)inst);
}

static uint32_t
exec_branch(
	reg_num reg, 
	uint32_t link)
{
	printf("unconditional branch to r%d, %s.\n", reg, link?"link":"no-link");
	if (link)
		exec_cpu->reg[LR_NUM] = exec_cpu->reg[PC_NUM];
	exec_cpu->reg[PC_NUM] = exec_cpu->reg[reg];
	return BR_TAKEN_CYCLES;
}

static uint32_t
exec_cnt_bits(
	reg_num rd,
	reg_num rs,
	uint32_t zero_one)
{
	uint32_t value = REG(rs);
	uint32_t bit_cnt = 0;
	if (zero_one){
		if (value>>16 == 0) {bit_cnt += 16; value <<= 16;}
		if (value>>24 == 0) {bit_cnt += 8; value <<= 8;}
		if (value>>28 == 0) {bit_cnt += 4; value <<= 4;}
		if (value>>30 == 0) {bit_cnt += 2; value <<= 2;}
		if (value>>31 == 0) { bit_cnt++; if (value == 0) bit_cnt++;}
	}else{
		if (value>>16 == 0xffff) {bit_cnt += 16; value <<= 16;}
		if (value>>24 == 0xff) {bit_cnt += 8; value <<= 8;}
		if (value>>28 == 0xf) {bit_cnt += 4; value <<= 4;}
		if (value>>30 == 0x3) {bit_cnt += 2; value <<= 2;}
		if (value>>31 == 0x1) {bit_cnt++; if (value == 0xC0000000) bit_cnt++;}
	}
	REG(rd) = bit_cnt;
	return CNTL_CYCLES;
}

static uint32_t
exec_mul(
	reg_num rd, 
	reg_num rs1,
	reg_num rs2,
	reg_num rs3, 
	uint32_t add,
	uint32_t flag)
{
	int value = 0;
	if (add){
		printf("multiply with add r%d = r%d * r%d + r%d, %supdate-flags\n",
			rd, rs1, rs2, rs3, flag?"":"no-");
		value = REG(rs3);
	}
	else
		printf("multiply r%d = r%d * r%d, %supdate-flags\n",
			rd, rs1, rs2, flag?"":"no-");
	value += REG(rs1) * REG(rs2);
	REG(rd) = value;
	if (flag){
		FLAGS.Z = (value == 0);
		FLAGS.N = (value < 0);
	}
	return MUL_CYCLES;
}

static uint32_t
exec_long_mul(
	reg_num rd_low, 
	reg_num rd_high, 
	reg_num rs1, 
	reg_num rs2, 
	uint32_t unsign, 
	uint32_t add, 
	uint32_t flag)
{
	int64_t value = add?((((uint64_t)REG(rd_high))<<32) + REG(rd_low)):0ll;
	if (unsign)
		value += (uint64_t)REG(rs1) * (uint64_t)REG(rs2);
	else
		value += (int64_t)REG(rs1) * (int64_t)REG(rs2);
	if (flag){
		FLAGS.Z = (value == 0ll);
		FLAGS.N = (value < 0ll);
	}
	REG(rd_low) = (uint32_t)value;
	REG(rd_high) = (uint32_t)(value>>32);
	if (add)
		printf("%s long multiply with add {r%d,r%d} = r%d * r%d + {r%d,r%d}, %supdate-flags\n",
			unsign?"unsigned":"signed", rd_high, rd_low, rs1, rs2, rd_high, rd_low, flag?"":"no-");
	else
		printf("%s long multiply with add {r%d,r%d} = r%d * r%d, %supdate-flags\n",
			unsign?"unsigned":"signed", rd_high, rd_low, rs1, rs2, flag?"":"no-");
	return MULL_CYCLES;
}

static uint32_t
exec_test_cond(
	cond_code cond)
{
	switch (cond){
	case COND_EQ: 	// equal, Z=1
		return FLAGS.Z;
	case COND_NEQ:	// not-equal, Z=0
		return (FLAGS.Z == 0);
	case COND_UGE:	// unsigned-greater-equal, C=1
		return FLAGS.C;
	case COND_ULT:	// unsigned-less-than C=0
		return (FLAGS.C == 0);
	case COND_NEG:	// negative, N=1
		return FLAGS.N;
	case COND_NN:	// not-negative, N=0
		return (FLAGS.N == 0);
	case COND_OV:	// overflow, V=1
		return FLAGS.V;
	case COND_NV:	// not-overflow, V=0
		return (FLAGS.V == 0);
	case COND_UGT:	// unsigned-greater-than, C=1 and Z=0
		return (FLAGS.C && (FLAGS.Z == 0));
	case COND_ULE:	// unsigned-less-equal, C=0 OR Z=1
		return ((FLAGS.C == 0) || FLAGS.Z);
	case COND_SGE:	// signed-greater-equel, N=V
		return (FLAGS.N == FLAGS.V);
	case COND_SLT:	// signed-less-than, N!=V
		return (FLAGS.N != FLAGS.V);
	case COND_SGT: 	// signed-greater-than, N=V and Z=0
		return ((FLAGS.N == FLAGS.V) && (FLAGS.Z == 0));
	case COND_SLE: 	// signed-less-equal, Z=1 or N!=V
		return (FLAGS.Z || (FLAGS.N != FLAGS.V));
	case COND_AL:
		return 1;
	default:
		return 0;
	}
}

static uint32_t
exec_shift(
	uint32_t val,
	uint32_t s,
	shift_type shift,
	uint32_t *carry_ptr)
{
	*carry_ptr = FLAGS.C;
	switch (shift){
	case LEFT:
		if (s) *carry_ptr = ((val & (1<<(32-s))) != 0);
		val <<= s; break;
	case LOGIC_RIGHT:
		if (s == 0){
			*carry_ptr = ((val & (1<<31)) != 0);
			val = 0;
		}else{
			*carry_ptr = ((val & (1<<(s-1))) != 0);
			val >>= s;
		}
		break;
	case ARITH_RIGHT:
		if (s == 0){
			val = (int32_t)val >> 31;
			*carry_ptr = (val != 0);
		}else{
			*carry_ptr = ((val & (1<<(s-1))) != 0);
			val = (int32_t)val >> s;
		}
		break;
	case ROTATE:
		if (s) {
			*carry_ptr = ((val & (1<<(s-1))) != 0);
			val = (((val & ((1<<s)-1))<<(32-s)) | (val >> s));
		}else{
			*carry_ptr = (val & 1);
			val = (val >> 1) | (FLAGS.C<<31);
		}
		break;
	}
	return val;
}

static uint32_t
exec_alu_compute(
	uint32_t value1,
	uint32_t value2,
	alu_opcode opcode)
{
	uint32_t res = 0;
	switch (opcode){
		case ALU_AND:
		case ALU_CMP_AND:
			res = value1 & value2; break;
		case ALU_XOR:
		case ALU_CMP_XOR:
			res = value1 ^ value2; break;
		case ALU_SUB:
		case ALU_CMP_SUB:
			res = value1 + ~value2 + 1; break;
		case ALU_RSUB:
			res = value2 + ~value1 + 1; break;
		case ALU_ADD:
		case ALU_CMP_ADD:
			res = value1 + value2; break;
		case ALU_ADC:
			res = value1 + value2 + FLAGS.C; break;
		case ALU_SBC:
			res = value1 + ~value2 + FLAGS.C; break;
		case ALU_RSBC:
			res = value2 + ~value1 + FLAGS.C; break;
		case ALU_OR:
			res = value1 | value2; break;
		case ALU_COND_MOV:
			res = value2; break;
		case ALU_AND_NOT:
			res = value1 & (~value2); break;
		case ALU_COND_MVN:
			res = ~value2; break;
	}
	return res;
}

static void
exec_update_flags(
	uint32_t res,
	uint32_t value1,
	uint32_t value2,
	uint32_t shift_c,
	alu_opcode opcode)
{
	FLAGS.N = ((int32_t)res < 0);
	FLAGS.Z = (res == 0);
	// borrow = ~carry? Sub-Carry is actually Sub-Borrow?
	switch (opcode){
	case ALU_SUB:
	case ALU_SBC:
	case ALU_CMP_SUB:
		FLAGS.V = ((((value1^value2)>>31) != 0) && (((res^value2)>>31) == 0));
		res -= (opcode == ALU_SBC)? FLAGS.C: 1;
		FLAGS.C = (res < value1 || res < (~value2));
		break;
	case ALU_RSUB:
	case ALU_RSBC:
		FLAGS.V = ((((value1^value2)>>31) != 0) && (((res^value1)>>31) == 0));
		res -= (opcode == ALU_RSBC)? FLAGS.C: 1;
		FLAGS.C = (res < (~value1) || res < value2);
		break;
	case ALU_ADD:
	case ALU_ADC:
	case ALU_CMP_ADD:
		FLAGS.V = ((((value1^value2)>>31) == 0) && (((res^value1)>>31) != 0));
		if (opcode == ALU_ADC) res -= FLAGS.C;
		FLAGS.C = (res < value1 || res < value2);
		break;
	default:
		FLAGS.C = shift_c;
		break;
	}
}

const static char *alu_inst_fmt[] = {
	"ALU %s r%d = r%d %s (r%d %s %d)\n",
	"ALU %s r%d = r%d %s (r%d %s r%d)\n"
};
const static char *cmp_inst_fmt[] = {
	"ALU update-flags r%d %s (r%d %s %d)\n",
	"ALU update-flags r%d %s (r%d %s r%d)\n"
};
const static char *cmov_inst_fmt[] = {
	"ALU %s when %s r%d = %s (r%d %s %d)\n",
	"ALU %s when %s r%d = %s (r%d %s r%d)\n"
};
static uint32_t
exec_alu_reg_imm(
	reg_num rd,
	reg_num rs1,
	reg_num rs2,
	uint32_t ss,
	uint32_t regimm,
	shift_type shift,
	alu_opcode opcode,
	uint32_t flag)
{
	uint32_t write_res = 0;
	uint32_t latency = 0;
	if (opcode < ALU_CMP_AND || opcode == ALU_OR || opcode == ALU_AND_NOT){
		write_res = 1;
		latency = ALU_CYCLES;
		printf(alu_inst_fmt[regimm], flag?"update-flags":"",
			rd, rs1, alu_op_str[opcode], rs2, shift_str[shift], ss);
	}else if (opcode >= ALU_CMP_AND && opcode <= ALU_CMP_ADD){
		printf(cmp_inst_fmt[regimm],
			rs1, alu_op_str[opcode], rs2, shift_str[shift], ss);
		flag = 1;
		latency = ALU_CYCLES;
	}
	else{
		cond_code cond = (rs1 & 0x10)? rs1 & 0xf: COND_AL;
		write_res = exec_test_cond(cond);
		printf(cmov_inst_fmt[regimm], flag?"update-flags":"",
			cond_str[cond], rd, alu_op_str[opcode], rs2, shift_str[shift], ss);
		if (!write_res){
			latency = CMOV_NOWR_CYCLES;
			goto exec_alu_reg_imm_end;
		}else
			latency = CMOV_WRITE_CYCLES;
	}
	uint32_t value1 = REG(rs1), shift_c;
	uint32_t value2 = exec_shift(REG(rs2), regimm? REG(ss): ss, shift, &shift_c);
	uint32_t res = exec_alu_compute(value1, value2, opcode);
	if (flag) exec_update_flags(res, value1, value2, shift_c, opcode);
	if (write_res) REG(rd) = res;
exec_alu_reg_imm_end:
	return latency;
}

static uint32_t
exec_alu_imm_rotate(
	reg_num rd,
	reg_num rs,
	uint32_t imm9,
	uint32_t rotate,
	alu_opcode opcode,
	uint32_t flag)
{
	if (opcode < ALU_CMP_AND || opcode == ALU_OR || opcode == ALU_AND_NOT)
		printf("%supdate-flags r%d = r%d %s (%d <> %d)\n", flag?"":"no-",
			rd, rs, alu_op_str[opcode], imm9, rotate);
	else if (opcode >= ALU_CMP_AND && opcode <= ALU_CMP_ADD)
		printf("update-flags r%d %s (%d <> %d)\n",
			rs, alu_op_str[opcode], imm9, rotate);
	else
		printf("%supdate-flags when %s r%d = %s (%d <> %d)\n", flag?"":"no-",
			cond_str[(rs & 0x10)? rs & 0xf: COND_AL], rd, alu_op_str[opcode], imm9, rotate);
	return 0;
}

static uint32_t
exec_reg_shift_ld_st(
	reg_num rbase,
	reg_num rindex,
	reg_num rdata,
	uint32_t imm5,
	shift_type shift,
	uint32_t loadstore,
	uint32_t writeback,
	uint32_t byteword,
	uint32_t addsub,
	uint32_t prepost)
{
	if (!prepost) writeback = 1;
	printf("%s %s data:r%d addr:r%d %s-%s offset: (r%d %s %d) %swrite-back\n",
		loadstore?"load":"store", byteword?"byte":"word", rdata, rbase,
		prepost?"pre":"post", addsub?"add":"sub", rindex, shift_str[shift],
		imm5, writeback?"":"no-");
	return 0;
}

static uint32_t
exec_half_ext_ld_st(
	reg_num rbase,
	reg_num roffset,
	reg_num rdata,
	uint32_t imm5,
	uint32_t loadstore,
	uint32_t writeback,
	uint32_t immreg,
	uint32_t addsub,
	uint32_t prepost,
	uint32_t bytehalf,
	uint32_t signzero)
{
	if (!prepost) writeback = 1;
	if (immreg)
		printf("%s %s-extend %s data:r%d addr:r%d %s-%s offset: %d %swrite-back\n",
			loadstore?"load":"store", signzero?"sign":"zero", bytehalf?"byte":"half",
			rdata, rbase, prepost?"pre":"post", addsub?"add":"sub", (imm5<<5)+roffset,
			writeback?"":"no-");
	else
		printf("%s %s-extend %s data:r%d addr:r%d %s-%s offset: r%d %swrite-back\n",
			loadstore?"load":"store", signzero?"sign":"zero", bytehalf?"byte":"half",
			rdata, rbase, prepost?"pre":"post", addsub?"add":"sub", roffset, writeback?"":"no-");
	return 0;
}

static uint32_t
exec_imm_ld_st(
	reg_num rbase,
	reg_num rdata,
	uint32_t imm14,
	uint32_t loadstore,
	uint32_t writeback,
	uint32_t byteword,
	uint32_t addsub,
	uint32_t prepost)
{
	if (!prepost) writeback = 1;
	printf("%s data:r%d addr:r%d %s-%s offset:%d %swrite-back\n",
		loadstore?"load":"store", rdata, rbase, prepost?"pre":"post",
		addsub?"add":"sub", imm14, writeback?"":"no-");
	return 0;
}

static uint32_t
exec_multi_ld_st(
	reg_num rbase,
	uint32_t reglist,
	uint32_t loadstore,
	uint32_t writeback,
	uint32_t addsub,
	uint32_t prepost)
{
	if (!prepost) writeback = 1;
	printf("multiple %s list:%08x addr:r%d %s-%s %swrite-back\n",
		loadstore?"load":"store", reglist, rbase, prepost?"pre":"post",
		addsub?"add":"sub", writeback?"":"no-");
	return 0;
}

static uint32_t
exec_cond_branch(
	cond_code cond,
	int32_t offset,
	uint32_t link)
{
	offset *= 4;
	printf("branch %s, %d, %s\n", cond_str[cond], offset, link?"link":"not-link");
	if (!exec_test_cond(cond))
		return BR_NTAKEN_CYCLES;
	// branch taken.
	if (link)
		REG(LR_NUM) = REG(PC_NUM);
	REG(PC_NUM) += offset;
	return BR_TAKEN_CYCLES;
}

static uint32_t
exec_software_trap(
	uint32_t trapno)
{
	printf("software trap 0x%x\n", trapno);
	exec_cpu->reg[0] = syscall((syscall_num)trapno, exec_cpu);
	exec_cpu->stats.nsyscall ++;
	return SYSCALL_CYCLES;
}

uint32_t
inst_dispatch(
	cpu_t *cpu,
	general_inst inst)
{
	exec_cpu = cpu;
	uint32_t latency = 0;
	switch ((inst_ident)inst.id){
		case REG_ALU:
			if ((inst.func & FUNC_NOT_ALU) == 0){
				latency = exec_alu_reg_imm(inst.rd, inst.rn, inst.rm, inst.rs, 
					inst.func & FUNC_REG_IMM, FUNC_GET_SHIFT(inst.func), inst.opcode, inst.flag);
			}
			else if (inst.func == FUNC_MUL_BR_CNT){
				if (inst.opcode & OP_BRANCH_CNT){
					if (inst.opcode & OP_CNT)
						latency = exec_cnt_bits(inst.rd, inst.rm, inst.opcode & OP_CNT_ZERO);
					else latency = exec_branch(inst.rm, inst.flag);
				}
				else if (inst.opcode & OP_MUL_LONG)
					latency = exec_long_mul(inst.rd, inst.rs, inst.rn, inst.rm,
						inst.opcode & OP_MUL_UNSIGN, inst.opcode & OP_MUL_ADD, inst.flag);
				else
					latency = exec_mul(inst.rd, inst.rn, inst.rm, inst.rs,
						inst.opcode & OP_MUL_ADD, inst.flag);
			}
			else
				illegal_inst(&inst);
			break;
		case IMM_ALU:
		{
			imm9_inst *i_ptr = (imm9_inst*)(&inst);
			latency = exec_alu_imm_rotate(i_ptr->rd, i_ptr->rn, i_ptr->imm, i_ptr->rotate,
				i_ptr->opcode, i_ptr->flag);
			break;
		}
		case REG_LDST:
		{
			reg_ldst_inst *i_ptr = (reg_ldst_inst*)(&inst);
			if ((i_ptr->func & (~FUNC_SHIFT_MASK)) == 0)
				latency = exec_reg_shift_ld_st(i_ptr->rn, i_ptr->rm, i_ptr->rd, i_ptr->rs,
					FUNC_GET_SHIFT(inst.func), i_ptr->loadstore, i_ptr->writeback,
					i_ptr->byteword, i_ptr->addsub, i_ptr->prepost);
			else
				latency = exec_half_ext_ld_st(i_ptr->rn, i_ptr->rm, i_ptr->rd, i_ptr->rs,
					i_ptr->loadstore, i_ptr->writeback, i_ptr->byteword, i_ptr->addsub,
					i_ptr->prepost, i_ptr->func & FUNC_BYTE_HALF, i_ptr->func & FUNC_SIGN_EXT);
			break;
		}
		case IMM_LDST:
		{
			imm14_inst *i_ptr = (imm14_inst*)(&inst);
			latency = exec_imm_ld_st(i_ptr->rn, i_ptr->rd, i_ptr->imm, i_ptr->loadstore,
				i_ptr->writeback, i_ptr->byteword, i_ptr->addsub, i_ptr->prepost);
			break;
		}
		case MUL_LDST:
		{
			multi_ldst_inst *i_ptr = (multi_ldst_inst*)(&inst);
			uint32_t reglist = ((i_ptr->rlisthigh)<<6) | (i_ptr->rlistlow);
			if (i_ptr->func == 1) reglist <<= 16;
			latency = exec_multi_ld_st(i_ptr->rn, reglist, i_ptr->loadstore, i_ptr->writeback,
				i_ptr->addsub, i_ptr->prepost);
			break;
		}
		case COND_BRANCH:
		{
			cond_branch_inst *i_ptr = (cond_branch_inst*)(&inst);
			latency = exec_cond_branch(i_ptr->cond, i_ptr->offset, i_ptr->link);
			break;
		}
		case SOFT_TRAP:
		{
			soft_trap_inst *i_ptr = (soft_trap_inst*)(&inst);
			if (i_ptr->opcode != 0xff)
				illegal_inst(&inst);
			else
				latency = exec_software_trap(i_ptr->trapno);
			break;
		}
		default:
			illegal_inst(&inst);
			break;
	}
	return latency;
}