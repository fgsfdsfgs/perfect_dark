#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "game/debug.h"
#include "lib/debughud.h"
#include "lib/mema.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

/**
 * mema - memory (ad hoc) allocation system.
 *
 * Mema's heap is 300KB and is itself allocated out of memp's stage pool.
 * Memp resets its stage pool each time a new stage is loaded, which means mema
 * is also reset each time a stage is loaded.
 *
 * Unlike memp, mema supports freeing of individual allocations. This makes it
 * a good system to use when the allocation is somewhat temporary and should be
 * freed without having to load a new stage. It's used by the (inaccessible)
 * Perfect Head editor, file listings and room code.
 *
 * Mema tracks what has been allocated by storing references to free spaces in
 * its spaces array. The allocations themselves are not referenced. When
 * initialising the spaces array, the first element is set to the entire heap
 * and the remaining elements are set to 0.
 *
 * This creates a bit of a terminology problem. Just remember that a memaspace
 * is not an allocation; it's a free space that is available for allocation.
 *
 * Due to the ability to free individual allocations, both the heap and the
 * spaces array can become fragmented. Mema supports defragmenting the spaces
 * array: entries are ordered by address, and back to back entries are merged.
 * The data in the heap itself is never moved, as that would require updating
 * pointers throughout the game code which mema cannot do.
 */

#define MAX_SPACES 124

struct memaspace {
	uintptr_t addr;
	u64 size;
};

/**
 * This structure contains dummy entries before and after the spaces array.
 * These are used as start and end markers, but could have been avoided by
 * using loop counters (eg. a typical i < numspaces loop).
 */
struct memaheap {
	u32 unk000;
	struct memaspace start;
	struct memaspace spaces[MAX_SPACES];
	struct memaspace end1;
	struct memaspace end2;
};

uintptr_t g_MemaHeapStart;
u64 g_MemaHeapSize;
struct memaheap g_MemaHeap;

#if VERSION == VERSION_PAL_BETA
u32 g_MemaLeastEverFree = 1000000;
#endif

void memaSwap(struct memaspace *a, struct memaspace *b)
{
	uintptr_t tempaddr = a->addr;
	u64 tempsize = a->size;
	a->addr = b->addr;
	a->size = b->size;
	b->addr = tempaddr;
	b->size = tempsize;
}

void memaMerge(struct memaspace *a, struct memaspace *b)
{
	a->size += b->size;
	b->addr = 0;
	b->size = 0;
}

bool memaDefragPass(struct memaheap *heap)
{
	bool merged = false;
	struct memaspace *prev = &heap->start;
	struct memaspace *curr = &heap->spaces[0];
	struct memaspace *last = &heap->spaces[MAX_SPACES - 1];
	uintptr_t addr = 0;

	while (curr <= last) {
		if (curr->size != 0) {
			if (curr->addr < addr) {
				memaSwap(curr, prev);
			}

			if (prev->size + addr == curr->addr) {
				memaMerge(prev, curr);
				curr = prev;
				merged = true;
			}

			prev = curr;
			addr = curr->addr;
		}

		curr++;
	}

	return merged;
}

void memaDefrag(void)
{
	while (memaDefragPass(&g_MemaHeap));
}

/**
 * Defrag the spaces list in an attempt to free up any slot.
 *
 * If none can be found, return the smallest run of free space so it can be
 * overwritten by the caller.
 */
struct memaspace *memaMakeSlot(struct memaheap *heap)
{
	struct memaspace *curr = &heap->spaces[0];
	struct memaspace *best;
	u32 min;
	s32 i;

	// Do 124 passes over the list. This ensures the list is in order by the
	// end. Though in most cases it's roughly in order anyway, and the excessive
	// looping is just wasting CPU cycles. In reality this situation probably
	// never occurs.
	for (i = 0; i < MAX_SPACES; i++) {
		while (curr <= &heap->spaces[MAX_SPACES - 1]) {
			if (curr->size == 0) {
				return curr;
			}

			if ((uintptr_t) curr[1].addr < (uintptr_t) curr[0].addr) {
				memaSwap(&curr[0], &curr[1]);
			}

			if (curr[1].addr == curr[0].size + curr[0].addr) {
				// Found two that can be merged
				curr[0].size += curr[1].size;
				curr[1].addr = 0;
				curr[1].size = 0;

				return &curr[1];
			}

			curr++;
		}

		curr = &heap->spaces[0];
	}

	// If this code is reached then the spaces list is so badly and unrepairably
	// fragmented that we can't find any slot to record the free space in.
	// Find the smallest run of free space and use that instead.
	// The caller will overwrite it with its own free allocation, causing the
	// original run of free space to be unusable until the mema heap is reset.
	min = 0xffffffff;
	best = curr;

	while (curr <= &heap->spaces[MAX_SPACES - 1]) {
		if (curr->size < min) {
			best = curr;
			min = curr->size;
		}

		curr++;
	}

	return best;
}

