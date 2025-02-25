#ifndef _IN_LIB_MEMA_H
#define _IN_LIB_MEMA_H
#include <ultra64.h>
#include "data.h"
#include "types.h"
#include "lib/memshared.h"

void memaDefrag(void);
void memaInit(void);
void memaReset(void);
void memaFree(void *addr, size_t size);

#ifdef MEMORY_DEBUG
void *_memaAlloc(size_t size, char* label);
#define memaAlloc(size) _memaAlloc(size, MEM_GET_LABEL())
#else
void *memaAlloc(size_t size);
#endif

#endif
