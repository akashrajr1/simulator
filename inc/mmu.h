#ifndef _UC32SIM_MMU_H
#define _UC32SIM_MMU_H
/*
 * Paging: 4K pages, 2-level page tables
 * 1st level page table:
 *		4-kilobytes, 1024 entries
 *		indexed by 10-most-significant-bits of virtual address [31-22]
 *		4-byte for each entry:
 *			[31-12] 20-most-significant-bits of 2nd level page table base address
 * 2nd level page table:
 *		4-kilobytes, 1024 entries
 *		indexed by 10-middle-bits of virtual address [21-12]
 *		4-byte for each entry:
 *			[31-12] 20-most-significant-bits of physical page address
 *
 * TLB: Instruction-TLB / Data-TLB
 *		8-entries each, full-set-associative
 *			20-bits virtual page no. -> 20-bits physical page no.
 *			eviction: Round-Robin/LRU
 *
 * Use virtual page on host as physcal page for the program being simulated.
 */
#include <stdint.h>

typedef enum{
	MM_BYTE = 1,
	MM_HALFWORD = 2,
	MM_WORD = 4
} MemSize;

// int mm_store(uintptr_t vaddr, uint32_t value, MemSize mmsz);
// int mm_load(uintptr_t vaddr, uint32_t* value_store, MemSize mmsz);
#endif /* !_UC32SIM_MMU_H */