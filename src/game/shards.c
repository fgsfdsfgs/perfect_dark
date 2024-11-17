#include <ultra64.h>
#include "constants.h"
#include "data.h"
#include "game/atan2f.h"
#include "game/dlights.h"
#include "game/file.h"
#include "game/propsnd.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/gfxmemory.h"
#include "bss.h"
#include "lib/mtx.h"
#include "lib/rng.h"
#include "types.h"

s32 g_MaxShards;
struct shard *g_Shards;

s32 g_NextShardNum = 0;
bool g_ShardsActive = false;

void shardCreate(RoomNum room, struct coord *pos, f32 rotx, f32 size, s32 type);

void shardsCreate(struct coord *pos, f32 *rotx, f32 *roty, f32 *rotz, f32 relxmin, f32 relxmax, f32 relymin, f32 relymax, s32 type, struct prop *prop)
{
	s32 y;
	s32 x;
	s32 speci;
	s32 speci2;
	struct coord thispos;
	f32 f0;
	f32 f20;
	f32 f30;
	f32 spec;
	s32 ymax;
	s32 xmax;
	struct coord basepos;
	f32 spcc[3];
	f32 spc0[3];
	f32 spac;

	spcc[0] = rotx[0];
	spcc[1] = rotx[1];
	spcc[2] = rotx[2];

	f0 = sqrtf(spcc[0] * spcc[0] + spcc[1] * spcc[1] + spcc[2] * spcc[2]);

	spcc[0] *= 1.0f / f0;
	spcc[1] *= 1.0f / f0;
	spcc[2] *= 1.0f / f0;

	relxmin *= f0;
	relxmax *= f0;

	spc0[0] = roty[0];
	spc0[1] = roty[1];
	spc0[2] = roty[2];

	f0 = sqrtf(spc0[0] * spc0[0] + spc0[1] * spc0[1] + spc0[2] * spc0[2]);

	spc0[0] *= 1.0f / f0;
	spc0[1] *= 1.0f / f0;
	spc0[2] *= 1.0f / f0;

	relymin *= f0;
	relymax *= f0;

	f30 = atan2f(rotz[0], rotz[2]);

	f20 = relxmax - relxmin;
	spac = relymax - relymin;

	spec = sqrtf(f20 * spac / (f32) (g_MaxShards / 2));
	speci = spec;
	speci2 = speci;

	basepos.x = (pos->f[0] + (relxmin + (speci >> 1)) * spcc[0]) + spc0[0] * (relymin + (speci >> 1));
	basepos.y = (pos->f[1] + (relxmin + (speci >> 1)) * spcc[1]) + spc0[1] * (relymin + (speci >> 1));
	basepos.z = (pos->f[2] + (relxmin + (speci >> 1)) * spcc[2]) + spc0[2] * (relymin + (speci >> 1));

	if (type == SHARDTYPE_GLASS) {
		psCreate(NULL, NULL, SFX_GLASS_SHATTER, -1,
				-1, 0, 0, PSTYPE_NONE, &prop->pos, -1.0f, prop->rooms, -1, -1.0f, -1.0f, -1.0f);
	} else if (type == SHARDTYPE_BOTTLE) {
		/**
		 * @bug: This attempts to play one of two bottle breaking sounds
		 * randomly, but one of the SFX IDs isn't valid. This is why on Villa
		 * when you shoot the bottles, half of them don't play any breaking
		 * sound effect.
		 */
		psCreate(NULL, NULL, SFX_BOTTLE_BREAK + (rngRandom() % 2) * 2, -1,
				-1, 0, 0, PSTYPE_NONE, &prop->pos, -1.0f, prop->rooms, -1, -1.0f, -1.0f, -1.0f);
	} else if (type == SHARDTYPE_WOOD) {
		psCreate(NULL, NULL, SFX_WOOD_BREAK, -1,
				-1, 0, 0, PSTYPE_NONE, &prop->pos, -1.0f, prop->rooms, -1, -1.0f, -1.0f, -1.0f);
	}

	xmax = f20 / speci;
	ymax = spac / speci2;

	for (y = 0; y < ymax; y++) {
		f32 f20 = y * (f32) speci2;

		for (x = 0; x < xmax; x++) {
			thispos.x = basepos.f[0] + x * (f32) speci * spcc[0] + spc0[0] * f20;
			thispos.y = basepos.f[1] + x * (f32) speci * spcc[1] + spc0[1] * f20;
			thispos.z = basepos.f[2] + x * (f32) speci * spcc[2] + spc0[2] * f20;

			shardCreate(prop->rooms[0], &thispos, f30, (RANDOMFRAC() * 0.7f + 0.1f) * spec, type);
		}
	}
}

