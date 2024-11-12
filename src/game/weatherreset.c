#include <ultra64.h>
#include "constants.h"
#include "game/weather.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

s32 g_WeatherActive = false;
u32 var800623f4 = 0x00000000;
u32 var800623f8 = 0x00000000;
u32 var800623fc = 0x00000000;

#ifndef PLATFORM_N64

static void weatherResetRooms(void)
{
	if (g_CurWeatherConfig->flags & WEATHERFLAG_INCLUDE) {
		// weather is present only in skiprooms, so mark every room as weatherproof
		for (s32 i = 0; i < g_Vars.roomcount; ++i) {
			g_Rooms[i].extra_flags |= ROOMFLAG_EX_WEATHERPROOF;
		}
		// then unmark skiprooms
		for (s32 i = 0; i < WEATHERCFG_MAX_SKIPROOMS && g_CurWeatherConfig->skiprooms[i]; ++i) {
			const RoomNum room = g_CurWeatherConfig->skiprooms[i];
			if (room >= 0 && room < g_Vars.roomcount) {
				g_Rooms[room].extra_flags &= ~ROOMFLAG_EX_WEATHERPROOF;
			}
		}
	} else {
		// weather is present in all rooms except skiprooms
		for (s32 i = 0; i < WEATHERCFG_MAX_SKIPROOMS && g_CurWeatherConfig->skiprooms[i]; ++i) {
			const RoomNum room = g_CurWeatherConfig->skiprooms[i];
			if (room >= 0 && room < g_Vars.roomcount) {
				g_Rooms[room].extra_flags |= ROOMFLAG_EX_WEATHERPROOF;
			}
		}
	}
}

#endif

void weatherReset(void)
{
	g_WeatherActive = false;
	g_WeatherData = NULL;

#ifndef PLATFORM_N64
	// find the weather config for this stage, if any
	g_CurWeatherConfig = &g_DefaultWeatherConfig;
	for (s32 i = 0; i < ARRAYCOUNT(g_WeatherConfig) && g_WeatherConfig[i].stagenum; ++i) {
		if (g_WeatherConfig[i].stagenum == g_Stages[g_StageIndex].id) {
			g_CurWeatherConfig = &g_WeatherConfig[i];
			break;
		}
	}

	if (g_CurWeatherConfig != &g_DefaultWeatherConfig)
#else
	if ((g_StageIndex == STAGEINDEX_CHICAGO
				|| g_StageIndex == STAGEINDEX_AIRBASE
				|| g_StageIndex == STAGEINDEX_G5BUILDING
				|| g_StageIndex == STAGEINDEX_CRASHSITE)
			&& PLAYERCOUNT() < 2)
#endif
	{
		g_WeatherData = mempAlloc(sizeof(struct weatherdata), MEMPOOL_STAGE);
		g_WeatherData->particledata[0] = weatherAllocateParticles();
		g_WeatherData->type = -1;
		g_WeatherData->windanglerad = 0;
		g_WeatherData->unk0c = 0;
		g_WeatherData->unk10 = 1;

#ifdef PLATFORM_N64
		if (g_StageIndex == STAGEINDEX_CHICAGO || g_StageIndex == STAGEINDEX_G5BUILDING) {
			g_WeatherData->windspeed = 20;
		} else if (g_StageIndex == STAGEINDEX_CRASHSITE) {
			g_WeatherData->windspeed = 10;
		} else {
			g_WeatherData->windspeed = 5;
		}
#else
		g_WeatherData->windspeed = g_CurWeatherConfig->windspeed;
		// set up the weatherproof flags in all rooms
		if (g_Vars.stagenum != STAGE_TITLE) {
			weatherResetRooms();
		}
#endif

		g_WeatherData->audiohandles[0] = 0;
		g_WeatherData->audiohandles[1] = 0;
		g_WeatherData->audiohandles[2] = 0;
		g_WeatherData->audiohandles[3] = 0;
		g_WeatherData->unk44 = 0;
		g_WeatherData->unk94 = -1;
		g_WeatherData->unk48 = 1;
		g_WeatherData->unk4c = 0;
		g_WeatherData->unk50 = 0;
		g_WeatherData->unk54 = 0;
		g_WeatherData->unk58[0].unk00 = 0;
		g_WeatherData->unk58[1].unk00 = 0;
		g_WeatherData->unk58[2].unk00 = 0;
		g_WeatherData->unk58[3].unk00 = 1;
		g_WeatherData->unk58[0].unk04 = 1;
		g_WeatherData->unk58[0].unk08 = 10;

		if (g_WeatherData->unk58[0].unk08 < 0) {
			g_WeatherData->unk58[0].unk08 = -g_WeatherData->unk58[0].unk08;
		}

		g_WeatherData->unk58[0].unk08 = 0;
		g_WeatherData->unk58[1].unk08 = 0;
		g_WeatherData->unk58[2].unk08 = 0;
		g_WeatherData->unk58[3].unk08 = 0;
		g_WeatherData->unkb8 = 150;
		g_WeatherData->unkc0 = 0;
		g_WeatherData->unkc4 = 0;
		g_WeatherData->unkc8 = 15;
		g_WeatherData->unk88 = 1;
		g_WeatherData->unk90 = 0;
		g_WeatherData->intensity = 0;
		g_WeatherData->unkd0 = 0;
		g_WeatherData->unkd4 = 0;

		g_WeatherActive = true;
	}
}
