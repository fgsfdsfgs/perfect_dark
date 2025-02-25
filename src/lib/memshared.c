#include <ultra64.h>
#include "system.h"
#include "constants.h"
#include "bss.h"
#include "game/debug.h"
#include "lib/debughud.h"
#include "lib/memshared.h"
#include "data.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>

#ifdef MEMORY_DEBUG
char* lastAttemptedFree = NULL;
#endif

bool memDefragPass(struct memorypool *pool)
{
	size_t freeSpace;
	bool foundFreeSpace = false;

	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == NULL)
		{
			freeSpace = i;
			foundFreeSpace = true;
			break;
		}
	}
	if (!foundFreeSpace)
	{
		return false;
	}
	for (size_t i = freeSpace; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr != NULL)
		{
#ifdef MEMORY_DEBUG
			pool->allocations[freeSpace].label = pool->allocations[i].label;
			pool->allocations[i].label = NULL;
#endif
			pool->allocations[freeSpace].addr = pool->allocations[i].addr;
			pool->allocations[i].addr = NULL;
			pool->allocations[freeSpace].size = pool->allocations[i].size;
			pool->allocations[i].size = 0;
			return true;
		}
	}
	return false;
}

void memDefrag(struct memorypool *pool)
{
	while (memDefragPass(pool));
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == NULL)
		{
			pool->nextAlloc = i;
			return;
		}
	}
}

void memInit(struct memorypool *pool, size_t initialCapacity, char* label)
{
	pool->nextAlloc = 0;
	pool->slots = initialCapacity;
	size_t allocationSize = pool->slots * sizeof(*pool->allocations);
	void* allocations = sysMemZeroAlloc(allocationSize);
	if (allocations != NULL)
	{
		pool->allocations = allocations;
	}
	else
	{
		sysFatalError("Could not allocate %u bytes for memory pool %s.", allocationSize, label);
	}
}

void memFreeIndex(struct memorypool *pool, size_t index)
{
#ifdef MEMORY_DEBUG
	lastAttemptedFree = pool->allocations[index].label;
#endif
	sysMemFree(pool->allocations[index].addr);
#ifdef MEMORY_DEBUG
	pool->allocations[index].label = NULL;
#endif
	pool->allocations[index].addr = NULL;
	pool->allocations[index].size = 0;
}

void memReset(struct memorypool *pool)
{
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr != NULL)
		{
			memFreeIndex(pool, i);
		}
	}
	pool->nextAlloc = 0;
}

/*
 * Expands the size of the allocation tracker.
 * On failure, will call memDefrag.
 * Returns true on success, false on failure.
 */
bool memExpand(struct memorypool *pool)
{
	size_t oldSize = pool->slots;
	size_t newSize = pool->slots * 2;
	void* tmp = sysMemRealloc(pool->allocations, newSize * sizeof(*pool->allocations));
	if (tmp == NULL)
	{
		memDefrag(pool);
		return false;
	}
	pool->allocations = (struct allocslot*)tmp;
	pool->slots = newSize;
	memset(tmp + (oldSize * sizeof(*pool->allocations)), 0, (newSize - oldSize) * sizeof(*pool->allocations));
	return true;
}

#ifdef MEMORY_DEBUG
void *_memAlloc(struct memorypool *pool, size_t size, char* label)
#else
void *memAlloc(struct memorypool *pool, size_t size)
#endif
{
	void *allocation;
	if (pool->nextAlloc >= pool->slots)
	{
		memExpand(pool);
		// memorypool may have been expanded (pool->slots is now higher)
		// or defragmented (pool->nextAlloc is now lower)
		if (pool->nextAlloc >= pool->slots)
		{
			return NULL;
		}
	}

	allocation = sysMemAlloc(size);
	if (allocation == NULL)
	{
		return NULL;
	}
#ifdef MEMORY_DEBUG
	pool->allocations[pool->nextAlloc].label = label;
#endif
	pool->allocations[pool->nextAlloc].addr = allocation;
	pool->allocations[pool->nextAlloc].size = size;
	pool->nextAlloc++;
	return allocation;
}

void memFree(struct memorypool *pool, void *addr, size_t size)
{
	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == addr)
		{
			memFreeIndex(pool, i);
			return;
		}
	}
}
