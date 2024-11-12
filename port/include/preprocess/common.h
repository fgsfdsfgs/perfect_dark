#ifndef _IN_PREPROCESS_COMMON_H
#define _IN_PREPROCESS_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <PR/ultratypes.h>
#include "types.h"
#include "constants.h"

#include "platform.h"
#include "system.h"
#include "romdata.h"
#include "preprocess.h"

#define MAX_PTR_MARKERS (1024 * 8)

#define PD_ALIGN(val, size) (((val) + ((size) - 1)) & ~((size) - 1))

#define PD_PTR_BASE(x, b) (void *)((u8 *)b + (uintptr_t)x)
#define PD_PTR_BASEOFS(x, b, d) (void *)((u8 *)b - d + (uintptr_t)x)

static inline f32 swapF32(f32 x) { *(u32*)&x = PD_BE32(*(u32*)&x); return x; }
static inline u32 swapU32(u32 x) { return PD_BE32(x); }
static inline s32 swapS32(s32 x) { return PD_BE32(x); }
static inline u16 swapU16(u16 x) { return PD_BE16(x); }
static inline s16 swapS16(s16 x) { return PD_BE16(x); }
static inline void* swapPtr(void** x) { return (void*)PD_BEPTR((uintptr_t)x); }
static inline struct coord swapCrd(struct coord crd) { crd.x = swapF32(crd.x); crd.y = swapF32(crd.y); crd.z = swapF32(crd.z); return crd; }
static inline u32 swapUnk(u32 x) { assert(0 && "unknown type"); return x; }

#define PD_SWAPPED_VAL(x) _Generic((x), \
	f32: swapF32, \
	u32: swapU32, \
	s32: swapS32, \
	u16: swapU16, \
	s16: swapS16, \
	struct coord: swapCrd, \
	default: swapUnk	\
)(x)

#define PD_SWAP_VAL(x) x = PD_SWAPPED_VAL(x)

#define PD_SWAP_PTR(x) x = swapPtr((void *)(x))

// ptr marker functions

struct ptrmarker {
	u32 ptr_src;
	uintptr_t ptr_host;
};

void ptrAdd(u32 ptr_src, uintptr_t ptr_host);
struct ptrmarker* ptrFind(uintptr_t ptr_src);
void ptrReset(void);

#endif
