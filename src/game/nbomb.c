#include <ultra64.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/chraction.h"
#include "game/dlights.h"
#include "game/game_006900.h"
#include "game/nbomb.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/prop.h"
#include "game/objectives.h"
#include "game/atan2f.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/savebuffer.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#include "platform.h"

s16 var8009cb00;
s32 var8009cb04;
struct nbomb g_Nbombs[6];
u32 var8009cbf8;

bool g_NbombsActive = false;
f32 var80061644 = 100;
u32 var80061648 = 0;

#define MAKEVERTEX(i, src) \
	vertices[i].x = src.x * var80061644; \
	vertices[i].y = src.y * var80061644; \
	vertices[i].z = src.z * var80061644; \
	vertices[i].s = src.y * 256.0f * 32.0f; \
	vertices[i].t = atan2f(src.x, src.z) / M_TAU * 256.0f * 32.0f; \
	vertices[i].colour = var80061648 * 0; \
\
	var80061648 = 1 - var80061648; \
\
	if (var8009cb04 && vertices[i].t == 0) { \
		vertices[i].t = 256 * 32; \
	} \
\
	vertices[i].t += var8009cb00;

Gfx *func0f006c80(Gfx *gdl, struct coord *arg1, struct coord *arg2, struct coord *arg3, u8 arg4, u8 arg5, u8 arg6, u8 arg7, s8 depth)
{
	struct coord sp7c;
	struct coord sp70;
	struct coord sp64;
	f32 dist;
	Vtx *vertices;

	sp7c.x = arg2->x + arg1->x;
	sp7c.y = arg2->y + arg1->y;
	sp7c.z = arg2->z + arg1->z;

	dist = sqrtf(sp7c.f[0] * sp7c.f[0] + sp7c.f[1] * sp7c.f[1] + sp7c.f[2] * sp7c.f[2]);

	sp7c.x /= dist;
	sp7c.y /= dist;
	sp7c.z /= dist;

	sp70.x = arg3->x + arg2->x;
	sp70.y = arg3->y + arg2->y;
	sp70.z = arg3->z + arg2->z;

	dist = sqrtf(sp70.f[0] * sp70.f[0] + sp70.f[1] * sp70.f[1] + sp70.f[2] * sp70.f[2]);

	sp70.x /= dist;
	sp70.y /= dist;
	sp70.z /= dist;

	sp64.x = arg1->x + arg3->x;
	sp64.y = arg1->y + arg3->y;
	sp64.z = arg1->z + arg3->z;

	dist = sqrtf(sp64.f[0] * sp64.f[0] + sp64.f[1] * sp64.f[1] + sp64.f[2] * sp64.f[2]);

	sp64.x /= dist;
	sp64.y /= dist;
	sp64.z /= dist;

	vertices = gfxAllocateVertices(3);

	MAKEVERTEX(0, sp7c);
	MAKEVERTEX(1, sp70);
	MAKEVERTEX(2, sp64);

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 3, arg7);

	if (depth == 0) {
		gSPTri4(gdl++,
				arg4, arg7, arg7 + 2,
				arg5, arg7 + 1, arg7,
				arg6, arg7 + 2, arg7 + 1,
				arg7, arg7 + 1, arg7 + 2);
	} else {
		gdl = func0f006c80(gdl, arg1, &sp7c, &sp64, arg4, arg7, arg7 + 2, arg7 + 3, depth - 1);
		gdl = func0f006c80(gdl, arg2, &sp70, &sp7c, arg5, arg7 + 1, arg7, arg7 + 3, depth - 1);
		gdl = func0f006c80(gdl, arg3, &sp64, &sp70, arg6, arg7 + 2, arg7 + 1, arg7 + 3, depth - 1);
		gdl = func0f006c80(gdl, &sp7c, &sp70, &sp64, arg7, arg7 + 1, arg7 + 2, arg7 + 3, depth - 1);
	}

	return gdl;
}

