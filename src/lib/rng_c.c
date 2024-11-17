#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

u64 g_RngSeed = 0xab8d9f7781280783;

/**
 * Generate a random number between 0 and 4294967295.
 */
u32 rngRandom(void)
{
	g_RngSeed = ((g_RngSeed << 63) >> 31 | (g_RngSeed << 31) >> 32) ^ (g_RngSeed << 44) >> 32;
	g_RngSeed = ((g_RngSeed >> 20) & 0xfff) ^ g_RngSeed;

	return g_RngSeed;
}

/**
 * Set the given seed as the RNG seed. Add 1 to make sure it isn't 0.
 */
void rngSetSeed(u64 seed)
{
	g_RngSeed = seed + 1;
}

/**
 * Rotate the given seed using the same algorithm as rngRandom().
 *
 * Store the new 64-bit seed at the pointed address and return the same seed
 * cast as a u32.
 */
u32 rngRotateSeed(u64 *seed)
{
	*seed = ((*seed << 63) >> 31 | (*seed << 31) >> 32) ^ (*seed << 44) >> 32;
	*seed = ((*seed >> 20) & 0xfff) ^ *seed;

	return *seed;
}
