#ifndef _UC32SIM_CACHE_H
#define _UC32SIM_CACHE_H
#include "declarations.h"
/*
 * Instruction-Cache: tag/valid
 * Data-Cache: tag/valid/hi-dirty/lo-dirty
 * 			   write-back/write-allocate
 * physical-tag/virtual index
 * 16-kilobytes each
 * 32-bytes cache line
 * 4-way set associative
 * LRU eviction/Random	
 *
 * I-Cache on self-modifying code:
 *		explicit synchronize
 *
 * addressing:
 *	bits in cache line: 5-bits (32-bytes, lower 2 bits ignored for word-granulity)
 *							using va[4-0]
 *	bits in index:		7-bits (128 lines)
 *							using va[11-5]
 *  bits in tag:		20-bits
 *							using physical page number		
 */
#define CACHE_LINE_WORDS	8
#define WORD_SIZE			4
#define CACHE_LINE_SIZE		32
#define CACHE_SET_ASSOC		4
#define NCACHE_SET			128
#define READ_HIT_CYCLE		1
#define WRITE_HIT_CYCLE		1

#define CACHE_INDEX(va)	(((va)>>5) & 127)
#define CACHE_OFFSET(va) (((va)>>2) & 7)
#define CACHE_ALIGN(va)	((va) & 0xFE0)

typedef enum{
	MM_WORD = 0,
	MM_BYTE,
	MM_HALFWORD,
} mem_size;

/* 
 * miss latency depends on MMU behaviours.
 */
typedef struct {
	uint32_t read_hit;
	// uint32_t read_miss;
	uint32_t write_hit;
	// uint32_t write_miss;
} cache_latency;

typedef struct {
	uint32_t i_hit;
	uint32_t i_miss;
	uint32_t d_hit;
	uint32_t d_miss;
} cache_stats;

typedef enum {
	CACHE_EVICT_LRU = 0,
	CACHE_EVICT_RAND
} cache_evict;

typedef struct {
	uint32_t data[CACHE_SET_ASSOC][CACHE_LINE_WORDS];
	uint8_t	 valid[CACHE_SET_ASSOC];
	uint8_t	 dirty[CACHE_SET_ASSOC];
	uint8_t	 order[CACHE_SET_ASSOC];
	void* 	 pa[CACHE_SET_ASSOC];
} cache_set_t;

struct _cache {
	cache_set_t icache[NCACHE_SET];
	cache_set_t dcache[NCACHE_SET];
	mmu_t *mmu;
	cache_stats stats;
	cache_latency latency;
	cache_evict eviction;
};


cache_t *cache_init(mmu_t *mmu, cache_latency *latency_ptr, cache_evict eviction);
void cache_destroy(cache_t *cache);
/*
 * LOAD functions will return the loaded memory (zero extend if not word-long), 
 * STORE function will store *LEAST-significant-bytes* according to size,
 * both write latency cycle count to *latency_ptr
 * for the simulated program, ALL memory access goes through these functions.
 */
uint32_t cache_load(cache_t *cache, uint32_t va, mem_type type, uint32_t *latency_store);
void cache_dstore(cache_t *cache, uint32_t va, uint32_t value, mem_size size, uint32_t *latency_store);

#endif /* !_UC32SIM_CACHE_H */