Gfx *func0f0073ac(Gfx *gdl, struct coord *pos, f32 arg2)
{
	Mtxf *modelmtx = gfxAllocateMatrix();
	Vtx *vertices;
	Col *colours;
	Mtxf sp104;
	struct coord spf8[1];
	struct coord spb0[] = {
		{ 0,  0,  1  },
		{ 1,  0,  0  },
		{ 0,  0,  -1 },
		{ -1, 0,  0  },
		{ 0,  1,  0  },
		{ 0,  -1, 0  },
	};

	var80061644 = arg2;

	spf8[0].x = 0.0f;
	spf8[0].y = 0.0f;
	spf8[0].z = -100.0f;

	mtx4LoadIdentity(&sp104);
	mtx4LoadTranslation(pos, &sp104);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp104);
	mtxF2L(&sp104, modelmtx);

	gSPMatrix(gdl++, osVirtualToPhysical(modelmtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	texSelect(&gdl, &g_TexGeneralConfigs[10], 2, 1, 2, true, NULL);

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	texSelect(&gdl, NULL, 2, 1, 2, true, NULL);

	gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);

	var8009cb00 = 2048.0f * g_20SecIntervalFrac;
	var8009cb04 = 0;

	vertices = gfxAllocateVertices(6);

	MAKEVERTEX(0, spb0[0]);
	MAKEVERTEX(1, spb0[1]);
	MAKEVERTEX(2, spb0[2]);
	MAKEVERTEX(3, spb0[3]);
	MAKEVERTEX(4, spb0[4]);
	MAKEVERTEX(5, spb0[5]);

	colours = gfxAllocateColours(1);

	colours[0].word = PD_BE32(0xff00007f);

	gSPColor(gdl++, osVirtualToPhysical(colours), 1);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 6, 0);

	gdl = func0f006c80(gdl, &spb0[0], &spb0[4], &spb0[1], 0, 4, 1, 6, 2);
	gdl = func0f006c80(gdl, &spb0[1], &spb0[4], &spb0[2], 1, 4, 2, 6, 2);
	gdl = func0f006c80(gdl, &spb0[1], &spb0[5], &spb0[0], 1, 5, 0, 6, 2);
	gdl = func0f006c80(gdl, &spb0[2], &spb0[5], &spb0[1], 2, 5, 1, 6, 2);

	var8009cb04 = 1;

	vertices = gfxAllocateVertices(6);

	MAKEVERTEX(0, spb0[0]);
	MAKEVERTEX(1, spb0[1]);
	MAKEVERTEX(2, spb0[2]);
	MAKEVERTEX(3, spb0[3]);
	MAKEVERTEX(4, spb0[4]);
	MAKEVERTEX(5, spb0[5]);

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 6, 0);

	gdl = func0f006c80(gdl, &spb0[2], &spb0[4], &spb0[3], 2, 4, 3, 6, 2);
	gdl = func0f006c80(gdl, &spb0[3], &spb0[4], &spb0[0], 3, 4, 0, 6, 2);
	gdl = func0f006c80(gdl, &spb0[3], &spb0[5], &spb0[2], 3, 5, 2, 6, 2);
	gdl = func0f006c80(gdl, &spb0[0], &spb0[5], &spb0[3], 0, 5, 3, 6, 2);

	return gdl;
}

Gfx *func0f008558(Gfx *gdl, s32 depth)
{
	Vtx *vertices;
	struct coord sp5c[] = {
		{ 0,  0,  1  },
		{ 1,  0,  0  },
		{ 0,  0,  -1 },
		{ -1, 0,  0  },
		{ 0,  1,  0  },
		{ 0,  -1, 0  },
	};

	var80061648 = 0;

	var8009cb04 = 0;

	vertices = gfxAllocateVertices(6);

	MAKEVERTEX(0, sp5c[0]);
	MAKEVERTEX(1, sp5c[1]);
	MAKEVERTEX(2, sp5c[2]);
	MAKEVERTEX(3, sp5c[3]);
	MAKEVERTEX(4, sp5c[4]);
	MAKEVERTEX(5, sp5c[5]);

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 6, 0);

	gdl = func0f006c80(gdl, &sp5c[0], &sp5c[4], &sp5c[1], 0, 4, 1, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[1], &sp5c[4], &sp5c[2], 1, 4, 2, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[1], &sp5c[5], &sp5c[0], 1, 5, 0, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[2], &sp5c[5], &sp5c[1], 2, 5, 1, 6, depth);

	var8009cb04 = 1;

	vertices = gfxAllocateVertices(6);

	MAKEVERTEX(0, sp5c[0]);
	MAKEVERTEX(1, sp5c[1]);
	MAKEVERTEX(2, sp5c[2]);
	MAKEVERTEX(3, sp5c[3]);
	MAKEVERTEX(4, sp5c[4]);
	MAKEVERTEX(5, sp5c[5]);

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 6, 0);

	gdl = func0f006c80(gdl, &sp5c[2], &sp5c[4], &sp5c[3], 2, 4, 3, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[3], &sp5c[4], &sp5c[0], 3, 4, 0, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[3], &sp5c[5], &sp5c[2], 3, 5, 2, 6, depth);
	gdl = func0f006c80(gdl, &sp5c[0], &sp5c[5], &sp5c[3], 0, 5, 3, 6, depth);

	return gdl;
}

