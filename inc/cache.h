#ifndef _UC32SIM_CACHE_H
#define _UC32SIM_CACHE_H
#include <stdint.h> 
/*
 * Instruction-Cache: tag/valid
 * Data-Cache: tag/valid/hi-dirty/lo-dirty
 * 			   write-back/write-allocate
 * physical-tag/virtual index
 * 16-kilobytes each
 * 32-bytes cache line
 * 4-way set associative
 * Round-Robin/LRU eviction
 *
 * I-Cache on self-modifying code:
 *		explicit synchronize
 *
 * addressing:
 *	bits in cache line: 5-bits (32-bytes, lower 2 bits ignored for word-granulity)
 *							using va[4-0]
 *	bits in index:		7-bits (128 lines in a block)
 *							using va[11-5]
 *  bits in tag:		20-bits
 *							using physical page number		
 */
#define CACHE_LINE_WORDS	8
#define WORD_SIZE			4
#define CACHE_LINE_SIZE		32

#define READ_HIT_CYCLE		1
#define WRITE_HIT_CYCLE		1

typedef enum{
	MM_BYTE = 1,
	MM_HALFWORD = 2,
	MM_WORD = 4
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
	uint32_t d_read_hit;
	uint32_t d_read_miss;
	uint32_t d_write_hit;
	uint32_t d_write_miss;
} cache_stats;

typedef struct {
	cache_stats stats;
	cache_latency latency;
} cache_t;


cache_t *cache_init(cache_latency *latency_ptr);
void cache_destroy(cache_t *cache);
/*
 * LOAD functions will return the loaded memory (zero extend if not word-long), 
 * STORE function will store *LEAST-significant-bytes* according to size,
 * both write latency cycle count to *latency_ptr
 * for the simulated program, ALL memory access goes through these three functions.
 */
uint32_t cache_iload(cache_t *cache, uintptr_t va, uint32_t *latency_store);
uint32_t cache_dload(cache_t *cache, uintptr_t va, mem_size size, uint32_t *latency_store);
void cache_dstore(cache_t *cache, uintptr_t va, uint32_t value, mem_size size, uint32_t *latency_store);

#endif /* !_UC32SIM_CACHE_H */