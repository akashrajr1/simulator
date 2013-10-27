#include "inc/instruction.h"
#include <stdio.h>
void
illegal_inst()
{
	printf("***Unknow instruction***\n");
}

void
exec_branch(
	reg_num reg, 
	uint32_t link)
{
	printf("unconditional branch to r%d, %s.\n", reg, link?"link":"no-link");
}
void 
exec_mul(
	reg_num rd, 
	reg_num rs1,
	reg_num rs2,
	reg_num rs3, 
	uint32_t add,
	uint32_t flag)
{
	if (add)
		printf("multiply with add r%d = r%d * r%d + r%d, %supdate-flags\n",
			rd, rs1, rs2, rs3, flag?"":"no-");
	else
		printf("multiply r%d = r%d * r%d, %supdate-flags\n",
			rd, rs1, rs2, flag?"":"no-");
}
void
exec_long_mul(
	reg_num rd_low, 
	reg_num rd_high, 
	reg_num rs1, 
	reg_num rs2, 
	uint32_t unsign, 
	uint32_t add, 
	uint32_t flag)
{
	if (add)
		printf("%s long multiply with add {r%d,r%d} = r%d * r%d + {r%d,r%d}, %supdate-flags\n",
			unsign?"unsigned":"signed", rd_high, rd_low, rs1, rs2, rd_high, rd_low, flag?"":"no-");
	else
		printf("%s long multiply with add {r%d,r%d} = r%d * r%d, %supdate-flags\n",
			unsign?"unsigned":"signed", rd_high, rd_low, rs1, rs2, flag?"":"no-");
}

void
exec_alu_imm_shift(
	reg_num rd,
	reg_num rs1,
	reg_num rs2,
	uint32_t imm5,
	shift_type shift,
	alu_opcode opcode,
	uint32_t flag)
{
	if (opcode < ALU_CMP_AND || opcode == ALU_OR || opcode == ALU_AND_NOT)
		printf("ALU %s r%d = r%d %s (r%d %s %d)\n", flag?"update-flags":"", 
			rd, rs1, alu_op_str[opcode], rs2, shift_str[shift], imm5);
	else if (opcode >= ALU_CMP_AND && opcode <= ALU_CMP_ADD)
		printf("ALU update-flags r%d %s (r%d %s %d)\n", 
			rs1, alu_op_str[opcode], rs2, shift_str[shift], imm5);
	else
		printf("ALU %s when %s r%d = %s (r%d %s %d)\n", flag?"update-flags":"", 
			cond_str[(rs1 & 0x10)? rs1 & 0xf: COND_AL], rd, alu_op_str[opcode],
			rs2, shift_str[shift], imm5);
}

void exec_alu_imm_rotate(
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
}
void
exec_alu_reg_shift(
	reg_num rd,
	reg_num rs1,
	reg_num rs2,
	reg_num rss,
	shift_type shift,
	alu_opcode opcode,
	uint32_t flag)
{
	if (opcode < ALU_CMP_AND || opcode == ALU_OR || opcode == ALU_AND_NOT)
		printf("%supdate-flags r%d = r%d %s (r%d %s r%d)\n", flag?"":"no-",
			rd, rs1, alu_op_str[opcode], rs2, shift_str[shift], rss);
	else if (opcode >= ALU_CMP_AND && opcode <= ALU_CMP_ADD)
		printf("update-flags r%d %s (r%d %s r%d)\n",
			rs1, alu_op_str[opcode], rs2, shift_str[shift], rss);
	else
		printf("%supdate-flags when %s r%d = %s (r%d %s r%d)\n", flag?"":"no-",
			cond_str[(rs1 & 0x10)? rs1 & 0xf: COND_AL], rd, alu_op_str[opcode],
			rs2, shift_str[shift], rss);
}

void
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
}

void exec_half_ext_ld_st(
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
}
void
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
}

void
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
}

void
exec_cond_branch(
	cond_code cond,
	int32_t offset,
	uint32_t link)
{
	offset *= 2;
	printf("branch %s, %d, %s\n", cond_str[cond], offset, link?"link":"not-link");
}