void nbomb0f0093c4(f32 *arg0)
{
	arg0[0] = 0.0f;
	arg0[1] = RANDOMFRAC() - 0.5f;
	arg0[2] = RANDOMFRAC() - 0.5f;
	arg0[3] = RANDOMFRAC() - 0.5f;

	guNormalize(&arg0[1], &arg0[2], &arg0[3]);
}

void nbombReset(struct nbomb *nbomb)
{
	nbomb->age240 = 0;
#if VERSION >= VERSION_PAL_BETA
	nbomb->radius = 0;
#endif
#ifndef PLATFORM_N64
	nbomb->spawnframe240 = g_Vars.lvframe240;
#endif
}

/**
 * If nbomb->age240 is 0 to 310, return 127
 * If nbomb->age240 is 311 to 349, return a scaled number between 127 and 0
 * If nbomb->age240 is 350+, return 0
 */
s32 nbombCalculateAlpha(struct nbomb *nbomb)
{
	s32 alpha = 127;

	if (nbomb->age240 > TICKS(310)) {
		if (nbomb->age240 < TICKS(350)) {
			alpha = (TICKS(350) * 127 - nbomb->age240 * 127) / TICKS(40);
		} else {
			alpha = 0;
		}
	}

	return alpha;
}

/**
 * Allocate and populate a graphics display list for an nbomb dome.
 *
 * The gdl sets up a single vertex but has no colour or triangles.
 */
Gfx *nbombCreateGdl(void)
{
	Vtx *vertices;
#ifdef PLATFORM_64BIT
	u32 gdlsizes[] = { 0x0a30*2, 0x0330*2 }; // 1 player, 2+ players
#else
	u32 gdlsizes[] = { 0x0a30, 0x0330 }; // 1 player, 2+ players
#endif
	Gfx *gdlstart;
	Gfx *gdl;
	s32 index = 0;

	if (PLAYERCOUNT() >= 2) {
		index = 1;
	}

	var8009cb00 = (s32)(g_20SecIntervalFrac * 64.0f * 32.0f * 16.0f) % 0x800;

	gdl = gdlstart = gfxAllocate(gdlsizes[index]);

	texSelect(&gdl, &g_TexGeneralConfigs[10], 2, 1, 2, 1, NULL);

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	vertices = gfxAllocateVertices(1);

	vertices[0].z = 0;
	vertices[0].t = 0;
	vertices[0].colour = 0;
	vertices[0].y = vertices[0].z;
	vertices[0].x = vertices[0].z;
	vertices[0].s = vertices[0].t;

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 1, 0);

	if (index != 0) {
		u32 stack;
		gdl = func0f008558(gdl, 1);
	} else {
		gdl = func0f008558(gdl, 2);
	}

	gSPEndDisplayList(gdl++);

	return gdlstart;
}

struct sndstate *g_NbombAudioHandle = NULL;

Gfx *nbombRender(Gfx *gdl, struct nbomb *nbomb, Gfx *subgdl)
{
	f32 divider = 2048;
	Mtxf *mtx;
	Mtxf spc8;
	Mtxf sp88;
	Mtxf sp48;
	struct coord sp3c;
	u32 colour;
	Col *colours;

	mtx = gfxAllocateMatrix();
	var80061644 = 2000.0f;
	colour = nbombCalculateAlpha(nbomb);

	colours = gfxAllocateColours(2);
	colours[0].word = PD_BE32(colour);
	colours[1].word = PD_BE32(0xffffff00);

	gSPColor(gdl++, osVirtualToPhysical(colours), 2);

	sp3c.x = 0;
	sp3c.y = 0;
	sp3c.z = -100;

	mtx4LoadIdentity(&sp48);
	mtx4LoadTranslation(&nbomb->pos, &sp48);

	sp3c.x = 0;
	sp3c.y = nbomb->unk14 / divider * M_TAU;
	sp3c.z = 0;

	mtx4LoadRotation(&sp3c, &sp88);
	mtx00015f04(nbomb->radius / 2000.0f, &sp88);
	mtx4MultMtx4(&sp48, &sp88, &spc8);

	mtx00015be0(camGetWorldToScreenMtxf(), &spc8);
	mtxF2L(&spc8, mtx);

	gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	gSPDisplayList(gdl++, subgdl);

	return gdl;
}

