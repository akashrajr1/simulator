#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "inc/mmu.h"
#include "inc/cache.h"
#include "inc/error.h"

static uint32_t rand_num;
#define MMU_LCG_MUL	1664525
#define MMU_LCG_INC	1013904223
mmu_t *
mmu_init(
	mmu_latency *latency_ptr, 
	tlb_evict eviction)
{
	/**** DAMN!!! code was:
	 **** mmu_t *mmu = (mmu_t*)malloc(sizeof(mmu));
	 **** bloody hell, see that `sizeof(mmu)`?
	 http://stackoverflow.com/questions/1868719/sigsegv-seemingly-caused-by-printf
	 GuardMalloc solved everything ***/
	mmu_t *mmu = (mmu_t*)malloc(sizeof(mmu_t));
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
	mmu->eviction = eviction;
	if (mmu->eviction == TLB_EVICT_RAND)
		rand_num = (uint32_t)time(NULL);
	else{
		mmu->i_evic_cntr = (uint32_t)time(NULL) * MMU_LCG_MUL + MMU_LCG_INC;
		mmu->d_evic_cntr = mmu->i_evic_cntr * MMU_LCG_MUL + MMU_LCG_INC;
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
mmu_paging(
	mmu_t *mmu,
	uint32_t va,
	mem_type type,
	int32_t *latency_store)
{
	tlb_t *tlb = NULL;
	uint32_t ntlb = 0;
	if (type == MEM_INST){
		tlb = mmu->itlb;
		ntlb = NITLB;
	} else if (type == MEM_DATA){
		tlb = mmu->dtlb;
		ntlb = NDTLB;
	}
	int i;
	int32_t latency = (type == MEM_INST)?
		mmu->latency.itlb_hit: mmu->latency.dtlb_hit;
	void *pa = NULL;
	uint32_t vpn = PGNUM(va);
	for (i = 0; i < ntlb; i++)
		if (tlb[i].valid && tlb[i].vpn == vpn){
			pa = tlb[i].pa;
			if (type == MEM_INST) mmu->stats.nitlb_hit ++;
			else mmu->stats.ndtlb_hit ++;
			goto mmu_paging_end;
		}
	// tlb miss, try mmu_get_page
	if (type == MEM_INST) mmu->stats.nitlb_miss ++;
	else mmu->stats.ndtlb_miss ++;
	latency += mmu->latency.paging;
	pa = mmu_get_page(mmu, va, 0);
	if (pa == NULL){ // page fault!
		error_log(ADDR_INVAL, va);
		latency = -1;
		goto mmu_paging_end;
	}
	for (i = 0; i < ntlb; i++)
		if (tlb[i].valid == 0){
			tlb[i].vpn = vpn;
			tlb[i].pa = pa;
			tlb[i].valid = 1;
			goto mmu_paging_end;
		}
	// all entries are used, find one to evict.
	// victim should be unsigned
	uint32_t victim = 0;
	if (mmu->eviction == TLB_EVICT_SEQ){
		if (type == MEM_INST) victim = mmu->i_evic_cntr++;
		else victim = mmu->d_evic_cntr++;
	}else if (mmu->eviction == TLB_EVICT_RAND){
		victim = rand_num;
		rand_num = rand_num * MMU_LCG_MUL + MMU_LCG_INC;
	}
	victim %= ntlb;
	tlb[victim].vpn = vpn;
	tlb[victim].pa = pa;
mmu_paging_end:
	if (latency_store != NULL) *latency_store = latency;
	return pa;
}

void *
mmu_get_page(
	mmu_t *mmu,
	uint32_t va,
	int alloc)
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
		memset(*pte_ptr, 0, PGSIZE);
		// printf("allocate page at va:0x%08x -> %p\n", va & ~(0xfff), *pte_ptr);
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

uint32_t
mm_store(
	mmu_t *mmu,
	void *pa,
	const void *data)
{
	mmu->stats.nstore ++;
	memcpy(pa, data, CACHE_LINE_SIZE);
	return mmu->latency.mem_write;
}

uint32_t
mm_load(
	mmu_t *mmu,
	void *pa,
	void *data)
{
	mmu->stats.nload ++;
	memcpy(data, pa, CACHE_LINE_SIZE);
	return mmu->latency.mem_read;
}