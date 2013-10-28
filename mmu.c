#include <stdlib.h>
#include <string.h>
#include "inc/mmu.h"


mmu_t *mmu_init(){
	mmu_t *mmu = (mmu_t*)malloc(sizeof(mmu));
	memset(mmu, 0, sizeof(mmu_t));
	return mmu;
}

static void mmu_append_pgtbl(mmu_t *mmu, pgtbl_t *pgtbl){
	if (mmu->listend == NULL){
		mmu->ptlist = pgtbl;
		mmu->listend = pgtbl;
	}else{
		mmu->listend->next = pgtbl;
		mmu->listend = pgtbl;
	}
	memset(mmu->listend, 0, sizeof(pgtbl_t));
}

void *mmu_allocate_page(mmu_t *mmu, uintptr_t va){
	void *pa = NULL;
	pde_t *pde_ptr = &(mmu->pd_entries[PDX(va)]);
	if (*pde_ptr == NULL){	// pde doesn't exist, allocate a new page table
		*pde_ptr = (pde_t)malloc(sizeof(pgtbl_t));
		mmu_append_pgtbl(mmu, (pgtbl_t *)(*pde_ptr));
	}
	pte_t *pte_ptr = (pte_t*)(*pde_ptr) + PTX(va);
	if (*pte_ptr == NULL){	// pte doesn't exist, allocate a page
		*pte_ptr = malloc(PGSIZE);
		pa = *pte_ptr;
	}
	return pa;
}

void mmu_destroy(mmu_t *mmu){
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

void mm_store(uintptr_t va, uint32_t value, mem_size mmsz){

}
void mm_load(uintptr_t va, uint32_t* value_store, mem_size mmsz){

}