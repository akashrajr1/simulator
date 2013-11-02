#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "inc/mmu.h"
#include "inc/cache.h"
#include "inc/error.h"

static mmu_stats stats;

mmu_t *
mmu_init(mmu_latency *latency_ptr)
{
	mmu_t *mmu = (mmu_t*)malloc(sizeof(mmu));
	memset(mmu, 0, sizeof(mmu_t));
	if (latency_ptr)
		mmu->latency = *latency_ptr;
	else{
		mmu->latency.itlb_hit = ITLB_HIT_CYCLE;
		mmu->latency.dtlb_hit = DTLB_HIT_CYCLE;
		mmu->latency.mem_read = MEM_READ_CYCLE;
		mmu->latency.mem_write = MEM_WRITE_CYCLE;
		mmu->latency.paging = PAGING_CYCLE;
	}
	return mmu;
}

static void 
mmu_append_pgtbl(
	mmu_t *mmu,
	pgtbl_t *pgtbl)
{
	if (mmu->listend == NULL){
		mmu->ptlist = pgtbl;
		mmu->listend = pgtbl;
	}else{
		mmu->listend->next = pgtbl;
		mmu->listend = pgtbl;
	}
	memset(mmu->listend, 0, sizeof(pgtbl_t));
	mmu->stats.npgtbls ++;
}

void *
mmu_get_tlb(
	mmu_t *mmu,
	uintptr_t va,
	mem_type mtype,
	uint32_t *latency_store)
{
	return NULL;
}

void *
mmu_get_page(
	mmu_t *mmu,
	uintptr_t va,
	int alloc,
	uint32_t *latency_store)
{
	void *pa = NULL;
	pde_t *pde_ptr = &(mmu->pd_entries[PDX(va)]);
	if (*pde_ptr == NULL){	// pde doesn't exist, allocate a new page table
		if (!alloc) goto mmu_get_page_end;
		*pde_ptr = (pde_t)malloc(sizeof(pgtbl_t));
		mmu_append_pgtbl(mmu, (pgtbl_t *)(*pde_ptr));
	}
	pte_t *pte_ptr = (pte_t*)(*pde_ptr) + PTX(va);
	if (*pte_ptr == NULL){	// pte doesn't exist, allocate a page
		if (!alloc) goto mmu_get_page_end;
		*pte_ptr = malloc(PGSIZE);
		mmu->stats.npages ++;
	}
	pa = *pte_ptr;
mmu_get_page_end:
	return pa;
}

void
mmu_destroy(
	mmu_t *mmu)
{
	pgtbl_t *ptl_ptr;
	int i;
	while (mmu->ptlist){
		ptl_ptr = mmu->ptlist;
		mmu->ptlist = ptl_ptr->next;
		for (i = 0; i < NPTENTRIES; i++)
			free(ptl_ptr->pt_entries[i]);	// OK to be NULL
		free(ptl_ptr);
	}
	free(mmu);
}

void
mm_store(
	mmu_t *mmu,
	uintptr_t va,
	const uint32_t *data,
	uint32_t *latency_store)
{
	assert(va % CACHE_LINE_SIZE == 0);
	uint32_t *pa = mmu_get_page(mmu, va, 0, NULL);
	if (pa == NULL)
		error_process(ADDR_INVAL, va);
	pa = (void*)pa + PGOFF(va);
	int i;
	for (i = 0; i < CACHE_LINE_WORDS; i++)
		pa[i] = data[i];
}

void
mm_load(
	mmu_t *mmu,
	uintptr_t va,
	uint32_t *data,
	uint32_t *latency_store)
{
	assert(va % CACHE_LINE_SIZE == 0);
	uint32_t *pa = mmu_get_page(mmu, va, 0, NULL);
	if (pa == NULL)
		error_process(ADDR_INVAL, va);
	pa = (void*)pa + PGOFF(va);
	int i;
	for (i = 0; i < CACHE_LINE_WORDS; i++)
		data[i] = pa[i];
}