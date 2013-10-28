#ifndef _UC32SIM_ELF_H
#define _UC32SIM_ELF_H

#include "mmu.h"
#include <stdint.h>


#define ELF_MAGIC 0x464C457FU	/* "\x7FELF" in little endian */

#define EI_CLASS	4
#define EI_DATA  	5

#define ELFCLASS32	1
#define ELFDATA2LSB	1

#define ET_EXEC		2

#define EM_UNICORE	110

typedef struct {
	union {
		uint32_t e_magic;	// must equal ELF_MAGIC
		uint8_t e_ident[16];
	} e_id;
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf;

#define PT_LOAD     1
#define PT_NOTE     4
#define PF_X        1
#define PF_W        2
#define PF_R        4

typedef struct {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_va;
	uint32_t p_pa;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} Proghdr;


typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} Secthdr;

// Values for Proghdr::p_type
#define ELF_PROG_LOAD		1

// Flag bits for Proghdr::p_flags
#define ELF_PROG_FLAG_EXEC	1
#define ELF_PROG_FLAG_WRITE	2
#define ELF_PROG_FLAG_READ	4

// Values for Secthdr::sh_type
#define ELF_SHT_NULL		0
#define ELF_SHT_PROGBITS	1
#define ELF_SHT_SYMTAB		2
#define ELF_SHT_STRTAB		3

// Values for Secthdr::sh_name
#define ELF_SHN_UNDEF		0


FILE *elf_check(const char* filename, Elf **elf_store);
void elf_load(FILE *elf, Elf *elfhdr, mmu_t *mmu);

#endif /* !_UC32SIM_ELF_H */
