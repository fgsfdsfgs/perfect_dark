#include <ultra64.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/dlights.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/gfxmemory.h"
#include "game/sparks.h"
#include "game/weather.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/lv.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"

#define ABSF(val) ((val) > 0.0f ? (val) : -(val))

struct weatherdata *g_WeatherData = NULL;

#ifndef PLATFORM_N64

struct weathercfg g_WeatherConfig[WEATHERCFG_MAX_STAGES] = {
	{
		STAGE_G5BUILDING,
		WEATHERFLAG_CUTSCENE_ONLY,
		20.f,
		-800.f, 800.f,
		0.f,
	},
	{
		STAGE_CHICAGO,
		0,
		20.f,
		-800.f, 800.f,
		0.f,
		{
			ROOM_PETE_0062,
			ROOM_PETE_0061,
			ROOM_PETE_000F,
			ROOM_PETE_0052,
			ROOM_PETE_0045,
			ROOM_PETE_0044,
			ROOM_PETE_0043,
			ROOM_PETE_003D,
			ROOM_PETE_000E,
			ROOM_PETE_000D,
			ROOM_PETE_000C,
			ROOM_PETE_000B,
			ROOM_PETE_000A,
			ROOM_PETE_0009,
			ROOM_PETE_0008,
			ROOM_PETE_0007,
			ROOM_PETE_0006,
			ROOM_PETE_0005,
			ROOM_PETE_0004,
			ROOM_PETE_0037,
			ROOM_PETE_005B,
			ROOM_PETE_005F,
			ROOM_PETE_0060,
			ROOM_PETE_005D,
			ROOM_PETE_001D,
			ROOM_PETE_001F,
			ROOM_PETE_0027,
			ROOM_PETE_0050,
			ROOM_PETE_002C,
			ROOM_PETE_002F,
			ROOM_PETE_0030,
			ROOM_PETE_0011,
			ROOM_PETE_0024,
			ROOM_PETE_0033,
			ROOM_PETE_0034,
			ROOM_PETE_0035,
			ROOM_PETE_0036,
			ROOM_PETE_003C,
			ROOM_PETE_0046,
			ROOM_PETE_0047,
			ROOM_PETE_0049,
			ROOM_PETE_0055,
			ROOM_PETE_0056,
			ROOM_PETE_005E,
			ROOM_PETE_0063,
			ROOM_PETE_0069,
			ROOM_PETE_0003,
			ROOM_PETE_0016,
			ROOM_PETE_004F,
			ROOM_PETE_004B,
			ROOM_PETE_003F,
			ROOM_PETE_0013,
			ROOM_PETE_0019,
			0
		}
	},
	{
		STAGE_AIRBASE,
		WEATHERFLAG_INCLUDE | WEATHERFLAG_FORCE_WINDDIR,
		5.f,
		-800.f, 800.f,
		-2000.f,
		{
			ROOM_CAVE_0088,
			ROOM_CAVE_0091,
			ROOM_CAVE_0087,
			ROOM_CAVE_0090,
			ROOM_CAVE_0086,
			ROOM_CAVE_0080,
			ROOM_CAVE_0083,
			ROOM_CAVE_0084,
			ROOM_CAVE_0085,
			ROOM_CAVE_007F,
			ROOM_CAVE_007E,
			ROOM_CAVE_0082,
			ROOM_CAVE_008F,
			ROOM_CAVE_008E,
			ROOM_CAVE_007B,
			ROOM_CAVE_007C,
			ROOM_CAVE_007D,
			ROOM_CAVE_0081,
			ROOM_CAVE_006F,
			0
		},
		1.5707963705063f,
		0.f,
		-5.f
	},
	{
		STAGE_CRASHSITE,
		0,
		10.f,
		-500.f, 500.f,
		0.f,
		{
			ROOM_AZT_001F,
			ROOM_AZT_0020,
			ROOM_AZT_0021,
			ROOM_AZT_0022,
			ROOM_AZT_0023,
			ROOM_AZT_0024,
			ROOM_AZT_0051,
			ROOM_AZT_0052,
			ROOM_AZT_0053,
			ROOM_AZT_0054,
			ROOM_AZT_0055,
			ROOM_AZT_0056,
			ROOM_AZT_0057,
			ROOM_AZT_0058,
			ROOM_AZT_0059,
			ROOM_AZT_005A,
			ROOM_AZT_005B,
			ROOM_AZT_005C,
			ROOM_AZT_005D,
			ROOM_AZT_005E,
			ROOM_AZT_005F,
			ROOM_AZT_0060,
			ROOM_AZT_0061,
			ROOM_AZT_0062,
			ROOM_AZT_0063,
			ROOM_AZT_0064,
			ROOM_AZT_002D,
			ROOM_AZT_0040,
			ROOM_AZT_0041,
			ROOM_AZT_0042,
			ROOM_AZT_0043,
			ROOM_AZT_0044,
			ROOM_AZT_0045,
			ROOM_AZT_0046,
			ROOM_AZT_0047,
			ROOM_AZT_0048,
			ROOM_AZT_0049,
			ROOM_AZT_004A,
			ROOM_AZT_004B,
			ROOM_AZT_004C,
			ROOM_AZT_004D,
			ROOM_AZT_004E,
			ROOM_AZT_004F,
			ROOM_AZT_0050,
			0
		}
	},
};

const struct weathercfg g_DefaultWeatherConfig = {
	0,
	0,
	20.f,
	-800.f, 800.f,
	0.f
};

const struct weathercfg *g_CurWeatherConfig = &g_DefaultWeatherConfig;

#endif

Gfx *weatherRender(Gfx *gdl)
{
	struct weatherdata *weather;

	if (!g_WeatherData) {
		return gdl;
	}

#ifdef PLATFORM_N64
	if (g_StageIndex == STAGEINDEX_AIRBASE && g_Vars.currentplayer->cam_pos.z < -2000.0f) {
		return gdl;
	}

	if (g_StageIndex == STAGEINDEX_G5BUILDING && g_Vars.tickmode != TICKMODE_CUTSCENE) {
		return gdl;
	}
#else
	if (g_CurWeatherConfig->zmax && g_Vars.currentplayer->cam_pos.z < g_CurWeatherConfig->zmax) {
		return gdl;
	}

	if ((g_CurWeatherConfig->flags & WEATHERFLAG_CUTSCENE_ONLY) && g_Vars.tickmode != TICKMODE_CUTSCENE) {
		return gdl;
	}
#endif

	weather = g_WeatherData;

	gSPDisplayList(gdl++, &var800613a0);
	gSPDisplayList(gdl++, &var80061380);

	if (weather->type == WEATHERTYPE_SNOW) {
		texSelect(&gdl, &g_TexGeneralConfigs[1], 2, 1, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetColorDither(gdl++, G_CD_NOISE);
		gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);
		gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
		gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
		gDPSetAlphaDither(gdl++, G_AD_NOISE);
	}

	switch (weather->type) {
	case WEATHERTYPE_RAIN:
		gdl = weatherRenderRain(gdl, weather, 0);
		break;
	case WEATHERTYPE_SNOW:
		gdl = weatherRenderSnow(gdl, weather, 0);
		break;
	}

	return gdl;
}

void weatherSetBoundaries(struct weatherparticledata *data, s32 index, f32 min, f32 max)
{
	((f32 *)(&data->boundarymin))[index] = min;
	((f32 *)(&data->boundarymax))[index] = max;
	((f32 *)(&data->boundaryrange))[index] = ABS(min) + ABS(max);
}

struct weatherparticledata *weatherAllocateParticles(void)
{
	struct weatherparticledata *data = mempAlloc(sizeof(struct weatherparticledata), MEMPOOL_STAGE);
	u32 i;

	data->unk3e80.x = 0;
	data->unk3e80.y = 0;
	data->unk3e80.z = 0;

	weatherSetBoundaries(data, 0, -800, 800);

#ifdef PLATFORM_N64
	if ((u32)g_StageIndex == STAGEINDEX_CRASHSITE) {
		weatherSetBoundaries(data, 1, -500, 500);
	} else {
		weatherSetBoundaries(data, 1, -800, 800);
	}
#else
	weatherSetBoundaries(data, 1, g_CurWeatherConfig->ymin, g_CurWeatherConfig->ymax);
#endif

	weatherSetBoundaries(data, 2, -800, 800);

	i = 0;

	while (i != ARRAYCOUNT(g_WeatherData->particledata[0]->unk3ec8)) {
		data->unk3ec8[i++] = 0;
	}

	for (i = 0; i != (s32)ARRAYCOUNT(g_WeatherData->particledata[0]->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];
		particle->pos.x = RANDOMFRAC() * 1600 - 800;
		particle->pos.y = RANDOMFRAC() * 1600 - 800;
		particle->pos.z = RANDOMFRAC() * 1600 - 800;

		particle->inc.x = RANDOMFRAC() * 10 - 5;
		particle->inc.y = -10 - RANDOMFRAC() * 40;
		particle->inc.z = RANDOMFRAC() * 10 - 5;

		particle->unk1c = RANDOMFRAC() + 0.7f;

		particle->active = false;
	}

	return data;
}

void func0f131610(struct weatherdata *weather)
{
	weather->unk94 = 0;
	weather->unk98 = (rngRandom() & 7) + 1;
	weather->unk9c = (rngRandom() & 7) + 1;
	weather->unka0 = (rngRandom() & 7) + 1;
	weather->unka4 = (rngRandom() & 0xf) + 10;
}

void func0f131678(s32 arg0)
{
	s32 i;

	for (i = 0; i < arg0 + 1; i++) {
		if ((g_WeatherData->unk58[i].unk08 > 0 && g_WeatherData->unk58[i].unk04 > 0.0f)
				|| (g_WeatherData->unk58[i].unk08 < 1 && g_WeatherData->unk58[i].unk00 > 0)) {
			g_WeatherData->unk58[i].unk04 = 0.0f;
			g_WeatherData->unk58[i].unk08 = 100;

			if (g_WeatherData->unk58[i].unk08 < 0) {
				g_WeatherData->unk58[i].unk08 = -g_WeatherData->unk58[i].unk08;
			}
		}
	}
}

void weatherSetIntensity(s32 intensity)
{
	s32 dotheloop = -1;
	s32 special = -1;
	s32 i;

	if (intensity == g_WeatherData->intensity) {
		return;
	}

	switch (g_WeatherData->type) {
	case WEATHERTYPE_RAIN:
		if (g_WeatherData->intensity == 0) {
			func0f131610(g_WeatherData);
		}

		special = -1;
		dotheloop = 1;

		switch (intensity) {
		case 0:
			g_WeatherData->unkd4 = 0;
			g_WeatherData->unkbc = 100;
			g_WeatherData->unkc0 = 100;

			if (g_WeatherData->unkc0 < 0) {
				g_WeatherData->unkc0 = -g_WeatherData->unkc0;
			}

			dotheloop = -1;
			g_WeatherData->unkc4 = -1;
			break;
		case 1:
			g_WeatherData->unkc8 = 15;
			g_WeatherData->unkd4 = 200;
			g_WeatherData->unkbc = 100;
			g_WeatherData->unkc0 = 100;

			if (g_WeatherData->unkc0 < 0) {
				g_WeatherData->unkc0 = -g_WeatherData->unkc0;
			}

			special = 0;
			g_WeatherData->unkc4 = 0.001f;
			break;
		case 2:
			g_WeatherData->unkc8 = 18;
			g_WeatherData->unkd4 = 400;
			g_WeatherData->unkbc = 150;
			g_WeatherData->unkc0 = 100;

			if (g_WeatherData->unkc0 < 0) {
				g_WeatherData->unkc0 = -g_WeatherData->unkc0;
			}

			special = 1;
			g_WeatherData->unkc4 = 0.01f;
			break;
		case 3:
			g_WeatherData->unkc8 = 30;
			g_WeatherData->unkd4 = 500;
			g_WeatherData->unkbc = 300;
			g_WeatherData->unkc0 = 100;

			if (g_WeatherData->unkc0 < 0) {
				g_WeatherData->unkc0 = -g_WeatherData->unkc0;
			}

			special = 2;
			g_WeatherData->unkc4 = 0.03f;
			break;
		}
		break;
	case WEATHERTYPE_SNOW:
		switch (intensity) {
		case 0:
			g_WeatherData->unkd4 = 0;
			break;
		case 1:
			g_WeatherData->unkd4 = 500;
			break;
		}
		break;
	}

	if (dotheloop >= 0) {
		for (i = 0; i != 3; i++) {
			if (i != special) {
				g_WeatherData->unk58[i].unk04 = 0;
				g_WeatherData->unk58[i].unk08 = 100;

				if (g_WeatherData->unk58[i].unk08 < 0) {
					g_WeatherData->unk58[i].unk08 = -g_WeatherData->unk58[i].unk08;
				}
			}
		}
	}

	if (special >= 0) {
		g_WeatherData->unk58[special].unk04 = 1;
		g_WeatherData->unk58[special].unk08 = 100;

		if (g_WeatherData->unk58[special].unk08 < 0) {
			g_WeatherData->unk58[special].unk08 = -g_WeatherData->unk58[special].unk08;
		}
	}

	g_WeatherData->intensity = intensity;
}

u32 g_RainSpeedExtra;
u32 g_SnowSpeed;
u32 g_SnowSpeedExtra;

void weatherTickRain(struct weatherdata *weather)
{ \
	s32 lVar6 = 0;
	s32 relativetotal = 0; // eg. -10 if deleted 10 particles, +10 if created 10
	struct weatherparticledata *data;
	s32 i;
	s32 iVar10;
	f32 rand;
	s32 lvupdate;

	mainOverrideVariable("rainspeedxtra", &g_RainSpeedExtra);

	if (weather->unk90 > 0) {
		weather->unk88 += (weather->unk8c - weather->unk88) / weather->unk90;
	}

	weather->unk90--;

	if (weather->unk58[0].unk08 > 0) {
		weather->unk58[0].unk00 += (weather->unk58[0].unk04 - weather->unk58[0].unk00) / weather->unk58[0].unk08;
		weather->unk58[0].unk08--;
	}

	if (weather->unk58[1].unk08 > 0) {
		weather->unk58[1].unk00 += (weather->unk58[1].unk04 - weather->unk58[1].unk00) / weather->unk58[1].unk08;
		weather->unk58[1].unk08--;
	}

	if (weather->unk58[2].unk08 > 0) {
		weather->unk58[2].unk00 += (weather->unk58[2].unk04 - weather->unk58[2].unk00) / weather->unk58[2].unk08;
		weather->unk58[2].unk08--;
	}

	// Rain noise
	for (i = 0; i != 4; i++) {
		s32 sounds[] = {
			0x80b7,
			0x80b6,
			0x80b8,
			-1,
		};

		iVar10 = weather->unk58[i].unk00 * 32767.0f * weather->unk88;

		if (lvIsPaused()) {
			 iVar10 = 0;
		}

		if (iVar10 > 0) {
			if (weather->audiohandles[i] == 0 && sounds[i] >= 0) {
				weather->unkf8 = sounds[i];
				sndStart(var80095200, weather->unkf8, &weather->audiohandles[i], -1,
						-1, -1, -1, -1);
			}

			if (weather->audiohandles[i] != 0) {
				if (sndGetState(weather->audiohandles[i]) != AL_STOPPED) {
					sndAdjust(&weather->audiohandles[i], 0, iVar10 * 3 / 4, -1,
							weather->unkf8, 1, 1, -1, 1);
				}
			}
		} else {
			if (weather->audiohandles[i] != 0) {
				audioStop(weather->audiohandles[i]);
			}
		}
	}

	if (weather->unkc0 > 0) {
		lvupdate = g_Vars.lvupdate60;

		if (weather->unkc0 < lvupdate) {
			weather->unkb8 = weather->unkbc;
		} else {
			weather->unkb8 += lvupdate * ((weather->unkbc - weather->unkb8) / weather->unkc0);
			weather->unkc0 -= lvupdate;
		}
	}

#ifdef PLATFORM_N64
	if (g_StageIndex == STAGEINDEX_AIRBASE) {
		// Force weather direction - but Air Base doesn't use rain...
		weather->windanglerad = 1.5707963705063f;
		weather->windspeedz = -weather->windspeed;
		weather->windspeedx = 0;
	}
#else
	if (g_CurWeatherConfig->flags & WEATHERFLAG_FORCE_WINDDIR) {
		// force weather direction if requested
		weather->windanglerad = g_CurWeatherConfig->windanglerad;
		weather->windspeedz = g_CurWeatherConfig->windspeedz;
		weather->windspeedx = g_CurWeatherConfig->windspeedx;
	}
#endif
	else if (weather->unk10 > 0) {
		s32 lvupdate = g_Vars.lvupdate60;

		if (weather->unk10 < lvupdate) {
			weather->windanglerad = weather->unk0c;
		} else {
			weather->windanglerad += lvupdate * ((weather->unk0c - weather->windanglerad) / weather->unk10);
			weather->unk10 -= lvupdate;
		}

		if (weather->windanglerad > M_TAU) {
			weather->windanglerad = 0;
		}

		weather->windspeedx = cosf(weather->windanglerad) * weather->windspeed;
		weather->windspeedz = sinf(weather->windanglerad) * weather->windspeed;
		lVar6 = 1;
	} else if (RANDOMFRAC() > 0.99f) {
		rand = RANDOMFRAC();

		weather->unk0c = (rand + rand) * M_PI;
		weather->unk10 = (weather->unk0c - weather->windanglerad) / (PAL ? 0.012f : 0.01f);

		if (weather->unk10 < 0) {
			weather->unk10 = -weather->unk10;
		}
	}

	data = g_WeatherData->particledata[0];

	for (i = 0; i != ARRAYCOUNT(data->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];

		particle->pos.x += particle->inc.x * LVUPDATE60FREAL();
		particle->pos.y += particle->inc.y * LVUPDATE60FREAL();
		particle->pos.z += particle->inc.z * LVUPDATE60FREAL();

		if (particle->pos.y < data->boundarymin.y) {
			lVar6 = 2;

			// Reset particle
			particle->pos.x = data->boundarymin.x + RANDOMFRAC() * (ABS(data->boundarymin.x) + ABS(data->boundarymax.x));
			particle->pos.z = data->boundarymin.z + RANDOMFRAC() * (ABS(data->boundarymin.z) + ABS(data->boundarymax.z));

			particle->unk1c = RANDOMFRAC() + 0.7f;

			particle->inc.y = -(RANDOMFRAC() * g_RainSpeedExtra + weather->unkc8);

			if (ABS(relativetotal) < 2 && weather->unkd0 != weather->unkd4) {
				if (weather->unkd0 < weather->unkd4) {
					if ((particle->active & 3) == 0) {
						particle->active = true;
						relativetotal++;
						weather->unkd0++;
					}
				} else {
					if (particle->active & 3) {
						particle->active = false;
						relativetotal--;
						weather->unkd0--;
					}
				}
			}
		}

		if (lVar6 > 0) {
			particle->inc.x = weather->windspeedx * particle->unk1c;
			particle->inc.z = weather->windspeedz * particle->unk1c;
		}
	}

	if (weather->intensity == 0 && weather->unkd0 < 100) {
		func0f131678(3);
	}
}

u32 g_RainSpeedExtra = 20;
u32 g_SnowSpeed = 15;
u32 g_SnowSpeedExtra = 10;

void weatherTickSnow(struct weatherdata *weather)
{ \
	s32 lVar7 = 0;
	s32 relativetotal = 0; // eg. -10 if deleted 10 particles, +10 if created 10
	f32 rand;
	s32 lvupdate;
	s32 i;
	struct weatherparticledata *data;

	mainOverrideVariable("snowspeed", &g_SnowSpeed);
	mainOverrideVariable("snowspeedxtra", &g_SnowSpeedExtra);

#ifdef PLATFORM_N64
	if (g_StageIndex == STAGEINDEX_AIRBASE) {
		weather->windanglerad = 1.5707963705063f;
		weather->windspeedz = -weather->windspeed;
		weather->windspeedx = 0;
	}
#else
	if (g_CurWeatherConfig->flags & WEATHERFLAG_FORCE_WINDDIR) {
		// force weather direction if requested
		weather->windanglerad = g_CurWeatherConfig->windanglerad;
		weather->windspeedz = g_CurWeatherConfig->windspeedz;
		weather->windspeedx = g_CurWeatherConfig->windspeedx;
	}
#endif
	else if (weather->unk10 > 0) {
		s32 lvupdate = g_Vars.lvupdate60;

		if (weather->unk10 < lvupdate) {
			weather->windanglerad = weather->unk0c;
		} else {
			weather->windanglerad += lvupdate * ((weather->unk0c - weather->windanglerad) / weather->unk10);
			weather->unk10 -= lvupdate;
		}

		if (weather->windanglerad > M_TAU) {
			weather->windanglerad = 0;
		}

		weather->windspeedx = cosf(weather->windanglerad) * weather->windspeed;
		weather->windspeedz = sinf(weather->windanglerad) * weather->windspeed;
		lVar7 = 1;
	} else if (RANDOMFRAC() > 0.99f) {
		rand = RANDOMFRAC();

		weather->unk0c = (rand + rand) * M_PI;
		weather->unk10 = (weather->unk0c - weather->windanglerad) / (PAL ? 0.012f : 0.01f);

		if (weather->unk10 < 0) {
			weather->unk10 = -weather->unk10;
		}
	}

	data = g_WeatherData->particledata[0];

	// 0
	data->unk3ec8[0] += 0.04f * LVUPDATE60FREAL();

	if (data->unk3ec8[0] < 0) {
		data->unk3ec8[0] += M_TAU;
	}

	if (data->unk3ec8[0] > M_TAU) {
		data->unk3ec8[0] -= M_TAU;
	}

	// 1
	data->unk3ec8[1] += -0.03f * LVUPDATE60FREAL();

	if (data->unk3ec8[1] < 0) {
		data->unk3ec8[1] += M_TAU;
	}

	if (data->unk3ec8[1] > M_TAU) {
		data->unk3ec8[1] -= M_TAU;
	}

	// 2
	data->unk3ec8[2] += 0.04f * LVUPDATE60FREAL();

	if (data->unk3ec8[2] < 0) {
		data->unk3ec8[2] += M_TAU;
	}

	if (data->unk3ec8[2] > M_TAU) {
		data->unk3ec8[2] -= M_TAU;
	}

	// 3
	data->unk3ec8[3] += 0.03f * LVUPDATE60FREAL();

	if (data->unk3ec8[3] < 0) {
		data->unk3ec8[3] += M_TAU;
	}

	if (data->unk3ec8[3] > M_TAU) {
		data->unk3ec8[3] -= M_TAU;
	}

	// 4
	data->unk3ec8[4] += 0.02f * LVUPDATE60FREAL();

	if (data->unk3ec8[4] < 0) {
		data->unk3ec8[4] += M_TAU;
	}

	if (data->unk3ec8[4] > M_TAU) {
		data->unk3ec8[4] -= M_TAU;
	}

	// 5
	data->unk3ec8[5] += 0.01f * LVUPDATE60FREAL();

	if (data->unk3ec8[5] < 0) {
		data->unk3ec8[5] += M_TAU;
	}

	if (data->unk3ec8[5] > M_TAU) {
		data->unk3ec8[5] -= M_TAU;
	}

	// 6
	data->unk3ec8[6] += -0.01f * LVUPDATE60FREAL();

	if (data->unk3ec8[6] < 0) {
		data->unk3ec8[6] += M_TAU;
	}

	if (data->unk3ec8[6] > M_TAU) {
		data->unk3ec8[6] -= M_TAU;
	}

	// 7
	data->unk3ec8[7] += -0.02f * LVUPDATE60FREAL();

	if (data->unk3ec8[7] < 0) {
		data->unk3ec8[7] += M_TAU;
	}

	if (data->unk3ec8[7] > M_TAU) {
		data->unk3ec8[7] -= M_TAU;
	}

	for (i = 0; i < ARRAYCOUNT(data->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];

		particle->pos.x += particle->inc.x * LVUPDATE60FREAL();
		particle->pos.y += particle->inc.y * LVUPDATE60FREAL();
		particle->pos.z += particle->inc.z * LVUPDATE60FREAL();

		if (particle->pos.y < data->boundarymin.y) {
			lVar7 = 2;

			particle->pos.x = data->boundarymin.f[0] + RANDOMFRAC() * (ABS(data->boundarymin.f[0]) + ABS(data->boundarymax.f[0]));
			particle->pos.z = data->boundarymin.f[2] + RANDOMFRAC() * (ABS(data->boundarymin.f[2]) + ABS(data->boundarymax.f[2]));

			particle->unk1c = RANDOMFRAC() + 0.7f;

			particle->inc.y = -(g_SnowSpeed / 10.0f) - (RANDOMFRAC() * g_SnowSpeedExtra) / 10.0f;
			particle->inc.x = weather->windspeedx * particle->unk1c;
			particle->inc.z = weather->windspeedz * particle->unk1c;

			if (ABS(relativetotal) < 20 && weather->unkd0 != weather->unkd4) {
				if (weather->unkd0 < weather->unkd4) {
					if ((particle->active & 3) == 0) {
						particle->active = true;
						relativetotal++;
						weather->unkd0++;
					}
				} else if (particle->active & 3) {
					particle->active = false;
					relativetotal--;
					weather->unkd0--;
				}
			}
		}

		if (lVar7 > 0) {
			particle->inc.x = weather->windspeedx * particle->unk1c;
			particle->inc.z = weather->windspeedz * particle->unk1c;
		}
	}
}

