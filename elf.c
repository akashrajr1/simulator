#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "inc/elf.h"
#include "inc/instruction.h"

int read_elf(const char* filename){
	static Elf elfhdr;
	static Proghdr ph;
	FILE *elf = fopen(filename, "rb");
	if (!elf){
		perror("read_elf:");
		exit(1);
	}
	fread(&elfhdr, sizeof(Elf), 1, elf);
	if (elfhdr.e_id.e_magic != ELF_MAGIC
		|| elfhdr.e_id.e_ident[EI_CLASS] != ELFCLASS32
		|| elfhdr.e_id.e_ident[EI_DATA] != ELFDATA2LSB
		|| elfhdr.e_type != ET_EXEC
		|| elfhdr.e_machine != EM_UNICORE){
		fprintf(stderr, "read_elf: invalid ELF %s\n", filename);
		exit(1);	
	}
	printf("32-bit LSB ELF.\n%d program headers, starting from 0x%x.\n",
		elfhdr.e_phnum, elfhdr.e_phoff);
	int i;
	fseek(elf, elfhdr.e_phoff, SEEK_SET);
	for (i = 0; i < elfhdr.e_phnum; i++){
		fread(&ph, sizeof(Proghdr), 1, elf);
		if (ph.p_type != PT_LOAD){
			fprintf(stderr, "Only static loading is supported: %d\n", ph.p_type);
			continue;
		}
		printf("offset: %08x  va: %08x  filesz: %08x  memsz: %08x  flags: %x  align: %08x\n",
			ph.p_offset, ph.p_va, ph.p_filesz, ph.p_memsz, ph.p_flags, ph.p_align);
		//
		break;
		//
	}
	printf("execution entry: 0x%08x\n", elfhdr.e_entry);

#if 1
#define N_INST 1024
	void *inst_buf = malloc(4*N_INST);
	fseek(elf, elfhdr.e_entry - ph.p_va + ph.p_offset, SEEK_SET);
	fread(inst_buf, 4, N_INST, elf);
	for (i = 0; i < N_INST; i++){
		general_inst inst = *((general_inst*)inst_buf+i);
		printf("0x%x %08x: ", elfhdr.e_entry + 4*i, *((int*)inst_buf+i));
		if (4*i == 0x78){
			dispatch_inst(inst);
			continue;	
		}
		dispatch_inst(inst);
	}
#endif
	return 0;
}