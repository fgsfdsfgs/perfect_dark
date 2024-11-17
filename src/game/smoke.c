#include <ultra64.h>
#include "constants.h"
#include "game/dlights.h"
#include "game/gfxmemory.h"
#include "game/prop.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/smoke.h"
#include "game/bg.h"
#include "game/room.h"
#include "game/file.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

struct smoke *g_Smokes;
s32 g_MaxSmokes;
Mtx var800a3448;
Mtx var800a3488;

struct smoketype g_SmokeTypes[] = {
	//       duration
	//       |    fadespeed
	//       |    |   spreadspeed
	//       |    |   |    size
	//       |    |   |    |    bgrotatespeed
	//       |    |   |    |    |                  red
	//       |    |   |    |    |                  |     green
	//       |    |   |    |    |                  |     |     blue
	//       |    |   |    |    |                  |     |     |     fgrotatespeed
	//       |    |   |    |    |                  |     |     |     |                numclouds
	//       |    |   |    |    |                  |     |     |     |                |
#if PAL
	/*00*/ { 1,   50, 82,  0,   0,                 0x80, 0x80, 0x80, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_NONE
	/*01*/ { 183, 50, 37,  60,  0.024,             0x50, 0x50, 0x60, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_ELECTRICAL
	/*02*/ { 183, 50, 41,  20,  0.012,             0x80, 0x80, 0x80, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_MINI
	/*03*/ { 233, 50, 100, 100, 0.012,             0xc0, 0xc0, 0xc0, 0.36,            144, 0.18,             0.36,              1 },
	/*04*/ { 233, 50, 50,  80,  0.024,             0x40, 0x40, 0x40, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_SMALL
	/*05*/ { 283, 50, 41,  190, 0.018000001087785, 0x40, 0x40, 0x40, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_MEDIUM
	/*06*/ { 316, 50, 58,  300, 0.012,             0x40, 0x40, 0x40, 0.36,            144, 0.18,             0.36,              1 }, // SMOKETYPE_LARGE
	/*07*/ { 50,  50, 6,   15,  0.03600000217557,  0xff, 0xff, 0xff, 0.36,            144, 0.18,             0.36,              1 },
	/*08*/ { 16,  1,  5,   30,  0.03600000217557,  0xff, 0xff, 0xff, 2.4,             36,  0.18,             0.36,              1 }, // SMOKETYPE_ROCKETTAIL
	/*09*/ { 20,  1,  5,   16,  0.03600000217557,  0xe0, 0xe0, 0xe0, 3.6000001430511, 36,  0.18,             0.36,              1 }, // SMOKETYPE_GRENADETAIL
	/*10*/ { 750, 50, 58,  900, 0.012,             0x40, 0x40, 0x40, 0.36,            216, 0.18,             0.36,              1 },
	/*11*/ { 16,  1,  5,   30,  0.03600000217557,  0x18, 0x20, 0x40, 2.4,             36,  0.18,             0.36,              1 }, // SMOKETYPE_HOMINGTAIL
	/*12*/ { 41,  20, 5,   2,   0.03600000217557,  0xff, 0xff, 0xbf, 0.36,            180, 0.18,             0.36,              1 },
	/*13*/ { 10,  12, 5,   5,   0.03600000217557,  0x66, 0x40, 0x40, 1.2,             21,  0.18,             0.36,              1 }, // SMOKETYPE_SKCORPSE
	/*14*/ { 10,  12, 5,   5,   0.03600000217557,  0x66, 0x66, 0x00, 1.2,             21,  0.18,             0.36,              1 },
	/*15*/ { 41,  4,  4,   3,   0.03600000217557,  0xff, 0xff, 0xff, 0.36,            180, 0,                0.54,              0 }, // SMOKETYPE_MUZZLE_PISTOL
	/*16*/ { 41,  4,  5,   3,   0.03600000217557,  0xaf, 0xff, 0xaf, 0.36,            180, 0.108000010252,   0.36,              0 }, // SMOKETYPE_MUZZLE_REAPER
	/*17*/ { 41,  4,  3,   3,   0.03600000217557,  0xff, 0xff, 0xff, 0.36,            180, 0,                0.4200000166893,   0 }, // SMOKETYPE_MUZZLE_AUTOMATIC
	/*18*/ { 41,  4,  3,   3,   0.03600000217557,  0xaf, 0x8f, 0x6f, 0.36,            180, 0.12000000476837, 0.36,              0 }, // SMOKETYPE_MUZZLE_SHOTGUN
	/*19*/ { 41,  1,  2,   16,  0.03600000217557,  0xff, 0xff, 0x80, 3.6000001430511, 36,  0.18,             0.36,              1 }, // SMOKETYPE_PINBALL
	/*20*/ { 150, 8,  6,   18,  0.072000004351139, 0xff, 0xff, 0xff, 0.36,            0,   0.228,            0.084000006318092, 1 }, // SMOKETYPE_WATER
	/*21*/ { 183, 33, 37,  60,  0.024,             0x20, 0x20, 0x20, 0.36,            36,  1.8000000715256,  2.16,              6 }, // SMOKETYPE_DEBRIS
	/*22*/ { 183, 4,  6,   60,  0.03600000217557,  0xaf, 0x8f, 0x6f, 0.36,            36,  1.8000000715256,  0.36,              1 }, // SMOKETYPE_UFO
#else
	/*00*/ { 1,   60, 99,  0,   0,                 0x80, 0x80, 0x80, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_NONE
	/*01*/ { 220, 60, 45,  60,  0.02,              0x50, 0x50, 0x60, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_ELECTRICAL
	/*02*/ { 220, 60, 50,  20,  0.01,              0x80, 0x80, 0x80, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_MINI
	/*03*/ { 280, 60, 120, 100, 0.01,              0xc0, 0xc0, 0xc0, 0.3,             120, 0.15,             0.3,               1 },
	/*04*/ { 280, 60, 60,  80,  0.02,              0x40, 0x40, 0x40, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_SMALL
	/*05*/ { 340, 60, 50,  190, 0.015,             0x40, 0x40, 0x40, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_MEDIUM
	/*06*/ { 380, 60, 70,  300, 0.01,              0x40, 0x40, 0x40, 0.3,             120, 0.15,             0.3,               1 }, // SMOKETYPE_LARGE
	/*07*/ { 60,  60, 8,   15,  0.03,              0xff, 0xff, 0xff, 0.3,             120, 0.15,             0.3,               1 },
	/*08*/ { 20,  1,  6,   30,  0.03,              0xff, 0xff, 0xff, 2,               30,  0.15,             0.3,               1 }, // SMOKETYPE_ROCKETTAIL
	/*09*/ { 25,  1,  7,   16,  0.03,              0xe0, 0xe0, 0xe0, 3,               30,  0.15,             0.3,               1 }, // SMOKETYPE_GRENADETAIL
	/*10*/ { 900, 60, 70,  900, 0.01,              0x40, 0x40, 0x40, 0.3,             180, 0.15,             0.3,               1 },
	/*11*/ { 20,  1,  6,   30,  0.03,              0x18, 0x20, 0x40, 2,               30,  0.15,             0.3,               1 }, // SMOKETYPE_HOMINGTAIL
	/*12*/ { 50,  25, 7,   2,   0.03,              0xff, 0xff, 0xbf, 0.3,             150, 0.15,             0.3,               1 },
	/*13*/ { 12,  15, 7,   5,   0.03,              0x66, 0x40, 0x40, 1,               18,  0.15,             0.3,               1 }, // SMOKETYPE_SKCORPSE
	/*14*/ { 12,  15, 7,   5,   0.03,              0x66, 0x66, 0x00, 1,               18,  0.15,             0.3,               1 },
	/*15*/ { 50,  5,  5,   3,   0.03,              0xff, 0xff, 0xff, 0.3,             150, 0,                0.45,              0 }, // SMOKETYPE_MUZZLE_PISTOL
	/*16*/ { 50,  5,  6,   3,   0.03,              0xaf, 0xff, 0xaf, 0.3,             150, 0.09,             0.3,               0 }, // SMOKETYPE_MUZZLE_REAPER
	/*17*/ { 50,  5,  3,   3,   0.03,              0xff, 0xff, 0xff, 0.3,             150, 0,                0.35,              0 }, // SMOKETYPE_MUZZLE_AUTOMATIC
	/*18*/ { 50,  5,  3,   3,   0.03,              0xaf, 0x8f, 0x6f, 0.3,             150, 0.1,              0.3,               0 }, // SMOKETYPE_MUZZLE_SHOTGUN
	/*19*/ { 50,  1,  2,   16,  0.03,              0xff, 0xff, 0x80, 3,               30,  0.15,             0.3,               1 }, // SMOKETYPE_PINBALL
	/*20*/ { 180, 10, 8,   18,  0.06,              0xff, 0xff, 0xff, 0.3,             0,   0.19,             0.07,              1 }, // SMOKETYPE_WATER
	/*21*/ { 220, 40, 45,  60,  0.02,              0x20, 0x20, 0x20, 0.3,             30,  1.5,              1.8,               6 }, // SMOKETYPE_DEBRIS
	/*22*/ { 220, 5,  8,   60,  0.03,              0xaf, 0x8f, 0x6f, 0.3,             30,  1.5,              0.3,               1 }, // SMOKETYPE_UFO
#endif
};

Gfx *smokeRenderPart(struct smoke *smoke, struct smokepart *part, Gfx *gdl, struct coord *coord, f32 size)
{
	Vtx *vertices = gfxAllocateVertices(4);
	Col *colours = (Col *)gfxAllocateColours(1);
	Mtxf *mtx = camGetProjectionMtxF();
	struct coord spa0;
	struct coord sp94;
	struct coord sp88;
	struct coord sp7c;
	f32 sp78;
	f32 sp74;
	f32 sp70;
	f32 sp6c;
	f32 sp68;
	u8 alpha;
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	f32 sp5c;
	f32 sp58;
	f32 sp54;
	f32 distance;
	f32 range;
	f32 mult;
	f32 sp44;
	f32 sp40;
	f32 sp3c;
	f32 frac;
	u32 stack;

	if (g_SmokeTypes[smoke->type].fadespeed >= part->count) {
		alpha = part->alpha / (f32) g_SmokeTypes[smoke->type].fadespeed * part->count;
	} else {
		alpha = part->alpha;
	}

	sp78 = cosf(part->rot) * part->size;
	sp74 = sinf(part->rot) * part->size;

	sp70 = part->pos.x + 7.0f * sinf(part->offset1) * g_SmokeTypes[smoke->type].unk20;
	sp6c = part->pos.y;
	sp68 = part->pos.z + 7.0f * sinf(part->offset2) * g_SmokeTypes[smoke->type].unk20;

	sp5c = sp70 - campos->f[0];
	sp58 = sp6c - campos->f[1];
	sp54 = sp68 - campos->f[2];

	distance = sqrtf(sp5c * sp5c + sp58 * sp58 + sp54 * sp54);

	if (distance > 30000.0f) {
		return gdl;
	}

	range = distance * 0.5f;

	if (range > 100.0f) {
		range = 100.0f;
	}

	if (distance == 0.0f) {
		mult = 0.0f;
	} else {
		mult = (distance - range) / distance;
	}

	sp78 = sp78 * mult;
	sp74 = sp74 * mult;

	sp70 = campos->f[0] + sp5c * mult;
	sp6c = campos->f[1] + sp58 * mult;
	sp68 = campos->f[2] + sp54 * mult;

	spa0.f[0] = mtx->m[0][0] * sp78;
	spa0.f[1] = mtx->m[0][1] * sp78;
	spa0.f[2] = mtx->m[0][2] * sp78;
	sp94.f[0] = mtx->m[0][0] * sp74;
	sp94.f[1] = mtx->m[0][1] * sp74;
	sp94.f[2] = mtx->m[0][2] * sp74;
	sp88.f[0] = mtx->m[1][0] * sp78;
	sp88.f[1] = mtx->m[1][1] * sp78;
	sp88.f[2] = mtx->m[1][2] * sp78;
	sp7c.f[0] = mtx->m[1][0] * sp74;
	sp7c.f[1] = mtx->m[1][1] * sp74;
	sp7c.f[2] = mtx->m[1][2] * sp74;

	sp44 = (sp70 - spa0.f[0] - sp7c.f[0]) * size - coord->f[0];
	sp40 = (sp6c - spa0.f[1] - sp7c.f[1]) * size - coord->f[1];
	sp3c = (sp68 - spa0.f[2] - sp7c.f[2]) * size - coord->f[2];

	if (sp44 > 30000.0f || sp44 < -30000.0f
			|| sp40 > 30000.0f || sp40 < -30000.0f
			|| sp3c > 30000.0f || sp3c < -30000.0f) {
		return gdl;
	}

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		f32 alphamult = 0.5f;
		f32 value;

		distance = sqrtf((sp70 - g_Vars.currentplayer->eraserpos.f[0]) * (sp70 - g_Vars.currentplayer->eraserpos.f[0])
				+ (sp6c - g_Vars.currentplayer->eraserpos.f[1]) * (sp6c - g_Vars.currentplayer->eraserpos.f[1])
				+ (sp68 - g_Vars.currentplayer->eraserpos.f[2]) * (sp68 - g_Vars.currentplayer->eraserpos.f[2]));

		if (distance > g_Vars.currentplayer->eraserpropdist) {
			return gdl;
		}

		value = g_Vars.currentplayer->eraserpropdist - 150.0f;

		if (value < distance) {
			alphamult = (1.0f - (distance - value) / 150.0f) * 0.5f;
		}

		frac = distance / g_Vars.currentplayer->eraserpropdist;

		if (frac > 1.0f) {
			frac = 1.0f;
		}

		colours[0].r = frac * 255.0f;
		colours[0].g = (1.0f - frac) * 255.0f;
		colours[0].b = 0;
		colours[0].a = alpha * alphamult;
	} else {
		if (smoke->type != SMOKETYPE_PINBALL) {
			frac = roomGetFinalBrightnessForPlayer(smoke->prop->rooms[0]) * (1.0f / 255.0f);

			if (frac > 1) {
				frac = 1;
			}

			colours[0].r = (u32)(g_SmokeTypes[smoke->type].r * frac) & 0xff;
			colours[0].g = (u32)(g_SmokeTypes[smoke->type].g * frac) & 0xff;
			colours[0].b = (u32)(g_SmokeTypes[smoke->type].b * frac) & 0xff;
		} else {
			colours[0].r = g_SmokeTypes[smoke->type].r;
			colours[0].g = g_SmokeTypes[smoke->type].g;
			colours[0].b = g_SmokeTypes[smoke->type].b;
		}

		colours[0].a = alpha;
	}

	vertices[0].x = sp44;
	vertices[0].y = sp40;
	vertices[0].z = sp3c;
	vertices[0].s = 1760;
	vertices[0].t = 0;
	vertices[0].colour = 0;

	vertices[1].x = (sp70 + sp94.f[0] - sp88.f[0]) * size - coord->f[0];
	vertices[1].y = (sp6c + sp94.f[1] - sp88.f[1]) * size - coord->f[1];
	vertices[1].z = (sp68 + sp94.f[2] - sp88.f[2]) * size - coord->f[2];
	vertices[1].s = 0;
	vertices[1].t = 0;
	vertices[1].colour = 0;

	vertices[2].x = (sp70 + spa0.f[0] + sp7c.f[0]) * size - coord->f[0];
	vertices[2].y = (sp6c + spa0.f[1] + sp7c.f[1]) * size - coord->f[1];
	vertices[2].z = (sp68 + spa0.f[2] + sp7c.f[2]) * size - coord->f[2];
	vertices[2].s = 0;
	vertices[2].t = 1760;
	vertices[2].colour = 0;

	vertices[3].x = (sp70 - sp94.f[0] + sp88.f[0]) * size - coord->f[0];
	vertices[3].y = (sp6c - sp94.f[1] + sp88.f[1]) * size - coord->f[1];
	vertices[3].z = (sp68 - sp94.f[2] + sp88.f[2]) * size - coord->f[2];
	vertices[3].s = 1760;
	vertices[3].t = 1760;
	vertices[3].colour = 0;

	gSPColor(gdl++, osVirtualToPhysical(colours), 1);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

	gSPTri2(gdl++, 0, 1, 2, 0, 2, 3);

	return gdl;
}

struct smoke *smokeCreate(struct coord *pos, RoomNum *rooms, s16 type)
{
	struct smoke *smoke = NULL;
	s32 playercount = PLAYERCOUNT();
	s32 count = 0;
	s32 i;

	for (i = 0; i < g_MaxSmokes; i++) {
		if (g_Smokes[i].prop == NULL) {
			smoke = &g_Smokes[i];
			break;
		}

		if (playercount >= 2) {
			// Multiplayer - clear existing smoke if it's not any of the following
			if ((g_Smokes + i)->type != SMOKETYPE_BULLETIMPACT
					&& (g_Smokes + i)->type != SMOKETYPE_ROCKETTAIL
					&& (g_Smokes + i)->type != SMOKETYPE_HOMINGTAIL
					&& (g_Smokes + i)->type != SMOKETYPE_GRENADETAIL) {
				(g_Smokes + i)->age = g_SmokeTypes[(g_Smokes + i)->type].duration;
			}
		} else {
			// 1 player - if creating muzzle smoke, remove the third bullet impact smoke
			if (type >= SMOKETYPE_MUZZLE_PISTOL && type <= SMOKETYPE_MUZZLE_SHOTGUN) {
				struct smoke *smoke = g_Smokes + i;

				if (smoke->type == SMOKETYPE_BULLETIMPACT) {
					if (count == 3) {
						smoke->age = g_SmokeTypes[smoke->type].duration;
					}

					count++;
				}
			}
		}
	}

	if (smoke) {
		struct prop *prop = propAllocate();

		if (prop) {
			prop->type = PROPTYPE_SMOKE;
			prop->smoke = smoke;
			prop->pos.x = pos->x;
			prop->pos.y = pos->y;
			prop->pos.z = pos->z;

			for (i = 0; rooms[i] != -1 && i < ARRAYCOUNT(prop->rooms) - 1; i++) {
				prop->rooms[i] = rooms[i];
			}

			prop->rooms[i] = -1;
			propActivateThisFrame(prop);
			propEnable(prop);

			smoke->prop = prop;
			smoke->age = 0;
			smoke->type = type;
			smoke->source = NULL;
		}
	}

	return smoke;
}

bool smokeCreateForHand(struct coord *pos, RoomNum *rooms, s16 type, s32 handnum)
{
	struct smoke *smoke;
	s32 i;
	s32 j;

	for (i = 0; i < g_MaxSmokes; i++) {
		if (g_Smokes[i].prop
				&& g_Smokes[i].option == handnum
				&& g_Smokes[i].type >= SMOKETYPE_MUZZLE_PISTOL
				&& g_Smokes[i].type <= SMOKETYPE_MUZZLE_SHOTGUN) {
			bool fail = false;

			if (g_Smokes[i].age < g_SmokeTypes[g_Smokes[i].type].duration) {
				for (j = 0; j < ARRAYCOUNT(g_Smokes[i].parts); j++) {
					if (g_Smokes[i].parts[j].size == 0) {
						fail = true;
					}
				}

				if (fail) {
					return false;
				}
			}
		}
	}

	smoke = smokeCreate(pos, rooms, type);

	if (smoke) {
		smoke->option = handnum;
		return true;
	}

	return false;
}

/**
 * Creates smoke, unless there is already smoke for this prop and any of the
 * smoke parts have a size of zero. Perhaps the caller is supposed to check if
 * this function returns false and reuse the zero-sized smoke parts if so?
 */
bool smokeCreateWithSource(void *source, struct coord *pos, RoomNum *rooms, s16 type, bool srcispadeffect)
{
	struct smoke *smoke;
	s32 i;
	s32 j;
	bool checksmokes = true;

	if (type == SMOKETYPE_UFO) {
		checksmokes = false;
	}

	if (checksmokes) {
		for (i = 0; i < g_MaxSmokes; i++) {
			if (g_Smokes[i].prop && g_Smokes[i].source == source) {
				bool fail = false;

				if (g_Smokes[i].age < g_SmokeTypes[g_Smokes[i].type].duration) {
					for (j = 0; j < ARRAYCOUNT(g_Smokes[i].parts); j++) {
						if (g_Smokes[i].parts[j].size == 0) {
							fail = true;
						}
					}

					if (fail) {
						return false;
					}
				}
			}
		}
	}

	smoke = smokeCreate(pos, rooms, type);

	if (smoke) {
		smoke->source = source;
		smoke->option = srcispadeffect;
		return true;
	}

	return false;
}

void smokeCreateAtProp(struct prop *prop, s16 type)
{
	smokeCreateWithSource(prop, &prop->pos, prop->rooms, type, false);
}

void smokeCreateAtPadEffect(struct padeffectobj *effect, struct coord *pos, RoomNum *rooms, s16 type)
{
	smokeCreateWithSource(effect, pos, rooms, type, true);
}

void smokeClearForProp(struct prop *prop)
{
	s32 i;

	for (i = 0; i < g_MaxSmokes; i++) {
		if (g_Smokes[i].prop && g_Smokes[i].source == prop && g_Smokes[i].option == 0) {
			g_Smokes[i].age = g_SmokeTypes[g_Smokes[i].type].duration;
			g_Smokes[i].source = NULL;
		}
	}
}

struct smoke *smokeCreateSimple(struct coord *pos, RoomNum *rooms, s16 type)
{
	return smokeCreate(pos, rooms, type);
}

u32 smokeTick(struct prop *prop)
{
	s32 i;
	s32 j;
	s32 k;
	s32 lvupdate;
	struct smoke *smoke;
	struct smokepart *part;
	struct coord bbmin;
	struct coord bbmax;
	bool free;

	smoke = prop->smoke;

	if (g_Vars.lvupdate240 == 0) {
		return TICKOP_NONE;
	}

	// These tick values aren't adjusted for PAL,
	// so smoke will hang around for longer in PAL versions
	lvupdate = g_Vars.lvupdate60 < 15 ? (f32)g_Vars.lvupdate60 : 15.0f;

	for (i = 0; i < lvupdate; i++) {
		smoke->age++;

		part = smoke->parts;

		for (j = 0; j < ARRAYCOUNT(smoke->parts); j++) {
			if (part->size != 0.0f) {
				part->pos.y += g_SmokeTypes[smoke->type].unk1c;
				part->size += g_SmokeTypes[smoke->type].unk18;

				if (part->size < 0.0f) {
					part->size = 0.0f;
				}

				part->alpha -= g_SmokeTypes[smoke->type].fgrotatespeed;
				part->count++;
				part->rot += part->deltarot;
				part->offset1 += 0.02f + RANDOMFRAC() * 0.01f;
				part->offset2 += 0.02f + RANDOMFRAC() * 0.01f;

				if (part->alpha < 4.0f) {
					part->size = 0.0f;
				}
			}

			part++;
		}

		if (smoke->age < g_SmokeTypes[smoke->type].duration) {
			if (smoke->age % g_SmokeTypes[smoke->type].spreadspeed == 1) {
				part = smoke->parts;

				for (j = 0; j < ARRAYCOUNT(smoke->parts); j++) {
					if (smoke->parts[j].size == 0.0f) {
						if (g_SmokeTypes[smoke->type].size == 0) {
							part->size = (RANDOMFRAC() * 0.5f + 1.0f) * 0.33f;
						} else {
							part->size = g_SmokeTypes[smoke->type].size * (RANDOMFRAC() * 0.5f + 1.0f);
						}

						part->alpha = (rngRandom() % 70) + 110.0f;
						part->count = 0;
						part->rot = RANDOMFRAC() * M_BADTAU;
						part->deltarot = (0.5f - RANDOMFRAC()) * g_SmokeTypes[smoke->type].bgrotatespeed;

						if (smoke->type >= SMOKETYPE_MUZZLE_PISTOL && smoke->type <= SMOKETYPE_MUZZLE_SHOTGUN) {
							part->pos.x = g_Vars.currentplayer->hands[smoke->option].muzzlepos.x;
							part->pos.y = g_Vars.currentplayer->hands[smoke->option].muzzlepos.y;
							part->pos.z = g_Vars.currentplayer->hands[smoke->option].muzzlepos.z;
						} else if (smoke->sourceprop && smoke->option == 0) {
							part->pos.x = smoke->sourceprop->pos.x;
							part->pos.y = smoke->sourceprop->pos.y;
							part->pos.z = smoke->sourceprop->pos.z;
						} else {
							part->pos.x = prop->pos.x;
							part->pos.y = prop->pos.y;
							part->pos.z = prop->pos.z;
						}

						if (smoke->type == SMOKETYPE_WATER) {
							part->pos.x += RANDOMFRAC() * 70.0f - 35.0f;
							part->pos.y += RANDOMFRAC() * 40.0f - 25.0f;
							part->pos.z += RANDOMFRAC() * 40.0f - 20.0f;
							part->alpha *= 0.23f;
							part->size *= RANDOMFRAC() + 1.0f;
						}

						part->offset1 = RANDOMFRAC() * 0.5f;
						part->offset2 = RANDOMFRAC() * 0.5f;

						if (smoke->age > g_SmokeTypes[smoke->type].duration - g_SmokeTypes[smoke->type].numclouds) {
							part->alpha *= (g_SmokeTypes[smoke->type].duration - smoke->age) / (f32)g_SmokeTypes[smoke->type].numclouds;
						}
						break;
					}

					part++;
				}
			}
		}
	}

	bbmin.x = prop->pos.x - 1.0f;
	bbmin.y = prop->pos.y - 1.0f;
	bbmin.z = prop->pos.z - 1.0f;

	bbmax.x = prop->pos.x + 1.0f;
	bbmax.y = prop->pos.y + 1.0f;
	bbmax.z = prop->pos.z + 1.0f;

	for (j = 0; j < ARRAYCOUNT(smoke->parts); j++) {
		if (smoke->parts[j].size != 0.0f) {
			for (k = 0; k < 3; k++) {
				if (bbmin.f[k] > smoke->parts[j].pos.f[k] - smoke->parts[j].size) {
					bbmin.f[k] = smoke->parts[j].pos.f[k] - smoke->parts[j].size;
				}

				if (bbmax.f[k] < smoke->parts[j].size + smoke->parts[j].pos.f[k]) {
					bbmax.f[k] = smoke->parts[j].size + smoke->parts[j].pos.f[k];
				}
			}
		}
	}

	bgFindEnteredRooms(&bbmin, &bbmax, prop->rooms, 7, false);

	if (smoke->age > g_SmokeTypes[smoke->type].spreadspeed) {
		free = true;

		for (j = 0; j < ARRAYCOUNT(smoke->parts); j++) {
			if (smoke->parts[j].size > 0.0f) {
				free = false;
				break;
			}
		}
	} else {
		free = false;
	}

	if (free) {
		smoke->prop = NULL;
		return TICKOP_FREE;
	}

	return TICKOP_NONE;
}

u32 smokeTickPlayer(struct prop *prop)
{
	Mtxf *matrix = camGetWorldToScreenMtxf();

	prop->z = -(matrix->m[0][2] * prop->pos.x + matrix->m[1][2] * prop->pos.y + matrix->m[2][2] * prop->pos.z + matrix->m[3][2]);

	if (prop->z < 100) {
		prop->z *= 0.5f;
	} else {
		prop->z -= 100;
	}

	prop->flags |= PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONTHISSCREENTHISTICK;

	return TICKOP_NONE;
}

Gfx *smokeRender(struct prop *prop, Gfx *gdl, bool xlupass)
{
	struct smoke *smoke = prop->smoke;
	s32 roomnum;
	struct screenbox screenbox;
	struct coord sp8c;
	f32 sp88;
	s32 i;
	struct coord *coord;
	struct coord worldoffset;
	bool near = true;

	if (!xlupass) {
		return gdl;
	}

	i = 0;
	roomnum = prop->rooms[i];

	while (roomnum != -1) {
		if (g_Rooms[roomnum].gfxdata && g_Rooms[roomnum].loaded240 && (g_Rooms[roomnum].flags & ROOMFLAG_ONSCREEN)) {
			break;
		}

		i++;
		roomnum = prop->rooms[i];
	}

	if (roomnum != -1) {
		coord = roomGetPosPtr(roomnum);

		roomGetPos(roomnum, &worldoffset);

		if (smoke->parts[0].size > 0) {
			f32 x = smoke->parts[0].pos.x - worldoffset.x;
			f32 y = smoke->parts[0].pos.y - worldoffset.y;
			f32 z = smoke->parts[0].pos.z - worldoffset.z;

			if (x < -2000 || x > 2000 || y < -2000 || y > 2000 || z < -2000 || z > 2000) {
				near = false;
			}
		}

		if (func0f08e5a8(prop->rooms, &screenbox) > 0) {
			gdl = bgScissorWithinViewport(gdl, screenbox.xmin, screenbox.ymin, screenbox.xmax, screenbox.ymax);
		} else {
			gdl = bgScissorToViewport(gdl);
		}

		gSPClearGeometryMode(gdl++, G_CULL_BOTH | G_FOG);
		gSPMatrix(gdl++, osVirtualToPhysical(camGetOrthogonalMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		gdl = roomApplyMtx(gdl, roomnum);

		if (near) {
			gSPMatrix(gdl++, osVirtualToPhysical(&var800a3448), G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
		} else {
			gSPMatrix(gdl++, osVirtualToPhysical(&var800a3488), G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
		}

		gSPDisplayList(gdl++, g_TexGdl1);
		gDPSetColorDither(gdl++, G_CD_NOISE);

		if (near) {
			sp8c.x = coord->x * 10.0f;
			sp8c.y = coord->y * 10.0f;
			sp8c.z = coord->z * 10.0f;
			sp88 = 10;
		} else {
			sp8c.x = coord->x * 5;
			sp8c.y = coord->y * 5;
			sp8c.z = coord->z * 5;
			sp88 = 5;
		}

		for (i = 0; i < ARRAYCOUNT(smoke->parts); i++) {
			if (smoke->parts[i].size > 0) {
				gdl = smokeRenderPart(smoke, &smoke->parts[i], gdl, &sp8c, sp88);
			} else {
				smoke->parts[i].size = 0;
			}
		}

		gDPSetColorDither(gdl++, G_CD_BAYER);
		gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	}

	return gdl;
}

void smokeClearSomeTypes(void)
{
	s32 i;

	for (i = 0; i < g_MaxSmokes; i++) {
		if (g_Smokes[i].prop) {
			struct smoke *smoke = &g_Smokes[i];

			if (smoke->type != SMOKETYPE_BULLETIMPACT
					&& smoke->type != SMOKETYPE_ROCKETTAIL
					&& smoke->type != SMOKETYPE_HOMINGTAIL
					&& smoke->type != SMOKETYPE_GRENADETAIL) {
				smoke->age = g_SmokeTypes[smoke->type].duration;
			}
		}
	}
}
