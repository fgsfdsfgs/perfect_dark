#include <ultra64.h>
#include "constants.h"
#include "game/dlights.h"
#include "game/camera.h"
#include "game/chr.h"
#include "game/gfxmemory.h"
#include "game/tex.h"
#include "game/sparks.h"
#include "game/file.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

struct spark g_Sparks[100];
s32 g_NextSparkIndex;
u32 var800a3fc4;
struct sparkgroup g_SparkGroups[10];
s32 g_NextSparkGroupIndex;
u32 var800a4134;
u32 var800a4138;
u32 var800a413c;
u32 var800a4140;
u32 var800a4144;
u32 var800a4148;
u32 var800a414c;

struct sparktype g_SparkTypes[] = {
	//                                     weight
	//                                     |     max age
	//                                     |     |                                        decel
	//                                     |     |                                        |
#if PAL
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0x80ffffff, 0xffffffff, 0.024 },
	/*0x00*/ { 40,  -1,  30,   30,  0,  0, 2.4,              29,  29,  5,  1, 0x301010ff, 0x401010ff, 0.024 },
	/*0x00*/ { 40,  -1,  300,  200, 0,  0, 0.18,             5,   5,   4,  1, 0xffffff40, 0x560011a0, 0.024 },
	/*0x00*/ { 10,  1,   1200, 400, 0,  0, 0.18,             5,   5,   5,  1, 0xa0a0e000, 0xffffffff, 0.024 },
	/*0x00*/ { 40,  10,  10,   10,  0,  0, 3.6000001430511,  150, 75,  5,  1, 0x00ff6aff, 0xffffffff, 0.024 },
	/*0x00*/ { 15,  10,  20,   20,  0,  0, 0.78000003099442, 150, 75,  5,  1, 0xa0a0e0ff, 0xffffffff, 0.024 },
	/*0x00*/ { 1,   1,   120,  2,   0,  0, 0,                50,  50,  5,  1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 1,   1,   300,  50,  0,  0, 0,                5,   5,   5,  1, 0xffff8000, 0xffffffff, 0.024 },
	/*0x00*/ { 1,   1,   400,  100, 0,  0, 0,                8,   8,   5,  1, 0xffff8000, 0xffffffff, 0.024 },
	/*0x00*/ { 75,  100, 100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 75,  100, 20,   5,   0,  0, 1.8000000715256,  50,  50,  20, 1, 0xa0a0e0ff, 0xffffffff, 0.024 },
	/*0x00*/ { 20,  10,  100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 20,  10,  20,   5,   0,  0, 1.8000000715256,  50,  50,  20, 1, 0xa0a0e0ff, 0xffffffff, 0.024 },
	/*0x00*/ { 1,   0,   80,   1,   36, 6, 0,                100, 75,  15, 0, 0x0808f000, 0xffffffff, 0.024 },
	/*0x00*/ { 70,  0,   150,  15,  0,  0, 7.2000002861023,  33,  8,   3,  0, 0x11112880, 0xaaaaaa40, 0.024 },
	/*0x00*/ { 50,  28,  100,  1,   0,  0, 1.2,              50,  25,  10, 1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 300, 100, 100,  2,   0,  0, 0.36,             25,  1,   40, 2, 0xffff80ff, 0xffffffff, 0.119 },
	/*0x00*/ { 170, 80,  60,   5,   0,  0, 0.48000001907349, 25,  5,   10, 3, 0xa0a0e0ff, 0xffffffff, 0.119 },
	/*0x00*/ { 120, 40,  30,   7,   0,  0, 0.24000000953674, 50,  5,   15, 3, 0xffff80ff, 0xffffffff, 0.119 },
	/*0x00*/ { 80,  10,  10,   9,   0,  0, 0.12000000476837, 25,  5,   20, 3, 0xa0a0e0ff, 0xffffffff, 0.119 },
	/*0x00*/ { 100, 1,   100,  50,  0,  0, 2.4,              25,  16,  15, 1, 0xffff80ff, 0xffffffff, 0.024 },
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2.4,              100, 100, 30, 1, 0xff8080ff, 0xffff80ff, 0.024 },
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0x4fff4fff, 0xffffffff, 0.024 },
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2.4,              50,  50,  15, 1, 0xffff7f7f, 0xffffffff, 0.024 },
	/*0x00*/ { 40,  -1,  30,   10,  0,  0, 2.4,              41,  29,  10, 1, 0x301010ff, 0x401010ff, 0.024 },
	/*0x00*/ { 70,  0,   150,  15,  0,  0, 7.2000002861023,  33,  8,   3,  0, 0x1111a880, 0xaaaaff40, 0.024 },