void shardCreate(RoomNum room, struct coord *pos, f32 rotx, f32 size, s32 type)
{
	f32 velx = RANDOMFRAC() * 2.0f - 1.0f;
	f32 vely = RANDOMFRAC() * 1.12f - 0.12f;
	f32 velz = RANDOMFRAC() * 2.0f - 1.0f;
	s32 i;

	g_Shards[g_NextShardNum].type = type;
	g_Shards[g_NextShardNum].room = room;
	g_Shards[g_NextShardNum].age60 = 1;

	g_Shards[g_NextShardNum].pos.x = pos->x;
	g_Shards[g_NextShardNum].pos.y = pos->y;
	g_Shards[g_NextShardNum].pos.z = pos->z;

	g_Shards[g_NextShardNum].vel.x = velx * PALUPF(1.5f);
	g_Shards[g_NextShardNum].vel.y = vely * PALUPF(3.0f);
	g_Shards[g_NextShardNum].vel.z = velz * PALUPF(1.5f);

	g_Shards[g_NextShardNum].vertices[0].x = (RANDOMFRAC() * 0.5f + 1.0f) * size;
	g_Shards[g_NextShardNum].vertices[0].y = (RANDOMFRAC() * 0.5f + 1.0f) * size;
	g_Shards[g_NextShardNum].vertices[1].x = (RANDOMFRAC() * 0.5f + 1.0f) * size;
	g_Shards[g_NextShardNum].vertices[1].y = (RANDOMFRAC() * 0.5f + 1.0f) * -size;
	g_Shards[g_NextShardNum].vertices[2].x = (RANDOMFRAC() * 0.5f + 1.0f) * -size;
	g_Shards[g_NextShardNum].vertices[2].y = (RANDOMFRAC() * 0.5f + 1.0f) * -size;

	for (i = 0; i < 3; i++) {
		g_Shards[g_NextShardNum].vertices[i].z = 0;
		g_Shards[g_NextShardNum].vertices[i].s = 0;
		g_Shards[g_NextShardNum].vertices[i].t = 0;
		g_Shards[g_NextShardNum].vertices[i].colour = i * 4;
	}

	if (type == SHARDTYPE_WOOD) {
		s32 rand = rngRandom() % 100;

		if (rand < 20) {
			g_Shards[g_NextShardNum].colours[0].word = PD_BE32(0xbbbbbbf0);
			g_Shards[g_NextShardNum].colours[1].word = PD_BE32(0xaaaaaaf0);
			g_Shards[g_NextShardNum].colours[2].word = PD_BE32(0x777777f0);
		} else if (rand < 40) {
			g_Shards[g_NextShardNum].colours[0].word = PD_BE32(0x000000f0);
			g_Shards[g_NextShardNum].colours[1].word = PD_BE32(0x000000f0);
			g_Shards[g_NextShardNum].colours[2].word = PD_BE32(0x000000f0);
		} else if (rand < 60) {
			g_Shards[g_NextShardNum].colours[0].word = PD_BE32(0x553311f0);
			g_Shards[g_NextShardNum].colours[1].word = PD_BE32(0x553311f0);
			g_Shards[g_NextShardNum].colours[2].word = PD_BE32(0x553311f0);
		} else {
			g_Shards[g_NextShardNum].colours[0].word = PD_BE32(0xddaa88f0);
			g_Shards[g_NextShardNum].colours[1].word = PD_BE32(0xddaa88f0);
			g_Shards[g_NextShardNum].colours[2].word = PD_BE32(0xddaa88f0);
		}
	} else {
		s32 i;
		s32 j;

		g_Shards[g_NextShardNum].colours[0].r = 0x05;
		g_Shards[g_NextShardNum].colours[0].g = 0x05;
		g_Shards[g_NextShardNum].colours[0].b = 0x7e;

		g_Shards[g_NextShardNum].colours[1].r = 0x05;
		g_Shards[g_NextShardNum].colours[1].g = 0xfb;
		g_Shards[g_NextShardNum].colours[1].b = 0x7e;

		g_Shards[g_NextShardNum].colours[2].r = 0xfb;
		g_Shards[g_NextShardNum].colours[2].g = 0xfb;
		g_Shards[g_NextShardNum].colours[2].b = 0x7e;

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				g_Shards[g_NextShardNum].colours[i].bytes[j] = rngRandom() % 0xff;
			}
		}

		g_Shards[g_NextShardNum].colours[0].a = g_Shards[g_NextShardNum].colours[1].a = g_Shards[g_NextShardNum].colours[2].a = 0xff;
	}

	g_Shards[g_NextShardNum].rot.x = rotx;
	g_Shards[g_NextShardNum].rot.y = 0;
	g_Shards[g_NextShardNum].rot.z = RANDOMFRAC() * M_BADTAU;

	g_Shards[g_NextShardNum].rotspeed.x = RANDOMFRAC() * PALUPF(0.1f);
	g_Shards[g_NextShardNum].rotspeed.y = RANDOMFRAC() * PALUPF(0.1f);
	g_Shards[g_NextShardNum].rotspeed.z = RANDOMFRAC() * PALUPF(0.1f);

	g_NextShardNum++;

	if (g_NextShardNum >= g_MaxShards) {
		g_NextShardNum = 0;
	}

	if (!g_ShardsActive) {
		g_ShardsActive = true;
	}
}