void func0f0099a4(void)
{
	s32 i;

	g_NbombsActive = false;
	g_NbombAudioHandle = NULL;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		g_Nbombs[i].age240 = -1;

#if VERSION >= VERSION_NTSC_1_0
		g_Nbombs[i].audiohandle20 = NULL;
		g_Nbombs[i].audiohandle24 = NULL;
#endif
	}
}

void nbombInflictDamage(struct nbomb *nbomb)
{
	s32 index = 0;
	u32 stack;
	s16 propnums[256];
	struct coord bbmin;
	struct coord bbmax;
	RoomNum roomnums[54];
	s16 *propnumptr;
	s32 i;
	struct gset gset;

	gset.weaponnum = WEAPON_NBOMB;
	gset.weaponfunc = FUNC_PRIMARY;

	if (g_Vars.lvupdate240 <= 0 || nbomb->age240 > TICKS(350)) {
		return;
	}

	// Find rooms which intersect the nbomb dome's bbox
	bbmin.x = nbomb->pos.f[0] - nbomb->radius;
	bbmin.y = nbomb->pos.f[1] - nbomb->radius;
	bbmin.z = nbomb->pos.f[2] - nbomb->radius;

	bbmax.x = nbomb->pos.f[0] + nbomb->radius;
	bbmax.y = nbomb->pos.f[1] + nbomb->radius;
	bbmax.z = nbomb->pos.f[2] + nbomb->radius;

	for (i = 1; i < g_Vars.roomcount; i++) {
		if (!(bbmax.f[0] < g_Rooms[i].bbmin[0]
				|| bbmin.f[0] > g_Rooms[i].bbmax[0]
				|| bbmax.f[1] < g_Rooms[i].bbmin[1]
				|| bbmin.f[1] > g_Rooms[i].bbmax[1]
				|| bbmax.f[2] < g_Rooms[i].bbmin[2]
				|| bbmin.f[2] > g_Rooms[i].bbmax[2])) {
			if (index < 52) {
				roomnums[index] = i;
				index++;
				roomFlashLighting(i, -38, -180);
			}
		}
	}

	roomnums[index] = -1;

	if (1);

	// Iterate props in the affected rooms and damage any chrs
	roomGetProps(roomnums, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (prop->timetoregen == 0) {
			if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
				f32 xdiff = prop->pos.f[0] - nbomb->pos.f[0];
				f32 ydiff = prop->pos.f[1] - nbomb->pos.f[1];
				f32 zdiff = prop->pos.f[2] - nbomb->pos.f[2];

				f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

				if (dist < nbomb->radius) {
					struct chrdata *chr = prop->chr;

#if VERSION >= VERSION_NTSC_1_0
					if (chr)
#endif
					{
						struct coord vector = {0, 0, 0};
						f32 damage = 0.01f * g_Vars.lvupdate60freal;

						chrDamageByMisc(chr, damage, &vector, &gset, nbomb->ownerprop);

#if VERSION >= VERSION_NTSC_1_0
						if (chr->actiontype);
#endif

						chr->chrflags |= CHRCFLAG_TRIGGERSHOTLIST;

						if (chr->hidden & CHRHFLAG_CLOAKED) {
							chrUncloak(chr, true);
						}
					}

					if (1);
				}
			}
		}

		propnumptr++;
	}
}

void nbombTick(struct nbomb *nbomb)
{
	if (nbomb->age240 >= 0) {
		s32 age60;
#ifdef PLATFORM_N64
		s32 increment = (g_Vars.lvupdate240 + 2) >> 2;
		nbomb->age240 += increment;
#else
		s32 oldage240 = nbomb->age240;
		nbomb->age240 = (g_Vars.lvframe240 - nbomb->spawnframe240) >> 2;
		s32 increment = nbomb->age240 - oldage240;
#endif

		if (nbomb->age240 < TICKS(80)) {
			nbomb->radius = nbomb->age240 / (PAL ? 66.0f : 80.0f);
			nbomb->radius = sqrtf(sqrtf(nbomb->radius));
			nbomb->unk18 = 0;
		} else {
			nbomb->radius = sinf((nbomb->age240 - TICKS(80)) * (PAL ? 0.062800005078316f : 0.0523333363235f)) * 0.05f + 1.0f;

			// Return value is not used - could have been printed
			sinf((nbomb->age240 - TICKS(80)) * (PAL ? 0.062800005078316f : 0.0523333363235f));

			nbomb->unk18 = ((nbomb->age240 - TICKS(80)) / (PAL ? 225.0f : 270.0f)) * 3.0f;
		}

		nbomb->radius *= 500.0f;

		nbombInflictDamage(nbomb);

		age60 = nbomb->age240 / 4;

		if (age60 > 40) {
			age60 = 40;
		}

#if PAL
		nbomb->unk14 += increment * age60 * 60 / 50;
#else
		nbomb->unk14 += increment * age60;
#endif

		nbomb->unk14 %= 0x800;

		if (nbomb->age240 > (PAL ? 307 : 370)) {
			nbomb->age240 = -1;
		}
	}
}

