#include <ultra64.h>
#include "constants.h"
#include "game/gfxmemory.h"
#include "game/stubs/game_175f50.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/rzip.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/str.h"
#include "data.h"
#include "types.h"
#include "platform.h"

/**
 * This file handles memory usage for graphics related tasks.
 *
 * There are two pools, "gfx" and "vtx", which are used to store different data.
 *
 * The gfx pool (g_GfxBuffers) is sized based on the stage's -mgfx and -mgfxtra
 * arguments. It contains only the master display list's GBI bytecode.
 * The master gdl is passed through all rendering functions in the game engine,
 * where each appends to the display list.
 *
 * The vtx pool (g_VtxBuffers) is sized based on the stage's -mvtx argument.
 * It is used for auxiliary graphics data such as vertex arrays, matrices and
 * colours.
 *
 * Both the gfx and vtx pools are split into two buffers of equal size.
 * Only one buffer is active at a time - the other is being drawn to the screen
 * while the active one is being built. Each time a frame is finished the active
 * buffer index is swapped to the other one.
 *
 * Both the gfx and vtx pools have a third element in them, but this is just a
 * marker for the end of the second element's allocation.
 */

/**
 * On 64-bit platforms the Gfx struct is twice as large.
*/
#ifdef PLATFORM_64BIT
#define GFX_SIZE_MULTIPLIER 2
#else
#define GFX_SIZE_MULTIPLIER 1
#endif

u8 *g_GfxBuffers[NUM_GFXTASKS + 1];
u32 var800aa58c;
u8 *g_VtxBuffers[NUM_GFXTASKS + 1];
u8 *g_GfxMemPos;
u8 g_GfxActiveBufferIndex;
u32 g_GfxRequestedDisplayList;

u32 g_GfxSizesByPlayerCount[] = {
	0x00010000 * GFX_SIZE_MULTIPLIER,
	0x00018000 * GFX_SIZE_MULTIPLIER,
	0x00020000 * GFX_SIZE_MULTIPLIER,
	0x00028000 * GFX_SIZE_MULTIPLIER,
#ifndef PLATFORM_N64
	0x00030000 * GFX_SIZE_MULTIPLIER,
	0x00038000 * GFX_SIZE_MULTIPLIER,
	0x00040000 * GFX_SIZE_MULTIPLIER,
	0x00048000 * GFX_SIZE_MULTIPLIER,
#endif
};

u32 g_VtxSizesByPlayerCount[] = {
	0x00010000,
	0x00018000,
	0x00020000,
	0x00028000,
#ifndef PLATFORM_N64
	0x00030000,
	0x00038000,
	0x00040000,
	0x00048000,
#endif
};

s32 g_GfxNumSwapsPerBuffer[NUM_GFXTASKS] = {0, 1};
u32 g_GfxNumSwaps = 2;

/**
 * Allocate graphics memory from the heap. Presumably called on stage load.
 *
 * Comments in this function are strings that appear in an XBLA debug build.
 * They were likely in the N64 version but ifdeffed out.
 */
void gfxReset(void)
{
	s32 stack;

	if (argFindByPrefix(1, "-mgfx")) {
		// Argument specified master_dl_size\n
		s32 gfx;
		s32 gfxtra = 0;

		gfx = strtol(argFindByPrefix(1, "-mgfx"), NULL, 0) * 1024;

		if (argFindByPrefix(1, "-mgfxtra")) {
			// ******** Extra specified but are we in the correct game mode I wonder???\n
			if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && PLAYERCOUNT() == 2) {
				// ******** Extra Display List Memeory Required\n
				// ******** Shall steal from video buffer\n
				// ******** If you try and run hi-res then\n
				// ******** you're gonna shafted up the arse\n
				// ******** so don't blame me\n
				gfxtra = strtol(argFindByPrefix(1, "-mgfxtra"), NULL, 0) * 1024;
			} else {
				// ******** No we're not so there\n
			}
		}

		// ******** Original Amount required = %dK ber buffer\n
		// ******** Extra Amount required = %dK ber buffer\n
		// ******** Total of %dK (Double Buffered)\n
<<<<<<< HEAD
		g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1] = gfx + gfxtra;
#ifndef PLATFORM_N64
		if (PLAYERCOUNT() > MAX_LOCAL_PLAYERS) {
			g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1] *= MAX_PLAYERS / MAX_LOCAL_PLAYERS;
		}