Gfx *shardsRenderWood(Gfx *gdl)
{
	if (g_ShardsActive) {
		RoomNum prevroom = 0;
		s32 i;
		Mtxf shardmtx;
		s32 j;

		if (g_Vars.currentplayer->visionmode);

		gSPClearGeometryMode(gdl++, G_CULL_BOTH);
		gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
		gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);
		gDPSetCycleType(gdl++, G_CYC_2CYCLE);
		gDPSetTextureLOD(gdl++, G_TL_LOD);
		gSPMatrix(gdl++, osVirtualToPhysical(camGetOrthogonalMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		for (i = 0; i < g_MaxShards; i++) {
			if (g_Shards[i].age60 > 0 && g_Shards[i].type == SHARDTYPE_WOOD) {
				bool render = true;
				f32 alphamult = 1.0f;
				f32 xraydist;
				Mtxf *mtx = gfxAllocateMatrix();
				u32 stack;

				if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
					xraydist = sqrtf(ERASERSQDIST(g_Shards[i].pos.f));

					if (g_Vars.currentplayer->eraserpropdist < xraydist) {
						render = false;
					}

					if (g_Vars.currentplayer->eraserpropdist - 150.0f < xraydist) {
						alphamult = 1.0f - (xraydist - (g_Vars.currentplayer->eraserpropdist - 150.0f)) / 150.0f;
					}

					xraydist = xraydist / g_Vars.currentplayer->eraserpropdist;

					if (xraydist > 1.0f) {
						xraydist = 1.0f;
					}
				}

				if (render) {
					struct shard *shard = (struct shard *) ((u8 *)g_Shards + i * sizeof(struct shard));

					mtx4LoadRotationAndTranslation(&shard->pos, &shard->rot, &shardmtx);

					shardmtx.m[3][0] -= g_Vars.currentplayer->globaldrawworldoffset.x;
					shardmtx.m[3][1] -= g_Vars.currentplayer->globaldrawworldoffset.y;
					shardmtx.m[3][2] -= g_Vars.currentplayer->globaldrawworldoffset.z;

#if VERSION >= VERSION_NTSC_1_0
					if (shardmtx.m[3][0] < 10000 && shardmtx.m[3][0] > -10000
							&& shardmtx.m[3][1] < 10000 && shardmtx.m[3][1] > -10000
							&& shardmtx.m[3][2] < 10000 && shardmtx.m[3][2] > -10000)
#endif
					{
						mtxF2L(&shardmtx, mtx);

						gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

						if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
							Col *colours = gfxAllocateColours(3);

							if (g_Shards[i].age60 >= TICKS(100)) {
								f32 frac = g_Shards[i].age60 / (PAL ? 41.666664123535f : 50.0f);

								if (frac > 1) {
									frac = 1;
								}

								alphamult = (1.0f - frac) * alphamult;
							}

							alphamult *= 0.5f;

							for (j = 0; j < 3; j++) {
								colours[j].r = xraydist * 255.0f;
								colours[j].g = (1.0f - xraydist) * 255.0f;
								colours[j].b = 0;
								colours[j].a = g_Shards[g_NextShardNum].colours[j].a * alphamult;
							}

							gSPColor(gdl++, osVirtualToPhysical(colours), 3);
						} else {
							if (g_Shards[i].room != prevroom) {
								gdl = lightsSetForRoom(gdl, g_Shards[i].room);
								prevroom = g_Shards[i].room;
							}

							gSPColor(gdl++, osVirtualToPhysical(g_Shards[i].colours), 3);
						}

						gSPVertex(gdl++, osVirtualToPhysical(g_Shards[i].vertices), 3, 0);
						gSP1Triangle(gdl++, 0, 1, 2, 0);
					}
				}
			}
		}

		if (prevroom == 0) {
			gdl = lightsSetDefault(gdl);
		}

		gSPClearGeometryMode(gdl++, G_LIGHTING | G_TEXTURE_GEN);
		gSPMatrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
		gSPMatrix(gdl++, camGetMtxL173c(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	}

	return gdl;
}

Gfx *shardsRenderGlass(Gfx *gdl)
{
	if (g_ShardsActive) {
		RoomNum prevroom = 0;
		s32 i;
		Mtxf shardmtx;
		s32 j;

		if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
			texSelect(&gdl, NULL, 2, 1, 2, 1, NULL);
		} else {
			texSelect(&gdl, &g_TexShardConfigs[1], 2, 1, 2, 1, NULL);
		}

		gSPTexture(gdl++, 0x0d80, 0x0d80, 2, G_TX_RENDERTILE, G_ON);
		gDPSetCycleType(gdl++, G_CYC_2CYCLE);
		gDPSetTextureLOD(gdl++, G_TL_LOD);
		gSPClearGeometryMode(gdl++, G_CULL_BOTH);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);

		if (g_Vars.currentplayer->visionmode != VISIONMODE_XRAY) {
			gSPSetGeometryMode(gdl++, G_LIGHTING | G_TEXTURE_GEN);
		}

		gSPMatrix(gdl++, osVirtualToPhysical(camGetOrthogonalMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		for (i = 0; i < g_MaxShards; i++) {
			if (g_Shards[i].age60 > 0 && g_Shards[i].type != SHARDTYPE_WOOD) {
				bool render = true;
				f32 alphamult = 1.0f;
				f32 xraydist;
				Mtxf *mtx = gfxAllocateMatrix();
				u32 stack;

				if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
					xraydist = sqrtf(ERASERSQDIST(g_Shards[i].pos.f));

					if (g_Vars.currentplayer->eraserpropdist < xraydist) {
						render = false;
					}

					if (g_Vars.currentplayer->eraserpropdist - 150.0f < xraydist) {
						alphamult = 1.0f - (xraydist - (g_Vars.currentplayer->eraserpropdist - 150.0f)) / 150.0f;
					}

					xraydist = xraydist / g_Vars.currentplayer->eraserpropdist;

					if (xraydist > 1.0f) {
						xraydist = 1.0f;
					}
				}

				if (render) {
					struct shard *shard = (struct shard *) ((u8 *)g_Shards + i * sizeof(struct shard));

					mtx4LoadRotationAndTranslation(&shard->pos, &shard->rot, &shardmtx);

					shardmtx.m[3][0] -= g_Vars.currentplayer->globaldrawworldoffset.x;
					shardmtx.m[3][1] -= g_Vars.currentplayer->globaldrawworldoffset.y;
					shardmtx.m[3][2] -= g_Vars.currentplayer->globaldrawworldoffset.z;

#if VERSION >= VERSION_NTSC_1_0
					if (shardmtx.m[3][0] < 10000 && shardmtx.m[3][0] > -10000
							&& shardmtx.m[3][1] < 10000 && shardmtx.m[3][1] > -10000
							&& shardmtx.m[3][2] < 10000 && shardmtx.m[3][2] > -10000)
#endif
					{
						mtxF2L(&shardmtx, mtx);

						gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

						if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
							Col *colours = gfxAllocateColours(3);

							if (g_Shards[i].age60 >= TICKS(100)) {
								f32 frac = g_Shards[i].age60 / (PAL ? 41.666664123535f : 50.0f);

								if (frac > 1) {
									frac = 1;
								}

								alphamult = (1.0f - frac) * alphamult;
							}

							alphamult *= 0.5f;

							for (j = 0; j < 3; j++) {
								colours[j].r = xraydist * 255.0f;
								colours[j].g = (1.0f - xraydist) * 255.0f;
								colours[j].b = 0;
								colours[j].a = g_Shards[g_NextShardNum].colours[j].a * alphamult;
							}

							gSPColor(gdl++, osVirtualToPhysical(colours), 3);
						} else {
							if (g_Shards[i].room != prevroom) {
								gdl = lightsSetForRoom(gdl, g_Shards[i].room);
								prevroom = g_Shards[i].room;
							}

							gSPColor(gdl++, osVirtualToPhysical(g_Shards[i].colours), 3);
						}

						gSPVertex(gdl++, osVirtualToPhysical(g_Shards[i].vertices), 3, 0);
						gSP1Triangle(gdl++, 0, 1, 2, 0);
					}
				}
			}
		}

		if (prevroom == 0) {
			gdl = lightsSetDefault(gdl);
		}

		gSPClearGeometryMode(gdl++, G_LIGHTING | G_TEXTURE_GEN);
		gSPMatrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
		gSPMatrix(gdl++, camGetMtxL173c(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	}

	return gdl;
}

Gfx *shardsRender(Gfx *gdl)
{
	gdl = shardsRenderWood(gdl);
	gdl = shardsRenderGlass(gdl);

	return gdl;
}