void nbombsTick(void)
{
	s32 i;
	s32 youngest240 = 20000;
	s32 volume;

	if (g_Vars.lvupdate240 != 0) {
		g_NbombsActive = false;
	}

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Vars.lvupdate240 != 0 && g_Nbombs[i].age240 >= 0) {
			nbombTick(&g_Nbombs[i]);

			if (g_Nbombs[i].age240 < youngest240) {
				youngest240 = g_Nbombs[i].age240;
			}

			g_NbombsActive = true;
		}
	}

	volume = 0;

	if (youngest240 < TICKS(350)) {
		if (g_Vars.lvupdate240 != 0) {
			if (g_NbombAudioHandle == 0) {
				sndStart(var80095200, SFX_SHIP_HUM, &g_NbombAudioHandle, -1, -1, -1, -1, -1);
			}

			volume = AL_VOL_FULL;

			if (g_NbombAudioHandle) {
				f32 speed = menuGetSinOscFrac(20) * 0.02f + 0.4f;

				if (youngest240 > TICKS(300)) {
					volume = (1.0f - (f32)(youngest240 - TICKS(300)) / (PAL ? 41.0f : 50.0f)) * AL_VOL_FULL;
				}

				if (youngest240 >= TICKS(350)) {
					volume = 0;
				}

				audioPostEvent(g_NbombAudioHandle, AL_SNDP_VOL_EVT, volume);
				audioPostEvent(g_NbombAudioHandle, AL_SNDP_PITCH_EVT, *(s32 *)&speed);
			}
		} else {
			if (g_NbombAudioHandle && sndGetState(g_NbombAudioHandle) != AL_STOPPED) {
				audioStop(g_NbombAudioHandle);
			}
		}
	} else {
		if (g_NbombAudioHandle && sndGetState(g_NbombAudioHandle) != AL_STOPPED) {
			audioStop(g_NbombAudioHandle);
		}
	}

	if (g_Vars.lvupdate240 == 0) {
		for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
			if (g_Nbombs[i].age240 >= 0) {
				if (g_Nbombs[i].audiohandle20 && sndGetState(g_Nbombs[i].audiohandle20) != AL_STOPPED) {
					audioStop(g_Nbombs[i].audiohandle20);
#if VERSION < VERSION_NTSC_1_0
					g_Nbombs[i].audiohandle20 = NULL;
#endif
				}

				if (g_Nbombs[i].audiohandle24 && sndGetState(g_Nbombs[i].audiohandle24) != AL_STOPPED) {
					audioStop(g_Nbombs[i].audiohandle24);
#if VERSION < VERSION_NTSC_1_0
					g_Nbombs[i].audiohandle24 = NULL;
#endif
				}
			}
		}
	}
}

Gfx *nbombsRender(Gfx *gdl)
{
	s32 i;
	Gfx *subgdl = NULL;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 >= 0) {
			if (!subgdl) {
				subgdl = nbombCreateGdl();
			}

			gdl = nbombRender(gdl, &g_Nbombs[i], subgdl);
		}
	}

	return gdl;
}

void nbombCreateStorm(struct coord *pos, struct prop *ownerprop)
{
	u32 stack;
	s32 oldest240;
	s32 index;
	s32 i;

	oldest240 = -1;
	index = 0;

	g_NbombsActive = true;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 == -1
#if VERSION >= VERSION_NTSC_1_0
				&& g_Nbombs[i].audiohandle20 == NULL
				&& g_Nbombs[i].audiohandle24 == NULL
#endif
				) {
			index = i;
			break;
		}

		if (g_Nbombs[i].age240 > oldest240) {
			index = i;
			oldest240 = g_Nbombs[i].age240;
		}
	}

	nbombReset(&g_Nbombs[index]);

	g_Nbombs[index].pos.x = pos->x;
	g_Nbombs[index].pos.y = pos->y;
	g_Nbombs[index].pos.z = pos->z;
	g_Nbombs[index].age240 = 0;
	g_Nbombs[index].ownerprop = ownerprop;