#else
	/*0x00*/ { 100, 28,  100,  1,   0,  0, 2,                60,  60,  15, 1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x01*/ { 100, 28,  100,  1,   0,  0, 2,                60,  60,  15, 1, 0x80ffffff, 0xffffffff, 0.02 },
	/*0x02*/ { 40,  -1,  30,   30,  0,  0, 2,                35,  35,  5,  1, 0x301010ff, 0x401010ff, 0.02 },
	/*0x03*/ { 40,  -1,  300,  200, 0,  0, 0.15,             5,   5,   4,  1, 0xffffff40, 0x560011a0, 0.02 },
	/*0x04*/ { 10,  1,   1200, 400, 0,  0, 0.15,             5,   5,   5,  1, 0xa0a0e000, 0xffffffff, 0.02 },
	/*0x05*/ { 40,  10,  10,   10,  0,  0, 3,                180, 90,  5,  1, 0x00ff6aff, 0xffffffff, 0.02 },
	/*0x06*/ { 15,  10,  20,   20,  0,  0, 0.65,             180, 90,  5,  1, 0xa0a0e0ff, 0xffffffff, 0.02 },
	/*0x07*/ { 1,   1,   120,  2,   0,  0, 0,                60,  60,  5,  1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x08*/ { 1,   1,   300,  50,  0,  0, 0,                5,   5,   5,  1, 0xffff8000, 0xffffffff, 0.02 },
	/*0x09*/ { 1,   1,   400,  100, 0,  0, 0,                10,  10,  5,  1, 0xffff8000, 0xffffffff, 0.02 },
	/*0x0a*/ { 75,  100, 100,  1,   0,  0, 2,                60,  60,  15, 1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x0b*/ { 75,  100, 20,   5,   0,  0, 1.5,              60,  60,  20, 1, 0xa0a0e0ff, 0xffffffff, 0.02 },
	/*0x0c*/ { 20,  10,  100,  1,   0,  0, 2,                60,  60,  15, 1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x0d*/ { 20,  10,  20,   5,   0,  0, 1.5,              60,  60,  20, 1, 0xa0a0e0ff, 0xffffffff, 0.02 },
	/*0x0e*/ { 1,   0,   80,   1,   30, 5, 0,                120, 90,  15, 0, 0x0808f000, 0xffffffff, 0.02 },
	/*0x0f*/ { 70,  0,   150,  15,  0,  0, 6,                40,  10,  3,  0, 0x11112880, 0xaaaaaa40, 0.02 },
	/*0x10*/ { 50,  28,  100,  1,   0,  0, 1,                60,  30,  10, 1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x11*/ { 300, 100, 100,  2,   0,  0, 0.3,              30,  1,   40, 2, 0xffff80ff, 0xffffffff, 0.1  },
	/*0x12*/ { 170, 80,  60,   5,   0,  0, 0.4,              30,  5,   10, 3, 0xa0a0e0ff, 0xffffffff, 0.1  },
	/*0x13*/ { 120, 40,  30,   7,   0,  0, 0.2,              60,  5,   15, 3, 0xffff80ff, 0xffffffff, 0.1  },
	/*0x14*/ { 80,  10,  10,   9,   0,  0, 0.1,              30,  5,   20, 3, 0xa0a0e0ff, 0xffffffff, 0.1  },
	/*0x15*/ { 100, 1,   100,  50,  0,  0, 2,                30,  20,  15, 1, 0xffff80ff, 0xffffffff, 0.02 },
	/*0x16*/ { 100, 28,  100,  1,   0,  0, 2,                120, 120, 30, 1, 0xff8080ff, 0xffff80ff, 0.02 },
	/*0x17*/ { 100, 28,  100,  1,   0,  0, 2,                60,  60,  15, 1, 0x4fff4fff, 0xffffffff, 0.02 },
	/*0x18*/ { 100, 28,  100,  1,   0,  0, 2,                60,  60,  15, 1, 0xffff7f7f, 0xffffffff, 0.02 },
	/*0x19*/ { 40,  -1,  30,   10,  0,  0, 2,                50,  35,  10, 1, 0x301010ff, 0x401010ff, 0.02 },
	/*0x1a*/ { 70,  0,   150,  15,  0,  0, 6,                40,  10,  3,  0, 0x1111a880, 0xaaaaff40, 0.02 },
#endif
};

bool g_SparksAreActive = false;

void sparkCreate(struct coord *pos, struct sparktype *type)
{
	f32 tmp;
	f32 maxspeed = 0.0f;
	s32 i;
	struct spark *spark;

	spark = &g_Sparks[g_NextSparkIndex];
	g_NextSparkIndex = (g_NextSparkIndex + 1) % 100;

	spark->pos.x = 0.0f;
	spark->pos.y = 0.0f;
	spark->pos.z = 0.0f;

	spark->speed.x = (s32)(rngRandom() % (type->unk00 * 2 + 1)) - type->unk00;
	spark->speed.y = (s32)(rngRandom() % (type->unk00 * 2 + 1)) - type->unk00;
	spark->speed.z = (s32)(rngRandom() % (type->unk00 * 2 + 1)) - type->unk00;

	if (spark->speed.y == 0.0f) {
		spark->speed.y = -0.0001f;
	}

	for (i = 0; i < 3; i++) {
		if (spark->speed.f[i] > maxspeed) {
			maxspeed = spark->speed.f[i];
		}

		if (-spark->speed.f[i] > maxspeed) {
			maxspeed = -spark->speed.f[i];
		}
	}

	tmp = sqrtf(spark->speed.f[0] * spark->speed.f[0] + spark->speed.f[1] * spark->speed.f[1] + spark->speed.f[2] * spark->speed.f[2]);

	spark->speed.x *= maxspeed / tmp;
	spark->speed.y *= maxspeed / tmp;
	spark->speed.z *= maxspeed / tmp;

	spark->speed.y += type->unk00 / 2;

	spark->speed.x += pos->x;
	spark->speed.y += pos->y;
	spark->speed.z += pos->z;

#if PAL
	spark->speed.x *= 1.2f;
	spark->speed.y *= 1.2f;
	spark->speed.z *= 1.2f;
#endif

	if (spark->speed.y == 0.0f) {
		spark->speed.y = -0.0001f;
	}

	if (type->unk18 % 2) {
		spark->ttl = rngRandom() % type->maxage;
	} else {
		spark->ttl = type->maxage;
	}
}

/**
 * This function handles an out-of-memory situation when creating a spark, by
 * shrinking whichever spark group was about to be overwritten.
 */
void sparkgroupEnsureFreeSparkSlot(struct sparkgroup *group)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_SparkGroups); i++) {
		if (&g_SparkGroups[i] != group && g_SparkGroups[i].startindex == g_NextSparkIndex) {
			g_SparkGroups[i].startindex = (g_SparkGroups[i].startindex + 1) % 100;
			g_SparkGroups[i].numsparks--;

			if (g_SparkGroups[i].numsparks == 0) {
				g_SparkGroups[i].age = 0;
			}
		}
	}
}

void sparksCreate(s32 room, struct prop *prop, struct coord *pos, struct coord *arg3, struct coord *arg4, s32 typenum)
{
	struct sparkgroup *group = &g_SparkGroups[g_NextSparkGroupIndex];
	struct sparktype *type = &g_SparkTypes[typenum];
	struct coord grouppos;
	s32 i;

	if ((typenum == SPARKTYPE_BLOOD || typenum == SPARKTYPE_FLESH) && prop && prop->type == PROPTYPE_CHR) {
		struct chrdata *chr = prop->chr;
		u32 colours[3];
		u32 stack;

		chrGetBloodColour(chr->bodynum, NULL, colours);

		if (typenum == SPARKTYPE_BLOOD) {
			type->unk1c = colours[0];
			type->unk20 = colours[1];
		} else if (typenum == SPARKTYPE_FLESH) {
			type->unk20 = colours[2];
		}
	} else if (typenum == SPARKTYPE_PAINT) {
		type->unk1c = (rngRandom() % 2 ? 0xff000000 : 0) | (rngRandom() % 2 ? 0x00ff0000 : 0) | (rngRandom() % 2 ? 0x0000ff00 : 0) | 0xff;

		if (type->unk1c == 0x000000ff) {
			type->unk1c |= 0x0000ff00 << ((rngRandom() % 3) * 8);
		}

		type->unk20 = (rngRandom() % 2 ? 0xff000000 : 0) | (rngRandom() % 2 ? 0x00ff0000 : 0) | (rngRandom() % 2 ? 0x0000ff00 : 0) | 0xff;

		if (type->unk20 == 0xff) {
			type->unk20 |= 0x0000ff00 << ((rngRandom() % 3) * 8);
		}
	}

	g_NextSparkGroupIndex = (g_NextSparkGroupIndex + 1) % ARRAYCOUNT(g_SparkGroups);

	if (typenum == SPARKTYPE_SHALLOWWATER) {
		if (group->age != 0) {
			s32 newindex = -1;

			for (i = 0; i < ARRAYCOUNT(g_SparkGroups) && newindex < 0; i++) {
				if (g_SparkGroups[i].age == 0) {
					newindex = i;
				}
			}

			if (newindex == -1) {
				return;
			}

			group = &g_SparkGroups[newindex];
		}
	}

	if (arg3 != NULL && arg4 != NULL) {
		f32 f0 = sqrtf(arg4->f[0] * arg4->f[0] + arg4->f[1] * arg4->f[1] + arg4->f[2] * arg4->f[2]);

		arg4->x /= f0;
		arg4->y /= f0;
		arg4->z /= f0;

		f0 = -2.0f * (arg3->f[0] * arg4->f[0] + arg3->f[1] * arg4->f[1] + arg3->f[2] * arg4->f[2]);

		grouppos.x = arg3->x + f0 * arg4->f[0];
		grouppos.y = arg3->y + f0 * arg4->f[1];
		grouppos.z = arg3->z + f0 * arg4->f[2];

		f0 = sqrtf(grouppos.f[0] * grouppos.f[0] + grouppos.f[1] * grouppos.f[1] + grouppos.f[2] * grouppos.f[2]);

		if (f0 == 0.0f) {
			f0 = 1.0f;
		}

		grouppos.x *= type->unk02 / f0;
		grouppos.y *= type->unk02 / f0;
		grouppos.z *= type->unk02 / f0;
	} else if (arg3 != NULL && type->unk02 >= -1 && type->unk02 < 2) {
		grouppos.x = arg3->x * 10.0f * type->unk02;
		grouppos.y = arg3->y * 10.0f * type->unk02;
		grouppos.z = arg3->z * 10.0f * type->unk02;
	} else {
		grouppos.x = 0.0f;
		grouppos.y = 0.0f;
		grouppos.z = 0.0f;
	}

	group->type = typenum;
	group->age = 1;
	group->startindex = g_NextSparkIndex;
	group->numsparks = type->numsparks;
	group->pos.x = pos->x;
	group->pos.y = pos->y;
	group->pos.z = pos->z;
	group->room = room;
	group->prop = prop;

	for (i = 0; i < type->numsparks; i++) {
		sparkgroupEnsureFreeSparkSlot(group);
		sparkCreate(&grouppos, type);
	}

	switch (typenum) {
	case SPARKTYPE_DEFAULT:
		roomFlashLighting(group->room, 24, 32);
		break;
	case SPARKTYPE_ENVIRONMENTAL1:
		if (g_Vars.stagenum != STAGE_CRASHSITE) {
			roomFlashLighting(group->room, 32, 128);
		}
		break;
	case SPARKTYPE_ENVIRONMENTAL2:
		if (g_Vars.stagenum != STAGE_CRASHSITE) {
			roomFlashLighting(group->room, 64, 128);
		}
		break;
	case SPARKTYPE_ENVIRONMENTAL3:
		if (g_Vars.stagenum != STAGE_CRASHSITE) {
			roomFlashLighting(group->room, 200, 255);
		}
		break;
	}

	g_SparksAreActive = true;
}

Gfx *sparksRender(Gfx *gdl)
{
	struct sparkgroup *group;
	s32 axis;
	bool render;
	f32 f0;
	f32 f12;
	s32 i;
	s32 j;
	s32 k;
	struct coord *campos;
	struct sparktype *type;
	Col *colours;
	s32 index;
	f32 sp13c;
	f32 sp138;
	Mtxf *mtx;
	s32 v1;
	struct coord sp124;
	f32 sp120;
	s32 diff1;
	s32 diff2;
	f32 frac;
	Mtxf spd4;

	if (g_SparksAreActive) {
		if (ABS(g_Vars.currentplayer->cam_look.y) > ABS(g_Vars.currentplayer->cam_look.x)) {
			axis = ABS(g_Vars.currentplayer->cam_look.z) > ABS(g_Vars.currentplayer->cam_look.y) ? 2 : 1;
		} else {
			axis = ABS(g_Vars.currentplayer->cam_look.z) > ABS(g_Vars.currentplayer->cam_look.x) ? 2 : 0;
		}

		texSelect(&gdl, &g_TexSparkConfigs[0], 4, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);
		gDPSetCombineMode(gdl++, G_CC_CUSTOM_04, G_CC_CUSTOM_04);


		for (i = 0; i < ARRAYCOUNT(g_SparkGroups); i++) {
			group = &g_SparkGroups[i];

			if (group->age != 0) {
				render = true;
				sp138 = 1.0f;

				if (group->prop) {
					if ((group->prop->flags & PROPFLAG_ONTHISSCREENTHISTICK) == 0) {
						render = false;
					}
				} else {
					if ((g_Rooms[group->room].flags & ROOMFLAG_ONSCREEN) == 0) {
						render = false;
					}
				}

				if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
					f0 = sqrtf((group->pos.f[0] - g_Vars.currentplayer->eraserpos.f[0]) * (group->pos.f[0] - g_Vars.currentplayer->eraserpos.f[0])
							+ (group->pos.f[1] - g_Vars.currentplayer->eraserpos.f[1]) * (group->pos.f[1] - g_Vars.currentplayer->eraserpos.f[1])
							+ (group->pos.f[2] - g_Vars.currentplayer->eraserpos.f[2]) * (group->pos.f[2] - g_Vars.currentplayer->eraserpos.f[2]));

					f12 = g_Vars.currentplayer->eraserpropdist - 150.0f;

					sp13c = f0 / g_Vars.currentplayer->eraserpropdist;

					if (g_Vars.currentplayer->eraserpropdist < f0) {
						render = false;
					}

					if (f12 < f0) {
						sp138 = 1.0f - (f0 - f12) / 150.0f;
					}

					if (sp13c > 1.0f) {
						sp13c = 1.0f;
					}
				}

				if (render) {
					campos = &g_Vars.currentplayer->cam_pos;
					sp124.x = campos->x - group->pos.x;
					sp124.y = campos->y - group->pos.y;
					sp124.z = campos->z - group->pos.z;

					sp120 = sqrtf(sp124.f[0] * sp124.f[0] + sp124.f[1] * sp124.f[1] + sp124.f[2] * sp124.f[2]);

					if (sp120 > 20000.0f) {
						render = false;
					}
				}

				if (render) {
					type = &g_SparkTypes[group->type];
					colours = gfxAllocateColours(2);

					if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
						colours[0].word = PD_BE32(type->unk1c);
						colours[1].word = PD_BE32(type->unk20);
					} else if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
						v1 = ((u32) (sp13c * 255.0f) << 24) | ((u32) ((1.0f - sp13c) * 255.0f) << 16);

						// @bug? Second part also reads from type->unk1c
						colours[0].word = PD_BE32(v1 | (u32) (sp138 * (f32) (type->unk1c & 0xff)) | 0x3f00);
						colours[1].word = PD_BE32(v1 | (u32) (sp138 * (f32) (type->unk1c & 0xff)) | 0x3f00);
					} else {
						colours[0].word = PD_BE32(type->unk1c);
						colours[1].word = PD_BE32(type->unk20);
					}

					if (type->unk12 < type->maxage && type->unk12 < group->age) {
						diff1 = (type->maxage - type->unk12);
						diff2 = group->age - type->unk12;
						frac = (f32)(diff1 - diff2) / diff1;

						colours[0].a *= frac;
						colours[1].a *= frac;
					}

					gSPColor(gdl++, osVirtualToPhysical(colours), 2);

					sp120 *= 0.2f;
					sp120 *= viGetFovY() / 60.0f;
#ifndef PLATFORM_N64 // adjust scale for port
					sp120 *= (float)(SCREEN_WIDTH_LO * SCREEN_HEIGHT_LO) / (float)(SCREEN_WIDTH_HI * SCREEN_HEIGHT_HI);
#endif

					mtx4LoadIdentity(&spd4);

					spd4.m[0][0] = 0.05f;
					spd4.m[1][1] = 0.05f;
					spd4.m[2][2] = 0.05f;
					spd4.m[3][3] = 0.05f;

					mtx4SetTranslation(&group->pos, &spd4);
					mtx00015be0(camGetWorldToScreenMtxf(), &spd4);

					mtx = gfxAllocateMatrix();
					mtxF2L(&spd4, mtx);

					gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

					index = group->startindex;

					for (j = 0; j < group->numsparks; j++) {
						struct spark *spark = &g_Sparks[index];

						if (spark->ttl != 0) {
							Vtx *vertices = gfxAllocateVertices(3);
							f32 f2;

							for (k = 0; k < 3; k++) {
								vertices[k].s = 0;
								vertices[k].t = 0;
							}

							f2 = sqrtf(spark->speed.f[0] * spark->speed.f[0] + spark->speed.f[1] * spark->speed.f[1] + spark->speed.f[2] * spark->speed.f[2]);
							f2 = (sp120 + (type->unk04 + group->age * type->unk08)) / f2;

							vertices[0].x = spark->pos.x;
							vertices[0].y = spark->pos.y;
							vertices[0].z = spark->pos.z;
							vertices[0].colour = 4;

							vertices[1].x = spark->pos.x + spark->speed.f[0] * f2;
							vertices[1].y = spark->pos.y + spark->speed.f[1] * f2;
							vertices[1].z = spark->pos.z + spark->speed.f[2] * f2;
							vertices[1].colour = 0;

							vertices[2].x = spark->pos.x + spark->speed.f[0] * f2;
							vertices[2].y = spark->pos.y + spark->speed.f[1] * f2;
							vertices[2].z = spark->pos.z + spark->speed.f[2] * f2;
							vertices[2].colour = 0;

							vertices[0].s = 128;
							vertices[0].t = -256;
							vertices[1].s = -32;
							vertices[1].t = 496;
							vertices[2].s = 288;
							vertices[2].t = 496;

							switch (axis) {
							case 0:
								if (ABS(spark->speed.z) > ABS(spark->speed.y)) {
									vertices[1].y = vertices[1].y - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].y = vertices[2].y + type->unk06 + group->age * type->unk0a + (s32)sp120;
								} else {
									vertices[1].z = vertices[1].z - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].z = vertices[2].z + type->unk06 + group->age * type->unk0a + (s32)sp120;
								}
								break;
							case 1:
								if (ABS(spark->speed.x) > ABS(spark->speed.z)) {
									vertices[1].z = vertices[1].z - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].z = vertices[2].z + type->unk06 + group->age * type->unk0a + (s32)sp120;
								} else {
									vertices[1].x = vertices[1].x - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].x = vertices[2].x + type->unk06 + group->age * type->unk0a + (s32)sp120;
								}
								break;
							case 2:
								if (ABS(spark->speed.x) > ABS(spark->speed.y)) {
									vertices[1].y = vertices[1].y - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].y = vertices[2].y + type->unk06 + group->age * type->unk0a + (s32)sp120;
								} else {
									vertices[1].x = vertices[1].x - type->unk06 - group->age * type->unk0a - (s32)sp120;
									vertices[2].x = vertices[2].x + type->unk06 + group->age * type->unk0a + (s32)sp120;
								}
								break;
							}

							gSPVertex(gdl++, osVirtualToPhysical(vertices), 3, 0);
							gSP1Triangle(gdl++, 0, 1, 2, 0);
						}

						index = (index + 1) % ARRAYCOUNT(g_Sparks);
					}
				}
			}
		}
	}

	return gdl;
}
