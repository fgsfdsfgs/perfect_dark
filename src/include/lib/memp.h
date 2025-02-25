#ifndef _IN_LIB_MEMP_H
#define _IN_LIB_MEMP_H
#include <ultra64.h>
#include "data.h"
#include "types.h"
#include "lib/memshared.h"

void mempInit(void);
void mempRealloc(void *allocation, s32 newsize, u8 poolnum);
u32 mempGetPoolFree(u8 poolnum, u32 bank);
void mempResetPool(u8 pool);

#ifdef MEMORY_DEBUG
void *_mempGetNextStageAllocation(char* label);
void *_mempAlloc(u32 len, u8 pool, char* label);
void *_mempAllocFromRight(u32 len, u8 pool, char* label);
#define mempGetNextStageAllocation()	_mempGetNextStageAllocation(MEM_GET_LABEL())
#define mempAlloc(len, pool)			_mempAlloc(len, pool, MEM_GET_LABEL())
#define mempAllocFromRight(len, pool)	_mempAllocFromRight(len, pool, MEM_GET_LABEL())
#else
void *mempGetNextStageAllocation(void);
void *mempAlloc(u32 len, u8 pool);
void *mempAllocFromRight(u32 len, u8 pool);
#endif

#endif
