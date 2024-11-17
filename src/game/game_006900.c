#include <ultra64.h>
#include "constants.h"
#include "game/debug.h"
#include "game/game_006900.h"
#include "game/propsnd.h"
#include "game/bg.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

/**
 * Increments from 0 to 1 over 20 seconds then snaps back to 0.
 *
 * The value is commonly used for menus, so it increments while paused.
 */
f32 g_20SecIntervalFrac = 0;

/**
 * Increments from 0 to 1 over 80 seconds then snaps back to 0.
 *
 * The value is paused when the game is paused.
 */
f32 g_Lv80SecIntervalFrac = 0;

/**
 * Blends two colours together.
 *
 * The aweight argument is how much weight is given to colour A, on a scale of
 * 0 to 255.
 */
u32 colourBlend(u32 a, u32 b, u32 aweight)
{
	u32 bweight = 0xff - aweight;

	return (((aweight * ((a >> 24) & 0xff) + bweight * ((b >> 24) & 0xff)) >> 8) << 24)
		| (((aweight * ((a >> 16) & 0xff) + bweight * ((b >> 16) & 0xff)) >> 8) << 16)
		| (((aweight * ((a >> 8) & 0xff) + bweight * ((b >> 8) & 0xff)) >> 8) << 8)
		| ((aweight * (a & 0xff) + bweight * (b & 0xff)) >> 8);
}

void menuTickTimers(void)
{
#if VERSION >= VERSION_PAL_BETA
	g_20SecIntervalFrac = g_20SecIntervalFrac + g_Vars.diffframe240freal / 4800.0f;
#else
	g_20SecIntervalFrac = g_20SecIntervalFrac + g_Vars.diffframe240f / 4800.0f;
#endif

	if (g_20SecIntervalFrac > 1.0f) {
		g_20SecIntervalFrac -= 1.0f;
	}

	g_Lv80SecIntervalFrac = g_Lv80SecIntervalFrac + LVUPDATE60FREAL() / 4800.0f;

	if (g_Lv80SecIntervalFrac > 1.0f) {
		g_Lv80SecIntervalFrac -= 1.0f;
	}

#if PIRACYCHECKS
	{
		u32 *ptr = (u32 *)&mtxGetObfuscatedRomBase;
		u32 *end = (u32 *)&mtxF2L;
		u32 checksum = 0;

		while (ptr < end) {
			checksum ^= ~*ptr;
			checksum <<= 1;
			ptr++;
		}

		if (checksum != CHECKSUM_PLACEHOLDER) {
			u32 *ptr = (u32 *)&bgReset;
			ptr += rngRandom() % 0x40;
			end = &ptr[4];

			while (ptr < end) {
				*ptr = 0x00000012;
				ptr++;
			}
		}
	}
#endif
}

/**
 * Return a float that represents a frac on a sine wave, based on a global timer.
 *
 * A full oscillation goes from 0.5 -> 1 -> 0 -> 0.5.
 *
 * freq is the amount of oscillations per 20 seconds.
 */
f32 menuGetSinOscFrac(f32 freq)
{
	return sinf((freq * g_20SecIntervalFrac + freq * g_20SecIntervalFrac) * M_PI) / 2.0f + 0.5f;
}

/**
 * Return a float that represents a frac on a cosine wave, based on a global timer.
 *
 * A full oscillation goes from 1 -> 0 -> 1.
 *
 * freq is the amount of oscillations per 20 seconds.
 */
f32 menuGetCosOscFrac(f32 freq)
{
	return cosf((freq * g_20SecIntervalFrac + freq * g_20SecIntervalFrac) * M_PI) / 2.0f + 0.5f;
}

/**
 * Return a float that represents a linear frac, based on a global timer.
 *
 * A full interval goes from 0 to 1 at a constant speed,
 * then snaps back to 0 for the next interval.
 *
 * freq is the amount of intervals per 20 seconds.
 */
f32 menuGetLinearIntervalFrac(f32 freq)
{
	f32 tmp = g_20SecIntervalFrac * freq;

	tmp -= (s32)tmp;

	return tmp;
}

/**
 * Return a float that oscillates between 0 and 1, with a pause at each end.
 *
 * frac 0.00 to 0.25 -> result will be a scale from 0 to 1
 * frac 0.25 to 0.50 -> result will be 1
 * frac 0.50 to 0.75 -> result will be a scale from 1 to 0
 * frac 0.75 to 1.00 -> result will be 0
 *
 * It's used on the Combat Simulator's character selection menu
 * to determine the camera's zoom.
 */
f32 menuGetLinearOscPauseFrac(f32 frac)
{
	s32 ival = frac * 4.0f;
	f32 fval = frac * 4.0f - (f32)(ival / 4) * 4.0f;

	if (fval < 1.0f) {
		return fval;
	}

	if (fval < 2.0f) {
		return 1.0f;
	}

	if (fval < 3.0f) {
		return 1.0f - (fval - 2.0f);
	}

	return 0.0f;
}
