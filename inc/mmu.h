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

#define PTXSHIFT	12
#define PDXSHIFT	22

// virtual page number
#define PGNUM(va)	(((uintptr_t) (va)) >> PTXSHIFT)
// page directory index
#define PDX(va)		((((uintptr_t) (va)) >> PDXSHIFT) & 0x3FF)
// page table index
#define PTX(va)		((((uintptr_t) (va)) >> PTXSHIFT) & 0x3FF)
// offset in page
#define PGOFF(va)	(((uintptr_t) (va)) & 0xFFF)
// construct linear address from indexes and offset
#define PGADDR(d, t, o)	((void*) ((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))
// Page directory and page table constants.
#define NPDENTRIES	1024		// page directory entries per page directory
#define NPTENTRIES	1024		// page table entries per page table
#define PGSIZE		4096		// bytes mapped by a page
#define PGSHIFT		12			// log2(PGSIZE)

#define NITLB	8
#define NDTLB	8

#define ITLB_HIT_CYCLE		1
#define	DTLB_HIT_CYCLE		1
#define MEM_READ_CYCLE		30
#define MEM_WRITE_CYCLE		15
#define PAGING_CYCLE		(2 * MEM_READ_CYCLE + 4)

typedef struct{
	uint32_t itlb_hit;
	uint32_t dtlb_hit;
	uint32_t mem_read;
	uint32_t mem_write;
	uint32_t paging;
} mmu_latency;

typedef struct{
	uint32_t npages;
	uint32_t npgtbls;
	uint32_t nitlb_miss;
	uint32_t nitlb_hit;
	uint32_t ndtlb_miss;
	uint32_t ndtlb_hit;
} mmu_stats;

typedef void* pte_t;
typedef pte_t* pde_t;

typedef struct{
	uint32_t vpn;
	void *pa;
	uint32_t valid;
} tlb_t;

struct pgtbl{
	pte_t pt_entries[NPTENTRIES];
	struct pgtbl *next;
	struct pgtbl *prev;
};
typedef struct pgtbl pgtbl_t;

typedef struct{
	pde_t pd_entries[NPDENTRIES];
	tlb_t itlb[NITLB];
	uint32_t i_evic_cntr;
	tlb_t dtlb[NDTLB];
	uint32_t d_evic_cntr;
	pgtbl_t *ptlist;
	pgtbl_t *listend;
	mmu_stats stats;
	mmu_latency latency;
} mmu_t;

typedef enum {
	MEM_INST = 0,
	MEM_DATA
} mem_type;


/* functions for the simulator */
mmu_t *mmu_init(mmu_latency *latency_ptr);
void mmu_destroy(mmu_t *mmu);

/*  
 * the cache should first try mmu_get_tlb, if failed, call mmu_get_page
 * latency cycle count written in *latency_store
 *
 * the simulator also use mmu_get_page to allocate pages.
 */
void *mmu_get_tlb(mmu_t *mmu, uintptr_t va, mem_type mtype, uint32_t *latency_store);
void *mmu_get_page(mmu_t *mmu, uintptr_t va, int alloc, uint32_t *latency_store);
/* 
 * functions for the cache module 
 * MMU operates on 32-bytes (8-words) cache lines.
 * va should be aligned on 32-byte boundaries.
 * data points memory in host.
 */
void mm_store(mmu_t *mmu, uintptr_t va, const uint32_t *data, uint32_t *latency_store);
void mm_load(mmu_t *mmu, uintptr_t va, uint32_t *data, uint32_t *latency_store);
#endif /* !_UC32SIM_MMU_H */