#if VERSION >= VERSION_NTSC_1_0
	// Newer versions only play audio if the handles are null,
	// while ntsc-beta clears the handles then plays them unconditionally.
	if (g_Nbombs[index].audiohandle20 == NULL) {
		sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle20, -1, -1, -1, -1, -1);

		if (g_Nbombs[index].audiohandle20) {
			union audioparam param;
			param.f32 = 0.4f;
			audioPostEvent(g_Nbombs[index].audiohandle20, AL_SNDP_PITCH_EVT, param.s32);
		}
	}

	if (g_Nbombs[index].audiohandle24 == NULL) {
		sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle24, -1, -1, -1, -1, -1);

		if (g_Nbombs[index].audiohandle24) {
			union audioparam param;
			param.f32 = 0.4f;
			audioPostEvent(g_Nbombs[index].audiohandle24, AL_SNDP_PITCH_EVT, param.s32);
		}
	}
#else
	g_Nbombs[index].audiohandle20 = NULL;
	g_Nbombs[index].audiohandle24 = NULL;

	sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle20, -1, -1, -1, -1, -1);

	if (g_Nbombs[index].audiohandle20) {
		union audioparam param;
		param.f32 = 0.4f;
		audioPostEvent(g_Nbombs[index].audiohandle20, AL_SNDP_PITCH_EVT, param.s32);
	}

	sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle24, -1, -1, -1, -1, -1);

	if (g_Nbombs[index].audiohandle24) {
		union audioparam param;
		param.f32 = 0.4f;
		audioPostEvent(g_Nbombs[index].audiohandle24, AL_SNDP_PITCH_EVT, param.s32);
	}
#endif
}

bool doorIsOpenOrOpening(s32 tagnum)
{
	struct defaultobj *obj = objFindByTagId(tagnum);

	if (obj && obj->prop && obj->type == OBJTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *)obj;

		if (door->mode == DOORMODE_IDLE) {
			if (door->frac <= 0) {
				return false;
			}
			return true;
		} else if (door->mode == DOORMODE_OPENING) {
			return true;
		} else if (door->mode == DOORMODE_CLOSING) {
			return false;
		} else {
			return false;
		}
	}

	return false;
}

f32 gasGetDoorFrac(s32 tagnum)
{
	struct defaultobj *obj = objFindByTagId(tagnum);

	if (obj && obj->prop && obj->type == OBJTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *)obj;
		return door->frac;
	}

	return 0;
}

/**
 * Checks if the player is inside an nbomb storm, and if so renders the black
 * storm texture directly over the screen.
 */
Gfx *nbombRenderOverlay(Gfx *gdl)
{
	bool inside = false;
	struct coord campos;
	s32 finalalpha = 0;
	s32 i;
	u32 stack[2];
	s16 t;
	s16 s;
	u32 stack2[2];
	bool drawn = false;
	s32 stack3;
	Col *colours;
	Vtx *vertices;
	s16 viewleft;
	s16 viewtop;
	s16 viewright;
	s16 viewbottom;

	campos.x = g_Vars.currentplayer->cam_pos.x;
	campos.y = g_Vars.currentplayer->cam_pos.y;
	campos.z = g_Vars.currentplayer->cam_pos.z;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 >= 0 && g_Nbombs[i].age240 <= TICKS(350)) {
			f32 xdiff = campos.f[0] - g_Nbombs[i].pos.f[0];
			f32 ydiff = campos.f[1] - g_Nbombs[i].pos.f[1];
			f32 zdiff = campos.f[2] - g_Nbombs[i].pos.f[2];

			if (sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff) < g_Nbombs[i].radius) {
				u32 alpha = nbombCalculateAlpha(&g_Nbombs[i]);

				inside = true;

				if (alpha > finalalpha) {
					finalalpha = alpha;
				}
			}
		}
	}

	if (inside) {
		colours = gfxAllocateColours(1);
		vertices = gfxAllocateVertices(4);

		viewleft = viGetViewLeft() * 10;
		viewtop = viGetViewTop() * 10;
		viewright = (s16) (viGetViewLeft() + viGetViewWidth()) * 10;
		viewbottom = (s16) (viGetViewTop() + viGetViewHeight()) * 10;

		s = (s32) (8.0f * g_20SecIntervalFrac * 128.0f * 32.0f) % 2048;
		t = (s16) ((s32) (campos.f[1] * 8.0f) % 2048) + (s16) (2.0f * g_20SecIntervalFrac * 128.0f * 32.0f);

		drawn = true;

		if (1);
		if (1);

		gdl = func0f0d479c(gdl);

		if (1);

		texSelect(&gdl, &g_TexGeneralConfigs[10], 2, 1, 2, true, NULL);

		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
		gSPClearGeometryMode(gdl++, G_CULL_BOTH);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);
		gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
		gDPSetTexturePersp(gdl++, G_TP_PERSP);

		vertices[0].x = viewleft;
		vertices[0].y = viewtop;
		vertices[0].z = -10;

		vertices[1].x = viewright;
		vertices[1].y = viewtop;
		vertices[1].z = -10;

		vertices[2].x = viewright;
		vertices[2].y = viewbottom;
		vertices[2].z = -10;

		vertices[3].x = viewleft;
		vertices[3].y = viewbottom;
		vertices[3].z = -10;

		vertices[0].s = s;
		vertices[0].t = t;
		vertices[1].s = s + 160;
		vertices[1].t = t;
		vertices[2].s = s + 160;
		vertices[2].t = t + 960;
		vertices[3].s = s;
		vertices[3].t = t + 960;

		vertices[0].colour = 0;
		vertices[1].colour = 0;
		vertices[2].colour = 0;
		vertices[3].colour = 0;

		colours[0].word = PD_BE32(finalalpha);

		gSPColor(gdl++, osVirtualToPhysical(colours), 1);
		gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

		gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
	}

	if (drawn) {
		gdl = func0f0d49c8(gdl);
	}

	return gdl;
}