void weatherConfigureRain(u32 intensity)
{
	if (g_WeatherData) {
		g_WeatherData->type = WEATHERTYPE_RAIN;
		weatherSetIntensity(intensity);
	}
}

void weatherConfigureSnow(u32 intensity)
{
	if (g_WeatherData) {
		g_WeatherData->type = WEATHERTYPE_SNOW;
		weatherSetIntensity(intensity);
	}
}

bool weatherIsRoomWeatherProof(s32 room)
{
#ifdef PLATFORM_N64
	if (g_StageIndex == STAGEINDEX_CHICAGO) {
		// Rooms listed do not have weather
		if (room == ROOM_PETE_0062
				|| room == ROOM_PETE_0061
				|| room == ROOM_PETE_000F
				|| room == ROOM_PETE_0052
				|| room == ROOM_PETE_0045
				|| room == ROOM_PETE_0044
				|| room == ROOM_PETE_0043
				|| room == ROOM_PETE_003D
				|| room == ROOM_PETE_000E
				|| room == ROOM_PETE_000D
				|| room == ROOM_PETE_000C
				|| room == ROOM_PETE_000B
				|| room == ROOM_PETE_000A
				|| room == ROOM_PETE_0009
				|| room == ROOM_PETE_0008
				|| room == ROOM_PETE_0007
				|| room == ROOM_PETE_0006
				|| room == ROOM_PETE_0005
				|| room == ROOM_PETE_0004
				|| room == ROOM_PETE_0037
				|| room == ROOM_PETE_005B
				|| room == ROOM_PETE_005F
				|| room == ROOM_PETE_0060
				|| room == ROOM_PETE_005D
				|| room == ROOM_PETE_001D
				|| room == ROOM_PETE_001F
				|| room == ROOM_PETE_0027
				|| room == ROOM_PETE_0050
				|| room == ROOM_PETE_002C
				|| room == ROOM_PETE_002F
				|| room == ROOM_PETE_0030
				|| room == ROOM_PETE_0011
				|| room == ROOM_PETE_0024
				|| room == ROOM_PETE_0033
				|| room == ROOM_PETE_0034
				|| room == ROOM_PETE_0035
				|| room == ROOM_PETE_0036
				|| room == ROOM_PETE_003C
				|| room == ROOM_PETE_0046
				|| room == ROOM_PETE_0047
				|| room == ROOM_PETE_0049
				|| room == ROOM_PETE_0055
				|| room == ROOM_PETE_0056
				|| room == ROOM_PETE_005E
				|| room == ROOM_PETE_0063
				|| room == ROOM_PETE_0069
				|| room == ROOM_PETE_0003
				|| room == ROOM_PETE_0016
				|| room == ROOM_PETE_004F
				|| room == ROOM_PETE_004B
				|| room == ROOM_PETE_003F
				|| room == ROOM_PETE_0013
				|| room == ROOM_PETE_0019) {
			return true;
		}

		return false;
	}

	if (g_StageIndex == STAGEINDEX_AIRBASE) {
		// Rooms listed have weather
		if (room == ROOM_CAVE_0088
				|| room == ROOM_CAVE_0091
				|| room == ROOM_CAVE_0087
				|| room == ROOM_CAVE_0090
				|| room == ROOM_CAVE_0086
				|| room == ROOM_CAVE_0080
				|| room == ROOM_CAVE_0083
				|| room == ROOM_CAVE_0084
				|| room == ROOM_CAVE_0085
				|| room == ROOM_CAVE_007F
				|| room == ROOM_CAVE_007E
				|| room == ROOM_CAVE_0082
				|| room == ROOM_CAVE_008F
				|| room == ROOM_CAVE_008E
				|| room == ROOM_CAVE_007B
				|| room == ROOM_CAVE_007C
				|| room == ROOM_CAVE_007D
				|| room == ROOM_CAVE_0081
				|| room == ROOM_CAVE_006F) {
			return false;
		}

		return true;
	}

	if (g_StageIndex == STAGEINDEX_CRASHSITE) {
		// Rooms listed do not have weather
		if (room == ROOM_AZT_001F
				|| room == ROOM_AZT_0020
				|| room == ROOM_AZT_0021
				|| room == ROOM_AZT_0022
				|| room == ROOM_AZT_0023
				|| room == ROOM_AZT_0024
				|| room == ROOM_AZT_0051
				|| room == ROOM_AZT_0052
				|| room == ROOM_AZT_0053
				|| room == ROOM_AZT_0054
				|| room == ROOM_AZT_0055
				|| room == ROOM_AZT_0056
				|| room == ROOM_AZT_0057
				|| room == ROOM_AZT_0058
				|| room == ROOM_AZT_0059
				|| room == ROOM_AZT_005A
				|| room == ROOM_AZT_005B
				|| room == ROOM_AZT_005C
				|| room == ROOM_AZT_005D
				|| room == ROOM_AZT_005E
				|| room == ROOM_AZT_005F
				|| room == ROOM_AZT_0060
				|| room == ROOM_AZT_0061
				|| room == ROOM_AZT_0062
				|| room == ROOM_AZT_0063
				|| room == ROOM_AZT_0064
				|| room == ROOM_AZT_002D
				|| room == ROOM_AZT_0040
				|| room == ROOM_AZT_0041
				|| room == ROOM_AZT_0042
				|| room == ROOM_AZT_0043
				|| room == ROOM_AZT_0044
				|| room == ROOM_AZT_0045
				|| room == ROOM_AZT_0046
				|| room == ROOM_AZT_0047
				|| room == ROOM_AZT_0048
				|| room == ROOM_AZT_0049
				|| room == ROOM_AZT_004A
				|| room == ROOM_AZT_004B
				|| room == ROOM_AZT_004C
				|| room == ROOM_AZT_004D
				|| room == ROOM_AZT_004E
				|| room == ROOM_AZT_004F
				|| room == ROOM_AZT_0050) {
			return true;
		}

		return false;
	}
#else
	if (room >= 0 && room < g_Vars.roomcount) {
		// check room's extra_flags
		if (g_Rooms[room].extra_flags & ROOMFLAG_EX_WEATHERPROOF) {
			// this room has no weather
			return true;
		}
	}
#endif

	return false;
}

Gfx *weatherRenderRain(Gfx *gdl, struct weatherdata *weather, s32 arg2)
{
	s32 numtestrooms;
	u32 stack;
	s32 p;
	s32 i;
	s32 timings1[10];
	struct weatherparticledata *particledata;
	Mtxf *mtx;
	struct weatherparticle *particle;
	s32 timings2[8];
	s32 numsparksavailable;
	s32 testrooms[50];
	f32 f0;
	struct coord spca8;
	struct coord spc9c;
	struct coord spc90;
	s32 brightness;
	s32 soundnum;
	f32 scale;
	s32 badrooms[50];
	struct coord badbbmin[50];
	struct coord badbbmax[50];
	s32 numbadrooms;
	s32 bboxes[50][6];

	static u32 rainwidth = 1;
	static u32 raincol1 = 0xaaaaaa1f;
	static u32 raincol2 = 0x11111844;
	static u32 rainout = 50;
	static u32 cddiv = 2500;
	static u32 wetclip = 1;
	static u32 bounder = 1;
	static u32 trypitch = 22000;

	numsparksavailable = 1;
	numtestrooms = 0;
	numbadrooms = 0;

	mainOverrideVariable("raincol1", &raincol1);
	mainOverrideVariable("raincol2", &raincol2);
	mainOverrideVariable("rainwidth", &rainwidth);
	mainOverrideVariable("rainout", &rainout);
	mainOverrideVariable("cddiv", &cddiv);
	mainOverrideVariable("wetclip", &wetclip);
	mainOverrideVariable("bounder", &bounder);
	mainOverrideVariable("trypitch", &trypitch);

	if (g_Vars.lvupdate240 <= 0) {
		numsparksavailable = 0;
	}

	osGetCount();

	for (i = 0; i < ARRAYCOUNT(timings1); i++) {
		timings1[i] = 0;
	}

	texSelect(&gdl, &g_TexGeneralConfigs[1], 2, 1, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0);

	{
		struct coord campos;
		struct coord sp224;
		s32 numneighbours;
		f32 sp218[2];
		f32 sp214;
		Vtx *vertices;
		s32 n;
		bool ok;
		Mtxf worldtoscreenmtx;
		struct coord positions[4];
		s32 numtris;
		f32 cddiv2;
		f32 rainout2;
		f32 f2;
		f32 frac;
		s32 volume;
		s32 t;
		s32 j;
		f32 pitch;
		Col *colours;
		f32 tmp;
		u32 stack2;
		struct coord distcamtobbmax;
		struct coord distcamtobbmin;
#ifdef AVOID_UB
		RoomNum neighbours[21];
#else
		RoomNum neighbours[20];
#endif

		particledata = weather->particledata[arg2];
		numtris = 0;

		mtx4LoadIdentity(&worldtoscreenmtx);
		mtx00015be0(camGetWorldToScreenMtxf(), &worldtoscreenmtx);

		worldtoscreenmtx.m[3][0] = 0.0f;
		worldtoscreenmtx.m[3][1] = 0.0f;
		worldtoscreenmtx.m[3][2] = 0.0f;

		mtx = gfxAllocateMatrix();

		mtxF2L(&worldtoscreenmtx, mtx);

		gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

		campos.f[0] = g_Vars.currentplayer->cam_pos.f[0];
		campos.f[1] = g_Vars.currentplayer->cam_pos.f[1];
		campos.f[2] = g_Vars.currentplayer->cam_pos.f[2];

		sp224.f[0] = campos.f[0] - particledata->unk3e80.f[0];
		sp224.f[1] = campos.f[1] - particledata->unk3e80.f[1];
		sp224.f[2] = campos.f[2] - particledata->unk3e80.f[2];

		if (ABSF(sp224.f[0]) > ABSF(particledata->boundarymin.f[0]) + ABSF(particledata->boundarymax.f[0])
				|| ABSF(sp224.f[1]) > ABSF(particledata->boundarymin.f[1]) + ABSF(particledata->boundarymax.f[1])
				|| ABSF(sp224.f[2]) > ABSF(particledata->boundarymin.f[2]) + ABSF(particledata->boundarymax.f[2])) {
			sp224.f[0] = particledata->boundaryrange.f[0] / 2.0f;
			sp224.f[1] = particledata->boundaryrange.f[1] / 2.0f;
			sp224.f[2] = particledata->boundaryrange.f[2] / 2.0f;
		}

		for (p = 0; p < ARRAYCOUNT(particledata->particles); p++) {
			particle = &particledata->particles[p];

			// x
			f0 = particle->pos.f[0] - particledata->boundarymin.f[0] - sp224.f[0];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[0];
			}

			if (f0 > particledata->boundaryrange.f[0]) {
				f0 -= particledata->boundaryrange.f[0];
			}

			particle->pos.f[0] = particledata->boundarymin.f[0] + f0;

			// y
			f0 = particle->pos.f[1] - particledata->boundarymin.f[1] - sp224.f[1];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[1];
			}

			if (f0 > particledata->boundaryrange.f[1]) {
				f0 -= particledata->boundaryrange.f[1];
			}

			particle->pos.f[1] = particledata->boundarymin.f[1] + f0;

			// z
			f0 = particle->pos.f[2] - particledata->boundarymin.f[2] - sp224.f[2];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[2];
			}

			if (f0 > particledata->boundaryrange.f[2]) {
				f0 -= particledata->boundaryrange.f[2];
			}

			particle->pos.f[2] = particledata->boundarymin.f[2] + f0;
		}

		particledata->unk3e80.f[0] = campos.f[0];
		particledata->unk3e80.f[1] = campos.f[1];
		particledata->unk3e80.f[2] = campos.f[2];

		// Increase or decrease rain volume depending on whether the camera
		// is in a weatherproof room or not.
		if (weatherIsRoomWeatherProof(g_Vars.currentplayer->cam_room)) {
			if (weather->unk88 > 0.99f) {
				weather->unk8c = 0.65f;
				weather->unk90 = 9;
			}
		} else {
			if (weather->unk88 < 0.66f) {
				weather->unk8c = 1.0f;
				weather->unk90 = 7;
			}
		}

		// Update thunder and lightning
		if (g_Vars.lvupdate240 > 0) {
			g_SkyLightningActive = false;

			if (weather->unk94 < 0) {
				if (RANDOMFRAC() < weather->unkc4) {
					func0f131610(weather);
				}
			} else {
				if (weather->unk98 - 1 == weather->unk94
						|| weather->unk9c - 1 == weather->unk94
						|| weather->unka0 - 1 == weather->unk94) {
					g_SkyLightningActive = true;
				}

				if (weather->unk98 == weather->unk94
						|| weather->unk9c == weather->unk94
						|| weather->unka0 == weather->unk94) {
					brightness = 150;

					if (weather->unk9c == weather->unk94) {
						brightness = 200;
					}

					for (i = 1; i < g_Vars.roomcount; i++) {
						if (!weatherIsRoomWeatherProof(i)) {
							roomSetFlashBrightness(i, brightness);
						}
					}
				}

				if (weather->unka4 == weather->unk94) {
					soundnum = SFX_80BA_THUNDER;
					pitch = 0.4f + RANDOMFRAC() * 1.5f;
					frac = RANDOMFRAC();

					if (frac <= 0.2f && frac > .1f) {
						soundnum = SFX_80BB_THUNDER;
					}

					if (frac <= 0.3f && frac > 0.20000001788139f) {
						soundnum = SFX_80BC_THUNDER;
					}

					if (frac <= 0.4f && frac > .3f) {
						soundnum = SFX_80BD_THUNDER;
					}

					if (frac <= 0.5f && frac > .4f) {
						soundnum = SFX_80BE_THUNDER;
					}

					if (frac <= 0.6f && frac > .5f) {
						soundnum = SFX_80BF_LIGHTNING;
					}

					if (frac <= 0.7f && frac > 0.59999996423721f) {
						soundnum = SFX_80C0_LIGHTNING;
					}

					if (frac <= 0.8f && frac > .7f) {
						soundnum = SFX_80C1_LIGHTNING;
					}

					if (frac <= 0.9f && frac > 0.79999995231628f) {
						soundnum = SFX_80C2_LIGHTNING;
					}

					if (frac <= 1.0f && frac > .9f) {
						soundnum = SFX_80C3_LIGHTNING;
					}

					if (weather->audiohandles[3] == NULL) {
						weather->unkf8 = soundnum;
						sndStart(var80095200, soundnum, &weather->audiohandles[3], -1, -1, -1, -1, -1);
						weather->unk58[3].unk00 = 1;

						if (weather->audiohandles[3] != NULL) {
							volume = weather->unk88;

							if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
								volume /= 2;
							}

							sndAdjust(&weather->audiohandles[3], 0, volume, -1, weather->unkf8, 1.00f, 1, -1, 1);
							audioPostEvent(weather->audiohandles[3], AL_SNDP_PITCH_EVT, *(s32 *)&pitch);
						}
					}
				}

				weather->unk94++;

				if (weather->unk94 > 150) {
					weather->unk94 = -1;
				}
			}
		}

		if (wetclip) {
			distcamtobbmin.f[0] = particledata->boundarymin.f[0] + g_Vars.currentplayer->cam_pos.f[0];
			distcamtobbmax.f[0] = particledata->boundarymax.f[0] + g_Vars.currentplayer->cam_pos.f[0];
			distcamtobbmin.f[1] = particledata->boundarymin.f[1] + g_Vars.currentplayer->cam_pos.f[1];
			distcamtobbmax.f[1] = particledata->boundarymax.f[1] + g_Vars.currentplayer->cam_pos.f[1];
			distcamtobbmin.f[2] = particledata->boundarymin.f[2] + g_Vars.currentplayer->cam_pos.f[2];
			distcamtobbmax.f[2] = particledata->boundarymax.f[2] + g_Vars.currentplayer->cam_pos.f[2];

			if (numtestrooms < ARRAYCOUNT(testrooms)) {
				testrooms[numtestrooms] = g_Vars.currentplayer->cam_room;
				numtestrooms++;
			}

			for (scale = 1.0f, t = 0; t < numtestrooms; t++) {
				numneighbours = bgRoomGetNeighbours(testrooms[t], neighbours, 20);

				for (n = 0; n < numneighbours; n++) {
					if (g_Rooms[neighbours[n]].flags & ROOMFLAG_ONSCREEN) {
						ok = true;

						for (j = 0; j < numtestrooms; j++) {
							if (testrooms[j] == neighbours[n]) {
								ok = false;
							}
						}

						if (ok) {
							if (distcamtobbmax.f[0] < g_Rooms[neighbours[n]].bbmin[0] || distcamtobbmin.f[0] > g_Rooms[neighbours[n]].bbmax[0]) {
								ok = false;
							}

							if (distcamtobbmax.f[1] < g_Rooms[neighbours[n]].bbmin[1] || distcamtobbmin.f[1] > g_Rooms[neighbours[n]].bbmax[1]) {
								ok = false;
							}

							if (distcamtobbmax.f[2] < g_Rooms[neighbours[n]].bbmin[2] || distcamtobbmin.f[2] > g_Rooms[neighbours[n]].bbmax[2]) {
								ok = false;
							}
						}

						if (ok && numtestrooms < ARRAYCOUNT(testrooms)) {
							testrooms[numtestrooms] = neighbours[n];
							numtestrooms++;
						} else {
							// empty
						}
					}
				}
			}

			for (t = 0; t < numtestrooms; t++) {
				if (weatherIsRoomWeatherProof(testrooms[t])) {
					// @bug: Overflowing badbbmin and badbbmax if badrooms is full.
					// (These writes should be inside the if statement).
					badbbmin[numbadrooms].f[0] = g_Rooms[testrooms[t]].bbmin[0] / scale;
					badbbmin[numbadrooms].f[1] = g_Rooms[testrooms[t]].bbmin[1] / scale;
					badbbmin[numbadrooms].f[2] = g_Rooms[testrooms[t]].bbmin[2] / scale;

					badbbmax[numbadrooms].f[0] = g_Rooms[testrooms[t]].bbmax[0] / scale;
					badbbmax[numbadrooms].f[1] = g_Rooms[testrooms[t]].bbmax[1] / scale;
					badbbmax[numbadrooms].f[2] = g_Rooms[testrooms[t]].bbmax[2] / scale;

					if (numbadrooms < ARRAYCOUNT(badrooms)) {
						badrooms[numbadrooms] = testrooms[t];
						numbadrooms++;
					}
				}

				if (1);
			}

			for (i = 0; i < numbadrooms; i++) {
				bboxes[i][0] = g_Rooms[badrooms[i]].bbmin[0];
				bboxes[i][1] = g_Rooms[badrooms[i]].bbmin[1];
				bboxes[i][2] = g_Rooms[badrooms[i]].bbmin[2];
				bboxes[i][3] = g_Rooms[badrooms[i]].bbmax[0];
				bboxes[i][4] = g_Rooms[badrooms[i]].bbmax[1];
				bboxes[i][5] = g_Rooms[badrooms[i]].bbmax[2];
			}
		}

		colours = gfxAllocateColours(2);
		colours[0].word = PD_BE32(raincol1);
		colours[1].word = PD_BE32(raincol2);

		gSPColor(gdl++, osVirtualToPhysical(colours), 2);

		timings2[0] = osGetCount();

		for (p = 0; p < ARRAYCOUNT(particledata->particles); p++) {
			u32 stack[3];
			struct coord sp108;
			struct coord spfc;
			u32 stack2;
			struct weatherparticle *particle2 = &particledata->particles[p];
			s32 vtxindex = numtris * 3;
			struct coord spe4;
			bool draw = true;
			struct coord spd4;

			if (particle2->active & 3) {
				timings2[7] = osGetCount();

				sp108.f[0] = particle2->pos.f[0] + particledata->unk3e80.f[0];
				sp108.f[1] = particle2->pos.f[1] + particledata->unk3e80.f[1];
				sp108.f[2] = particle2->pos.f[2] + particledata->unk3e80.f[2];

				if (cam0f0b5b9c(&sp108, 150)) {
					timings1[7] = timings1[7] + osGetCount() - timings2[7];

					sp218[0] = particle2->pos.f[0];
					sp218[1] = particle2->pos.f[2];

					sp214 = sqrtf(sp218[0] * sp218[0] + sp218[1] * sp218[1]);

					if (sp214 < 0.00001f) {
						// empty
					} else {
						if (numtris == 0) {
							vertices = gfxAllocateVertices(12);
						}

						sp218[0] /= sp214;
						sp218[1] /= sp214;

						for (i = 0; i < 4; i++) {
							vertices[i + vtxindex].s = 0;
							vertices[i + vtxindex].t = 0;

							positions[i].f[0] = particle2->pos.f[0];
							positions[i].f[1] = particle2->pos.f[1];
							positions[i].f[2] = particle2->pos.f[2];
						}

						timings2[1] = osGetCount();
						timings2[2] = osGetCount();

						if (wetclip && numbadrooms > 0) {
							spca8.f[0] = spc90.f[0] = (particle2->pos.f[0] + particledata->unk3e80.f[0]) * scale;
							spca8.f[1] = spc90.f[1] = (particle2->pos.f[1] + particledata->unk3e80.f[1]) * scale;
							spca8.f[2] = spc90.f[2] = (particle2->pos.f[2] + particledata->unk3e80.f[2]) * scale;

							spc9c.f[0] = ((particle2->pos.f[0] - (weather->windspeedx * (rainout / 10.0f))) + particledata->unk3e80.f[0]) * scale;
							spc9c.f[1] = ((particle2->pos.f[1] + weather->unkb8) + particledata->unk3e80.f[1]) * scale;
							spc9c.f[2] = ((particle2->pos.f[2] - (weather->windspeedz * (rainout / 10.0f))) + particledata->unk3e80.f[2]) * scale;

							spfc.f[0] = spc9c.f[0] - spc90.f[0];
							spfc.f[1] = spc9c.f[1] - spc90.f[1];
							spfc.f[2] = spc9c.f[2] - spc90.f[2];

							if (spca8.f[0] < spc9c.f[0]) {
								tmp = spc9c.f[0];
								spc9c.f[0] = spca8.f[0];
								spca8.f[0] = tmp;
							}

							if (spca8.f[1] < spc9c.f[1]) {
								tmp = spca8.f[1];
								spca8.f[1] = spc9c.f[1];
								spc9c.f[1] = tmp;
							}

							if (spca8.f[2] < spc9c.f[2]) {
								tmp = spca8.f[2];
								spca8.f[2] = spc9c.f[2];
								spc9c.f[2] = tmp;
							}

							timings2[3] = osGetCount();

							for (i = 0; i < numbadrooms; i++) {
								if (spc9c.f[0] <= g_Rooms[badrooms[i]].bbmax[0]
										&& spca8.f[0] >= g_Rooms[badrooms[i]].bbmin[0]
										&& spc9c.f[2] <= g_Rooms[badrooms[i]].bbmax[2]
										&& spca8.f[2] >= g_Rooms[badrooms[i]].bbmin[2]
										&& spc9c.f[1] <= g_Rooms[badrooms[i]].bbmax[1]
										&& spca8.f[1] >= g_Rooms[badrooms[i]].bbmin[1]
										&& bounder
										&& bgTestLineIntersectsIntBbox(&spc90, &spfc, &bboxes[i][0], &bboxes[i][3])) {
									draw = false;
								}
							}

							timings1[3] = timings1[3] + osGetCount() - timings2[3];
						}

						timings1[2] = timings1[2] + osGetCount() - timings2[2];

						if (draw) {
							timings2[4] = osGetCount();

							cddiv2 = cddiv / 10.0f;
							rainout2 = rainout / 10.0f;
							f2 = rainwidth * (1.0f + sp214 / cddiv2);

							positions[0].f[0] += -sp218[1] * -f2;
							positions[0].f[2] += sp218[0] * -f2;

							positions[1].f[0] += -sp218[1] * f2;
							positions[1].f[2] += sp218[0] * f2;

							positions[3].f[0] -= weather->windspeedx * rainout2 + -sp218[1] * f2;
							positions[3].f[1] += weather->unkb8;
							positions[3].f[2] -= weather->windspeedz * rainout2 + sp218[0] * f2;

							positions[2].f[0] -= weather->windspeedx * rainout2 + -sp218[1] * -f2;
							positions[2].f[1] += weather->unkb8;
							positions[2].f[2] -= weather->windspeedz * rainout2 + sp218[0] * -f2;

							// @bug: Writing to offset 3 overflows the vertices allocation.
							// The vertices array has 12 elements and is iterated 4 times,
							// incrementing vtxindex by 3 each time. This is harmless though,
							// as it writes into unallocated space unless the displaylist is full.
							// And if it's full then you have bigger problems to worry about.
							// Writes also occur with the s and t values further below.
							vertices[vtxindex + 0].colour = 0;
							vertices[vtxindex + 1].colour = 0;
							vertices[vtxindex + 2].colour = 4;
							vertices[vtxindex + 3].colour = 4;

							if (numsparksavailable > 0) {
								spe4.f[0] = (particle2->pos.f[0] + particledata->unk3e80.f[0]) * scale;
								spe4.f[1] = (particle2->pos.f[1] + particledata->unk3e80.f[1]) * scale;
								spe4.f[2] = (particle2->pos.f[2] + particledata->unk3e80.f[2]) * scale;

								for (i = 0; i < numtestrooms; i++) {
									if (spe4.f[0] <= g_Rooms[testrooms[i]].bbmax[0]
											&& spe4.f[0] >= g_Rooms[testrooms[i]].bbmin[0]
											&& spe4.f[2] <= g_Rooms[testrooms[i]].bbmax[2]
											&& spe4.f[2] >= g_Rooms[testrooms[i]].bbmin[2]
											&& spe4.f[1] <= g_Rooms[testrooms[i]].bbmax[1]
											&& spe4.f[1] >= g_Rooms[testrooms[i]].bbmin[1]
											&& spe4.f[1] + scale * (particle2->inc.f[1] * 1) < g_Rooms[testrooms[i]].bbmin[1]) {
										spd4.f[0] = particle2->pos.f[0] + particledata->unk3e80.f[0];
										spd4.f[1] = g_Rooms[testrooms[i]].bbmin[1] / scale;
										spd4.f[2] = particle2->pos.f[2] + particledata->unk3e80.f[2];

										sparksCreate(testrooms[i], NULL, &spd4, &particle2->inc, NULL, SPARKTYPE_SHALLOWWATER);

										numsparksavailable--;
									}
								}
							}

							timings1[4] = timings1[4] + osGetCount() - timings2[4];
							timings2[5] = osGetCount();

							vertices[vtxindex + 0].t = 256;
							vertices[vtxindex + 1].s = 256;
							vertices[vtxindex + 1].t = 256;
							vertices[vtxindex + 2].s = 256;
							vertices[vtxindex + 3].t = 0;
							vertices[vtxindex + 3].s = 0;
							vertices[vtxindex + 2].t = 0;
							vertices[vtxindex + 0].s = 0;

							timings1[5] = timings1[5] + osGetCount() - timings2[5];
							timings2[6] = osGetCount();

							vertices[vtxindex + 0].x = positions[0].f[0];
							vertices[vtxindex + 0].y = positions[0].f[1];
							vertices[vtxindex + 0].z = positions[0].f[2];

							vertices[vtxindex + 1].x = positions[1].f[0];
							vertices[vtxindex + 1].y = positions[1].f[1];
							vertices[vtxindex + 1].z = positions[1].f[2];

							vertices[vtxindex + 2].x = positions[2].f[0];
							vertices[vtxindex + 2].y = positions[2].f[1];
							vertices[vtxindex + 2].z = positions[2].f[2];

							if (numtris == 3) {
								gSPVertex(gdl++, osVirtualToPhysical(vertices), 12, 0);
								gSPTri4(gdl++, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
								numtris = 0;
							} else {
								numtris++;
							}

							timings1[6] = timings1[6] + osGetCount() - timings2[6];
							timings1[1] = timings1[1] + osGetCount() - timings2[1];
						}
					}

					if (numtris);
				}
			}
		}

		if (numtris > 0) {
			gSPVertex(gdl++, osVirtualToPhysical(vertices), 12, 0);

			if (numtris == 1) {
				gSPTri1(gdl++, 0, 1, 2);
			}

			if (numtris == 2) {
				gSPTri2(gdl++, 0, 1, 2, 3, 4, 5);
			}

			if (numtris == 3) {
				gSPTri3(gdl++, 0, 1, 2, 3, 4, 5, 6, 7, 8);
			}
		}
	}

	osGetCount();

	return gdl;
}

