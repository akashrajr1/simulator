#ifndef _UC32SIM_CACHE_H
#define _UC32SIM_CACHE_H
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
#endif /* !_UC32SIM_CACHE_H */