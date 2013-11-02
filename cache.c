#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "inc/cache.h"
#include "inc/misc.h"
static uint32_t rand_num;
#define CACHE_LCG_MUL	214013
#define CACHE_LCG_INC	2531011


cache_t *cache_init(
	mmu_t *mmu,
	cache_latency *latency_ptr,
	cache_evict eviction)
{
	cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
	memset(cache, 0, sizeof(cache_t));
	if (latency_ptr != NULL)
		cache->latency = *latency_ptr;
	else{
		cache->latency.read_hit = READ_HIT_CYCLE;
		cache->latency.write_hit = WRITE_HIT_CYCLE;
	}
	cache->mmu = mmu;
	cache->eviction = eviction;
	if (eviction == CACHE_EVICT_RAND)
		rand_num = (uint32_t)time(NULL);
	return cache;
}

void
cache_destroy(
	cache_t *cache)
{
	free(cache);
}

static int
cache_load_line(
	cache_t *cache,
	uintptr_t va,
	mem_type type,
	uint32_t *latency_store)
{
	uint32_t latency = 0;
	void *pa = mmu_paging(cache->mmu, va, type, &latency);
	cache_set_t *cache_set = (type == MEM_INST)? cache->icache: cache->dcache;
	cache_set += CACHE_INDEX(va);
	int i;
	uint32_t data = 0;
	uint8_t order = 0;
	latency = max(latency, cache->latency.read_hit);
	for (i = 0; i < CACHE_SET_ASSOC; i++)
		if (cache_set->valid[i] && cache_set->pa[i] == pa){
			if (type == MEM_INST) cache->stats.i_hit ++;
			else cache->stats.d_hit ++;
			goto cache_load_lru;
		}
	// i-cache miss
	uint32_t *cache_line = NULL;
	if (type == MEM_INST) cache->stats.i_miss ++;
	else cache->stats.d_miss ++;
	for (i = 0; i < CACHE_SET_ASSOC; i++)
		if (cache_set->valid[i] == 0){
			cache_line = cache_set->data[i];
			break;
		}
	if (cache_line == NULL){ // all in use, pick one to evict
		if (cache->eviction == CACHE_EVICT_LRU){
			for (i = 0; i < CACHE_SET_ASSOC; i++)
				if (cache_set->order[i] == CACHE_SET_ASSOC-1)
					break;
		}else if (cache->eviction == CACHE_EVICT_RAND){
			i = rand_num % CACHE_SET_ASSOC;
			rand_num = rand_num * CACHE_LCG_MUL + CACHE_LCG_INC;
		}
		cache_line = cache_set->data[i];
		if (cache_set->dirty[i]) // write-back on eviction
			latency += mm_store(cache->mmu, pa + CACHE_ALIGN(va), cache_line);
	}
	cache_set->valid[i] = 1;
	cache_set->pa[i] = pa;
	latency += mm_load(cache->mmu, pa + CACHE_ALIGN(va), cache_line);
	cache_set->dirty[i] = 0;
cache_load_lru:
	cache_set->order[i] = 0; // recently used.
	int j;
	for (j = 0; j < CACHE_SET_ASSOC; j++)
		if (i != j)
			cache_set->order[j] ++;
	*latency_store = latency;
	return i;
}

uint32_t
cache_load(
	cache_t *cache,
	uintptr_t va,
	mem_type type,
	uint32_t *latency_store)
{
	uint32_t cache_line = cache_load_line(cache, va, type, latency_store);
	if (type == MEM_INST)
		return cache->icache[CACHE_INDEX(va)].data[cache_line][CACHE_OFFSET(va)];
	else return cache->dcache[CACHE_INDEX(va)].data[cache_line][CACHE_OFFSET(va)];
}

void
cache_dstore(
	cache_t *cache,
	uintptr_t va,
	uint32_t value,
	mem_size size,
	uint32_t *latency_store)
{
	uint32_t latency = 0;
	uint32_t cache_line = cache_load_line(cache, va, MEM_DATA, &latency);
	uint32_t *word_ptr = &(cache->dcache[CACHE_INDEX(va)].data[cache_line][CACHE_OFFSET(va)]);

	switch (size){
		case MM_WORD:
			*word_ptr = value;
			break;
		case MM_BYTE:
			*((uint8_t*)word_ptr + (va & 3)) =  (uint8_t)value;
			break;
		case MM_HALFWORD:
			*((uint16_t*)word_ptr + ((va>>1) & 1)) = (uint16_t)value;
	}
	*latency_store = max(cache->latency.write_hit, latency);
}