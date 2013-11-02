#include <stdlib.h>
#include <string.h>
#include "inc/cache.h"

cache_t *cache_init(cache_latency *latency_ptr)
{
	cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
	memset(cache, 0, sizeof(cache_t));
	if (latency_ptr != NULL)
		cache->latency = *latency_ptr;
	else{
		cache->latency.read_hit = READ_HIT_CYCLE;
		cache->latency.write_hit = WRITE_HIT_CYCLE;
	}
	return cache;
}

void
cache_destroy(
	cache_t *cache)
{
	free(cache);
}

uint32_t
cache_iload(
	cache_t *cache,
	uintptr_t va,
	uint32_t *latency_store)
{
	return 0;
}

uint32_t
cache_dload(
	cache_t *cache,
	uintptr_t va,
	mem_size size,
	uint32_t *latency_store)
{
	return 0;
}

void
cache_dstore(
	cache_t *cache,
	uintptr_t va,
	uint32_t value,
	mem_size size,
	uint32_t *latency_store)
{

}