void _memaFree(uintptr_t addr, u64 size)
{
	// Choose an index in the spaces array which we'll mark a space as free,
	// based on how far into the heap the allocation is. This is a rough
	// estimate and doesn't need to be any particular index, but the defrag
	// function tries to order the spaces by address so the closer we get to it
	// the less work the defrag function will have to do should it be called.
	s32 index = (addr - g_MemaHeapStart) * MAX_SPACES / g_MemaHeapSize;
	struct memaspace *curr = &g_MemaHeap.spaces[index];

	// If the entry is taken, keep moving forward until a zero is found.
	while (curr->size != 0) {
		curr++;
	}

	// If we reached the end of the spaces list, go backwards instead
	if (curr->addr == -1) {
		curr = &g_MemaHeap.spaces[index];

		while (curr->size != 0) {
			curr--;
		}

		if (curr->addr == 0) {
			curr = memaMakeSlot(&g_MemaHeap);
		}
	}

	// Mark this space as free
	curr->addr = addr;
	curr->size = size;
}

void memaInit(void)
{
	// empty
}

void memaReset(void *heapaddr, u64 heapsize)
{
	struct memaspace *space;

#ifdef PLATFORM_N64
#ifndef DEBUG
	// Adding an amount to the heap size here means that mema can allocate past
	// the end of its heap. This would overflow into the gun names language
	// file. Maybe the developers had an ifndef directive like we do, but they
	// meant for it to be ifdef instead?
	// @bug @dangerous
	heapsize += 0x8e0;
#endif
#endif

	g_MemaHeap.unk000 = 0;

	g_MemaHeap.start.addr = 0;
	g_MemaHeap.start.size = 0;

	g_MemaHeap.end1.addr = 0xffffffff;
	g_MemaHeap.end1.size = 0;
	g_MemaHeap.end2.addr = 0xffffffff;
	g_MemaHeap.end2.size = 0xffffffff;

	for (space = &g_MemaHeap.spaces[0]; space <= &g_MemaHeap.spaces[MAX_SPACES - 1]; space++) {
		space->addr = 0;
		space->size = 0;
	}

	g_MemaHeap.spaces[0].addr = g_MemaHeapStart = (uintptr_t) heapaddr;
	g_MemaHeap.spaces[0].size = g_MemaHeapSize = heapsize;

#if VERSION == VERSION_PAL_BETA
	g_MemaLeastEverFree = 1000000;
#endif
}

/**
 * Example printout of figures:
 *
 *     Mem Info
 *     memp: MP_LF_LEV
 *      F: 0 722352
 *      S: 972080 3668704
 *      Over: 2946352
 *     memp: MP_LF_ETER
 *      F: 0 0
 *      S: 601728 0
 *     mema:
 *      LF: 391728
 *     Audio Free: 13184
 *
 * Where two figures are shown in one line, the left refers to onboard memory
 * and the right refers to expansion pak memory.
 *
 * F means free.
 * S means size.
 *
 * "Over" shows how much it's over 4MB, if they were to try to fit the game into
 * onboard memory only. This shows "Free" if under 4MB.
 *
 * The ETER (permanent) pool has 0 free space because it's shrunk to fit once
 * the permanent allocations are done during startup. This pool fits entirely
 * in onboard memory, so the expansion size is 0.
 */
void memaPrint(void)
{
	s32 onboard;
	s32 expansion;
	s32 line = 1;
	s32 over;
	char buffer[124];

	memaDefragPass(&g_MemaHeap);

#ifdef DEBUG
#if VERSION == VERSION_PAL_BETA
	if (debugIsMemInfoEnabled()) {
		dhudSetFgColour(0xff, 0xff, 0xff, 0xff);
		dhudSetBgColour(0, 0, 0, 0xff);

		sprintf(buffer, "Lev0: %d", mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_ONBOARD));

		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		sprintf(buffer, "Lev1: %d", mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_EXPANSION));

		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		if (memaGetLongestFree() < g_MemaLeastEverFree) {
			g_MemaLeastEverFree = memaGetLongestFree();
		}

		sprintf(buffer, "mema: %d (%d)", memaGetLongestFree(), g_MemaLeastEverFree);

		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;
	}
#endif

