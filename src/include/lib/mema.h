#ifndef _IN_LIB_MEMA_H
#define _IN_LIB_MEMA_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void memaDefrag(void);
void memaInit(void);
void memaReset(void *ptr, u64 size);
void memaPrint(void);
void *memaAlloc(u64 size);
uintptr_t memaGrow(uintptr_t addr, u64 amount);
void _memaFree(uintptr_t addr, u64 size);
void memaFree(void *addr, u64 size);
u64 memaGetLongestFree(void);
bool memaRealloc(uintptr_t addr, u64 oldsize, u64 newsize);

#endif
