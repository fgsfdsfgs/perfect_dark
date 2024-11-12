#include <ultra64.h>
#include "constants.h"
#include "game/file.h"
#include "game/lang.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "platform.h"

extern u8 *g_LangBuffer;
extern s32 g_LangBufferSize;

void langReset(s32 stagenum)
{
	s32 i;
	s32 size;

	for (i = 0; i < ARRAYCOUNT(g_LangBanks); i++) {
		g_LangBanks[i] = NULL;
	}

#if VERSION >= VERSION_PAL_BETA
	// PAL and newer have to support switching languages mid-stage. To do this,
	// langReload iterates the bank pointers and reloads them if they're
	// non-zero. Here it's using langReload to do the initial load by setting
	// the desired banks to dummy (non-zero) values so langReload will load them.
	g_LangBanks[LANGBANK_GUN] = (void *)1;
	g_LangBanks[LANGBANK_MPMENU] = (void *)1;
	g_LangBanks[LANGBANK_PROPOBJ] = (void *)1;
	g_LangBanks[LANGBANK_MPWEAPONS] = (void *)1;
	g_LangBanks[LANGBANK_OPTIONS] = (void *)1;
	g_LangBanks[LANGBANK_MISC] = (void *)1;

	if (stagenum == STAGE_CREDITS) {
		g_LangBanks[LANGBANK_TITLE] = (void *)1;
	}

	if (stagenum == STAGE_CITRAINING) {
		size = 108000;
	} else {
		size = 56000;

		if (IS8MB()) {
			size = 68000;
		}
	}

#ifdef PLATFORM_64BIT
	size *= 2;
#endif

	g_LangBuffer = mempAlloc(ALIGN16(size), MEMPOOL_STAGE);
	g_LangBufferSize = size;

	langReload();
#else
	// Versions prior to PAL load the language directly
	g_LoadType = LOADTYPE_LANG; // find be a better way to do this..
	g_LangBanks[LANGBANK_GUN] = fileLoadToNew(langGetFileId(LANGBANK_GUN), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[LANGBANK_MPMENU] = fileLoadToNew(langGetFileId(LANGBANK_MPMENU), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[LANGBANK_PROPOBJ] = fileLoadToNew(langGetFileId(LANGBANK_PROPOBJ), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[LANGBANK_MPWEAPONS] = fileLoadToNew(langGetFileId(LANGBANK_MPWEAPONS), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[LANGBANK_OPTIONS] = fileLoadToNew(langGetFileId(LANGBANK_OPTIONS), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[LANGBANK_MISC] = fileLoadToNew(langGetFileId(LANGBANK_MISC), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);

	if (stagenum == STAGE_CREDITS) {
		g_LoadType = LOADTYPE_LANG;
		g_LangBanks[LANGBANK_TITLE] = fileLoadToNew(langGetFileId(LANGBANK_TITLE), FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);
	}
#endif
}
