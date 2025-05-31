#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include <locale.h>
#include <libintl.h>

u16 *g_FrameBuffers[NUM_FRAMEBUFFERS];

#ifndef PLATFORM_N64
#if VERSION >= VERSION_JPN_FINAL
u32 var8009d010pf[4];
s32 g_JpnMaxCacheItems;
#elif VERSION >= VERSION_PAL_BETA
u32 var8009d010pf[4];
#endif
#endif

void langInit(void)
{
//#ifdef PLATFORM_N64
	s32 i;

#if VERSION >= VERSION_JPN_FINAL
	g_JpnMaxCacheItems = IS4MB() ? 124 : 174;

	g_JpnCharCachePixels = mempAlloc(MAX_JPN_CACHE_ITEMS() * (sizeof(struct jpncharpixels) * 8), MEMPOOL_PERMANENT);
	g_JpnCacheCacheItems = mempAlloc(ALIGN16(MAX_JPN_CACHE_ITEMS() * sizeof(struct jpncacheitem)), MEMPOOL_PERMANENT);

	for (i = 0; i < MAX_JPN_CACHE_ITEMS(); i++) {
		g_JpnCacheCacheItems[i].ttl = 0;
		g_JpnCacheCacheItems[i].codepoint = -1;
	}
#else
	if (g_Jpn) {
		g_JpnCharCachePixels = mempAlloc(MAX_JPN_CACHE_ITEMS() * (sizeof(struct jpncharpixels) * 8), MEMPOOL_PERMANENT);
		g_JpnCacheCacheItems = mempAlloc(ALIGN16(MAX_JPN_CACHE_ITEMS() * sizeof(struct jpncacheitem)), MEMPOOL_PERMANENT);

		for (i = 0; i < MAX_JPN_CACHE_ITEMS(); i++) {
			g_JpnCacheCacheItems[i].ttl = 0;
			g_JpnCacheCacheItems[i].codepoint = -1;
		}
	}
#endif
//#else
	setlocale(LC_MESSAGES, "");
    bindtextdomain("perfect_dark", "build/locale");
	bind_textdomain_codeset("perfect_dark", "UTF-8");
    textdomain("perfect_dark");
//#endif
}
