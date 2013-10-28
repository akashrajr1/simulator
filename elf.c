#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "inc/mmu.h"
#include "inc/elf.h"
#include "inc/instruction.h"

static inline uint32_t min(uint32_t a, uint32_t b){
	return a < b? a: b;
}
static inline uint32_t max(uint32_t a, uint32_t b){
	return a > b? a: b;
}

FILE *elf_check(const char* filename, Elf **elf_store){
	static Elf elfhdr;
	Proghdr ph;
	FILE *elf = fopen(filename, "rb");
	if (!elf){
		perror("read_elf:");
		return elf;
	}
	fread(&elfhdr, sizeof(Elf), 1, elf);
	if (elfhdr.e_id.e_magic != ELF_MAGIC
		|| elfhdr.e_id.e_ident[EI_CLASS] != ELFCLASS32
		|| elfhdr.e_id.e_ident[EI_DATA] != ELFDATA2LSB
		|| elfhdr.e_type != ET_EXEC
		|| elfhdr.e_machine != EM_UNICORE){
		fprintf(stderr, "read_elf: invalid ELF %s\n", filename);
		fclose(elf);
		return NULL;
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
	}
	printf("execution entry: 0x%08x\n", elfhdr.e_entry);
	*elf_store = &elfhdr;
	return elf;
#if 0
	int n_inst = ph.p_memsz / 4;
	void *inst_buf = malloc(4*n_inst);
	fseek(elf, ph.p_offset, SEEK_SET);
	fread(inst_buf, 4, n_inst, elf);
	for (i = 0; i < n_inst; i++){
		general_inst inst = *((general_inst*)inst_buf+i);
		printf("0x%x %08x: ", ph.p_va + 4*i, *((int*)inst_buf+i));
		dispatch_inst(inst);
	}
	return 0;
#endif
}

void elf_load(FILE *elf, Elf *elfhdr, mmu_t *mmu){
	int i;
	for (i = 0; i < elfhdr->e_phnum; i++){
		Proghdr ph;
		fseek(elf, elfhdr->e_phoff + i * sizeof(Proghdr), SEEK_SET);
		fread(&ph, sizeof(Proghdr), 1, elf);
		if (ph.p_type != PT_LOAD)
			continue;
		uint32_t va = ph.p_va -  ph.p_va % PGSIZE;
		for (; va < ph.p_memsz + ph.p_va; va += PGSIZE){
			void *pa = mmu_allocate_page(mmu, va);
			// void *pa_start = va < ph.p_va? pa + ph.p_va - va: pa;
			memset(pa, 0, PGSIZE);
			// printf("----in virutal page %x, start from %lx, cleared %lx bytes\n", va, va + pa_start - pa, PGSIZE - (pa_start - pa));
			if (va < ph.p_filesz + ph.p_va){
				int read_start = max(ph.p_offset + va - ph.p_va, ph.p_offset);
				int short_of = (va < ph.p_va)?(ph.p_va - va):0;
				int read_size = min(PGSIZE - short_of, ph.p_filesz - (va - (ph.p_va - ph.p_va % PGSIZE)));
				fseek(elf, read_start, SEEK_SET);
				fread(pa + short_of, 1, read_size, elf);
				printf("----in virtual page %x, read file from %x, #%x bytes\n", va, read_start, read_size);
			}
		}
		// printf("loaded %d bytes to va %08x (%d bytes in memory)\n", ph.p_filesz, ph.p_va, ph.p_memsz);
	}
}