#if MATCHING
GLOBAL_ASM(
glabel weatherRenderSnow
.late_rodata
glabel var7f1b5784
.word 0x4096cbe4
glabel var7f1b5788
.word 0x40490fdb
glabel var7f1b578c
.word 0x3fc90fdb
glabel var7f1b5790
.word 0x3727c5ac
.text
/*  f1346b0:	27bdec68 */ 	addiu	$sp,$sp,-5016
/*  f1346b4:	afb1005c */ 	sw	$s1,0x5c($sp)
/*  f1346b8:	00a08825 */ 	or	$s1,$a1,$zero
/*  f1346bc:	afb30064 */ 	sw	$s3,0x64($sp)
/*  f1346c0:	00c09825 */ 	or	$s3,$a2,$zero
/*  f1346c4:	afbf007c */ 	sw	$ra,0x7c($sp)
/*  f1346c8:	afb70074 */ 	sw	$s7,0x74($sp)
/*  f1346cc:	afa41398 */ 	sw	$a0,0x1398($sp)
/*  f1346d0:	3c05800b */ 	lui	$a1,%hi(g_TexGeneralConfigs)
/*  f1346d4:	240e0002 */ 	addiu	$t6,$zero,0x2
/*  f1346d8:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f1346dc:	afbe0078 */ 	sw	$s8,0x78($sp)
/*  f1346e0:	afb60070 */ 	sw	$s6,0x70($sp)
/*  f1346e4:	afb5006c */ 	sw	$s5,0x6c($sp)
/*  f1346e8:	afb40068 */ 	sw	$s4,0x68($sp)
/*  f1346ec:	afb20060 */ 	sw	$s2,0x60($sp)
/*  f1346f0:	afb00058 */ 	sw	$s0,0x58($sp)
/*  f1346f4:	f7be0050 */ 	sdc1	$f30,0x50($sp)
/*  f1346f8:	f7bc0048 */ 	sdc1	$f28,0x48($sp)
/*  f1346fc:	f7ba0040 */ 	sdc1	$f26,0x40($sp)
/*  f134700:	f7b80038 */ 	sdc1	$f24,0x38($sp)
/*  f134704:	f7b60030 */ 	sdc1	$f22,0x30($sp)
/*  f134708:	f7b40028 */ 	sdc1	$f20,0x28($sp)
/*  f13470c:	0000b825 */ 	or	$s7,$zero,$zero
/*  f134710:	afaf0014 */ 	sw	$t7,0x14($sp)
/*  f134714:	afae0010 */ 	sw	$t6,0x10($sp)
/*  f134718:	8ca5b5a8 */ 	lw	$a1,%lo(g_TexGeneralConfigs)($a1)
/*  f13471c:	27a41398 */ 	addiu	$a0,$sp,0x1398
/*  f134720:	24060004 */ 	addiu	$a2,$zero,0x4
/*  f134724:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f134728:	afa01268 */ 	sw	$zero,0x1268($sp)
/*  f13472c:	0fc2ce70 */ 	jal	texSelect
/*  f134730:	00003825 */ 	or	$a3,$zero,$zero
/*  f134734:	8fb81398 */ 	lw	$t8,0x1398($sp)
/*  f134738:	8fa81268 */ 	lw	$t0,0x1268($sp)
/*  f13473c:	3c0eba00 */ 	lui	$t6,0xba00
/*  f134740:	27190008 */ 	addiu	$t9,$t8,0x8
/*  f134744:	afb91398 */ 	sw	$t9,0x1398($sp)
/*  f134748:	35ce1402 */ 	ori	$t6,$t6,0x1402
/*  f13474c:	af0e0000 */ 	sw	$t6,0x0($t8)
/*  f134750:	af000004 */ 	sw	$zero,0x4($t8)
/*  f134754:	8faf1398 */ 	lw	$t7,0x1398($sp)
/*  f134758:	3c19ba00 */ 	lui	$t9,0xba00
/*  f13475c:	37390602 */ 	ori	$t9,$t9,0x602
/*  f134760:	25f80008 */ 	addiu	$t8,$t7,0x8
/*  f134764:	afb81398 */ 	sw	$t8,0x1398($sp)
/*  f134768:	240e00c0 */ 	addiu	$t6,$zero,0xc0
/*  f13476c:	adee0004 */ 	sw	$t6,0x4($t7)
/*  f134770:	adf90000 */ 	sw	$t9,0x0($t7)
/*  f134774:	8faf1398 */ 	lw	$t7,0x1398($sp)
/*  f134778:	3c19b900 */ 	lui	$t9,0xb900
/*  f13477c:	3c0e0040 */ 	lui	$t6,0x40
/*  f134780:	25f80008 */ 	addiu	$t8,$t7,0x8
/*  f134784:	afb81398 */ 	sw	$t8,0x1398($sp)
/*  f134788:	35ce49d8 */ 	ori	$t6,$t6,0x49d8
/*  f13478c:	3739031d */ 	ori	$t9,$t9,0x31d
/*  f134790:	adf90000 */ 	sw	$t9,0x0($t7)
/*  f134794:	adee0004 */ 	sw	$t6,0x4($t7)
/*  f134798:	8faf1398 */ 	lw	$t7,0x1398($sp)
/*  f13479c:	3c19b900 */ 	lui	$t9,0xb900
/*  f1347a0:	37390002 */ 	ori	$t9,$t9,0x2
/*  f1347a4:	25f80008 */ 	addiu	$t8,$t7,0x8
/*  f1347a8:	afb81398 */ 	sw	$t8,0x1398($sp)
/*  f1347ac:	ade00004 */ 	sw	$zero,0x4($t7)
/*  f1347b0:	adf90000 */ 	sw	$t9,0x0($t7)
/*  f1347b4:	8fae1398 */ 	lw	$t6,0x1398($sp)
/*  f1347b8:	3c18ba00 */ 	lui	$t8,0xba00
/*  f1347bc:	37181001 */ 	ori	$t8,$t8,0x1001
/*  f1347c0:	25cf0008 */ 	addiu	$t7,$t6,0x8
/*  f1347c4:	afaf1398 */ 	sw	$t7,0x1398($sp)
/*  f1347c8:	adc00004 */ 	sw	$zero,0x4($t6)
/*  f1347cc:	add80000 */ 	sw	$t8,0x0($t6)
/*  f1347d0:	8fb91398 */ 	lw	$t9,0x1398($sp)
/*  f1347d4:	3c0fba00 */ 	lui	$t7,0xba00
/*  f1347d8:	35ef0903 */ 	ori	$t7,$t7,0x903
/*  f1347dc:	272e0008 */ 	addiu	$t6,$t9,0x8
/*  f1347e0:	afae1398 */ 	sw	$t6,0x1398($sp)
/*  f1347e4:	24180c00 */ 	addiu	$t8,$zero,0xc00
/*  f1347e8:	af380004 */ 	sw	$t8,0x4($t9)
/*  f1347ec:	af2f0000 */ 	sw	$t7,0x0($t9)
/*  f1347f0:	8fb91398 */ 	lw	$t9,0x1398($sp)
/*  f1347f4:	3c0ffcff */ 	lui	$t7,0xfcff
/*  f1347f8:	3c18ff32 */ 	lui	$t8,0xff32
/*  f1347fc:	272e0008 */ 	addiu	$t6,$t9,0x8
/*  f134800:	afae1398 */ 	sw	$t6,0x1398($sp)
/*  f134804:	37187f3f */ 	ori	$t8,$t8,0x7f3f
/*  f134808:	35ef99ff */ 	ori	$t7,$t7,0x99ff
/*  f13480c:	3c047f1b */ 	lui	$a0,%hi(var7f1b569c)
/*  f134810:	3c058008 */ 	lui	$a1,%hi(var8007f104)
/*  f134814:	af2f0000 */ 	sw	$t7,0x0($t9)
/*  f134818:	af380004 */ 	sw	$t8,0x4($t9)
/*  f13481c:	24a5f104 */ 	addiu	$a1,$a1,%lo(var8007f104)
/*  f134820:	2484569c */ 	addiu	$a0,$a0,%lo(var7f1b569c)
/*  f134824:	0c0036cc */ 	jal	mainOverrideVariable
/*  f134828:	afa81268 */ 	sw	$t0,0x1268($sp)
/*  f13482c:	3c047f1b */ 	lui	$a0,%hi(var7f1b56a8)
/*  f134830:	3c058008 */ 	lui	$a1,%hi(var8007f108)
/*  f134834:	24a5f108 */ 	addiu	$a1,$a1,%lo(var8007f108)
/*  f134838:	0c0036cc */ 	jal	mainOverrideVariable
/*  f13483c:	248456a8 */ 	addiu	$a0,$a0,%lo(var7f1b56a8)
/*  f134840:	3c047f1b */ 	lui	$a0,%hi(var7f1b56b4)
/*  f134844:	3c058008 */ 	lui	$a1,%hi(var8007f10c)
/*  f134848:	24a5f10c */ 	addiu	$a1,$a1,%lo(var8007f10c)
/*  f13484c:	0c0036cc */ 	jal	mainOverrideVariable
/*  f134850:	248456b4 */ 	addiu	$a0,$a0,%lo(var7f1b56b4)
/*  f134854:	3c047f1b */ 	lui	$a0,%hi(var7f1b56c0)
/*  f134858:	3c058008 */ 	lui	$a1,%hi(var8007f110)
/*  f13485c:	24a5f110 */ 	addiu	$a1,$a1,%lo(var8007f110)
/*  f134860:	0c0036cc */ 	jal	mainOverrideVariable
/*  f134864:	248456c0 */ 	addiu	$a0,$a0,%lo(var7f1b56c0)
/*  f134868:	0013c880 */ 	sll	$t9,$s3,0x2
/*  f13486c:	02397021 */ 	addu	$t6,$s1,$t9
/*  f134870:	27b001cc */ 	addiu	$s0,$sp,0x1cc
/*  f134874:	8dd20024 */ 	lw	$s2,0x24($t6)
/*  f134878:	afa00198 */ 	sw	$zero,0x198($sp)
/*  f13487c:	0c00566c */ 	jal	mtx4LoadIdentity
/*  f134880:	02002025 */ 	or	$a0,$s0,$zero
/*  f134884:	0fc2d5be */ 	jal	camGetWorldToScreenMtxf
/*  f134888:	00000000 */ 	nop
/*  f13488c:	00402025 */ 	or	$a0,$v0,$zero
/*  f134890:	0c0056f8 */ 	jal	mtx00015be0
/*  f134894:	02002825 */ 	or	$a1,$s0,$zero
/*  f134898:	4480a000 */ 	mtc1	$zero,$f20
/*  f13489c:	00000000 */ 	nop
/*  f1348a0:	e7b401fc */ 	swc1	$f20,0x1fc($sp)
/*  f1348a4:	e7b40200 */ 	swc1	$f20,0x200($sp)
/*  f1348a8:	0fc59e66 */ 	jal	gfxAllocateMatrix
/*  f1348ac:	e7b40204 */ 	swc1	$f20,0x204($sp)
/*  f1348b0:	00408825 */ 	or	$s1,$v0,$zero
/*  f1348b4:	02002025 */ 	or	$a0,$s0,$zero
/*  f1348b8:	0c005815 */ 	jal	mtxF2L
/*  f1348bc:	00402825 */ 	or	$a1,$v0,$zero
/*  f1348c0:	8fb01398 */ 	lw	$s0,0x1398($sp)
/*  f1348c4:	3c190102 */ 	lui	$t9,0x102
/*  f1348c8:	37390040 */ 	ori	$t9,$t9,0x40
/*  f1348cc:	26180008 */ 	addiu	$t8,$s0,0x8
/*  f1348d0:	afb81398 */ 	sw	$t8,0x1398($sp)
/*  f1348d4:	02202025 */ 	or	$a0,$s1,$zero
/*  f1348d8:	0c012d20 */ 	jal	osVirtualToPhysical
/*  f1348dc:	ae190000 */ 	sw	$t9,0x0($s0)
/*  f1348e0:	3c05800a */ 	lui	$a1,%hi(g_Vars)
/*  f1348e4:	24a59fc0 */ 	addiu	$a1,$a1,%lo(g_Vars)
/*  f1348e8:	ae020004 */ 	sw	$v0,0x4($s0)
/*  f1348ec:	8ca30284 */ 	lw	$v1,0x284($a1)
/*  f1348f0:	00001025 */ 	or	$v0,$zero,$zero
/*  f1348f4:	02408025 */ 	or	$s0,$s2,$zero
/*  f1348f8:	c4641bb0 */ 	lwc1	$f4,0x1bb0($v1)
/*  f1348fc:	0000f025 */ 	or	$s8,$zero,$zero
/*  f134900:	e7a40234 */ 	swc1	$f4,0x234($sp)
/*  f134904:	c4661bb4 */ 	lwc1	$f6,0x1bb4($v1)
/*  f134908:	c7aa0234 */ 	lwc1	$f10,0x234($sp)
/*  f13490c:	e7a60238 */ 	swc1	$f6,0x238($sp)
/*  f134910:	c4681bb8 */ 	lwc1	$f8,0x1bb8($v1)
/*  f134914:	e7a8023c */ 	swc1	$f8,0x23c($sp)
/*  f134918:	c6443e80 */ 	lwc1	$f4,0x3e80($s2)
/*  f13491c:	c7a80238 */ 	lwc1	$f8,0x238($sp)
/*  f134920:	46045181 */ 	sub.s	$f6,$f10,$f4
/*  f134924:	e7a60228 */ 	swc1	$f6,0x228($sp)
/*  f134928:	c64a3e84 */ 	lwc1	$f10,0x3e84($s2)
/*  f13492c:	4606a03c */ 	c.lt.s	$f20,$f6
/*  f134930:	460a4101 */ 	sub.s	$f4,$f8,$f10
/*  f134934:	c7a8023c */ 	lwc1	$f8,0x23c($sp)
/*  f134938:	e7a4022c */ 	swc1	$f4,0x22c($sp)
/*  f13493c:	c64a3e88 */ 	lwc1	$f10,0x3e88($s2)
/*  f134940:	460a4101 */ 	sub.s	$f4,$f8,$f10
/*  f134944:	45000003 */ 	bc1f	.L0f134954
/*  f134948:	e7a40230 */ 	swc1	$f4,0x230($sp)
/*  f13494c:	10000003 */ 	b	.L0f13495c
/*  f134950:	46003006 */ 	mov.s	$f0,$f6
.L0f134954:
/*  f134954:	c7a00228 */ 	lwc1	$f0,0x228($sp)
/*  f134958:	46000007 */ 	neg.s	$f0,$f0
.L0f13495c:
/*  f13495c:	c6423e98 */ 	lwc1	$f2,0x3e98($s2)
/*  f134960:	4602a03c */ 	c.lt.s	$f20,$f2
/*  f134964:	00000000 */ 	nop
/*  f134968:	45020004 */ 	bc1fl	.L0f13497c
/*  f13496c:	46001387 */ 	neg.s	$f14,$f2
/*  f134970:	10000002 */ 	b	.L0f13497c
/*  f134974:	46001386 */ 	mov.s	$f14,$f2
/*  f134978:	46001387 */ 	neg.s	$f14,$f2
.L0f13497c:
/*  f13497c:	c6423e8c */ 	lwc1	$f2,0x3e8c($s2)
/*  f134980:	4602a03c */ 	c.lt.s	$f20,$f2
/*  f134984:	00000000 */ 	nop
/*  f134988:	45020004 */ 	bc1fl	.L0f13499c
/*  f13498c:	46001307 */ 	neg.s	$f12,$f2
/*  f134990:	10000002 */ 	b	.L0f13499c
/*  f134994:	46001306 */ 	mov.s	$f12,$f2
/*  f134998:	46001307 */ 	neg.s	$f12,$f2
.L0f13499c:
/*  f13499c:	460e6200 */ 	add.s	$f8,$f12,$f14
/*  f1349a0:	4600403c */ 	c.lt.s	$f8,$f0
/*  f1349a4:	00000000 */ 	nop
/*  f1349a8:	4503003b */ 	bc1tl	.L0f134a98
/*  f1349ac:	3c013f00 */ 	lui	$at,0x3f00
/*  f1349b0:	c7a0022c */ 	lwc1	$f0,0x22c($sp)
/*  f1349b4:	4600a03c */ 	c.lt.s	$f20,$f0
/*  f1349b8:	00000000 */ 	nop
/*  f1349bc:	45020004 */ 	bc1fl	.L0f1349d0
/*  f1349c0:	46000387 */ 	neg.s	$f14,$f0
/*  f1349c4:	10000002 */ 	b	.L0f1349d0
/*  f1349c8:	46000386 */ 	mov.s	$f14,$f0
/*  f1349cc:	46000387 */ 	neg.s	$f14,$f0
.L0f1349d0:
/*  f1349d0:	c64c3e9c */ 	lwc1	$f12,0x3e9c($s2)
/*  f1349d4:	460ca03c */ 	c.lt.s	$f20,$f12
/*  f1349d8:	00000000 */ 	nop
/*  f1349dc:	45020004 */ 	bc1fl	.L0f1349f0
/*  f1349e0:	46006407 */ 	neg.s	$f16,$f12
/*  f1349e4:	10000002 */ 	b	.L0f1349f0
/*  f1349e8:	46006406 */ 	mov.s	$f16,$f12
/*  f1349ec:	46006407 */ 	neg.s	$f16,$f12
.L0f1349f0:
/*  f1349f0:	c6403e90 */ 	lwc1	$f0,0x3e90($s2)
/*  f1349f4:	4600a03c */ 	c.lt.s	$f20,$f0
/*  f1349f8:	00000000 */ 	nop
/*  f1349fc:	45020004 */ 	bc1fl	.L0f134a10
/*  f134a00:	46000087 */ 	neg.s	$f2,$f0
/*  f134a04:	10000002 */ 	b	.L0f134a10
/*  f134a08:	46000086 */ 	mov.s	$f2,$f0
/*  f134a0c:	46000087 */ 	neg.s	$f2,$f0
.L0f134a10:
/*  f134a10:	46101280 */ 	add.s	$f10,$f2,$f16
/*  f134a14:	c7a00230 */ 	lwc1	$f0,0x230($sp)
/*  f134a18:	460e503c */ 	c.lt.s	$f10,$f14
/*  f134a1c:	00000000 */ 	nop
/*  f134a20:	4503001d */ 	bc1tl	.L0f134a98
/*  f134a24:	3c013f00 */ 	lui	$at,0x3f00
/*  f134a28:	4600a03c */ 	c.lt.s	$f20,$f0
/*  f134a2c:	44802000 */ 	mtc1	$zero,$f4
/*  f134a30:	45020004 */ 	bc1fl	.L0f134a44
/*  f134a34:	46000307 */ 	neg.s	$f12,$f0
/*  f134a38:	10000002 */ 	b	.L0f134a44
/*  f134a3c:	46000306 */ 	mov.s	$f12,$f0
/*  f134a40:	46000307 */ 	neg.s	$f12,$f0
.L0f134a44:
/*  f134a44:	c64e3ea0 */ 	lwc1	$f14,0x3ea0($s2)
/*  f134a48:	460ea03c */ 	c.lt.s	$f20,$f14
/*  f134a4c:	00000000 */ 	nop
/*  f134a50:	45020004 */ 	bc1fl	.L0f134a64
/*  f134a54:	46007407 */ 	neg.s	$f16,$f14
/*  f134a58:	10000002 */ 	b	.L0f134a64
/*  f134a5c:	46007406 */ 	mov.s	$f16,$f14
/*  f134a60:	46007407 */ 	neg.s	$f16,$f14
.L0f134a64:
/*  f134a64:	c6403e94 */ 	lwc1	$f0,0x3e94($s2)
/*  f134a68:	4600203c */ 	c.lt.s	$f4,$f0
/*  f134a6c:	00000000 */ 	nop
/*  f134a70:	45020004 */ 	bc1fl	.L0f134a84
/*  f134a74:	46000087 */ 	neg.s	$f2,$f0
/*  f134a78:	10000002 */ 	b	.L0f134a84
/*  f134a7c:	46000086 */ 	mov.s	$f2,$f0
/*  f134a80:	46000087 */ 	neg.s	$f2,$f0
.L0f134a84:
/*  f134a84:	46101180 */ 	add.s	$f6,$f2,$f16
/*  f134a88:	460c303c */ 	c.lt.s	$f6,$f12
/*  f134a8c:	00000000 */ 	nop
/*  f134a90:	4500000b */ 	bc1f	.L0f134ac0
/*  f134a94:	3c013f00 */ 	lui	$at,0x3f00
.L0f134a98:
/*  f134a98:	44810000 */ 	mtc1	$at,$f0
/*  f134a9c:	c6483ea4 */ 	lwc1	$f8,0x3ea4($s2)
/*  f134aa0:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f134aa4:	e7aa0228 */ 	swc1	$f10,0x228($sp)
/*  f134aa8:	c6443ea8 */ 	lwc1	$f4,0x3ea8($s2)
/*  f134aac:	46002182 */ 	mul.s	$f6,$f4,$f0
/*  f134ab0:	e7a6022c */ 	swc1	$f6,0x22c($sp)
/*  f134ab4:	c6483eac */ 	lwc1	$f8,0x3eac($s2)
/*  f134ab8:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f134abc:	e7aa0230 */ 	swc1	$f10,0x230($sp)
.L0f134ac0:
/*  f134ac0:	4480a000 */ 	mtc1	$zero,$f20
/*  f134ac4:	24043e80 */ 	addiu	$a0,$zero,0x3e80
.L0f134ac8:
/*  f134ac8:	c6423e98 */ 	lwc1	$f2,0x3e98($s2)
/*  f134acc:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f134ad0:	c7a80228 */ 	lwc1	$f8,0x228($sp)
/*  f134ad4:	02001825 */ 	or	$v1,$s0,$zero
/*  f134ad8:	46022181 */ 	sub.s	$f6,$f4,$f2
/*  f134adc:	46083001 */ 	sub.s	$f0,$f6,$f8
/*  f134ae0:	4614003c */ 	c.lt.s	$f0,$f20
/*  f134ae4:	00000000 */ 	nop
/*  f134ae8:	45020004 */ 	bc1fl	.L0f134afc
/*  f134aec:	c64c3ea4 */ 	lwc1	$f12,0x3ea4($s2)
/*  f134af0:	c64a3ea4 */ 	lwc1	$f10,0x3ea4($s2)
/*  f134af4:	460a0000 */ 	add.s	$f0,$f0,$f10
/*  f134af8:	c64c3ea4 */ 	lwc1	$f12,0x3ea4($s2)
.L0f134afc:
/*  f134afc:	4600603c */ 	c.lt.s	$f12,$f0
/*  f134b00:	00000000 */ 	nop
/*  f134b04:	45020003 */ 	bc1fl	.L0f134b14
/*  f134b08:	46001100 */ 	add.s	$f4,$f2,$f0
/*  f134b0c:	460c0001 */ 	sub.s	$f0,$f0,$f12
/*  f134b10:	46001100 */ 	add.s	$f4,$f2,$f0
.L0f134b14:
/*  f134b14:	c6060004 */ 	lwc1	$f6,0x4($s0)
/*  f134b18:	e6040000 */ 	swc1	$f4,0x0($s0)
/*  f134b1c:	c64c3e9c */ 	lwc1	$f12,0x3e9c($s2)
/*  f134b20:	c7aa022c */ 	lwc1	$f10,0x22c($sp)
/*  f134b24:	460c3201 */ 	sub.s	$f8,$f6,$f12
/*  f134b28:	460a4001 */ 	sub.s	$f0,$f8,$f10
/*  f134b2c:	4614003c */ 	c.lt.s	$f0,$f20
/*  f134b30:	00000000 */ 	nop
/*  f134b34:	45020004 */ 	bc1fl	.L0f134b48
/*  f134b38:	c6423ea8 */ 	lwc1	$f2,0x3ea8($s2)
/*  f134b3c:	c6443ea8 */ 	lwc1	$f4,0x3ea8($s2)
/*  f134b40:	46040000 */ 	add.s	$f0,$f0,$f4
/*  f134b44:	c6423ea8 */ 	lwc1	$f2,0x3ea8($s2)
.L0f134b48:
/*  f134b48:	4600103c */ 	c.lt.s	$f2,$f0
/*  f134b4c:	00000000 */ 	nop
/*  f134b50:	45020003 */ 	bc1fl	.L0f134b60
/*  f134b54:	46006180 */ 	add.s	$f6,$f12,$f0
/*  f134b58:	46020001 */ 	sub.s	$f0,$f0,$f2
/*  f134b5c:	46006180 */ 	add.s	$f6,$f12,$f0
.L0f134b60:
/*  f134b60:	c6080008 */ 	lwc1	$f8,0x8($s0)
/*  f134b64:	e6060004 */ 	swc1	$f6,0x4($s0)
/*  f134b68:	c64e3ea0 */ 	lwc1	$f14,0x3ea0($s2)
/*  f134b6c:	c7a40230 */ 	lwc1	$f4,0x230($sp)
/*  f134b70:	460e4281 */ 	sub.s	$f10,$f8,$f14
/*  f134b74:	46045001 */ 	sub.s	$f0,$f10,$f4
/*  f134b78:	4614003c */ 	c.lt.s	$f0,$f20
/*  f134b7c:	00000000 */ 	nop
/*  f134b80:	45020004 */ 	bc1fl	.L0f134b94
/*  f134b84:	c6423eac */ 	lwc1	$f2,0x3eac($s2)
/*  f134b88:	c6463eac */ 	lwc1	$f6,0x3eac($s2)
/*  f134b8c:	46060000 */ 	add.s	$f0,$f0,$f6
/*  f134b90:	c6423eac */ 	lwc1	$f2,0x3eac($s2)
.L0f134b94:
/*  f134b94:	4600103c */ 	c.lt.s	$f2,$f0
/*  f134b98:	00000000 */ 	nop
/*  f134b9c:	45020003 */ 	bc1fl	.L0f134bac
/*  f134ba0:	46007200 */ 	add.s	$f8,$f14,$f0
/*  f134ba4:	46020001 */ 	sub.s	$f0,$f0,$f2
/*  f134ba8:	46007200 */ 	add.s	$f8,$f14,$f0
.L0f134bac:
/*  f134bac:	24420020 */ 	addiu	$v0,$v0,0x20
/*  f134bb0:	26100020 */ 	addiu	$s0,$s0,0x20
/*  f134bb4:	1444ffc4 */ 	bne	$v0,$a0,.L0f134ac8
/*  f134bb8:	e4680008 */ 	swc1	$f8,0x8($v1)
/*  f134bbc:	c7aa0234 */ 	lwc1	$f10,0x234($sp)
/*  f134bc0:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f134bc4:	00006825 */ 	or	$t5,$zero,$zero
/*  f134bc8:	e64a3e80 */ 	swc1	$f10,0x3e80($s2)
/*  f134bcc:	c7a40238 */ 	lwc1	$f4,0x238($sp)
/*  f134bd0:	c64a3e98 */ 	lwc1	$f10,0x3e98($s2)
/*  f134bd4:	27ac126c */ 	addiu	$t4,$sp,0x126c
/*  f134bd8:	e6443e84 */ 	swc1	$f4,0x3e84($s2)
/*  f134bdc:	c7a6023c */ 	lwc1	$f6,0x23c($sp)
/*  f134be0:	27b30144 */ 	addiu	$s3,$sp,0x144
/*  f134be4:	2411008c */ 	addiu	$s1,$zero,0x8c
/*  f134be8:	e6463e88 */ 	swc1	$f6,0x3e88($s2)
/*  f134bec:	8ca30284 */ 	lw	$v1,0x284($a1)
/*  f134bf0:	27b0126c */ 	addiu	$s0,$sp,0x126c
/*  f134bf4:	c4681bb0 */ 	lwc1	$f8,0x1bb0($v1)
/*  f134bf8:	460a4100 */ 	add.s	$f4,$f8,$f10
/*  f134bfc:	e7a4016c */ 	swc1	$f4,0x16c($sp)
/*  f134c00:	c6483e8c */ 	lwc1	$f8,0x3e8c($s2)
/*  f134c04:	c4661bb0 */ 	lwc1	$f6,0x1bb0($v1)
/*  f134c08:	46083280 */ 	add.s	$f10,$f6,$f8
/*  f134c0c:	e7aa0178 */ 	swc1	$f10,0x178($sp)
/*  f134c10:	c6463e9c */ 	lwc1	$f6,0x3e9c($s2)
/*  f134c14:	c4641bb4 */ 	lwc1	$f4,0x1bb4($v1)
/*  f134c18:	46062200 */ 	add.s	$f8,$f4,$f6
/*  f134c1c:	e7a80170 */ 	swc1	$f8,0x170($sp)
/*  f134c20:	c6443e90 */ 	lwc1	$f4,0x3e90($s2)
/*  f134c24:	c46a1bb4 */ 	lwc1	$f10,0x1bb4($v1)
/*  f134c28:	46045180 */ 	add.s	$f6,$f10,$f4
/*  f134c2c:	e7a6017c */ 	swc1	$f6,0x17c($sp)
/*  f134c30:	c64a3ea0 */ 	lwc1	$f10,0x3ea0($s2)
/*  f134c34:	c4681bb8 */ 	lwc1	$f8,0x1bb8($v1)
/*  f134c38:	460a4100 */ 	add.s	$f4,$f8,$f10
/*  f134c3c:	e7a40174 */ 	swc1	$f4,0x174($sp)
/*  f134c40:	c6483e94 */ 	lwc1	$f8,0x3e94($s2)
/*  f134c44:	c4661bb8 */ 	lwc1	$f6,0x1bb8($v1)
/*  f134c48:	46083280 */ 	add.s	$f10,$f6,$f8
/*  f134c4c:	e7aa0180 */ 	swc1	$f10,0x180($sp)
/*  f134c50:	8c6e1ba0 */ 	lw	$t6,0x1ba0($v1)
/*  f134c54:	1900005b */ 	blez	$t0,.L0f134dc4
/*  f134c58:	afae126c */ 	sw	$t6,0x126c($sp)
.L0f134c5c:
/*  f134c5c:	8d840000 */ 	lw	$a0,0x0($t4)
/*  f134c60:	afad0184 */ 	sw	$t5,0x184($sp)
/*  f134c64:	afac00ac */ 	sw	$t4,0xac($sp)
/*  f134c68:	afa81268 */ 	sw	$t0,0x1268($sp)
/*  f134c6c:	02602825 */ 	or	$a1,$s3,$zero
/*  f134c70:	24060014 */ 	addiu	$a2,$zero,0x14
/*  f134c74:	0fc5916a */ 	jal	bgRoomGetNeighbours
/*  f134c78:	00001025 */ 	or	$v0,$zero,$zero
/*  f134c7c:	8fa81268 */ 	lw	$t0,0x1268($sp)
/*  f134c80:	8fac00ac */ 	lw	$t4,0xac($sp)
/*  f134c84:	1840004a */ 	blez	$v0,.L0f134db0
/*  f134c88:	8fad0184 */ 	lw	$t5,0x184($sp)
/*  f134c8c:	3c0b800a */ 	lui	$t3,%hi(g_Rooms)
/*  f134c90:	27a70144 */ 	addiu	$a3,$sp,0x144
/*  f134c94:	0002c040 */ 	sll	$t8,$v0,0x1
/*  f134c98:	03075021 */ 	addu	$t2,$t8,$a3
/*  f134c9c:	8d6b4928 */ 	lw	$t3,%lo(g_Rooms)($t3)
/*  f134ca0:	84e50000 */ 	lh	$a1,0x0($a3)
.L0f134ca4:
/*  f134ca4:	24e70002 */ 	addiu	$a3,$a3,0x2
/*  f134ca8:	24040001 */ 	addiu	$a0,$zero,0x1
/*  f134cac:	00b10019 */ 	multu	$a1,$s1
/*  f134cb0:	0000c812 */ 	mflo	$t9
/*  f134cb4:	01793021 */ 	addu	$a2,$t3,$t9
/*  f134cb8:	94ce0000 */ 	lhu	$t6,0x0($a2)
/*  f134cbc:	31d80004 */ 	andi	$t8,$t6,0x4
/*  f134cc0:	53000039 */ 	beqzl	$t8,.L0f134da8
/*  f134cc4:	00ea082b */ 	sltu	$at,$a3,$t2
/*  f134cc8:	19000009 */ 	blez	$t0,.L0f134cf0
/*  f134ccc:	00001025 */ 	or	$v0,$zero,$zero
/*  f134cd0:	27a3126c */ 	addiu	$v1,$sp,0x126c
.L0f134cd4:
/*  f134cd4:	8c6f0000 */ 	lw	$t7,0x0($v1)
/*  f134cd8:	24420001 */ 	addiu	$v0,$v0,0x1
/*  f134cdc:	15e50002 */ 	bne	$t7,$a1,.L0f134ce8
/*  f134ce0:	00000000 */ 	nop
/*  f134ce4:	00002025 */ 	or	$a0,$zero,$zero
.L0f134ce8:
/*  f134ce8:	1448fffa */ 	bne	$v0,$t0,.L0f134cd4
/*  f134cec:	24630004 */ 	addiu	$v1,$v1,0x4
.L0f134cf0:
/*  f134cf0:	10800025 */ 	beqz	$a0,.L0f134d88
/*  f134cf4:	c7a40178 */ 	lwc1	$f4,0x178($sp)
/*  f134cf8:	c4c60018 */ 	lwc1	$f6,0x18($a2)
/*  f134cfc:	c7aa016c */ 	lwc1	$f10,0x16c($sp)
/*  f134d00:	4606203c */ 	c.lt.s	$f4,$f6
/*  f134d04:	c7a4017c */ 	lwc1	$f4,0x17c($sp)
/*  f134d08:	45030007 */ 	bc1tl	.L0f134d28
/*  f134d0c:	00002025 */ 	or	$a0,$zero,$zero
/*  f134d10:	c4c80024 */ 	lwc1	$f8,0x24($a2)
/*  f134d14:	460a403c */ 	c.lt.s	$f8,$f10
/*  f134d18:	00000000 */ 	nop
/*  f134d1c:	45020003 */ 	bc1fl	.L0f134d2c
/*  f134d20:	c4c6001c */ 	lwc1	$f6,0x1c($a2)
/*  f134d24:	00002025 */ 	or	$a0,$zero,$zero
.L0f134d28:
/*  f134d28:	c4c6001c */ 	lwc1	$f6,0x1c($a2)
.L0f134d2c:
/*  f134d2c:	c7aa0170 */ 	lwc1	$f10,0x170($sp)
/*  f134d30:	4606203c */ 	c.lt.s	$f4,$f6
/*  f134d34:	c7a40180 */ 	lwc1	$f4,0x180($sp)
/*  f134d38:	45030007 */ 	bc1tl	.L0f134d58
/*  f134d3c:	00002025 */ 	or	$a0,$zero,$zero
/*  f134d40:	c4c80028 */ 	lwc1	$f8,0x28($a2)
/*  f134d44:	460a403c */ 	c.lt.s	$f8,$f10
/*  f134d48:	00000000 */ 	nop
/*  f134d4c:	45020003 */ 	bc1fl	.L0f134d5c
/*  f134d50:	c4c60020 */ 	lwc1	$f6,0x20($a2)
/*  f134d54:	00002025 */ 	or	$a0,$zero,$zero
.L0f134d58:
/*  f134d58:	c4c60020 */ 	lwc1	$f6,0x20($a2)
.L0f134d5c:
/*  f134d5c:	c7aa0174 */ 	lwc1	$f10,0x174($sp)
/*  f134d60:	4606203c */ 	c.lt.s	$f4,$f6
/*  f134d64:	00000000 */ 	nop
/*  f134d68:	45030007 */ 	bc1tl	.L0f134d88
/*  f134d6c:	00002025 */ 	or	$a0,$zero,$zero
/*  f134d70:	c4c8002c */ 	lwc1	$f8,0x2c($a2)
/*  f134d74:	460a403c */ 	c.lt.s	$f8,$f10
/*  f134d78:	00000000 */ 	nop
/*  f134d7c:	45000002 */ 	bc1f	.L0f134d88
/*  f134d80:	00000000 */ 	nop
/*  f134d84:	00002025 */ 	or	$a0,$zero,$zero
.L0f134d88:
/*  f134d88:	10800006 */ 	beqz	$a0,.L0f134da4
/*  f134d8c:	29010032 */ 	slti	$at,$t0,0x32
/*  f134d90:	10200004 */ 	beqz	$at,.L0f134da4
/*  f134d94:	0008c880 */ 	sll	$t9,$t0,0x2
/*  f134d98:	02197021 */ 	addu	$t6,$s0,$t9
/*  f134d9c:	adc50000 */ 	sw	$a1,0x0($t6)
/*  f134da0:	25080001 */ 	addiu	$t0,$t0,0x1
.L0f134da4:
/*  f134da4:	00ea082b */ 	sltu	$at,$a3,$t2
.L0f134da8:
/*  f134da8:	5420ffbe */ 	bnezl	$at,.L0f134ca4
/*  f134dac:	84e50000 */ 	lh	$a1,0x0($a3)
.L0f134db0:
/*  f134db0:	25ad0001 */ 	addiu	$t5,$t5,0x1
/*  f134db4:	01a8082a */ 	slt	$at,$t5,$t0
/*  f134db8:	1420ffa8 */ 	bnez	$at,.L0f134c5c
/*  f134dbc:	258c0004 */ 	addiu	$t4,$t4,0x4
/*  f134dc0:	00006825 */ 	or	$t5,$zero,$zero
.L0f134dc4:
/*  f134dc4:	19000037 */ 	blez	$t0,.L0f134ea4
/*  f134dc8:	2411008c */ 	addiu	$s1,$zero,0x8c
/*  f134dcc:	3c013f80 */ 	lui	$at,0x3f80
/*  f134dd0:	3c14800a */ 	lui	$s4,%hi(g_Rooms)
/*  f134dd4:	4481a000 */ 	mtc1	$at,$f20
/*  f134dd8:	26944928 */ 	addiu	$s4,$s4,%lo(g_Rooms)
/*  f134ddc:	27ac126c */ 	addiu	$t4,$sp,0x126c
/*  f134de0:	27b61078 */ 	addiu	$s6,$sp,0x1078
/*  f134de4:	27b50bc8 */ 	addiu	$s5,$sp,0xbc8
/*  f134de8:	2413000c */ 	addiu	$s3,$zero,0xc
/*  f134dec:	27b00e20 */ 	addiu	$s0,$sp,0xe20
.L0f134df0:
/*  f134df0:	8d840000 */ 	lw	$a0,0x0($t4)
/*  f134df4:	afad0184 */ 	sw	$t5,0x184($sp)
/*  f134df8:	afac00ac */ 	sw	$t4,0xac($sp)
/*  f134dfc:	0fc4ca87 */ 	jal	weatherIsRoomWeatherProof
/*  f134e00:	afa81268 */ 	sw	$t0,0x1268($sp)
/*  f134e04:	8fa81268 */ 	lw	$t0,0x1268($sp)
/*  f134e08:	8fac00ac */ 	lw	$t4,0xac($sp)
/*  f134e0c:	10400022 */ 	beqz	$v0,.L0f134e98
/*  f134e10:	8fad0184 */ 	lw	$t5,0x184($sp)
/*  f134e14:	02f30019 */ 	multu	$s7,$s3
/*  f134e18:	8d860000 */ 	lw	$a2,0x0($t4)
/*  f134e1c:	8e980000 */ 	lw	$t8,0x0($s4)
/*  f134e20:	2ae10032 */ 	slti	$at,$s7,0x32
/*  f134e24:	0017c880 */ 	sll	$t9,$s7,0x2
/*  f134e28:	02d97021 */ 	addu	$t6,$s6,$t9
/*  f134e2c:	00002812 */ 	mflo	$a1
/*  f134e30:	02051821 */ 	addu	$v1,$s0,$a1
/*  f134e34:	02a52021 */ 	addu	$a0,$s5,$a1
/*  f134e38:	00d10019 */ 	multu	$a2,$s1
/*  f134e3c:	00007812 */ 	mflo	$t7
/*  f134e40:	030f1021 */ 	addu	$v0,$t8,$t7
/*  f134e44:	c4440018 */ 	lwc1	$f4,0x18($v0)
/*  f134e48:	46142183 */ 	div.s	$f6,$f4,$f20
/*  f134e4c:	e4660000 */ 	swc1	$f6,0x0($v1)
/*  f134e50:	c448001c */ 	lwc1	$f8,0x1c($v0)
/*  f134e54:	46144283 */ 	div.s	$f10,$f8,$f20
/*  f134e58:	e46a0004 */ 	swc1	$f10,0x4($v1)
/*  f134e5c:	c4440020 */ 	lwc1	$f4,0x20($v0)
/*  f134e60:	46142183 */ 	div.s	$f6,$f4,$f20
/*  f134e64:	e4660008 */ 	swc1	$f6,0x8($v1)
/*  f134e68:	c4480024 */ 	lwc1	$f8,0x24($v0)
/*  f134e6c:	46144283 */ 	div.s	$f10,$f8,$f20
/*  f134e70:	e48a0000 */ 	swc1	$f10,0x0($a0)
/*  f134e74:	c4440028 */ 	lwc1	$f4,0x28($v0)
/*  f134e78:	46142183 */ 	div.s	$f6,$f4,$f20
/*  f134e7c:	e4860004 */ 	swc1	$f6,0x4($a0)
/*  f134e80:	c448002c */ 	lwc1	$f8,0x2c($v0)
/*  f134e84:	46144283 */ 	div.s	$f10,$f8,$f20
/*  f134e88:	10200003 */ 	beqz	$at,.L0f134e98
/*  f134e8c:	e48a0008 */ 	swc1	$f10,0x8($a0)
/*  f134e90:	adc60000 */ 	sw	$a2,0x0($t6)
/*  f134e94:	26f70001 */ 	addiu	$s7,$s7,0x1
.L0f134e98:
/*  f134e98:	25ad0001 */ 	addiu	$t5,$t5,0x1
/*  f134e9c:	15a8ffd4 */ 	bne	$t5,$t0,.L0f134df0
/*  f134ea0:	258c0004 */ 	addiu	$t4,$t4,0x4
.L0f134ea4:
/*  f134ea4:	3c013f80 */ 	lui	$at,0x3f80
/*  f134ea8:	4481a000 */ 	mtc1	$at,$f20
/*  f134eac:	1ae0007c */ 	blez	$s7,.L0f1350a0
/*  f134eb0:	00001825 */ 	or	$v1,$zero,$zero
/*  f134eb4:	3c068008 */ 	lui	$a2,%hi(var8007f104)
/*  f134eb8:	8cc6f104 */ 	lw	$a2,%lo(var8007f104)($a2)
/*  f134ebc:	3c048008 */ 	lui	$a0,%hi(var8007f100)
/*  f134ec0:	3c0b800a */ 	lui	$t3,%hi(g_Rooms)
/*  f134ec4:	44862000 */ 	mtc1	$a2,$f4
/*  f134ec8:	8d6b4928 */ 	lw	$t3,%lo(g_Rooms)($t3)
/*  f134ecc:	8c84f100 */ 	lw	$a0,%lo(var8007f100)($a0)
/*  f134ed0:	27a51078 */ 	addiu	$a1,$sp,0x1078
/*  f134ed4:	04c10005 */ 	bgez	$a2,.L0f134eec
/*  f134ed8:	46802020 */ 	cvt.s.w	$f0,$f4
/*  f134edc:	3c014f80 */ 	lui	$at,0x4f80
/*  f134ee0:	44813000 */ 	mtc1	$at,$f6
/*  f134ee4:	00000000 */ 	nop
/*  f134ee8:	46060000 */ 	add.s	$f0,$f0,$f6
.L0f134eec:
/*  f134eec:	44844000 */ 	mtc1	$a0,$f8
/*  f134ef0:	27a20264 */ 	addiu	$v0,$sp,0x264
/*  f134ef4:	04810005 */ 	bgez	$a0,.L0f134f0c
/*  f134ef8:	468045a0 */ 	cvt.s.w	$f22,$f8
/*  f134efc:	3c014f80 */ 	lui	$at,0x4f80
/*  f134f00:	44815000 */ 	mtc1	$at,$f10
/*  f134f04:	00000000 */ 	nop
/*  f134f08:	460ab580 */ 	add.s	$f22,$f22,$f10
.L0f134f0c:
/*  f134f0c:	8cb80000 */ 	lw	$t8,0x0($a1)
/*  f134f10:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f134f14:	03110019 */ 	multu	$t8,$s1
/*  f134f18:	00007812 */ 	mflo	$t7
/*  f134f1c:	016f2021 */ 	addu	$a0,$t3,$t7
/*  f134f20:	50770034 */ 	beql	$v1,$s7,.L0f134ff4
/*  f134f24:	c4920018 */ 	lwc1	$f18,0x18($a0)
.L0f134f28:
/*  f134f28:	c4920018 */ 	lwc1	$f18,0x18($a0)
/*  f134f2c:	8cb80004 */ 	lw	$t8,0x4($a1)
/*  f134f30:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f134f34:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f134f38:	03110019 */ 	multu	$t8,$s1
/*  f134f3c:	24a50004 */ 	addiu	$a1,$a1,0x0004
/*  f134f40:	24420030 */ 	addiu	$v0,$v0,0x30
/*  f134f44:	00007812 */ 	mflo	$t7
/*  f134f48:	46009481 */ 	sub.s	$f18,$f18,$f0
/*  f134f4c:	e452ffd0 */ 	swc1	$f18,-0x30($v0)
/*  f134f50:	e452ffe8 */ 	swc1	$f18,-0x18($v0)
/*  f134f54:	c4920024 */ 	lwc1	$f18,0x24($a0)
/*  f134f58:	c450ffe8 */ 	lwc1	$f16,-0x18($v0)
/*  f134f5c:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f134f60:	46168401 */ 	sub.s	$f16,$f16,$f22
/*  f134f64:	e450ffe8 */ 	swc1	$f16,-0x18($v0)
/*  f134f68:	46009480 */ 	add.s	$f18,$f18,$f0
/*  f134f6c:	46169400 */ 	add.s	$f16,$f18,$f22
/*  f134f70:	e452ffdc */ 	swc1	$f18,-0x24($v0)
/*  f134f74:	e450fff4 */ 	swc1	$f16,-0xc($v0)
/*  f134f78:	c490001c */ 	lwc1	$f16,0x1c($a0)
/*  f134f7c:	46148403 */ 	div.s	$f16,$f16,$f20
/*  f134f80:	46008401 */ 	sub.s	$f16,$f16,$f0
/*  f134f84:	e450ffd4 */ 	swc1	$f16,-0x2c($v0)
/*  f134f88:	e450ffec */ 	swc1	$f16,-0x14($v0)
/*  f134f8c:	c4900028 */ 	lwc1	$f16,0x28($a0)
/*  f134f90:	c452ffec */ 	lwc1	$f18,-0x14($v0)
/*  f134f94:	46148403 */ 	div.s	$f16,$f16,$f20
/*  f134f98:	46169481 */ 	sub.s	$f18,$f18,$f22
/*  f134f9c:	e452ffec */ 	swc1	$f18,-0x14($v0)
/*  f134fa0:	46008400 */ 	add.s	$f16,$f16,$f0
/*  f134fa4:	46168480 */ 	add.s	$f18,$f16,$f22
/*  f134fa8:	e450ffe0 */ 	swc1	$f16,-0x20($v0)
/*  f134fac:	e452fff8 */ 	swc1	$f18,-0x8($v0)
/*  f134fb0:	c4920020 */ 	lwc1	$f18,0x20($a0)
/*  f134fb4:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f134fb8:	46009481 */ 	sub.s	$f18,$f18,$f0
/*  f134fbc:	e452ffd8 */ 	swc1	$f18,-0x28($v0)
/*  f134fc0:	e452fff0 */ 	swc1	$f18,-0x10($v0)
/*  f134fc4:	c492002c */ 	lwc1	$f18,0x2c($a0)
/*  f134fc8:	c450fff0 */ 	lwc1	$f16,-0x10($v0)
/*  f134fcc:	016f2021 */ 	addu	$a0,$t3,$t7
/*  f134fd0:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f134fd4:	46168401 */ 	sub.s	$f16,$f16,$f22
/*  f134fd8:	e450fff0 */ 	swc1	$f16,-0x10($v0)
/*  f134fdc:	46009480 */ 	add.s	$f18,$f18,$f0
/*  f134fe0:	46169400 */ 	add.s	$f16,$f18,$f22
/*  f134fe4:	e452ffe4 */ 	swc1	$f18,-0x1c($v0)
/*  f134fe8:	1477ffcf */ 	bne	$v1,$s7,.L0f134f28
/*  f134fec:	e450fffc */ 	swc1	$f16,-0x4($v0)
/*  f134ff0:	c4920018 */ 	lwc1	$f18,0x18($a0)
.L0f134ff4:
/*  f134ff4:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f134ff8:	24420030 */ 	addiu	$v0,$v0,0x30
/*  f134ffc:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f135000:	46009481 */ 	sub.s	$f18,$f18,$f0
/*  f135004:	e452ffd0 */ 	swc1	$f18,-0x30($v0)
/*  f135008:	e452ffe8 */ 	swc1	$f18,-0x18($v0)
/*  f13500c:	c4920024 */ 	lwc1	$f18,0x24($a0)
/*  f135010:	c450ffe8 */ 	lwc1	$f16,-0x18($v0)
/*  f135014:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f135018:	46168401 */ 	sub.s	$f16,$f16,$f22
/*  f13501c:	e450ffe8 */ 	swc1	$f16,-0x18($v0)
/*  f135020:	46009480 */ 	add.s	$f18,$f18,$f0
/*  f135024:	46169400 */ 	add.s	$f16,$f18,$f22
/*  f135028:	e452ffdc */ 	swc1	$f18,-0x24($v0)
/*  f13502c:	e450fff4 */ 	swc1	$f16,-0xc($v0)
/*  f135030:	c490001c */ 	lwc1	$f16,0x1c($a0)
/*  f135034:	46148403 */ 	div.s	$f16,$f16,$f20
/*  f135038:	46008401 */ 	sub.s	$f16,$f16,$f0
/*  f13503c:	e450ffd4 */ 	swc1	$f16,-0x2c($v0)
/*  f135040:	e450ffec */ 	swc1	$f16,-0x14($v0)
/*  f135044:	c4900028 */ 	lwc1	$f16,0x28($a0)
/*  f135048:	c452ffec */ 	lwc1	$f18,-0x14($v0)
/*  f13504c:	46148403 */ 	div.s	$f16,$f16,$f20
/*  f135050:	46169481 */ 	sub.s	$f18,$f18,$f22
/*  f135054:	e452ffec */ 	swc1	$f18,-0x14($v0)
/*  f135058:	46008400 */ 	add.s	$f16,$f16,$f0
/*  f13505c:	46168480 */ 	add.s	$f18,$f16,$f22
/*  f135060:	e450ffe0 */ 	swc1	$f16,-0x20($v0)
/*  f135064:	e452fff8 */ 	swc1	$f18,-0x8($v0)
/*  f135068:	c4920020 */ 	lwc1	$f18,0x20($a0)
/*  f13506c:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f135070:	46009481 */ 	sub.s	$f18,$f18,$f0
/*  f135074:	e452ffd8 */ 	swc1	$f18,-0x28($v0)
/*  f135078:	e452fff0 */ 	swc1	$f18,-0x10($v0)
/*  f13507c:	c492002c */ 	lwc1	$f18,0x2c($a0)
/*  f135080:	c450fff0 */ 	lwc1	$f16,-0x10($v0)
/*  f135084:	46149483 */ 	div.s	$f18,$f18,$f20
/*  f135088:	46168401 */ 	sub.s	$f16,$f16,$f22
/*  f13508c:	e450fff0 */ 	swc1	$f16,-0x10($v0)
/*  f135090:	46009480 */ 	add.s	$f18,$f18,$f0
/*  f135094:	46169400 */ 	add.s	$f16,$f18,$f22
/*  f135098:	e452ffe4 */ 	swc1	$f18,-0x1c($v0)
/*  f13509c:	e450fffc */ 	swc1	$f16,-0x4($v0)
.L0f1350a0:
/*  f1350a0:	3c017f1b */ 	lui	$at,%hi(var7f1b5784)
/*  f1350a4:	c4385784 */ 	lwc1	$f24,%lo(var7f1b5784)($at)
/*  f1350a8:	3c017f1b */ 	lui	$at,%hi(var7f1b5788)
/*  f1350ac:	c4365788 */ 	lwc1	$f22,%lo(var7f1b5788)($at)
/*  f1350b0:	3c017f1b */ 	lui	$at,%hi(var7f1b578c)
/*  f1350b4:	c434578c */ 	lwc1	$f20,%lo(var7f1b578c)($at)
/*  f1350b8:	02408825 */ 	or	$s1,$s2,$zero
/*  f1350bc:	27b01168 */ 	addiu	$s0,$sp,0x1168
/*  f1350c0:	27b31268 */ 	addiu	$s3,$sp,0x1268
.L0f1350c4:
/*  f1350c4:	0c0068f7 */ 	jal	sinf
/*  f1350c8:	c62c3ec8 */ 	lwc1	$f12,0x3ec8($s1)
/*  f1350cc:	e6000000 */ 	swc1	$f0,0x0($s0)
/*  f1350d0:	0c0068f4 */ 	jal	cosf
/*  f1350d4:	c62c3ec8 */ 	lwc1	$f12,0x3ec8($s1)
/*  f1350d8:	e6000004 */ 	swc1	$f0,0x4($s0)
/*  f1350dc:	c6263ec8 */ 	lwc1	$f6,0x3ec8($s1)
/*  f1350e0:	0c0068f7 */ 	jal	sinf
/*  f1350e4:	46143300 */ 	add.s	$f12,$f6,$f20
/*  f1350e8:	e6000008 */ 	swc1	$f0,0x8($s0)
/*  f1350ec:	c6283ec8 */ 	lwc1	$f8,0x3ec8($s1)
/*  f1350f0:	0c0068f4 */ 	jal	cosf
/*  f1350f4:	46144300 */ 	add.s	$f12,$f8,$f20
/*  f1350f8:	e600000c */ 	swc1	$f0,0xc($s0)
/*  f1350fc:	c62a3ec8 */ 	lwc1	$f10,0x3ec8($s1)
/*  f135100:	0c0068f7 */ 	jal	sinf
/*  f135104:	46165300 */ 	add.s	$f12,$f10,$f22
/*  f135108:	e6000010 */ 	swc1	$f0,0x10($s0)
/*  f13510c:	c6243ec8 */ 	lwc1	$f4,0x3ec8($s1)
/*  f135110:	0c0068f4 */ 	jal	cosf
/*  f135114:	46162300 */ 	add.s	$f12,$f4,$f22
/*  f135118:	e6000014 */ 	swc1	$f0,0x14($s0)
/*  f13511c:	c6263ec8 */ 	lwc1	$f6,0x3ec8($s1)
/*  f135120:	0c0068f7 */ 	jal	sinf
/*  f135124:	46183300 */ 	add.s	$f12,$f6,$f24
/*  f135128:	e6000018 */ 	swc1	$f0,0x18($s0)
/*  f13512c:	c6283ec8 */ 	lwc1	$f8,0x3ec8($s1)
/*  f135130:	0c0068f4 */ 	jal	cosf
/*  f135134:	46184300 */ 	add.s	$f12,$f8,$f24
/*  f135138:	26100020 */ 	addiu	$s0,$s0,0x20
/*  f13513c:	0213082b */ 	sltu	$at,$s0,$s3
/*  f135140:	26310004 */ 	addiu	$s1,$s1,0x4
/*  f135144:	1420ffdf */ 	bnez	$at,.L0f1350c4
/*  f135148:	e600fffc */ 	swc1	$f0,-0x4($s0)
/*  f13514c:	0fc59e73 */ 	jal	gfxAllocateColours
/*  f135150:	24040010 */ 	addiu	$a0,$zero,0x10
/*  f135154:	00408025 */ 	or	$s0,$v0,$zero
/*  f135158:	240310ef */ 	addiu	$v1,$zero,0x10ef
/*  f13515c:	00402025 */ 	or	$a0,$v0,$zero
/*  f135160:	240700ff */ 	addiu	$a3,$zero,0xff
/*  f135164:	24060011 */ 	addiu	$a2,$zero,0x11
/*  f135168:	2405ff00 */ 	addiu	$a1,$zero,-256
.L0f13516c:
/*  f13516c:	0066001a */ 	div	$zero,$v1,$a2
/*  f135170:	3c198008 */ 	lui	$t9,%hi(var8007f10c)
/*  f135174:	8f39f10c */ 	lw	$t9,%lo(var8007f10c)($t9)
/*  f135178:	0000c012 */ 	mflo	$t8
/*  f13517c:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f135180:	03257024 */ 	and	$t6,$t9,$a1
/*  f135184:	14c00002 */ 	bnez	$a2,.L0f135190
/*  f135188:	00000000 */ 	nop
/*  f13518c:	0007000d */ 	break	0x7
.L0f135190:
/*  f135190:	2401ffff */ 	addiu	$at,$zero,-1
/*  f135194:	14c10004 */ 	bne	$a2,$at,.L0f1351a8
/*  f135198:	3c018000 */ 	lui	$at,0x8000
/*  f13519c:	14610002 */ 	bne	$v1,$at,.L0f1351a8
/*  f1351a0:	00000000 */ 	nop
/*  f1351a4:	0006000d */ 	break	0x6
.L0f1351a8:
/*  f1351a8:	2463ff01 */ 	addiu	$v1,$v1,-255
/*  f1351ac:	01d87825 */ 	or	$t7,$t6,$t8
/*  f1351b0:	14e3ffee */ 	bne	$a3,$v1,.L0f13516c
/*  f1351b4:	ac8ffffc */ 	sw	$t7,-0x4($a0)
/*  f1351b8:	8fb11398 */ 	lw	$s1,0x1398($sp)
/*  f1351bc:	3c18073c */ 	lui	$t8,0x73c
/*  f1351c0:	37180040 */ 	ori	$t8,$t8,0x40
/*  f1351c4:	262e0008 */ 	addiu	$t6,$s1,0x8
/*  f1351c8:	afae1398 */ 	sw	$t6,0x1398($sp)
/*  f1351cc:	02002025 */ 	or	$a0,$s0,$zero
/*  f1351d0:	0c012d20 */ 	jal	osVirtualToPhysical
/*  f1351d4:	ae380000 */ 	sw	$t8,0x0($s1)
/*  f1351d8:	27b301cc */ 	addiu	$s3,$sp,0x1cc
/*  f1351dc:	4480c000 */ 	mtc1	$zero,$f24
/*  f1351e0:	ae220004 */ 	sw	$v0,0x4($s1)
/*  f1351e4:	0260b025 */ 	or	$s6,$s3,$zero
/*  f1351e8:	0000f025 */ 	or	$s8,$zero,$zero
/*  f1351ec:	02408025 */ 	or	$s0,$s2,$zero
/*  f1351f0:	24150030 */ 	addiu	$s5,$zero,0x30
/*  f1351f4:	27b40264 */ 	addiu	$s4,$sp,0x264
.L0f1351f8:
/*  f1351f8:	8e19000c */ 	lw	$t9,0xc($s0)
/*  f1351fc:	24110001 */ 	addiu	$s1,$zero,0x1
/*  f135200:	332e0003 */ 	andi	$t6,$t9,0x3
/*  f135204:	51c0024b */ 	beqzl	$t6,.L0f135b34
/*  f135208:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f13520c:	0c012144 */ 	jal	osGetCount
/*  f135210:	00000000 */ 	nop
/*  f135214:	afa21354 */ 	sw	$v0,0x1354($sp)
/*  f135218:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f13521c:	c64a3e80 */ 	lwc1	$f10,0x3e80($s2)
/*  f135220:	27a40124 */ 	addiu	$a0,$sp,0x124
/*  f135224:	3c0540a0 */ 	lui	$a1,0x40a0
/*  f135228:	46045180 */ 	add.s	$f6,$f10,$f4
/*  f13522c:	e7a60124 */ 	swc1	$f6,0x124($sp)
/*  f135230:	c60a0004 */ 	lwc1	$f10,0x4($s0)
/*  f135234:	c6483e84 */ 	lwc1	$f8,0x3e84($s2)
/*  f135238:	460a4100 */ 	add.s	$f4,$f8,$f10
/*  f13523c:	e7a40128 */ 	swc1	$f4,0x128($sp)
/*  f135240:	c6080008 */ 	lwc1	$f8,0x8($s0)
/*  f135244:	c6463e88 */ 	lwc1	$f6,0x3e88($s2)
/*  f135248:	46083280 */ 	add.s	$f10,$f6,$f8
/*  f13524c:	0fc2d6e7 */ 	jal	cam0f0b5b9c
/*  f135250:	e7aa012c */ 	swc1	$f10,0x12c($sp)
/*  f135254:	50400237 */ 	beqzl	$v0,.L0f135b34
/*  f135258:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f13525c:	0c012144 */ 	jal	osGetCount
/*  f135260:	00000000 */ 	nop
/*  f135264:	8fb8137c */ 	lw	$t8,0x137c($sp)
/*  f135268:	8fb91354 */ 	lw	$t9,0x1354($sp)
/*  f13526c:	03027821 */ 	addu	$t7,$t8,$v0
/*  f135270:	01f97023 */ 	subu	$t6,$t7,$t9
/*  f135274:	afae137c */ 	sw	$t6,0x137c($sp)
/*  f135278:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f13527c:	e7a4021c */ 	swc1	$f4,0x21c($sp)
/*  f135280:	c6060008 */ 	lwc1	$f6,0x8($s0)
/*  f135284:	c7a4021c */ 	lwc1	$f4,0x21c($sp)
/*  f135288:	e7a60220 */ 	swc1	$f6,0x220($sp)
/*  f13528c:	c7a80220 */ 	lwc1	$f8,0x220($sp)
/*  f135290:	46084282 */ 	mul.s	$f10,$f8,$f8
/*  f135294:	00000000 */ 	nop
/*  f135298:	46042182 */ 	mul.s	$f6,$f4,$f4
/*  f13529c:	0c012974 */ 	jal	sqrtf
/*  f1352a0:	46065300 */ 	add.s	$f12,$f10,$f6
/*  f1352a4:	3c017f1b */ 	lui	$at,%hi(var7f1b5790)
/*  f1352a8:	c4285790 */ 	lwc1	$f8,%lo(var7f1b5790)($at)
/*  f1352ac:	46000506 */ 	mov.s	$f20,$f0
/*  f1352b0:	4608003c */ 	c.lt.s	$f0,$f8
/*  f1352b4:	00000000 */ 	nop
/*  f1352b8:	4503021e */ 	bc1tl	.L0f135b34
/*  f1352bc:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f1352c0:	8fb80198 */ 	lw	$t8,0x198($sp)
/*  f1352c4:	e7b80260 */ 	swc1	$f24,0x260($sp)
/*  f1352c8:	00187880 */ 	sll	$t7,$t8,0x2
/*  f1352cc:	000fc880 */ 	sll	$t9,$t7,0x2
/*  f1352d0:	032fc823 */ 	subu	$t9,$t9,$t7
/*  f1352d4:	0019c880 */ 	sll	$t9,$t9,0x2
/*  f1352d8:	17000004 */ 	bnez	$t8,.L0f1352ec
/*  f1352dc:	afb90080 */ 	sw	$t9,0x80($sp)
/*  f1352e0:	0fc59e59 */ 	jal	gfxAllocateVertices
/*  f1352e4:	24040008 */ 	addiu	$a0,$zero,0x8
/*  f1352e8:	afa20214 */ 	sw	$v0,0x214($sp)
.L0f1352ec:
/*  f1352ec:	c7a4021c */ 	lwc1	$f4,0x21c($sp)
/*  f1352f0:	c7a60220 */ 	lwc1	$f6,0x220($sp)
/*  f1352f4:	8fae0214 */ 	lw	$t6,0x214($sp)
/*  f1352f8:	46142283 */ 	div.s	$f10,$f4,$f20
/*  f1352fc:	8faf0080 */ 	lw	$t7,0x80($sp)
/*  f135300:	27a2019c */ 	addiu	$v0,$sp,0x19c
/*  f135304:	01cf1821 */ 	addu	$v1,$t6,$t7
/*  f135308:	46143203 */ 	div.s	$f8,$f6,$f20
/*  f13530c:	e7aa021c */ 	swc1	$f10,0x21c($sp)
/*  f135310:	e7a80220 */ 	swc1	$f8,0x220($sp)
.L0f135314:
/*  f135314:	a4600008 */ 	sh	$zero,0x8($v1)
/*  f135318:	a460000a */ 	sh	$zero,0xa($v1)
/*  f13531c:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f135320:	2442000c */ 	addiu	$v0,$v0,0xc
/*  f135324:	0053082b */ 	sltu	$at,$v0,$s3
/*  f135328:	e444fff4 */ 	swc1	$f4,-0xc($v0)
/*  f13532c:	c60a0004 */ 	lwc1	$f10,0x4($s0)
/*  f135330:	2463000c */ 	addiu	$v1,$v1,0xc
/*  f135334:	e44afff8 */ 	swc1	$f10,-0x8($v0)
/*  f135338:	c6060008 */ 	lwc1	$f6,0x8($s0)
/*  f13533c:	1420fff5 */ 	bnez	$at,.L0f135314
/*  f135340:	e446fffc */ 	swc1	$f6,-0x4($v0)
/*  f135344:	1ae00097 */ 	blez	$s7,.L0f1355a4
/*  f135348:	00000000 */ 	nop
/*  f13534c:	c6483e80 */ 	lwc1	$f8,0x3e80($s2)
/*  f135350:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f135354:	00001825 */ 	or	$v1,$zero,$zero
/*  f135358:	3c048008 */ 	lui	$a0,%hi(var8007f100)
/*  f13535c:	46044280 */ 	add.s	$f10,$f8,$f4
/*  f135360:	e7aa0118 */ 	swc1	$f10,0x118($sp)
/*  f135364:	c6080004 */ 	lwc1	$f8,0x4($s0)
/*  f135368:	c6463e84 */ 	lwc1	$f6,0x3e84($s2)
/*  f13536c:	c7ae0118 */ 	lwc1	$f14,0x118($sp)
/*  f135370:	46083100 */ 	add.s	$f4,$f6,$f8
/*  f135374:	e7a4011c */ 	swc1	$f4,0x11c($sp)
/*  f135378:	c6060008 */ 	lwc1	$f6,0x8($s0)
/*  f13537c:	c64a3e88 */ 	lwc1	$f10,0x3e88($s2)
/*  f135380:	c7a0011c */ 	lwc1	$f0,0x11c($sp)
/*  f135384:	46065200 */ 	add.s	$f8,$f10,$f6
/*  f135388:	1ae00086 */ 	blez	$s7,.L0f1355a4
/*  f13538c:	e7a80120 */ 	swc1	$f8,0x120($sp)
/*  f135390:	c7b00120 */ 	lwc1	$f16,0x120($sp)
/*  f135394:	8c84f100 */ 	lw	$a0,%lo(var8007f100)($a0)
.L0f135398:
/*  f135398:	5220007e */ 	beqzl	$s1,.L0f135594
/*  f13539c:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f1353a0:	00750019 */ 	multu	$v1,$s5
/*  f1353a4:	0000c812 */ 	mflo	$t9
/*  f1353a8:	02991021 */ 	addu	$v0,$s4,$t9
/*  f1353ac:	c4520018 */ 	lwc1	$f18,0x18($v0)
/*  f1353b0:	460e903c */ 	c.lt.s	$f18,$f14
/*  f1353b4:	00000000 */ 	nop
/*  f1353b8:	45020076 */ 	bc1fl	.L0f135594
/*  f1353bc:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f1353c0:	c4540024 */ 	lwc1	$f20,0x24($v0)
/*  f1353c4:	4614703c */ 	c.lt.s	$f14,$f20
/*  f1353c8:	00000000 */ 	nop
/*  f1353cc:	45020071 */ 	bc1fl	.L0f135594
/*  f1353d0:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f1353d4:	c444001c */ 	lwc1	$f4,0x1c($v0)
/*  f1353d8:	4600203c */ 	c.lt.s	$f4,$f0
/*  f1353dc:	00000000 */ 	nop
/*  f1353e0:	4502006c */ 	bc1fl	.L0f135594
/*  f1353e4:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f1353e8:	c44a0028 */ 	lwc1	$f10,0x28($v0)
/*  f1353ec:	460a003c */ 	c.lt.s	$f0,$f10
/*  f1353f0:	00000000 */ 	nop
/*  f1353f4:	45020067 */ 	bc1fl	.L0f135594
/*  f1353f8:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f1353fc:	c4580020 */ 	lwc1	$f24,0x20($v0)
/*  f135400:	4610c03c */ 	c.lt.s	$f24,$f16
/*  f135404:	00000000 */ 	nop
/*  f135408:	45020062 */ 	bc1fl	.L0f135594
/*  f13540c:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f135410:	c45a002c */ 	lwc1	$f26,0x2c($v0)
/*  f135414:	461a803c */ 	c.lt.s	$f16,$f26
/*  f135418:	00000000 */ 	nop
/*  f13541c:	4502005d */ 	bc1fl	.L0f135594
/*  f135420:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f135424:	c4400000 */ 	lwc1	$f0,0x0($v0)
/*  f135428:	460e003c */ 	c.lt.s	$f0,$f14
/*  f13542c:	00000000 */ 	nop
/*  f135430:	4502001e */ 	bc1fl	.L0f1354ac
/*  f135434:	44844000 */ 	mtc1	$a0,$f8
/*  f135438:	c446000c */ 	lwc1	$f6,0xc($v0)
/*  f13543c:	c7a2011c */ 	lwc1	$f2,0x11c($sp)
/*  f135440:	4606703c */ 	c.lt.s	$f14,$f6
/*  f135444:	00000000 */ 	nop
/*  f135448:	45020018 */ 	bc1fl	.L0f1354ac
/*  f13544c:	44844000 */ 	mtc1	$a0,$f8
/*  f135450:	c4480004 */ 	lwc1	$f8,0x4($v0)
/*  f135454:	4602403c */ 	c.lt.s	$f8,$f2
/*  f135458:	00000000 */ 	nop
/*  f13545c:	45020013 */ 	bc1fl	.L0f1354ac
/*  f135460:	44844000 */ 	mtc1	$a0,$f8
/*  f135464:	c4440010 */ 	lwc1	$f4,0x10($v0)
/*  f135468:	4604103c */ 	c.lt.s	$f2,$f4
/*  f13546c:	00000000 */ 	nop
/*  f135470:	4502000e */ 	bc1fl	.L0f1354ac
/*  f135474:	44844000 */ 	mtc1	$a0,$f8
/*  f135478:	c44a0008 */ 	lwc1	$f10,0x8($v0)
/*  f13547c:	4610503c */ 	c.lt.s	$f10,$f16
/*  f135480:	00000000 */ 	nop
/*  f135484:	45020009 */ 	bc1fl	.L0f1354ac
/*  f135488:	44844000 */ 	mtc1	$a0,$f8
/*  f13548c:	c4460014 */ 	lwc1	$f6,0x14($v0)
/*  f135490:	4606803c */ 	c.lt.s	$f16,$f6
/*  f135494:	00000000 */ 	nop
/*  f135498:	45020004 */ 	bc1fl	.L0f1354ac
/*  f13549c:	44844000 */ 	mtc1	$a0,$f8
/*  f1354a0:	1000003b */ 	b	.L0f135590
/*  f1354a4:	00008825 */ 	or	$s1,$zero,$zero
/*  f1354a8:	44844000 */ 	mtc1	$a0,$f8
.L0f1354ac:
/*  f1354ac:	44801000 */ 	mtc1	$zero,$f2
/*  f1354b0:	04810005 */ 	bgez	$a0,.L0f1354c8
/*  f1354b4:	468045a0 */ 	cvt.s.w	$f22,$f8
/*  f1354b8:	3c014f80 */ 	lui	$at,0x4f80
/*  f1354bc:	44812000 */ 	mtc1	$at,$f4
/*  f1354c0:	00000000 */ 	nop
/*  f1354c4:	4604b580 */ 	add.s	$f22,$f22,$f4
.L0f1354c8:
/*  f1354c8:	4600703c */ 	c.lt.s	$f14,$f0
/*  f1354cc:	44805000 */ 	mtc1	$zero,$f10
/*  f1354d0:	c44c000c */ 	lwc1	$f12,0xc($v0)
/*  f1354d4:	c45c0008 */ 	lwc1	$f28,0x8($v0)
/*  f1354d8:	45000002 */ 	bc1f	.L0f1354e4
/*  f1354dc:	c45e0014 */ 	lwc1	$f30,0x14($v0)
/*  f1354e0:	46127081 */ 	sub.s	$f2,$f14,$f18
.L0f1354e4:
/*  f1354e4:	460e603c */ 	c.lt.s	$f12,$f14
/*  f1354e8:	44804000 */ 	mtc1	$zero,$f8
/*  f1354ec:	45020003 */ 	bc1fl	.L0f1354fc
/*  f1354f0:	4602503c */ 	c.lt.s	$f10,$f2
/*  f1354f4:	46147081 */ 	sub.s	$f2,$f14,$f20
/*  f1354f8:	4602503c */ 	c.lt.s	$f10,$f2
.L0f1354fc:
/*  f1354fc:	00000000 */ 	nop
/*  f135500:	45020004 */ 	bc1fl	.L0f135514
/*  f135504:	46001307 */ 	neg.s	$f12,$f2
/*  f135508:	10000002 */ 	b	.L0f135514
/*  f13550c:	46001306 */ 	mov.s	$f12,$f2
/*  f135510:	46001307 */ 	neg.s	$f12,$f2
.L0f135514:
/*  f135514:	46166003 */ 	div.s	$f0,$f12,$f22
/*  f135518:	c7a60260 */ 	lwc1	$f6,0x260($sp)
/*  f13551c:	4600303c */ 	c.lt.s	$f6,$f0
/*  f135520:	46000086 */ 	mov.s	$f2,$f0
/*  f135524:	45020003 */ 	bc1fl	.L0f135534
/*  f135528:	461c803c */ 	c.lt.s	$f16,$f28
/*  f13552c:	e7a00260 */ 	swc1	$f0,0x260($sp)
/*  f135530:	461c803c */ 	c.lt.s	$f16,$f28
.L0f135534:
/*  f135534:	00000000 */ 	nop
/*  f135538:	45020003 */ 	bc1fl	.L0f135548
/*  f13553c:	4610f03c */ 	c.lt.s	$f30,$f16
/*  f135540:	46188081 */ 	sub.s	$f2,$f16,$f24
/*  f135544:	4610f03c */ 	c.lt.s	$f30,$f16
.L0f135548:
/*  f135548:	00000000 */ 	nop
/*  f13554c:	45020003 */ 	bc1fl	.L0f13555c
/*  f135550:	4602403c */ 	c.lt.s	$f8,$f2
/*  f135554:	461a8081 */ 	sub.s	$f2,$f16,$f26
/*  f135558:	4602403c */ 	c.lt.s	$f8,$f2
.L0f13555c:
/*  f13555c:	00000000 */ 	nop
/*  f135560:	45020004 */ 	bc1fl	.L0f135574
/*  f135564:	46001307 */ 	neg.s	$f12,$f2
/*  f135568:	10000002 */ 	b	.L0f135574
/*  f13556c:	46001306 */ 	mov.s	$f12,$f2
/*  f135570:	46001307 */ 	neg.s	$f12,$f2
.L0f135574:
/*  f135574:	46166003 */ 	div.s	$f0,$f12,$f22
/*  f135578:	c7a40260 */ 	lwc1	$f4,0x260($sp)
/*  f13557c:	4600203c */ 	c.lt.s	$f4,$f0
/*  f135580:	00000000 */ 	nop
/*  f135584:	45020003 */ 	bc1fl	.L0f135594
/*  f135588:	24630001 */ 	addiu	$v1,$v1,0x1
/*  f13558c:	e7a00260 */ 	swc1	$f0,0x260($sp)
.L0f135590:
/*  f135590:	24630001 */ 	addiu	$v1,$v1,0x1
.L0f135594:
/*  f135594:	1477ff80 */ 	bne	$v1,$s7,.L0f135398
/*  f135598:	c7a0011c */ 	lwc1	$f0,0x11c($sp)
/*  f13559c:	4480c000 */ 	mtc1	$zero,$f24
/*  f1355a0:	00000000 */ 	nop
.L0f1355a4:
/*  f1355a4:	52200163 */ 	beqzl	$s1,.L0f135b34
/*  f1355a8:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f1355ac:	c6000000 */ 	lwc1	$f0,0x0($s0)
/*  f1355b0:	c60e0004 */ 	lwc1	$f14,0x4($s0)
/*  f1355b4:	c6020008 */ 	lwc1	$f2,0x8($s0)
/*  f1355b8:	46000282 */ 	mul.s	$f10,$f0,$f0
/*  f1355bc:	00000000 */ 	nop
/*  f1355c0:	460e7182 */ 	mul.s	$f6,$f14,$f14
/*  f1355c4:	46065200 */ 	add.s	$f8,$f10,$f6
/*  f1355c8:	46021102 */ 	mul.s	$f4,$f2,$f2
/*  f1355cc:	0c012974 */ 	jal	sqrtf
/*  f1355d0:	46082300 */ 	add.s	$f12,$f4,$f8
/*  f1355d4:	c6060004 */ 	lwc1	$f6,0x4($s0)
/*  f1355d8:	c60a0000 */ 	lwc1	$f10,0x0($s0)
/*  f1355dc:	46003103 */ 	div.s	$f4,$f6,$f0
/*  f1355e0:	46005603 */ 	div.s	$f24,$f10,$f0
/*  f1355e4:	e7a40108 */ 	swc1	$f4,0x108($sp)
/*  f1355e8:	c6080008 */ 	lwc1	$f8,0x8($s0)
/*  f1355ec:	46004683 */ 	div.s	$f26,$f8,$f0
/*  f1355f0:	4618c282 */ 	mul.s	$f10,$f24,$f24
/*  f1355f4:	00000000 */ 	nop
/*  f1355f8:	461ad182 */ 	mul.s	$f6,$f26,$f26
/*  f1355fc:	0c012974 */ 	jal	sqrtf
/*  f135600:	46065300 */ 	add.s	$f12,$f10,$f6
/*  f135604:	4600c083 */ 	div.s	$f2,$f24,$f0
/*  f135608:	c7a40108 */ 	lwc1	$f4,0x108($sp)
/*  f13560c:	3c013f80 */ 	lui	$at,0x3f80
/*  f135610:	3c068008 */ 	lui	$a2,%hi(var8007f104)
/*  f135614:	46000287 */ 	neg.s	$f10,$f0
/*  f135618:	8cc6f104 */ 	lw	$a2,%lo(var8007f104)($a2)
/*  f13561c:	e7aa1148 */ 	swc1	$f10,0x1148($sp)
/*  f135620:	c7aa0220 */ 	lwc1	$f10,0x220($sp)
/*  f135624:	27a2019c */ 	addiu	$v0,$sp,0x19c
/*  f135628:	001ec083 */ 	sra	$t8,$s8,0x2
/*  f13562c:	4600d703 */ 	div.s	$f28,$f26,$f0
/*  f135630:	46022202 */ 	mul.s	$f8,$f4,$f2
/*  f135634:	46001787 */ 	neg.s	$f30,$f2
/*  f135638:	e7a8114c */ 	swc1	$f8,0x114c($sp)
/*  f13563c:	461c2182 */ 	mul.s	$f6,$f4,$f28
/*  f135640:	46005107 */ 	neg.s	$f4,$f10
/*  f135644:	44815000 */ 	mtc1	$at,$f10
/*  f135648:	e7a4115c */ 	swc1	$f4,0x115c($sp)
/*  f13564c:	c7a4021c */ 	lwc1	$f4,0x21c($sp)
/*  f135650:	e7aa1160 */ 	swc1	$f10,0x1160($sp)
/*  f135654:	44865000 */ 	mtc1	$a2,$f10
/*  f135658:	e7a41164 */ 	swc1	$f4,0x1164($sp)
/*  f13565c:	e7a61144 */ 	swc1	$f6,0x1144($sp)
/*  f135660:	04c10005 */ 	bgez	$a2,.L0f135678
/*  f135664:	46805120 */ 	cvt.s.w	$f4,$f10
/*  f135668:	3c014f80 */ 	lui	$at,0x4f80
/*  f13566c:	44815000 */ 	mtc1	$at,$f10
/*  f135670:	00000000 */ 	nop
/*  f135674:	460a2100 */ 	add.s	$f4,$f4,$f10
.L0f135678:
/*  f135678:	461c2302 */ 	mul.s	$f12,$f4,$f28
/*  f13567c:	4486d000 */ 	mtc1	$a2,$f26
/*  f135680:	330e0007 */ 	andi	$t6,$t8,0x7
/*  f135684:	000e7940 */ 	sll	$t7,$t6,0x5
/*  f135688:	27b91168 */ 	addiu	$t9,$sp,0x1168
/*  f13568c:	01f92821 */ 	addu	$a1,$t7,$t9
/*  f135690:	4680d120 */ 	cvt.s.w	$f4,$f26
/*  f135694:	04c10005 */ 	bgez	$a2,.L0f1356ac
/*  f135698:	00002025 */ 	or	$a0,$zero,$zero
/*  f13569c:	3c014f80 */ 	lui	$at,0x4f80
/*  f1356a0:	44815000 */ 	mtc1	$at,$f10
/*  f1356a4:	00000000 */ 	nop
/*  f1356a8:	460a2100 */ 	add.s	$f4,$f4,$f10
.L0f1356ac:
/*  f1356ac:	44865000 */ 	mtc1	$a2,$f10
/*  f1356b0:	46082382 */ 	mul.s	$f14,$f4,$f8
/*  f1356b4:	4480b000 */ 	mtc1	$zero,$f22
/*  f1356b8:	3c014f80 */ 	lui	$at,0x4f80
/*  f1356bc:	04c10004 */ 	bgez	$a2,.L0f1356d0
/*  f1356c0:	46805120 */ 	cvt.s.w	$f4,$f10
/*  f1356c4:	44814000 */ 	mtc1	$at,$f8
/*  f1356c8:	00000000 */ 	nop
/*  f1356cc:	46082100 */ 	add.s	$f4,$f4,$f8
.L0f1356d0:
/*  f1356d0:	c7aa1148 */ 	lwc1	$f10,0x1148($sp)
/*  f1356d4:	44864000 */ 	mtc1	$a2,$f8
/*  f1356d8:	3c014f80 */ 	lui	$at,0x4f80
/*  f1356dc:	460a2402 */ 	mul.s	$f16,$f4,$f10
/*  f1356e0:	04c10004 */ 	bgez	$a2,.L0f1356f4
/*  f1356e4:	46804120 */ 	cvt.s.w	$f4,$f8
/*  f1356e8:	44815000 */ 	mtc1	$at,$f10
/*  f1356ec:	00000000 */ 	nop
/*  f1356f0:	460a2100 */ 	add.s	$f4,$f4,$f10
.L0f1356f4:
/*  f1356f4:	44864000 */ 	mtc1	$a2,$f8
/*  f1356f8:	461e2482 */ 	mul.s	$f18,$f4,$f30
/*  f1356fc:	3c014f80 */ 	lui	$at,0x4f80
/*  f135700:	04c10004 */ 	bgez	$a2,.L0f135714
/*  f135704:	468042a0 */ 	cvt.s.w	$f10,$f8
/*  f135708:	44812000 */ 	mtc1	$at,$f4
/*  f13570c:	00000000 */ 	nop
/*  f135710:	46045280 */ 	add.s	$f10,$f10,$f4
.L0f135714:
/*  f135714:	46065502 */ 	mul.s	$f20,$f10,$f6
/*  f135718:	00000000 */ 	nop
/*  f13571c:	2442000c */ 	addiu	$v0,$v0,0xc
/*  f135720:	10560018 */ 	beq	$v0,$s6,.L0f135784
/*  f135724:	00a41821 */ 	addu	$v1,$a1,$a0
.L0f135728:
/*  f135728:	c47e0000 */ 	lwc1	$f30,0x0($v1)
/*  f13572c:	c47c0004 */ 	lwc1	$f28,0x4($v1)
/*  f135730:	c45afff4 */ 	lwc1	$f26,-0xc($v0)
/*  f135734:	461e6602 */ 	mul.s	$f24,$f12,$f30
/*  f135738:	c44afff8 */ 	lwc1	$f10,-0x8($v0)
/*  f13573c:	c448fffc */ 	lwc1	$f8,-0x4($v0)
/*  f135740:	461c7182 */ 	mul.s	$f6,$f14,$f28
/*  f135744:	24840008 */ 	addiu	$a0,$a0,0x8
/*  f135748:	2442000c */ 	addiu	$v0,$v0,0xc
/*  f13574c:	461c8102 */ 	mul.s	$f4,$f16,$f28
/*  f135750:	00a41821 */ 	addu	$v1,$a1,$a0
/*  f135754:	461e9782 */ 	mul.s	$f30,$f18,$f30
/*  f135758:	4606c180 */ 	add.s	$f6,$f24,$f6
/*  f13575c:	461ca702 */ 	mul.s	$f28,$f20,$f28
/*  f135760:	4604b100 */ 	add.s	$f4,$f22,$f4
/*  f135764:	4606d180 */ 	add.s	$f6,$f26,$f6
/*  f135768:	461cf700 */ 	add.s	$f28,$f30,$f28
/*  f13576c:	e446ffe8 */ 	swc1	$f6,-0x18($v0)
/*  f135770:	46045100 */ 	add.s	$f4,$f10,$f4
/*  f135774:	461c4700 */ 	add.s	$f28,$f8,$f28
/*  f135778:	e444ffec */ 	swc1	$f4,-0x14($v0)
/*  f13577c:	1456ffea */ 	bne	$v0,$s6,.L0f135728
/*  f135780:	e45cfff0 */ 	swc1	$f28,-0x10($v0)
.L0f135784:
/*  f135784:	c47e0000 */ 	lwc1	$f30,0x0($v1)
/*  f135788:	c47c0004 */ 	lwc1	$f28,0x4($v1)
/*  f13578c:	c45afff4 */ 	lwc1	$f26,-0xc($v0)
/*  f135790:	461e6602 */ 	mul.s	$f24,$f12,$f30
/*  f135794:	c44afff8 */ 	lwc1	$f10,-0x8($v0)
/*  f135798:	c448fffc */ 	lwc1	$f8,-0x4($v0)
/*  f13579c:	461c7182 */ 	mul.s	$f6,$f14,$f28
/*  f1357a0:	24840008 */ 	addiu	$a0,$a0,0x8
/*  f1357a4:	461c8102 */ 	mul.s	$f4,$f16,$f28
/*  f1357a8:	00000000 */ 	nop
/*  f1357ac:	461e9782 */ 	mul.s	$f30,$f18,$f30
/*  f1357b0:	4606c180 */ 	add.s	$f6,$f24,$f6
/*  f1357b4:	461ca702 */ 	mul.s	$f28,$f20,$f28
/*  f1357b8:	4604b100 */ 	add.s	$f4,$f22,$f4
/*  f1357bc:	4606d180 */ 	add.s	$f6,$f26,$f6
/*  f1357c0:	461cf700 */ 	add.s	$f28,$f30,$f28
/*  f1357c4:	e446fff4 */ 	swc1	$f6,-0xc($v0)
/*  f1357c8:	46045100 */ 	add.s	$f4,$f10,$f4
/*  f1357cc:	461c4700 */ 	add.s	$f28,$f8,$f28
/*  f1357d0:	e444fff8 */ 	swc1	$f4,-0x8($v0)
/*  f1357d4:	e45cfffc */ 	swc1	$f28,-0x4($v0)
/*  f1357d8:	3c014316 */ 	lui	$at,0x4316
/*  f1357dc:	4481b000 */ 	mtc1	$at,$f22
/*  f1357e0:	c6423e98 */ 	lwc1	$f2,0x3e98($s2)
/*  f1357e4:	c6000000 */ 	lwc1	$f0,0x0($s0)
/*  f1357e8:	4480c000 */ 	mtc1	$zero,$f24
/*  f1357ec:	46161100 */ 	add.s	$f4,$f2,$f22
/*  f1357f0:	3c014180 */ 	lui	$at,0x4180
/*  f1357f4:	33c40001 */ 	andi	$a0,$s8,0x1
/*  f1357f8:	4600c406 */ 	mov.s	$f16,$f24
/*  f1357fc:	4604003c */ 	c.lt.s	$f0,$f4
/*  f135800:	00000000 */ 	nop
/*  f135804:	45020004 */ 	bc1fl	.L0f135818
/*  f135808:	c6423e8c */ 	lwc1	$f2,0x3e8c($s2)
/*  f13580c:	46020201 */ 	sub.s	$f8,$f0,$f2
/*  f135810:	46164401 */ 	sub.s	$f16,$f8,$f22
/*  f135814:	c6423e8c */ 	lwc1	$f2,0x3e8c($s2)
.L0f135818:
/*  f135818:	46161281 */ 	sub.s	$f10,$f2,$f22
/*  f13581c:	4600503c */ 	c.lt.s	$f10,$f0
/*  f135820:	00000000 */ 	nop
/*  f135824:	45020004 */ 	bc1fl	.L0f135838
/*  f135828:	4610c03c */ 	c.lt.s	$f24,$f16
/*  f13582c:	46020181 */ 	sub.s	$f6,$f0,$f2
/*  f135830:	46163400 */ 	add.s	$f16,$f6,$f22
/*  f135834:	4610c03c */ 	c.lt.s	$f24,$f16
.L0f135838:
/*  f135838:	00000000 */ 	nop
/*  f13583c:	45020004 */ 	bc1fl	.L0f135850
/*  f135840:	46008307 */ 	neg.s	$f12,$f16
/*  f135844:	10000002 */ 	b	.L0f135850
/*  f135848:	46008306 */ 	mov.s	$f12,$f16
/*  f13584c:	46008307 */ 	neg.s	$f12,$f16
.L0f135850:
/*  f135850:	46166003 */ 	div.s	$f0,$f12,$f22
/*  f135854:	c7a40260 */ 	lwc1	$f4,0x260($sp)
/*  f135858:	4600203c */ 	c.lt.s	$f4,$f0
/*  f13585c:	00000000 */ 	nop
/*  f135860:	45020003 */ 	bc1fl	.L0f135870
/*  f135864:	c64c3e9c */ 	lwc1	$f12,0x3e9c($s2)
/*  f135868:	e7a00260 */ 	swc1	$f0,0x260($sp)
/*  f13586c:	c64c3e9c */ 	lwc1	$f12,0x3e9c($s2)
.L0f135870:
/*  f135870:	c60e0004 */ 	lwc1	$f14,0x4($s0)
/*  f135874:	c7b20260 */ 	lwc1	$f18,0x260($sp)
/*  f135878:	46166200 */ 	add.s	$f8,$f12,$f22
/*  f13587c:	4600c406 */ 	mov.s	$f16,$f24
/*  f135880:	8fb80214 */ 	lw	$t8,0x214($sp)
/*  f135884:	8fae0080 */ 	lw	$t6,0x80($sp)
/*  f135888:	4608703c */ 	c.lt.s	$f14,$f8
/*  f13588c:	00000000 */ 	nop
/*  f135890:	45020004 */ 	bc1fl	.L0f1358a4
/*  f135894:	c6403e90 */ 	lwc1	$f0,0x3e90($s2)
/*  f135898:	460c7281 */ 	sub.s	$f10,$f14,$f12
/*  f13589c:	46165401 */ 	sub.s	$f16,$f10,$f22
/*  f1358a0:	c6403e90 */ 	lwc1	$f0,0x3e90($s2)
.L0f1358a4:
/*  f1358a4:	46160181 */ 	sub.s	$f6,$f0,$f22
/*  f1358a8:	460e303c */ 	c.lt.s	$f6,$f14
/*  f1358ac:	00000000 */ 	nop
/*  f1358b0:	45020004 */ 	bc1fl	.L0f1358c4
/*  f1358b4:	4610c03c */ 	c.lt.s	$f24,$f16
/*  f1358b8:	46007101 */ 	sub.s	$f4,$f14,$f0
/*  f1358bc:	46162400 */ 	add.s	$f16,$f4,$f22
/*  f1358c0:	4610c03c */ 	c.lt.s	$f24,$f16
.L0f1358c4:
/*  f1358c4:	00000000 */ 	nop
/*  f1358c8:	45020004 */ 	bc1fl	.L0f1358dc
/*  f1358cc:	46008307 */ 	neg.s	$f12,$f16
/*  f1358d0:	10000002 */ 	b	.L0f1358dc
/*  f1358d4:	46008306 */ 	mov.s	$f12,$f16
/*  f1358d8:	46008307 */ 	neg.s	$f12,$f16
.L0f1358dc:
/*  f1358dc:	46166003 */ 	div.s	$f0,$f12,$f22
/*  f1358e0:	4600903c */ 	c.lt.s	$f18,$f0
/*  f1358e4:	00000000 */ 	nop
/*  f1358e8:	45020003 */ 	bc1fl	.L0f1358f8
/*  f1358ec:	c64e3ea0 */ 	lwc1	$f14,0x3ea0($s2)
/*  f1358f0:	46000486 */ 	mov.s	$f18,$f0
/*  f1358f4:	c64e3ea0 */ 	lwc1	$f14,0x3ea0($s2)
.L0f1358f8:
/*  f1358f8:	c6020008 */ 	lwc1	$f2,0x8($s0)
/*  f1358fc:	4600c406 */ 	mov.s	$f16,$f24
/*  f135900:	46167200 */ 	add.s	$f8,$f14,$f22
/*  f135904:	33c70002 */ 	andi	$a3,$s8,0x2
/*  f135908:	4608103c */ 	c.lt.s	$f2,$f8
/*  f13590c:	44814000 */ 	mtc1	$at,$f8
/*  f135910:	45020004 */ 	bc1fl	.L0f135924
/*  f135914:	c6403e94 */ 	lwc1	$f0,0x3e94($s2)
/*  f135918:	460e1281 */ 	sub.s	$f10,$f2,$f14
/*  f13591c:	46165401 */ 	sub.s	$f16,$f10,$f22
/*  f135920:	c6403e94 */ 	lwc1	$f0,0x3e94($s2)
.L0f135924:
/*  f135924:	030e1021 */ 	addu	$v0,$t8,$t6
/*  f135928:	46160181 */ 	sub.s	$f6,$f0,$f22
/*  f13592c:	4602303c */ 	c.lt.s	$f6,$f2
/*  f135930:	00000000 */ 	nop
/*  f135934:	45020004 */ 	bc1fl	.L0f135948
/*  f135938:	4610c03c */ 	c.lt.s	$f24,$f16
/*  f13593c:	46001101 */ 	sub.s	$f4,$f2,$f0
/*  f135940:	46162400 */ 	add.s	$f16,$f4,$f22
/*  f135944:	4610c03c */ 	c.lt.s	$f24,$f16
.L0f135948:
/*  f135948:	00000000 */ 	nop
/*  f13594c:	45020004 */ 	bc1fl	.L0f135960
/*  f135950:	46008307 */ 	neg.s	$f12,$f16
/*  f135954:	10000002 */ 	b	.L0f135960
/*  f135958:	46008306 */ 	mov.s	$f12,$f16
/*  f13595c:	46008307 */ 	neg.s	$f12,$f16
.L0f135960:
/*  f135960:	46166003 */ 	div.s	$f0,$f12,$f22
/*  f135964:	4600903c */ 	c.lt.s	$f18,$f0
/*  f135968:	00000000 */ 	nop
/*  f13596c:	45000002 */ 	bc1f	.L0f135978
/*  f135970:	00000000 */ 	nop
/*  f135974:	46000486 */ 	mov.s	$f18,$f0
.L0f135978:
/*  f135978:	46089282 */ 	mul.s	$f10,$f18,$f8
/*  f13597c:	0004c0c0 */ 	sll	$t8,$a0,0x3
/*  f135980:	00077043 */ 	sra	$t6,$a3,0x1
/*  f135984:	000e78c0 */ 	sll	$t7,$t6,0x3
/*  f135988:	00184140 */ 	sll	$t0,$t8,0x5
/*  f13598c:	25e50008 */ 	addiu	$a1,$t7,0x8
/*  f135990:	27060008 */ 	addiu	$a2,$t8,0x8
/*  f135994:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135998:	0006c140 */ 	sll	$t8,$a2,0x5
/*  f13599c:	a4480008 */ 	sh	$t0,0x8($v0)
/*  f1359a0:	a4580014 */ 	sh	$t8,0x14($v0)
/*  f1359a4:	44033000 */ 	mfc1	$v1,$f6
/*  f1359a8:	a4580020 */ 	sh	$t8,0x20($v0)
/*  f1359ac:	a448002c */ 	sh	$t0,0x2c($v0)
/*  f1359b0:	0003c880 */ 	sll	$t9,$v1,0x2
/*  f1359b4:	a0590007 */ 	sb	$t9,0x7($v0)
/*  f1359b8:	a0590013 */ 	sb	$t9,0x13($v0)
/*  f1359bc:	a059001f */ 	sb	$t9,0x1f($v0)
/*  f1359c0:	a059002b */ 	sb	$t9,0x2b($v0)
/*  f1359c4:	0005c940 */ 	sll	$t9,$a1,0x5
/*  f1359c8:	000f1940 */ 	sll	$v1,$t7,0x5
/*  f1359cc:	a459000a */ 	sh	$t9,0xa($v0)
/*  f1359d0:	a4590016 */ 	sh	$t9,0x16($v0)
/*  f1359d4:	a4430022 */ 	sh	$v1,0x22($v0)
/*  f1359d8:	a443002e */ 	sh	$v1,0x2e($v0)
/*  f1359dc:	c7a4019c */ 	lwc1	$f4,0x19c($sp)
/*  f1359e0:	24010001 */ 	addiu	$at,$zero,0x1
/*  f1359e4:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f1359e8:	440f4000 */ 	mfc1	$t7,$f8
/*  f1359ec:	00000000 */ 	nop
/*  f1359f0:	a44f0000 */ 	sh	$t7,0x0($v0)
/*  f1359f4:	c7aa01a0 */ 	lwc1	$f10,0x1a0($sp)
/*  f1359f8:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f1359fc:	44183000 */ 	mfc1	$t8,$f6
/*  f135a00:	00000000 */ 	nop
/*  f135a04:	a4580002 */ 	sh	$t8,0x2($v0)
/*  f135a08:	c7a401a4 */ 	lwc1	$f4,0x1a4($sp)
/*  f135a0c:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f135a10:	440f4000 */ 	mfc1	$t7,$f8
/*  f135a14:	00000000 */ 	nop
/*  f135a18:	a44f0004 */ 	sh	$t7,0x4($v0)
/*  f135a1c:	c7aa01a8 */ 	lwc1	$f10,0x1a8($sp)
/*  f135a20:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135a24:	44183000 */ 	mfc1	$t8,$f6
/*  f135a28:	00000000 */ 	nop
/*  f135a2c:	a458000c */ 	sh	$t8,0xc($v0)
/*  f135a30:	c7a401ac */ 	lwc1	$f4,0x1ac($sp)
/*  f135a34:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f135a38:	440f4000 */ 	mfc1	$t7,$f8
/*  f135a3c:	00000000 */ 	nop
/*  f135a40:	a44f000e */ 	sh	$t7,0xe($v0)
/*  f135a44:	c7aa01b0 */ 	lwc1	$f10,0x1b0($sp)
/*  f135a48:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135a4c:	44183000 */ 	mfc1	$t8,$f6
/*  f135a50:	00000000 */ 	nop
/*  f135a54:	a4580010 */ 	sh	$t8,0x10($v0)
/*  f135a58:	c7a401b4 */ 	lwc1	$f4,0x1b4($sp)
/*  f135a5c:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f135a60:	440f4000 */ 	mfc1	$t7,$f8
/*  f135a64:	00000000 */ 	nop
/*  f135a68:	a44f0018 */ 	sh	$t7,0x18($v0)
/*  f135a6c:	c7aa01b8 */ 	lwc1	$f10,0x1b8($sp)
/*  f135a70:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135a74:	44183000 */ 	mfc1	$t8,$f6
/*  f135a78:	00000000 */ 	nop
/*  f135a7c:	a458001a */ 	sh	$t8,0x1a($v0)
/*  f135a80:	c7a401bc */ 	lwc1	$f4,0x1bc($sp)
/*  f135a84:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f135a88:	440f4000 */ 	mfc1	$t7,$f8
/*  f135a8c:	00000000 */ 	nop
/*  f135a90:	a44f001c */ 	sh	$t7,0x1c($v0)
/*  f135a94:	c7aa01c0 */ 	lwc1	$f10,0x1c0($sp)
/*  f135a98:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135a9c:	44183000 */ 	mfc1	$t8,$f6
/*  f135aa0:	00000000 */ 	nop
/*  f135aa4:	a4580024 */ 	sh	$t8,0x24($v0)
/*  f135aa8:	c7a401c4 */ 	lwc1	$f4,0x1c4($sp)
/*  f135aac:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f135ab0:	440f4000 */ 	mfc1	$t7,$f8
/*  f135ab4:	00000000 */ 	nop
/*  f135ab8:	a44f0026 */ 	sh	$t7,0x26($v0)
/*  f135abc:	c7aa01c8 */ 	lwc1	$f10,0x1c8($sp)
/*  f135ac0:	4600518d */ 	trunc.w.s	$f6,$f10
/*  f135ac4:	44183000 */ 	mfc1	$t8,$f6
/*  f135ac8:	00000000 */ 	nop
/*  f135acc:	a4580028 */ 	sh	$t8,0x28($v0)
/*  f135ad0:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f135ad4:	8fb11398 */ 	lw	$s1,0x1398($sp)
/*  f135ad8:	3c180470 */ 	lui	$t8,0x470
/*  f135adc:	15c10012 */ 	bne	$t6,$at,.L0f135b28
/*  f135ae0:	26390008 */ 	addiu	$t9,$s1,0x8
/*  f135ae4:	afb91398 */ 	sw	$t9,0x1398($sp)
/*  f135ae8:	37180060 */ 	ori	$t8,$t8,0x60
/*  f135aec:	ae380000 */ 	sw	$t8,0x0($s1)
/*  f135af0:	afa00198 */ 	sw	$zero,0x198($sp)
/*  f135af4:	0c012d20 */ 	jal	osVirtualToPhysical
/*  f135af8:	8fa40214 */ 	lw	$a0,0x214($sp)
/*  f135afc:	ae220004 */ 	sw	$v0,0x4($s1)
/*  f135b00:	8fae1398 */ 	lw	$t6,0x1398($sp)
/*  f135b04:	3c19b100 */ 	lui	$t9,0xb100
/*  f135b08:	3c187654 */ 	lui	$t8,0x7654
/*  f135b0c:	25cf0008 */ 	addiu	$t7,$t6,0x8
/*  f135b10:	afaf1398 */ 	sw	$t7,0x1398($sp)
/*  f135b14:	37183210 */ 	ori	$t8,$t8,0x3210
/*  f135b18:	37394602 */ 	ori	$t9,$t9,0x4602
/*  f135b1c:	add90000 */ 	sw	$t9,0x0($t6)
/*  f135b20:	10000003 */ 	b	.L0f135b30
/*  f135b24:	add80004 */ 	sw	$t8,0x4($t6)
.L0f135b28:
/*  f135b28:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f135b2c:	afae0198 */ 	sw	$t6,0x198($sp)
.L0f135b30:
/*  f135b30:	27de0001 */ 	addiu	$s8,$s8,0x1
.L0f135b34:
/*  f135b34:	240101f4 */ 	addiu	$at,$zero,0x1f4
/*  f135b38:	17c1fdaf */ 	bne	$s8,$at,.L0f1351f8
/*  f135b3c:	26100020 */ 	addiu	$s0,$s0,0x20
/*  f135b40:	8faf0198 */ 	lw	$t7,0x198($sp)
/*  f135b44:	8fb01398 */ 	lw	$s0,0x1398($sp)
/*  f135b48:	3c0e0470 */ 	lui	$t6,0x470
/*  f135b4c:	19e0000f */ 	blez	$t7,.L0f135b8c
/*  f135b50:	26180008 */ 	addiu	$t8,$s0,0x8
/*  f135b54:	afb81398 */ 	sw	$t8,0x1398($sp)
/*  f135b58:	35ce0060 */ 	ori	$t6,$t6,0x60
/*  f135b5c:	ae0e0000 */ 	sw	$t6,0x0($s0)
/*  f135b60:	0c012d20 */ 	jal	osVirtualToPhysical
/*  f135b64:	8fa40214 */ 	lw	$a0,0x214($sp)
/*  f135b68:	ae020004 */ 	sw	$v0,0x4($s0)
/*  f135b6c:	8faf1398 */ 	lw	$t7,0x1398($sp)
/*  f135b70:	3c18b100 */ 	lui	$t8,0xb100
/*  f135b74:	37180002 */ 	ori	$t8,$t8,0x2
/*  f135b78:	25f90008 */ 	addiu	$t9,$t7,0x8
/*  f135b7c:	afb91398 */ 	sw	$t9,0x1398($sp)
/*  f135b80:	240e3210 */ 	addiu	$t6,$zero,0x3210
/*  f135b84:	adee0004 */ 	sw	$t6,0x4($t7)
/*  f135b88:	adf80000 */ 	sw	$t8,0x0($t7)
.L0f135b8c:
/*  f135b8c:	8fbf007c */ 	lw	$ra,0x7c($sp)
/*  f135b90:	8fa21398 */ 	lw	$v0,0x1398($sp)
/*  f135b94:	d7b40028 */ 	ldc1	$f20,0x28($sp)
/*  f135b98:	d7b60030 */ 	ldc1	$f22,0x30($sp)
/*  f135b9c:	d7b80038 */ 	ldc1	$f24,0x38($sp)
/*  f135ba0:	d7ba0040 */ 	ldc1	$f26,0x40($sp)
/*  f135ba4:	d7bc0048 */ 	ldc1	$f28,0x48($sp)
/*  f135ba8:	d7be0050 */ 	ldc1	$f30,0x50($sp)
/*  f135bac:	8fb00058 */ 	lw	$s0,0x58($sp)
/*  f135bb0:	8fb1005c */ 	lw	$s1,0x5c($sp)
/*  f135bb4:	8fb20060 */ 	lw	$s2,0x60($sp)
/*  f135bb8:	8fb30064 */ 	lw	$s3,0x64($sp)
/*  f135bbc:	8fb40068 */ 	lw	$s4,0x68($sp)
/*  f135bc0:	8fb5006c */ 	lw	$s5,0x6c($sp)
/*  f135bc4:	8fb60070 */ 	lw	$s6,0x70($sp)
/*  f135bc8:	8fb70074 */ 	lw	$s7,0x74($sp)
/*  f135bcc:	8fbe0078 */ 	lw	$s8,0x78($sp)
/*  f135bd0:	03e00008 */ 	jr	$ra
/*  f135bd4:	27bd1398 */ 	addiu	$sp,$sp,0x1398
);