Gfx *gasRender(Gfx *gdl)
{
	bool show = false;
	f32 alphafrac = 1.0f;
	struct coord campos;
	s16 layer2t;
	u32 alpha;
	s32 i;
	bool drawn = false;

	const s32 gasrooms[] = {
		ROOM_LUE_0092,
		ROOM_LUE_0093,
		ROOM_LUE_0094,
		ROOM_LUE_0095,
		ROOM_LUE_0096,
		ROOM_LUE_0097,
		ROOM_LUE_0098,
		ROOM_LUE_0099,
		ROOM_LUE_009A,
		ROOM_LUE_0091,
		ROOM_LUE_008F,
		ROOM_LUE_0090,
	};

	if (g_Vars.stagenum == STAGE_ESCAPE) {
		f32 intensityfrac = 1.0f;

		campos.x = g_Vars.currentplayer->cam_pos.x;
		campos.y = g_Vars.currentplayer->cam_pos.y;
		campos.z = g_Vars.currentplayer->cam_pos.z;

		for (i = 0; i < ARRAYCOUNT(gasrooms); i++) {
			if (bgRoomContainsCoord(&campos, gasrooms[i])) {
				show = true;
			}
		}

		if (!show) {
			// Outside of the gas rooms list - check distance to abitrary point
			f32 distance = sqrtf(
					(campos.f[0] - -1473.0f) * (campos.f[0] - -1473.0f) +
					(campos.f[1] - -308.0f) * (campos.f[1] - -308.0f) +
					(campos.f[2] - -13660.0f) * (campos.f[2] - -13660.0f));

			if (distance < 1328.0f) {
				show = true;
				alphafrac = 1.0f - distance / 1328.0f;
				intensityfrac = gasGetDoorFrac(0x32);
			}
		} else {
			if (bgRoomContainsCoord(&campos, 0x91)) {
				// In the small room between the first two doors
				f32 frac1 = gasGetDoorFrac(0x30);
				f32 frac2 = gasGetDoorFrac(0x31);

				if (frac2 > frac1) {
					intensityfrac = frac2;
				} else {
					intensityfrac = frac1;
				}

				intensityfrac += 0.2f;
			}
		}

		alphafrac *= intensityfrac;

		if (show && g_Vars.tickmode == TICKMODE_CUTSCENE) {
			if (g_CutsceneCurAnimFrame60 < 2180) {
				show = false;
			} else if (g_CutsceneCurAnimFrame60 < 2600) {
				f32 tmp = (g_CutsceneCurAnimFrame60 - 2180) / 420.0f;
				alphafrac *= tmp;
			}
		}

		if (show) {
			Col *colours = gfxAllocateColours(1);
			Vtx *vertices = gfxAllocateVertices(8);
			s16 viewleft = viGetViewLeft() * 10;
			s16 viewtop = viGetViewTop() * 10;
			s16 viewright = (s16) (viGetViewLeft() + viGetViewWidth()) * 10;
			s16 viewbottom = (s16) (viGetViewTop() + viGetViewHeight()) * 10;
			f32 lookx = g_Vars.currentplayer->cam_look.x;
			f32 lookz = g_Vars.currentplayer->cam_look.z;
			f32 camposx = g_Vars.currentplayer->cam_pos.x;
			f32 camposz = g_Vars.currentplayer->cam_pos.z;
			f32 f2;
			f32 f16;
			s32 stack;
			f32 sp78;
			s16 layer2s;
			s16 layer1s;
			s16 layer1t;

			f2 = (camposx + camposz) / 3000.0f;
			f16 = (f2 - (s32) f2);

			sp78 = atan2f(-lookx, lookz) / M_BADTAU;

			layer2s = ((s32) (2.0f * ((menuGetSinOscFrac(4.0f) - 0.5f) / 6.0f + sp78 + f16 * 1.5f) * 128.0f * 32.0f) % 2048);
			layer1s = ((s32) (2.0f * ((menuGetCosOscFrac(4.0f) - 0.5f) / -9.0f + sp78 + f16) * 128.0f * 32.0f) % 2048);

			layer2t = (s16) ((s32) (campos.y * 8.0f) % 2048) + (s16) (2.0f * g_20SecIntervalFrac * 128.0f * 32.0f);
			layer1t = (s16) ((s32) (campos.y * 8.0f) % 2048) + (s16) (2.0f * g_20SecIntervalFrac * 64.0f * 32.0f);

			drawn = true;

			gdl = func0f0d479c(gdl);

			if (1);

			texSelect(&gdl, &g_TexGeneralConfigs[6], 4, 1, 2, true, NULL);

			gDPPipeSync(gdl++);
			gDPSetCycleType(gdl++, G_CYC_1CYCLE);
			gDPSetAlphaCompare(gdl++, G_AC_NONE);
			gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
			gSPClearGeometryMode(gdl++, G_CULL_BOTH);
			gDPSetColorDither(gdl++, G_CD_DISABLE);
			gDPSetTextureFilter(gdl++, G_TF_BILERP);
			gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
			gDPSetTexturePersp(gdl++, G_TP_PERSP);

			vertices[0].x = viewleft;
			vertices[0].y = viewtop;
			vertices[0].z = -10;

			vertices[1].x = viewright;
			vertices[1].y = viewtop;
			vertices[1].z = -10;

			vertices[2].x = viewright;
			vertices[2].y = viewbottom;
			vertices[2].z = -10;

			vertices[3].x = viewleft;
			vertices[3].y = viewbottom;
			vertices[3].z = -10;

			vertices[4].x = viewleft;
			vertices[4].y = viewtop;
			vertices[4].z = -10;

			vertices[5].x = viewright;
			vertices[5].y = viewtop;
			vertices[5].z = -10;

			vertices[6].x = viewright;
			vertices[6].y = viewbottom;
			vertices[6].z = -10;

			vertices[7].x = viewleft;
			vertices[7].y = viewbottom;
			vertices[7].z = -10;

			vertices[0].s = layer1s;
			vertices[0].t = layer1t;
			vertices[1].s = layer1s + 960;
			vertices[1].t = layer1t;
			vertices[2].s = layer1s + 960;
			vertices[2].t = layer1t + 640;
			vertices[3].s = layer1s;
			vertices[3].t = layer1t + 640;

			vertices[0].colour = 0;
			vertices[1].colour = 0;
			vertices[2].colour = 0;
			vertices[3].colour = 0;

			vertices[4].s = layer2s;
			vertices[4].t = layer2t;
			vertices[5].s = layer2s + 640;
			vertices[5].t = layer2t;
			vertices[6].s = layer2s + 640;
			vertices[6].t = layer2t + 480;
			vertices[7].s = layer2s;
			vertices[7].t = layer2t + 480;

			vertices[4].colour = 0;
			vertices[5].colour = 0;
			vertices[6].colour = 0;
			vertices[7].colour = 0;

			alpha = 127.0f * alphafrac;

			colours[0].word = PD_BE32(0x3faf1100 | alpha);

			gSPColor(gdl++, osVirtualToPhysical(colours), 1);
			gSPVertex(gdl++, osVirtualToPhysical(vertices), 8, 0);

			gSPTri4(gdl++, 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4);
		}
	}

	if (drawn) {
		gdl = func0f0d49c8(gdl);
	}

	return gdl;
}
