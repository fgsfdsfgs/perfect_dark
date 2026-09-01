#ifndef _IN_LIB_MEMSHARED_H
#define _IN_LIB_MEMSHARED_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "game/debug.h"
#include "lib/debughud.h"
#include "lib/mema.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>

struct allocslot {
	void* addr;
	size_t size;
#ifdef DEBUG
	char* label;
#endif
};

struct memorypool {
	u32 nextAlloc;
	size_t slots;
	struct allocslot* allocations;
};

bool memDefragPass(struct memorypool *pool);
void memDefrag(struct memorypool *pool);
void memInit(struct memorypool *pool, size_t initialCapacity, char* label);
void memReset(struct memorypool *pool);
bool memExpand(struct memorypool *pool);
void memFree(struct memorypool *pool, void *addr, size_t size);

#ifdef DEBUG
void *_memAlloc(struct memorypool *pool, size_t size, char* label);
#define MEM_LINETOSTR(line)		#line
#define MEM_GET_LABEL()			__FILE__ ":" MEM_LINETOSTR(__LINE__)
#define memAlloc(pool, size)	_memAlloc(pool, size, MEM_GET_LABEL())
#else
void *memAlloc(struct memorypool *pool, size_t size);
#endif

#endif