void
exec_software_trap(
	uint32_t trapno)
{
	printf("software trap 0x%x\n", trapno);
}

void
dispatch_inst(
	general_inst inst)
{
	switch ((inst_ident)inst.id){
		case REG_ALU:
			if ((inst.func & (~FUNC_SHIFT_MASK)) == 0){
				exec_alu_imm_shift(inst.rd, inst.rn, inst.rm, inst.rs, 
					FUNC_GET_SHIFT(inst.func), inst.opcode, inst.flag);
			}
			else if ((inst.func & (~FUNC_SHIFT_MASK)) == 1){
				exec_alu_reg_shift(inst.rd, inst.rn, inst.rm, inst.rs,
					FUNC_GET_SHIFT(inst.func), inst.opcode, inst.flag);
			}
			else if (inst.func == FUNC_MUL_BR){
				if (inst.opcode == OP_BRANCH)
					exec_branch(inst.rm, inst.flag);
				else if (inst.opcode & MUL_LONG)
					exec_long_mul(inst.rd, inst.rs, inst.rn, inst.rm,
						inst.opcode & MUL_UNSIGN, inst.opcode & MUL_ADD, inst.flag);
				else
					exec_mul(inst.rd, inst.rn, inst.rm, inst.rs,
						inst.opcode & MUL_ADD, inst.flag);
			}
			else
				illegal_inst();
			break;
		case IMM_ALU:
		{
			imm9_inst *i_ptr = (imm9_inst*)(&inst);
			exec_alu_imm_rotate(i_ptr->rd, i_ptr->rn, i_ptr->imm, i_ptr->rotate,
				i_ptr->opcode, i_ptr->flag);
			break;
		}
		case REG_LDST:
		{
			reg_ldst_inst *i_ptr = (reg_ldst_inst*)(&inst);
			if ((i_ptr->func & (~FUNC_SHIFT_MASK)) == 0)
				exec_reg_shift_ld_st(i_ptr->rn, i_ptr->rm, i_ptr->rd, i_ptr->rs,
					FUNC_GET_SHIFT(inst.func), i_ptr->loadstore, i_ptr->writeback,
					i_ptr->byteword, i_ptr->addsub, i_ptr->prepost);
			else
				exec_half_ext_ld_st(i_ptr->rn, i_ptr->rm, i_ptr->rd, i_ptr->rs,
					i_ptr->loadstore, i_ptr->writeback, i_ptr->byteword, i_ptr->addsub,
					i_ptr->prepost, i_ptr->func & FUNC_BYTE_HALF, i_ptr->func & FUNC_SIGN_EXT);
			break;
		}
		case IMM_LDST:
		{
			imm14_inst *i_ptr = (imm14_inst*)(&inst);
			exec_imm_ld_st(i_ptr->rn, i_ptr->rd, i_ptr->imm, i_ptr->loadstore,
				i_ptr->writeback, i_ptr->byteword, i_ptr->addsub, i_ptr->prepost);
			break;
		}
		case MUL_LDST:
		{
			multi_ldst_inst *i_ptr = (multi_ldst_inst*)(&inst);
			uint32_t reglist = ((i_ptr->rlisthigh)<<6) | (i_ptr->rlistlow);
			if (i_ptr->func == 1) reglist <<= 16;
			exec_multi_ld_st(i_ptr->rn, reglist, i_ptr->loadstore, i_ptr->writeback,
				i_ptr->addsub, i_ptr->prepost);
			break;
		}
		case COND_BRANCH:
		{
			cond_branch_inst *i_ptr = (cond_branch_inst*)(&inst);
			exec_cond_branch(i_ptr->cond, i_ptr->offset, i_ptr->link);
			break;
		}
		case SOFT_TRAP:
		{
			soft_trap_inst *i_ptr = (soft_trap_inst*)(&inst);
			if (i_ptr->opcode != 0xff)
				illegal_inst();
			else
				exec_software_trap(i_ptr->trapno);
			break;
		}
		default:
			illegal_inst();
			break;
	}
}