#if VERSION == VERSION_NTSC_BETA
	if (debugIsMemInfoEnabled()) {
		dhudSetFgColour(0xff, 0xff, 0xff, 0xff);
		dhudSetBgColour(0, 0, 0, 0xff);

		dhudSetPos(30, line);
		dhudPrintString("Mem Info");
		line++;

		dhudSetPos(30, line);
		dhudPrintString("memp: MP_LF_LEV");
		line++;

		onboard = mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_ONBOARD);
		expansion = mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_EXPANSION);
		sprintf(buffer, "F: %d %d", onboard, expansion);
		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		onboard = mempGetPoolSize(MEMPOOL_STAGE, MEMBANK_ONBOARD);
		expansion = mempGetPoolSize(MEMPOOL_STAGE, MEMBANK_EXPANSION);
		sprintf(buffer, "S: %d %d", onboard, expansion);
		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		over = mempGetPoolSize(MEMPOOL_STAGE, MEMBANK_EXPANSION)
			- mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_EXPANSION)
			- mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_ONBOARD);

		if (over >= 0) {
			sprintf(buffer, "Over: %d", over);
		} else {
			sprintf(buffer, "Free: %d", -over);
		}

		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		dhudSetPos(30, line);
		dhudPrintString("memp: MP_LF_ETER");
		line++;

		onboard = mempGetPoolFree(MEMPOOL_PERMANENT, MEMBANK_ONBOARD);
		expansion = mempGetPoolFree(MEMPOOL_PERMANENT, MEMBANK_EXPANSION);
		sprintf(buffer, "F: %d %d", onboard, expansion);
		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		onboard = mempGetPoolSize(MEMPOOL_PERMANENT, MEMBANK_ONBOARD);
		expansion = mempGetPoolSize(MEMPOOL_PERMANENT, MEMBANK_EXPANSION);
		sprintf(buffer, "S: %d %d", onboard, expansion);
		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		dhudSetPos(30, line);
		dhudPrintString("mema:");
		line++;

		sprintf(buffer, "LF: %d", memaGetLongestFree());
		dhudSetPos(31, line);
		dhudPrintString(buffer);
		line++;

		sprintf(buffer, "Audio Free: %d", g_SndHeap.base + (g_SndHeap.len - (uintptr_t) g_SndHeap.cur));
		dhudSetPos(30, line);
		dhudPrintString(buffer);
		line++;
	}
#endif
#endif
}

void *memaAlloc(u64 size)
{
	uintptr_t addr;
	uintptr_t diff;
	s32 i;

	struct memaspace *curr;
	uintptr_t bestdiff;
	struct memaspace *best;

	if (1);

	curr = &g_MemaHeap.spaces[0];
	bestdiff = 0xffffffff;
	best = NULL;

	// Iterate up to the first 16 spaces, looking for the
	// smallest space that will accommodate the requested size.
	for (i = 0; i < 16; i++, curr++) {
		if (curr->size < size) {
			continue;
		}

		if (curr->addr == 0xffffffff) {
			// Reached the end
			break;
		}

		diff = curr->size - size;

		if (diff < bestdiff) {
			bestdiff = diff;
			best = curr;

			// Stop looking if the space is small enough
#if VERSION >= VERSION_NTSC_1_0
			if (diff < 64 || (IS8MB() && diff < size / 4))
#else
			if (diff < 64 || diff < size / 4)
#endif
			{
				break;
			}
		}
	}

	if (best == NULL) {
		// Keep iterating until we find a space that is big enough to fit.
		// The last space is marked as size 0xffffffff which prevents this loop
		// from iterating past the end of the spaces array.
		while (curr->size < size) {
			curr++;
		}

		if (curr->addr == 0xffffffff) {
			// There was no space, so attempt to free up some space
			// by doing several defrag passes
			for (i = 0; i < 8; i++) {
				memaDefragPass(&g_MemaHeap);
			}

			curr = &g_MemaHeap.spaces[0];

			while (curr->size < size) {
				curr++;
			}

			if (curr->addr == 0xffffffff) {
				return NULL;
			}
		}

		best = curr;
	}

	addr = best->addr;
	best->addr += size;
	best->size -= size;

	if (best->size == 0) {
		best->addr = 0;
	}

	return (void *)addr;
}

/**
 * Grow the allocation which currently *ends at* the given address.
 */
uintptr_t memaGrow(uintptr_t addr, u64 amount)
{
	struct memaspace *curr = &g_MemaHeap.spaces[0];

	while (curr->addr != -1) {
		if (curr->addr == addr && curr->size >= amount) {
			goto found;
		}

		curr++;
	}

	return 0;

found:
	curr->addr += amount;
	curr->size -= amount;

	if (curr->size == 0) {
		curr->addr = 0;
	}

	return addr;
}

void memaFree(void *addr, u64 size)
{
	_memaFree((uintptr_t) addr, size);
}

void mema00012cd4(void)
{
	// empty
}

/**
 * Find and return the largest amount of contiguous free space in the pool.
 * ie. the biggest allocation that mema can currently make.
 */
u64 memaGetLongestFree(void)
{
	struct memaspace *curr;
	s32 biggest = 0;

	memaDefrag();

	curr = &g_MemaHeap.spaces[0];

	while (curr->addr != -1) {
		if (curr->size > biggest) {
			biggest = curr->size;
		}

		curr++;
	}

	if (biggest) {
		return biggest;
	}

	return 0;
}

bool memaRealloc(uintptr_t addr, u64 oldsize, u64 newsize)
{
	if (newsize > oldsize) {
		if (!memaGrow(addr + oldsize, newsize - oldsize)) {
			return false;
		}
	} else if (oldsize > newsize) {
		memaFree((void *)(addr + newsize), oldsize - newsize);
	}

	return true;
}

u32 memaGetSize(void)
{
	return g_MemaHeapSize;
}