const char var7f1b569c[] = "snowwidth";
const char var7f1b56a8[] = "snowheight";
const char var7f1b56b4[] = "snowcol1";
const char var7f1b56c0[] = "snowcol2";

u32 var8007f100 = 50;
u32 var8007f104 = 5;
u32 var8007f108 = 10;
u32 var8007f10c = 0x8888aaff;
u32 var8007f110 = 0xffffff7f;
#else
Gfx *weatherRenderSnow(Gfx *gdl, struct weatherdata *weather, s32 arg2)
{
	struct weatherparticledata *particledata;
	struct weatherparticle *particle;
	s32 j;
	s32 k;
	s32 p;
	u32 sp137c[1];
	u32 stack[15];
	u32 sp1354[1];
	bool a0;
	bool s1;
	f32 f22 = 0.0f;
	s32 sp126c[50];
	s32 sp1268;
	f32 sp1168[8][4][2];
	struct coord sp115c;
	struct coord sp1150;
	f32 sp114c;
	f32 sp1148;
	f32 sp1144;
	s32 j2;
	s32 sp1078[50];
	struct coord spe20[50];
	struct coord spbc8[50];
	f32 sp264[50][12];
	f32 sp260;
	s32 s7;
	Col *colours;
	f32 f0;
	s32 numneighbours;
	f32 f20;
	f32 f2;
	f32 f0_3;
	struct coord sp234;
	struct coord sp228;
	f32 sp220;
	f32 sp21c;
	Mtxf *mtx;
	Vtx *vertices; // 214
	Vtx *vtxbatch;
	Mtxf sp1cc;
	struct coord sp19c[4];
	s32 sp198;
	f32 f24;
	s32 i; // 184
	struct coord sp178;
	struct coord sp16c;
#ifdef AVOID_UB
	RoomNum sp144[21]; // prevent bgRoomGetNeighbours from writing out of bounds
#else
	RoomNum sp144[20];
#endif
	struct coord sp124;
	struct coord sp118;
	f32 f26;
	f32 sp108;
	f32 f16;
	s32 numcolours = 16;
	f32 range = 150.0f;

	static u32 var8007f100 = 50;
	static u32 snowwidth = 5;
	static u32 snowheight = 10;
	static u32 snowcol1 = 0x8888aaff;
	static u32 snowcol2 = 0xffffff7f;

	s7 = 0;
	sp1268 = 0;

	texSelect(&gdl, &g_TexGeneralConfigs[0], 4, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0);

	mainOverrideVariable("snowwidth", &snowwidth);
	mainOverrideVariable("snowheight", &snowheight);
	mainOverrideVariable("snowcol1", &snowcol1);
	mainOverrideVariable("snowcol2", &snowcol2);

	particledata = weather->particledata[arg2];

	sp198 = 0;

	mtx4LoadIdentity(&sp1cc);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp1cc);

	sp1cc.m[3][0] = 0.0f;
	sp1cc.m[3][1] = 0.0f;
	sp1cc.m[3][2] = 0.0f;

	mtx = gfxAllocateMatrix();

	mtxF2L(&sp1cc, mtx);

	gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	sp234.f[0] = g_Vars.currentplayer->cam_pos.f[0];
	sp234.f[1] = g_Vars.currentplayer->cam_pos.f[1];
	sp234.f[2] = g_Vars.currentplayer->cam_pos.f[2];

	sp228.f[0] = sp234.f[0] - particledata->unk3e80.f[0];
	sp228.f[1] = sp234.f[1] - particledata->unk3e80.f[1];
	sp228.f[2] = sp234.f[2] - particledata->unk3e80.f[2];

	if (ABSF(sp228.f[0]) > ABSF(particledata->boundarymin.f[0]) + ABSF(particledata->boundarymax.f[0])
			|| ABSF(sp228.f[1]) > ABSF(particledata->boundarymin.f[1]) + ABSF(particledata->boundarymax.f[1])
			|| ABSF(sp228.f[2]) > ABSF(particledata->boundarymin.f[2]) + ABSF(particledata->boundarymax.f[2])) {
		sp228.f[0] = particledata->boundaryrange.f[0] / 2.0f;
		sp228.f[1] = particledata->boundaryrange.f[1] / 2.0f;
		sp228.f[2] = particledata->boundaryrange.f[2] / 2.0f;
	}

	// 4ac8
	for (p = 0; p < 500; p++) {
		particle = &particledata->particles[p];

		// x
		f0 = particle->pos.f[0] - particledata->boundarymin.f[0] - sp228.f[0];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[0];
		}

		if (f0 > particledata->boundaryrange.f[0]) {
			f0 -= particledata->boundaryrange.f[0];
		}

		particle->pos.f[0] = particledata->boundarymin.f[0] + f0;

		// y
		f0 = particle->pos.f[1] - particledata->boundarymin.f[1] - sp228.f[1];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[1];
		}

		if (f0 > particledata->boundaryrange.f[1]) {
			f0 -= particledata->boundaryrange.f[1];
		}

		particle->pos.f[1] = particledata->boundarymin.f[1] + f0;

		// z
		f0 = particle->pos.f[2] - particledata->boundarymin.f[2] - sp228.f[2];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[2];
		}

		if (f0 > particledata->boundaryrange.f[2]) {
			f0 -= particledata->boundaryrange.f[2];
		}

		particle->pos.f[2] = particledata->boundarymin.f[2] + f0;
	}

	// 4bbc
	particledata->unk3e80.f[0] = sp234.f[0];
	particledata->unk3e80.f[1] = sp234.f[1];
	particledata->unk3e80.f[2] = sp234.f[2];

	sp16c.f[0] = particledata->boundarymin.f[0] + g_Vars.currentplayer->cam_pos.f[0];
	sp178.f[0] = particledata->boundarymax.f[0] + g_Vars.currentplayer->cam_pos.f[0];
	sp16c.f[1] = particledata->boundarymin.f[1] + g_Vars.currentplayer->cam_pos.f[1];
	sp178.f[1] = particledata->boundarymax.f[1] + g_Vars.currentplayer->cam_pos.f[1];
	sp16c.f[2] = particledata->boundarymin.f[2] + g_Vars.currentplayer->cam_pos.f[2];
	sp178.f[2] = particledata->boundarymax.f[2] + g_Vars.currentplayer->cam_pos.f[2];

	if (sp1268 < 50) {
		sp126c[sp1268] = g_Vars.currentplayer->cam_room;
		sp1268++;
	}

	// 4c54
	for (i = 0; i < sp1268; i++) {
		numneighbours = bgRoomGetNeighbours(sp126c[i], sp144, 20);

		for (j2 = 0; j2 < numneighbours; j2++) {
			a0 = true;

			if (g_Rooms[sp144[j2]].flags & ROOMFLAG_ONSCREEN) {
				for (k = 0; k < sp1268; k++) {
					if (sp126c[k] == sp144[j2]) {
						a0 = false;
					}
				}

				if (a0) {
					if (sp178.f[0] < g_Rooms[sp144[j2]].bbmin[0] || sp16c.f[0] > g_Rooms[sp144[j2]].bbmax[0]) {
						a0 = false;
					}

					if (sp178.f[1] < g_Rooms[sp144[j2]].bbmin[1] || sp16c.f[1] > g_Rooms[sp144[j2]].bbmax[1]) {
						a0 = false;
					}

					if (sp178.f[2] < g_Rooms[sp144[j2]].bbmin[2] || sp16c.f[2] > g_Rooms[sp144[j2]].bbmax[2]) {
						a0 = false;
					}
				}

				if (a0 && sp1268 < 50) {
					sp126c[sp1268] = sp144[j2];
					sp1268++;
				} else {
					// empty
				}
			}
		}
	}

	// 4dc0
	for (i = 0; i < sp1268; i++) {
		if (weatherIsRoomWeatherProof(sp126c[i])) {
			spe20[s7].f[0] = g_Rooms[sp126c[i]].bbmin[0] / 1;
			spe20[s7].f[1] = g_Rooms[sp126c[i]].bbmin[1] / 1;
			spe20[s7].f[2] = g_Rooms[sp126c[i]].bbmin[2] / 1;

			spbc8[s7].f[0] = g_Rooms[sp126c[i]].bbmax[0] / 1;
			spbc8[s7].f[1] = g_Rooms[sp126c[i]].bbmax[1] / 1;
			spbc8[s7].f[2] = g_Rooms[sp126c[i]].bbmax[2] / 1;

			if (s7 < 50) {
				sp1078[s7] = sp126c[i];
				s7++;
			} else {
				// empty
			}
		}

	}

	// 4ea4
	for (j = 0; j < s7; j++) {
		if (0);
		sp264[j][6] = sp264[j][0] = g_Rooms[sp1078[j]].bbmin[0] / 1 - snowwidth;
		sp264[j][9] = sp264[j][3] = g_Rooms[sp1078[j]].bbmax[0] / 1 + snowwidth;
		sp264[j][6] -= var8007f100;
		sp264[j][9] += var8007f100;

		sp264[j][7] = sp264[j][1] = g_Rooms[sp1078[j]].bbmin[1] / 1 - snowwidth;
		sp264[j][10] = sp264[j][4] = g_Rooms[sp1078[j]].bbmax[1] / 1 + snowwidth;
		sp264[j][7] -= var8007f100;
		sp264[j][10] += var8007f100;

		sp264[j][8] = sp264[j][2] = g_Rooms[sp1078[j]].bbmin[2] / 1 - snowwidth;
		sp264[j][11] = sp264[j][5] = g_Rooms[sp1078[j]].bbmax[2] / 1 + snowwidth;
		sp264[j][8] -= var8007f100;
		sp264[j][11] += var8007f100;
	}

	// 4ff0
	for (j = 0; j < 8; j++) {
		sp1168[j][0][0] = sinf(particledata->unk3ec8[j]);
		sp1168[j][0][1] = cosf(particledata->unk3ec8[j]);
		sp1168[j][1][0] = sinf(particledata->unk3ec8[j] + M_PI * 0.5f);
		sp1168[j][1][1] = cosf(particledata->unk3ec8[j] + M_PI * 0.5f);
		sp1168[j][2][0] = sinf(particledata->unk3ec8[j] + M_PI);
		sp1168[j][2][1] = cosf(particledata->unk3ec8[j] + M_PI);
		sp1168[j][3][0] = sinf(particledata->unk3ec8[j] + M_PI * 1.5f);
		sp1168[j][3][1] = cosf(particledata->unk3ec8[j] + M_PI * 1.5f);
	}

	// 514c
	colours = gfxAllocateColours(numcolours);

	for (j = 0; j < numcolours; j++) {
		u32 alpha = ((numcolours + 1) * 255 - j * 255) / (numcolours + 1);
		colours[j].word = PD_BE32((snowcol1 & 0xffffff00) | alpha);
	}

	gSPColor(gdl++, osVirtualToPhysical(colours), numcolours);

	// 51f8
	for (p = 0; p < 500; p++) {
		struct weatherparticle *particle = &particledata->particles[p];
		s32 tmp2;
		s1 = true;

		if (particle->active & 3) {
			sp1354[0] = osGetCount();

			sp124.f[0] = particle->pos.f[0] + particledata->unk3e80.f[0];
			sp124.f[1] = particle->pos.f[1] + particledata->unk3e80.f[1];
			sp124.f[2] = particle->pos.f[2] + particledata->unk3e80.f[2];

			if (cam0f0b5b9c(&sp124, 5)) {
				sp137c[0] = sp137c[0] + osGetCount() - sp1354[0];

				sp21c = particle->pos.f[0];
				sp220 = particle->pos.f[2];

				f20 = sqrtf(sp220 * sp220 + sp21c * sp21c);

				if (f20 < 0.00001f) {
					// empty
				} else {
					if (sp198 == 0) {
						vertices = gfxAllocateVertices(8);
					}

					sp260 = 0.0f;

					sp21c /= f20;
					sp220 /= f20;

					for (j = 0; j < 4; j++) {
						vertices[j + sp198 * 4].s = 0;
						vertices[j + sp198 * 4].t = 0;

						sp19c[j].f[0] = particle->pos.f[0];
						sp19c[j].f[1] = particle->pos.f[1];
						sp19c[j].f[2] = particle->pos.f[2];
					}

					// 5344
					if (s7 > 0) {
						sp118.f[0] = particle->pos.f[0] + particledata->unk3e80.f[0];
						sp118.f[1] = particle->pos.f[1] + particledata->unk3e80.f[1];
						sp118.f[2] = particle->pos.f[2] + particledata->unk3e80.f[2];

						for (j = 0; j < s7; j++) {
							// 5398
							if (s1
									&& sp264[j][6] < sp118.f[0]
									&& sp264[j][9] > sp118.f[0]
									&& sp264[j][7] < sp118.f[1]
									&& sp264[j][10] > sp118.f[1]
									&& sp264[j][8] < sp118.f[2]
									&& sp264[j][11] > sp118.f[2]) {
								if (sp264[j][0] < sp118.f[0]
										&& sp264[j][3] > sp118.f[0]
										&& sp264[j][1] < sp118.f[1]
										&& sp264[j][4] > sp118.f[1]
										&& sp264[j][2] < sp118.f[2]
										&& sp264[j][5] > sp118.f[2]) {
									s1 = false;
								} else {
									// 54a8
									f2 = 0.0f;

									// 54d8
									if (sp264[j][0] > sp118.f[0]) {
										f2 = sp118.f[0] - sp264[j][6];
									}

									// 54ec
									if (sp264[j][3] < sp118.f[0]) {
										f2 = sp118.f[0] - sp264[j][9];
									}

									// 5500
									f2 = ABSF(f2) / var8007f100;

									// 5524
									if (f2 > sp260) {
										sp260 = f2;
									}

									if (sp264[j][2] > sp118.f[2]) {
										f2 = sp118.f[2] - sp264[j][8];
									}

									if (sp264[j][5] < sp118.f[2]) {
										f2 = sp118.f[2] - sp264[j][11];
									}

									f2 = ABSF(f2) / var8007f100;

									if (f2 > sp260) {
										sp260 = f2;
									}
								}
							}
						}
					}

					// 559c
					if (s1) {
						s32 j;
						f32 val1;
						f32 val2;
						f32 val3;

						tmp2 = sp198 * 4;

						if (range);

						f0_3 = sqrtf(particle->pos.f[0] * particle->pos.f[0]
								+ particle->pos.f[1] * particle->pos.f[1]
								+ particle->pos.f[2] * particle->pos.f[2]);

						f24 = particle->pos.f[0] / f0_3;
						sp108 = particle->pos.f[1] / f0_3;
						f26 = particle->pos.f[2] / f0_3;

						// 55fc
						f0_3 = sqrtf(f24 * f24 + f26 * f26);

						val2 = f24 / f0_3;
						val1 = f26 / f0_3;
						val3 = -val2;

						sp114c = sp108 * val2;
						sp1148 = -f0_3;
						sp1144 = sp108 * val1;

						sp115c.f[0] = -sp220;
						sp115c.f[1] = 1.0f;
						sp115c.f[2] = sp21c;

						// 5720
						for (j = 0; j < 4; j++) {
							sp19c[j].f[0] += snowwidth * val1 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp114c * sp1168[(p >> 2) & 7][j][1];
							sp19c[j].f[1] += snowwidth *  f22 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp1148 * sp1168[(p >> 2) & 7][j][1];
							sp19c[j].f[2] += snowwidth * val3 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp1144 * sp1168[(p >> 2) & 7][j][1];
						}

						// 5784
						// x
						f16 = 0.0f;

						if (particle->pos.f[0] < particledata->boundarymin.f[0] + range) {
							f16 = particle->pos.f[0] - particledata->boundarymin.f[0] - range;
						}

						if (particle->pos.f[0] > particledata->boundarymax.f[0] - range) {
							f16 = particle->pos.f[0] - particledata->boundarymax.f[0] + range;
						}

						f16 = ABSF(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 5870
						// y
						f16 = 0.0f;

						if (particle->pos.f[1] < particledata->boundarymin.f[1] + range) {
							f16 = particle->pos.f[1] - particledata->boundarymin.f[1] - range;
						}

						if (particle->pos.f[1] > particledata->boundarymax.f[1] - range) {
							f16 = particle->pos.f[1] - particledata->boundarymax.f[1] + range;
						}

						f16 = ABSF(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 58f8
						// z
						f16 = 0.0f;

						if (particle->pos.f[2] < particledata->boundarymin.f[2] + range) {
							f16 = particle->pos.f[2] - particledata->boundarymin.f[2] - range;
						}

						if (particle->pos.f[2] > particledata->boundarymax.f[2] - range) {
							f16 = particle->pos.f[2] - particledata->boundarymax.f[2] + range;
						}

						f16 = ABSF(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 5978
						vertices[tmp2 + 0].colour = (s32) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 1].colour = (s32) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 2].colour = (s32) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 3].colour = (s32) (sp260 * 16.0f) * 4;

						// Note: Goal writes all the S's first then T's. XBLA uses ST pairs.
						// And the rain function uses a different order too.
						{
							u16 x1;
							u16 y1;
							u16 x2;
							u16 y2;

							y2 += 0;
							x1 = ((p & 1) >> 0) * 8;
							y1 = ((p & 2) >> 1) * 8;
							x2 = x1 + 8;
							y2 = y1 + 8;

							vertices[tmp2 + 0].s = x1 * 32;
							vertices[tmp2 + 0].t = y2 * 32;
							vertices[tmp2 + 1].s = x2 * 32;
							vertices[tmp2 + 1].t = y2 * 32;
							vertices[tmp2 + 2].s = x2 * 32;
							vertices[tmp2 + 2].t = y1 * 32;
							vertices[tmp2 + 3].s = x1 * 32;
							vertices[tmp2 + 3].t = y1 * 32;

							// rain order:
							//vertices[tmp2 + 0].t = y2 * 32;
							//vertices[tmp2 + 1].s = x2 * 32;
							//vertices[tmp2 + 1].t = y2 * 32;
							//vertices[tmp2 + 2].s = x2 * 32;
							//vertices[tmp2 + 3].t = y1 * 32;
							//vertices[tmp2 + 3].s = x1 * 32;
							//vertices[tmp2 + 2].t = y1 * 32;
							//vertices[tmp2 + 0].s = x1 * 32;

							if (sp198 && sp198 && sp198);
						}

						vertices[tmp2 + 0].x = sp19c[0].f[0];
						vertices[tmp2 + 0].y = sp19c[0].f[1];
						vertices[tmp2 + 0].z = sp19c[0].f[2];

						vertices[tmp2 + 1].x = sp19c[1].f[0];
						vertices[tmp2 + 1].y = sp19c[1].f[1];
						vertices[tmp2 + 1].z = sp19c[1].f[2];

						vertices[tmp2 + 2].x = sp19c[2].f[0];
						vertices[tmp2 + 2].y = sp19c[2].f[1];
						vertices[tmp2 + 2].z = sp19c[2].f[2];

						vertices[tmp2 + 3].x = sp19c[3].f[0];
						vertices[tmp2 + 3].y = sp19c[3].f[1];
						vertices[tmp2 + 3].z = sp19c[3].f[2];

						if (sp198 == 1) {
							gSPVertex(gdl++, osVirtualToPhysical(vertices), 8, 0);
							gSPTri4(gdl++, 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4);
							sp198 = 0;
						} else {
							sp198 = 1;
						}
					}
				}
			}
		}
	}

	if (sp198 > 0) {
		gSPVertex(gdl++, osVirtualToPhysical(vertices), 8, 0);
		gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
	}

	return gdl;
}
#endif

void weatherStop(void)
{
	if (g_WeatherData) {
		if (g_WeatherData->audiohandles[0]) {
			audioStop(g_WeatherData->audiohandles[0]);
		}

		if (g_WeatherData->audiohandles[1]) {
			audioStop(g_WeatherData->audiohandles[1]);
		}

		if (g_WeatherData->audiohandles[2]) {
			audioStop(g_WeatherData->audiohandles[2]);
		}

		if (g_WeatherData->audiohandles[3]) {
			audioStop(g_WeatherData->audiohandles[3]);
		}

		g_WeatherData = NULL;
	}
}
