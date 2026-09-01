#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/boot.h"
#include "lib/crash.h"
#include "lib/memp.h"
#include "lib/memshared.h"
#include "data.h"
#include "types.h"
#include "system.h"

/**
 * memp - memory pool allocation system.
 *
 * Memp is the main memory allocation system in the game.
 *
 * MEMPOOL_PERMANENT (index 1) is for permanent data and is never cleared.
 * MEMPOOL_STAGE (index 0) is for general data and is cleared on stage load.
 */

struct memorypool g_MempPools[2];

void mempInit(void)
{
	memInit(&g_MempPools[MEMPOOL_STAGE], 2048, "MEMPOOL_STAGE");
	memInit(&g_MempPools[MEMPOOL_PERMANENT], 32, "MEMPOOL_PERMANENT");
}

#ifdef MEMORY_DEBUG
void *_mempGetNextStageAllocation(char* label)
#else
void *mempGetNextStageAllocation(void)
#endif
{
	return memAlloc(&g_MempPools[MEMPOOL_STAGE], 4 * 1024 * 1024); // Give it 4MB so I don't have to figure out the right amount to allocate right now.
}

#ifdef MEMORY_DEBUG
void *_mempAlloc(u32 len, u8 pool, char* label)
#else
void *mempAlloc(u32 len, u8 pool)
#endif
{
	return memAlloc(&g_MempPools[pool], len);
}

#ifdef MEMORY_DEBUG
void *_mempAllocFromRight(u32 len, u8 pool, char* label)
#else
void *mempAllocFromRight(u32 len, u8 pool)
#endif
{
	return memAlloc(&g_MempPools[pool], len);
}

/**
 * Reallocate the given allocation in the given pool.
 * The pointer will remain unchanged.
 *
 * newsize must be less than (or equal to) the original size.
 */
void mempRealloc(void *allocation, s32 newsize, u8 poolnum)
{
	// Sometimes we receive a free attempt when the memory is still in use(?).
	// Just ignore this whole function. It'll be dealt with on new stage load.
	struct memorypool* pool = &g_MempPools[poolnum];

	if (newsize == 0)
	{
		memFree(pool, allocation, newsize);
		return;
	}

	for (size_t i = 0; i < pool->nextAlloc; i++)
	{
		if (pool->allocations[i].addr == allocation)
		{
			if (newsize <= pool->allocations[i].size)
			{
				pool->allocations[i].size = newsize;
			}
			return;
		}
	}
}

/**
 * Return the amount of free space in the given pool and bank.
 */
u32 mempGetPoolFree(u8 poolnum, u32 bank)
{
	return 4300; // Just lie and say we have enough for texLoad.
}

void mempResetPool(u8 pool)
{
	memReset(&g_MempPools[pool]);
}