#endif
=======
		g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1] = (gfx + gfxtra) * GFX_SIZE_MULTIPLIER;
>>>>>>> 49cd478622e3fefdd5c60fbd314ef8185d0cbf9d
	}

	if (argFindByPrefix(1, "-mvtx")) {
		// Argument specified mtxvtx_size\n
		g_VtxSizesByPlayerCount[PLAYERCOUNT() - 1] = strtol(argFindByPrefix(1, "-mvtx"), NULL, 0) * 1024;
#ifndef PLATFORM_N64
		if (PLAYERCOUNT() > MAX_LOCAL_PLAYERS) {
			g_VtxSizesByPlayerCount[PLAYERCOUNT() - 1] *= MAX_PLAYERS / MAX_LOCAL_PLAYERS;
		}
#endif
	}

	// %d Players : Allocating %d bytes for master dl's\n
	g_GfxBuffers[0] = mempAlloc(g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1] * NUM_GFXTASKS, MEMPOOL_STAGE);
	g_GfxBuffers[1] = g_GfxBuffers[0] + g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1];
	g_GfxBuffers[2] = g_GfxBuffers[1] + g_GfxSizesByPlayerCount[PLAYERCOUNT() - 1];

	// Allocating %d bytes for mtxvtx space\n
	g_VtxBuffers[0] = mempAlloc(g_VtxSizesByPlayerCount[PLAYERCOUNT() - 1] * NUM_GFXTASKS, MEMPOOL_STAGE);
	g_VtxBuffers[1] = g_VtxBuffers[0] + g_VtxSizesByPlayerCount[PLAYERCOUNT() - 1];
	g_VtxBuffers[2] = g_VtxBuffers[1] + g_VtxSizesByPlayerCount[PLAYERCOUNT() - 1];

	g_GfxActiveBufferIndex = 0;
	g_GfxRequestedDisplayList = false;
	g_GfxMemPos = g_VtxBuffers[0];
}

Gfx *gfxGetMasterDisplayList(void)
{
	g_GfxRequestedDisplayList = true;

	return (Gfx *)g_GfxBuffers[g_GfxActiveBufferIndex];
}

Vtx *gfxAllocateVertices(u32 count)
{
	void *ptr = g_GfxMemPos;
	g_GfxMemPos += count * sizeof(Vtx);
	g_GfxMemPos = (u8 *)ALIGN16((uintptr_t)g_GfxMemPos);

	return ptr;
}

void *gfxAllocateMatrix(void)
{
	void *ptr = g_GfxMemPos;
	g_GfxMemPos += sizeof(Mtx);

	return ptr;
}

/**
 * sizeof(LookAt) is 0x10 and it consists of two Light structs of 0x8 each.
 * The function allocates 0x8 for every count, so it could be allocating lights
 * instead, however it's only used for LookAts so it's named as LookAt.
 */
LookAt *gfxAllocateLookAt(s32 count)
{
	void *ptr = g_GfxMemPos;
#ifdef PLATFORM_64BIT
	g_GfxMemPos += count * (sizeof(LookAt) * 2);
#else
	g_GfxMemPos += count * (sizeof(LookAt) / 2);
#endif

	return ptr;
}

Col *gfxAllocateColours(s32 count)
{
	void *ptr = g_GfxMemPos;
	count = ALIGN16(count * sizeof(Col));
	g_GfxMemPos += count;

	return ptr;
}

void *gfxAllocate(u32 size)
{
	void *ptr = g_GfxMemPos;
	size = ALIGN16(size);
	g_GfxMemPos += size;

	return ptr;
}

void gfxSwapBuffers(void)
{
	g_GfxActiveBufferIndex ^= 1;
	g_GfxRequestedDisplayList = false;
	g_GfxMemPos = g_VtxBuffers[g_GfxActiveBufferIndex];
	g_GfxNumSwapsPerBuffer[g_GfxActiveBufferIndex] = g_GfxNumSwaps;
	g_GfxNumSwaps++;

	if (g_GfxNumSwaps == -1) {
		g_GfxNumSwaps = 2;
	}
}

s32 gfxGetFreeGfx(Gfx *gdl)
{
	return (Gfx *)g_GfxBuffers[g_GfxActiveBufferIndex + 1] - gdl;
}

u32 gfxGetFreeVtx(void)
{
	return g_VtxBuffers[g_GfxActiveBufferIndex + 1] - g_GfxMemPos;
}
