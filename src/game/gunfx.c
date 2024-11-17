#include <ultra64.h>
#include "constants.h"
#include "game/acosfasinf.h"
#include "game/bondgun.h"
#include "game/gunfx.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/mtxf2lbulk.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#define BOLTBEAMTICKMODE_MANUAL    0
#define BOLTBEAMTICKMODE_AUTOMATIC 1

struct casing g_Casings[20];
struct boltbeam g_BoltBeams[8];
struct lasersight g_LaserSights[MAX_PLAYERS];

void beamCreate(struct beam *beam, s32 weaponnum, struct coord *from, struct coord *to)
{
	f32 distance;

	beam->from.x = from->x;
	beam->from.y = from->y;
	beam->from.z = from->z;

	beam->dir.x = to->x - from->x;
	beam->dir.y = to->y - from->y;
	beam->dir.z = to->z - from->z;

	distance = sqrtf(beam->dir.f[0] * beam->dir.f[0] + beam->dir.f[1] * beam->dir.f[1] + beam->dir.f[2] * beam->dir.f[2]);

	if (distance > 0) {
		beam->dir.x *= 1.0f / distance;
		beam->dir.y *= 1.0f / distance;
		beam->dir.z *= 1.0f / distance;
	}

	if (distance > 10000) {
		distance = 10000;
	}

	beam->age = 0;
	beam->weaponnum = weaponnum;
	beam->maxdist = distance;

	if (distance < 500) {
		distance = 500;
	}

	if (weaponnum == -1) {
		beam->speed = 0;
		beam->mindist = distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = 0;
	} else if (weaponnum == WEAPON_LASER || weaponnum == WEAPON_WATCHLASER) {
		beam->speed = 0.25f * distance;
		beam->mindist = 0.6f * distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = (-0.1f - RANDOMFRAC() * 0.3f) * distance;
	} else if (weaponnum == -2) {
		beam->speed = 0;
		beam->mindist = distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = 0;
	} else {
		f32 tmp;
		beam->speed = 0.2f * distance;
		beam->mindist = 0.2f * distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		tmp = RANDOMFRAC();
		beam->dist = (tmp + tmp - 1) * beam->speed;
	}

	if (beam->dist >= beam->maxdist) {
		beam->age = -1;
	}
}

void beamCreateForHand(s32 handnum)
{
	struct player *player = g_Vars.currentplayer;
	struct hand *hand = player->hands + handnum;
	Mtxf *mtx = camGetWorldToScreenMtxf();
	f32 tmp;

	tmp = hand->hitpos.f[0] * mtx->m[0][2] + hand->hitpos.f[1] * mtx->m[1][2] + hand->hitpos.f[2] * mtx->m[2][2] + mtx->m[3][2];
	tmp = -tmp;

	if (tmp < hand->muzzlez) {
		// empty
	} else {
		struct beam *beam;
		s32 weaponnum = bgunGetWeaponNum(handnum);

		if (hand->gset.weaponnum == WEAPON_LASER && hand->gset.weaponfunc == FUNC_SECONDARY) {
			weaponnum = -2;
		}

		beam = &hand->beam;
		beamCreate(beam, weaponnum, &hand->muzzlepos, &hand->hitpos);

		if (beam->weaponnum == WEAPON_MAULER) {
			beam->weaponnum = -3 - (s32)player->hands[handnum].matmot1;
		}

		if (player->prop->chr && PLAYERCOUNT() >= 2) {
			struct chrdata *chr = player->prop->chr;
			struct coord disttolast;
			struct coord disttocur;
			f32 radians;

			if (chr->fireslots[handnum] == -1) {
				chr->fireslots[handnum] = bgunAllocateFireslot();
			}

			if (chr->fireslots[handnum] != -1) {
				disttolast.x = hand->hitpos.x - player->chrmuzzlelastpos[handnum].x;
				disttolast.y = hand->hitpos.y - player->chrmuzzlelastpos[handnum].y;
				disttolast.z = hand->hitpos.z - player->chrmuzzlelastpos[handnum].z;

				guNormalize(&disttolast.x, &disttolast.y, &disttolast.z);

				disttocur.x = hand->hitpos.x - hand->muzzlepos.x;
				disttocur.y = hand->hitpos.y - hand->muzzlepos.y;
				disttocur.z = hand->hitpos.z - hand->muzzlepos.z;

				guNormalize(&disttocur.x, &disttocur.y, &disttocur.z);

				radians = acosf(disttolast.f[0] * disttocur.f[0] + disttolast.f[1] * disttocur.f[1] + disttolast.f[2] * disttocur.f[2]);

				if (!(radians > 0.08725257f) || weaponnum == -2) {
					beamCreate(&g_Fireslots[chr->fireslots[handnum]].beam, weaponnum, &player->chrmuzzlelastpos[handnum], &hand->hitpos);

					if (g_Fireslots[chr->fireslots[handnum]].beam.weaponnum == WEAPON_MAULER) {
						g_Fireslots[chr->fireslots[handnum]].beam.weaponnum = -3 - (s32)player->hands[handnum].matmot1;
					}
				}
			}
		}
	}
}

Gfx *beamRenderGeneric(Gfx *gdl, struct textureconfig *texconfig,
		f32 arg2, struct coord *headpos, u32 headcolour,
		f32 arg5, struct coord *tailpos, u32 tailcolour)
{
	struct coord spe4;
	f32 length;
	Vtx *vertices;
	struct coord spd0;
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	Mtxf *spc8;
	Col *colours = gfxAllocateColours(2);
	Mtxf sp84;
	Mtxf *worldtoscreenmtx = camGetWorldToScreenMtxf();
	struct coord sp74 = {0, 0, 0};
	f32 mult;
	u32 stack[2];
	struct coord sp5c;

	spe4.f[0] = tailpos->f[0] - headpos->f[0];
	spe4.f[1] = tailpos->f[1] - headpos->f[1];
	spe4.f[2] = tailpos->f[2] - headpos->f[2];

	length = sqrtf(spe4.f[0] * spe4.f[0] + spe4.f[1] * spe4.f[1] + spe4.f[2] * spe4.f[2]);

	if (length < 0.00001f && length > -0.00001f) {
		return gdl;
	}

	spe4.f[0] /= length;
	spe4.f[1] /= length;
	spe4.f[2] /= length;

	mtx4TransformVec(camGetWorldToScreenMtxf(), headpos, &sp5c);

	if (sp5c.f[0] * arg2 > 10000.0f || sp5c.f[0] * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.f[1] * arg2 > 10000.0f || sp5c.f[1] * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.f[2] * arg2 > 10000.0f || sp5c.f[2] * arg2 < -10000.0f) {
		return gdl;
	}

	mtx4TransformVec(camGetWorldToScreenMtxf(), tailpos, &sp5c);

	if (sp5c.f[0] * arg2 > 10000.0f || sp5c.f[0] * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.f[1] * arg2 > 10000.0f || sp5c.f[1] * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.f[2] * arg2 > 10000.0f || sp5c.f[2] * arg2 < -10000.0f) {
		return gdl;
	}

	colours[0].word = PD_BE32(headcolour);
	colours[1].word = PD_BE32(tailcolour);

	spd0.f[0] = (spe4.f[1] * (campos->f[2] - (headpos->f[2] + length * spe4.f[2]))) - (spe4.f[2] * (campos->f[1] - (headpos->f[1] + length * spe4.f[1])));
	spd0.f[1] = (spe4.f[2] * (campos->f[0] - (headpos->f[0] + length * spe4.f[0]))) - (spe4.f[0] * (campos->f[2] - (headpos->f[2] + length * spe4.f[2])));
	spd0.f[2] = (spe4.f[0] * (campos->f[1] - (headpos->f[1] + length * spe4.f[1]))) - (spe4.f[1] * (campos->f[0] - (headpos->f[0] + length * spe4.f[0])));

	if (spd0.f[0] != 0.0f || spd0.f[1] != 0.0f || spd0.f[2] != 0.0f) {
		guNormalize(&spd0.f[0], &spd0.f[1], &spd0.f[2]);
	} else {
		spd0.f[0] = 0.0f;
		spd0.f[1] = 1.0f;
		spd0.f[2] = 0.0f;
	}

	vertices = gfxAllocateVertices(4);
	spc8 = gfxAllocateMatrix();

	mtx4LoadTranslation(headpos, &sp84);

	mtx00015f04(1.0f / arg2, &sp84);
	mtx00015be0(worldtoscreenmtx, &sp84);
	mtxF2L(&sp84, spc8);

	mult = arg5 * arg2;

	sp74.f[0] = spe4.f[0] * (length * arg2);
	sp74.f[1] = spe4.f[1] * (length * arg2);
	sp74.f[2] = spe4.f[2] * (length * arg2);

	vertices[0].x = spd0.f[0] * mult;
	vertices[0].y = spd0.f[1] * mult;
	vertices[0].z = spd0.f[2] * mult;
	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[0].colour = 0;

	vertices[1].x = -spd0.f[0] * mult;
	vertices[1].y = -spd0.f[1] * mult;
	vertices[1].z = -spd0.f[2] * mult;
	vertices[1].s = texconfig->width * 32;
	vertices[1].t = 0;
	vertices[1].colour = 0;

	vertices[2].x = sp74.f[0] - spd0.f[0] * mult;
	vertices[2].y = sp74.f[1] - spd0.f[1] * mult;
	vertices[2].z = sp74.f[2] - spd0.f[2] * mult;
	vertices[2].s = texconfig->width * 32;
	vertices[2].t = texconfig->height * 32;
	vertices[2].colour = 4;

	vertices[3].x = sp74.f[0] + spd0.f[0] * mult;
	vertices[3].y = sp74.f[1] + spd0.f[1] * mult;
	vertices[3].z = sp74.f[2] + spd0.f[2] * mult;
	vertices[3].s = 0;
	vertices[3].t = texconfig->height * 32;
	vertices[3].colour = 4;

	gSPClearGeometryMode(gdl++, G_CULL_BACK);
	gSPMatrix(gdl++, osVirtualToPhysical(spc8), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gSPColor(gdl++, osVirtualToPhysical(colours), 2);

	texSelect(&gdl, texconfig, 4, 1, 2, true, NULL);

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);
	gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);

	return gdl;
}

Gfx *beamRender(Gfx *gdl, struct beam *beam, bool arg2, u8 arg3)
{
	u32 stack;
	Mtxf *sp188;
	Mtxf sp148;

	if (arg3 < 5 && beam->age >= 0) {
		Col *colours = gfxAllocateColours(1);
		struct coord sp138;
		struct coord *campos = &g_Vars.currentplayer->cam_pos;
		f32 sp130;
		f32 sp12c = beam->mindist;
		Vtx *vertices;
		f32 sp124 = beam->dist;
		struct coord sp118;
		struct coord sp10c;
		struct coord sp100 = {0, 0, 0};
		struct coord spf4 = {0, 0, 0};
		f32 spf0 = 1.4142f;
		struct textureconfig *texconfig = &g_TexBeamConfigs[arg3];
		s32 i;
		Mtxf *worldtoscreenmtx = camGetWorldToScreenMtxf();
		s32 j;
		u32 stack1;
		s32 spd8;
		struct coord spcc;
		f32 tmp;
		f32 spc0[2];
		f32 spb8[2];
		f32 f14;
		f32 f16;
		f32 f18;
		f32 spa8;
		f32 spa4;

		switch (beam->weaponnum) {
		case WEAPON_CYCLONE:
			texconfig = &g_TexBeamConfigs[1];
			break;
		case WEAPON_TRANQUILIZER:
			texconfig = &g_TexBeamConfigs[3];
			break;
		case WEAPON_MAULER:
		case WEAPON_PHOENIX:
		case WEAPON_CALLISTO:
		case WEAPON_REAPER:
		case WEAPON_FARSIGHT:
			texconfig = &g_TexBeamConfigs[4];
			break;
		}

		if (beam->weaponnum == -1 || beam->weaponnum == WEAPON_CYCLONE) {
			colours[0].word = PD_BE32(0xffffff7f);
		} else {
			colours[0].word = 0xffffffff;
		}

		if (beam->weaponnum == WEAPON_LASER) {
			// Laser primary
			sp130 = 50.0f;
			texconfig = &g_TexLaserConfigs[0];
		} else if (beam->weaponnum == -2) {
			// Laser secondary
			sp130 = 10.0f;
			texconfig = &g_TexLaserConfigs[0];

			colours[0].a = 150 + (rngRandom() % 50);

			if ((rngRandom() % 5) == 0) {
				colours[0].r = colours[0].g = 255 - (rngRandom() % 100);
			}
		} else {
			sp130 = 30.0f;
		}

		if (beam->weaponnum <= -3) {
			// Mauler
			sp130 = sp130 * ((beam->weaponnum + 3) * 2.0f + 1.0f);
			texconfig = &g_TexBeamConfigs[4];
		}

		sp138.f[0] = beam->from.f[0];
		sp138.f[1] = beam->from.f[1];
		sp138.f[2] = beam->from.f[2];

		if (sp124 > 0.0f) {
			sp138.f[0] += sp124 * beam->dir.f[0];
			sp138.f[1] += sp124 * beam->dir.f[1];
			sp138.f[2] += sp124 * beam->dir.f[2];
		} else {
			sp12c += sp124;
			sp124 = 0.0f;
		}

		if (sp124 + sp12c > beam->maxdist) {
			sp12c = beam->maxdist - sp124;
		}

		sp10c.f[0] = (beam->dir.f[1] * (campos->f[2] - (sp138.f[2] + sp12c * beam->dir.f[2]))) - (beam->dir.f[2] * (campos->f[1] - (sp138.f[1] + sp12c * beam->dir.f[1])));
		sp10c.f[1] = (beam->dir.f[2] * (campos->f[0] - (sp138.f[0] + sp12c * beam->dir.f[0]))) - (beam->dir.f[0] * (campos->f[2] - (sp138.f[2] + sp12c * beam->dir.f[2])));
		sp10c.f[2] = (beam->dir.f[0] * (campos->f[1] - (sp138.f[1] + sp12c * beam->dir.f[1]))) - (beam->dir.f[1] * (campos->f[0] - (sp138.f[0] + sp12c * beam->dir.f[0])));

		if (sp10c.f[0] != 0.0f || sp10c.f[1] != 0.0f || sp10c.f[2] != 0.0f) {
			guNormalize(&sp10c.f[0], &sp10c.f[1], &sp10c.f[2]);

			sp10c.f[0] *= sp130;
			sp10c.f[1] *= sp130;
			sp10c.f[2] *= sp130;
		} else {
			sp10c.f[0] = 0.0f;
			sp10c.f[1] = sp130;
			sp10c.f[2] = 0.0f;
		}

		sp118.f[0] = beam->dir.f[1] * sp10c.f[2] - beam->dir.f[2] * sp10c.f[1];
		sp118.f[1] = beam->dir.f[2] * sp10c.f[0] - beam->dir.f[0] * sp10c.f[2];
		sp118.f[2] = beam->dir.f[0] * sp10c.f[1] - beam->dir.f[1] * sp10c.f[0];

		guNormalize(&sp118.f[0], &sp118.f[1], &sp118.f[2]);

		sp118.f[0] *= sp130;
		sp118.f[1] *= sp130;
		sp118.f[2] *= sp130;

		if (beam->weaponnum == WEAPON_LASER) {
			vertices = gfxAllocateVertices(8);
		} else {
			vertices = gfxAllocateVertices(4);
		}

		sp188 = gfxAllocateMatrix();

		if (sp12c > 0.0f
				&& sp138.f[0] > -32000.0f && sp138.f[0] < 32000.0f
				&& sp138.f[1] > -32000.0f && sp138.f[1] < 32000.0f
				&& sp138.f[2] > -32000.0f && sp138.f[2] < 32000.0f) {
			spd8 = true;
			mtx4LoadTranslation(&sp138, &sp148);
			mtx00015f04(0.1f, &sp148);
			mtx00015be0(worldtoscreenmtx, &sp148);

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (sp148.m[i][j] < -32000.0f || sp148.m[i][j] > 32000.0f) {
						spd8 = false;
						break;
					}
				}
			}

			if (spd8) {
				mtxF2L(&sp148, sp188);

				if (beam->weaponnum);

				if (beam->weaponnum == -2 && PLAYERCOUNT() == 1) {
					spcc.f[0] = sp138.f[0] + beam->dir.f[0] * sp12c;
					spcc.f[1] = sp138.f[1] + beam->dir.f[1] * sp12c;
					spcc.f[2] = sp138.f[2] + beam->dir.f[2] * sp12c;

					mtx4TransformVecInPlace(worldtoscreenmtx, &spcc);

					spb8[0] = spb8[1] = sp130 / 10;
					tmp = -spcc.f[2];

					cam0f0b4e68(spb8, tmp, spc0);

					if (spc0[0] < 2) {
						spcc.f[0] *= spc0[0] * 0.5f;
						spcc.f[1] *= spc0[0] * 0.5f;
						spcc.f[2] *= spc0[0] * 0.5f;
					}

					mtx4TransformVecInPlace(camGetProjectionMtxF(), &spcc);

					spcc.f[0] -= sp138.f[0];
					spcc.f[1] -= sp138.f[1];
					spcc.f[2] -= sp138.f[2];

					sp100.f[0] = spcc.f[0] * 10;
					sp100.f[1] = spcc.f[1] * 10;
					sp100.f[2] = spcc.f[2] * 10;
				} else {
					sp100.f[0] = beam->dir.f[0] * (sp12c * 10);
					sp100.f[1] = beam->dir.f[1] * (sp12c * 10);
					sp100.f[2] = beam->dir.f[2] * (sp12c * 10);
				}

				if (sp100.f[0] > -30000.0f && sp100.f[0] < 30000.0f
						&& sp100.f[1] > -30000.0f && sp100.f[1] < 30000.0f
						&& sp100.f[2] > -30000.0f && sp100.f[2] < 30000.0f) {
					vertices[0].x = sp10c.f[0];
					vertices[0].y = sp10c.f[1];
					vertices[0].z = sp10c.f[2];
					vertices[0].s = texconfig->width * 32;
					vertices[0].t = 0;
					vertices[0].colour = 0;

					vertices[1].x = -sp10c.f[0];
					vertices[1].y = -sp10c.f[1];
					vertices[1].z = -sp10c.f[2];
					vertices[1].s = 0;
					vertices[1].t = 0;
					vertices[1].colour = 0;

					vertices[2].x = sp100.f[0] + sp10c.f[0] * 0.9f;
					vertices[2].y = sp100.f[1] + sp10c.f[1] * 0.9f;
					vertices[2].z = sp100.f[2] + sp10c.f[2] * 0.9f;
					vertices[2].s = texconfig->width * 32;
					vertices[2].t = texconfig->height * 32;
					vertices[2].colour = 0;

					vertices[3].x = sp100.f[0] - sp10c.f[0] * 0.9f;
					vertices[3].y = sp100.f[1] - sp10c.f[1] * 0.9f;
					vertices[3].z = sp100.f[2] - sp10c.f[2] * 0.9f;
					vertices[3].s = 0;
					vertices[3].t = texconfig->height * 32;
					vertices[3].colour = 0;

					if (beam->weaponnum == WEAPON_LASER) {
						f14 = campos->f[0] - sp138.f[0];
						f16 = campos->f[1] - sp138.f[1];
						f18 = campos->f[2] - sp138.f[2];

						spa8 = f14 * f14 + f16 * f16 + f18 * f18;

						f14 = campos->f[0] - (sp138.f[0] + beam->dir.f[0] * sp12c);
						f16 = campos->f[1] - (sp138.f[1] + beam->dir.f[1] * sp12c);
						f18 = campos->f[2] - (sp138.f[2] + beam->dir.f[2] * sp12c);

						spa4 = f14 * f14 + f16 * f16 + f18 * f18;

						if (spa4 < spa8) {
							spf4.f[0] = sp100.f[0];
							spf4.f[1] = sp100.f[1];
							spf4.f[2] = sp100.f[2];
							spf0 *= 0.9f;
						}

						vertices[4].x = spf4.f[0] + sp118.f[0] * spf0;
						vertices[4].y = spf4.f[1] + sp118.f[1] * spf0;
						vertices[4].z = spf4.f[2] + sp118.f[2] * spf0;
						vertices[4].s = g_TexGroup03Configs[0].width * 32;
						vertices[4].t = g_TexGroup03Configs[0].height * 32;
						vertices[4].colour = 0;

						vertices[5].x = spf4.f[0] - sp118.f[0] * spf0;
						vertices[5].y = spf4.f[1] - sp118.f[1] * spf0;
						vertices[5].z = spf4.f[2] - sp118.f[2] * spf0;
						vertices[5].s = 0;
						vertices[5].t = 0;
						vertices[5].colour = 0;

						vertices[6].x = spf4.f[0] + sp10c.f[0] * spf0;
						vertices[6].y = spf4.f[1] + sp10c.f[1] * spf0;
						vertices[6].z = spf4.f[2] + sp10c.f[2] * spf0;
						vertices[6].s = 0;
						vertices[6].t = g_TexGroup03Configs[0].height * 32;
						vertices[6].colour = 0;

						vertices[7].x = spf4.f[0] - sp10c.f[0] * spf0;
						vertices[7].y = spf4.f[1] - sp10c.f[1] * spf0;
						vertices[7].z = spf4.f[2] - sp10c.f[2] * spf0;
						vertices[7].s = g_TexGroup03Configs[0].width * 32;
						vertices[7].t = 0;
						vertices[7].colour = 0;
					}

					gSPClearGeometryMode(gdl++, G_CULL_BACK);
					gSPMatrix(gdl++, osVirtualToPhysical(sp188), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
					gDPSetCycleType(gdl++, G_CYC_1CYCLE);
					gDPSetColorDither(gdl++, G_CD_DISABLE);
					gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
					gDPSetAlphaCompare(gdl++, G_AC_NONE);
					gDPSetTextureLOD(gdl++, G_TL_TILE);
					gDPSetTextureConvert(gdl++, G_TC_FILT);
					gDPSetCombineMode(gdl++, G_CC_BLENDIA, G_CC_BLENDIA);
					gSPColor(gdl++, osVirtualToPhysical(colours), 1);

					if (beam->weaponnum == WEAPON_LASER) {
						texSelect(&gdl, &g_TexGroup03Configs[0], 4, arg2, 2, true, NULL);

						gSPVertex(gdl++, osVirtualToPhysical(vertices), 8, 0);
						gSPTri2(gdl++, 4, 5, 6, 4, 5, 7);

						texSelect(&gdl, texconfig, 4, arg2, 2, true, NULL);

						gSPTri2(gdl++, 0, 2, 3, 0, 3, 1);
					} else {
						texSelect(&gdl, texconfig, 4, arg2, 2, true, NULL);

						gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);
						gSPTri2(gdl++, 0, 2, 3, 0, 3, 1);
					}
				}
			}
		}
	}

	return gdl;
}

void beamTick(struct beam *beam)
{
	if (beam->age >= 0) {
		if (beam->weaponnum == -2) {
			beam->age++;

			if (beam->age > 1) {
				beam->age = -1;
			}
		} else {
			if (g_Vars.lvupdate240 <= 8) {
				// Not lagging
				beam->dist += beam->speed * g_Vars.lvupdate60f;
			} else {
				// Lagging
				beam->dist += beam->speed * (2 + RANDOMFRAC() * 0.5f);
			}

			if (beam->dist >= beam->maxdist) {
				beam->age = -1;
			}
		}
	}
}

bool g_CasingsActive = false;

struct casing *casingCreate(struct modeldef *modeldef, Mtxf *mtx)
{
	s32 i;
	s32 j;
	f32 rot[3][3];
	struct casing *casing = g_Casings;
	struct casing *end = g_Casings + ARRAYCOUNT(g_Casings);
	u32 stack;

	while (casing < end && casing->modeldef != NULL) {
		casing++;
	}

	if (casing < end) {
		casing->modeldef = modeldef;
		casing->pos.x = mtx->m[3][0];
		casing->pos.y = mtx->m[3][1];
		casing->pos.z = mtx->m[3][2];

		mtx4ToMtx3(mtx, rot);

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				casing->rot[i][j] = rot[i][j] * 4096.0f;
			}
		}

		g_CasingsActive = true;

		return casing;
	}

	return NULL;
}

void casingCreateForHand(s32 handnum, f32 ground, Mtxf *mtx)
{
	f32 oldyspeed;
	struct casing *casing = NULL;
	struct player *player = g_Vars.currentplayer;
	s32 i;
	s32 j;
	Mtxf spec;
	f32 spc8[3][3];
	s32 weaponnum = bgunGetWeaponNum(handnum);
	s32 casingtype = -1;
	struct weaponfunc *func = gsetGetWeaponFunction2(&player->hands[handnum].gset);
	struct weapon *weapondef = weaponFindById(player->gunctrl.weaponnum);
	struct weaponfunc_shoot *shootfunc = NULL;
	struct modeldef *modeldef;

	if ((func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		shootfunc = (struct weaponfunc_shoot *)func;
	}

	if (func->ammoindex < 0) {
		return;
	}

	if (weapondef != NULL && shootfunc != NULL) {
		if (weapondef->ammos[func->ammoindex] != NULL) {
			casingtype = weapondef->ammos[func->ammoindex]->casingeject;
		}
	}

	if (casingtype < 0) {
		return;
	}

	if (PLAYERCOUNT() >= 2) {
		return;
	}

	mtx4Copy(mtx, &spec);

	modeldef = bgunGetCartModeldef();

	if (modeldef != NULL) {
		casing = casingCreate(modeldef, &spec);
	}

	if (casing != NULL) {
		struct coord spa4 = {0, 0, 0};
		Mtxf sp64;
		u32 magic = 0x15aca6;
		u32 sp5c;
		u32 stack[3];
		u32 sp4c;
		f32 newyspeed;
		f32 f0;

		casing->ground = ground;

		if (weaponnum == WEAPON_PP9I || weaponnum == WEAPON_CC13
				|| weaponnum == WEAPON_FALCON2 || weaponnum == WEAPON_MAGSEC4) {
			casing->speed.x = -(RANDOMFRAC() * 0.5333333f * 0.0625f + 0.5333333f);
			casing->speed.y = RANDOMFRAC() * 2.5f * 0.0625f + 2.5f;
			casing->speed.z = 0.0f;

			mtx4RotateVecInPlace(mtx, &casing->speed);

			spa4.x = 2.0f * RANDOMFRAC() * M_BADTAU * 0.0625f - 0.39263657f;
			spa4.y = 2.0f * RANDOMFRAC() * M_BADTAU * 0.0625f - 0.39263657f;
			spa4.z = 2.0f * RANDOMFRAC() * M_BADTAU * 0.0625f - 0.39263657f;

			mtx4LoadRotation(&spa4, &sp64);
			mtx4ToMtx3(&sp64, spc8);

			for (i = 0; i < 3; i++) {
				for (j = 0; j < 3; j++) {
					casing->rotspeed[i][j] = spc8[i][j] * 4096.0f;
				}
			}

			sp5c = ((s32)((rngRandom() >> 24) * magic) >> 10) + magic;
			f0 = (rngRandom() % sp5c) / PALUP((f32) (OS_CPU_COUNTER / 60));

			newyspeed = casing->speed.y - f0 * 0.2777778f;

			casing->pos.y += f0 * (casing->speed.y + newyspeed) * 0.5f;
			casing->pos.x += f0 * casing->speed.x;
			casing->pos.z += f0 * casing->speed.z;

			casing->speed.y = newyspeed;

			if (g_Vars.lvupdate240 > 0) {
				casing->speed.x += (player->hands[handnum].posmtx.m[3][0] - player->hands[handnum].prevmtx.m[3][0]) / g_Vars.lvupdate60freal;
				casing->speed.y += (player->hands[handnum].posmtx.m[3][1] - player->hands[handnum].prevmtx.m[3][1]) / g_Vars.lvupdate60freal;
				casing->speed.z += (player->hands[handnum].posmtx.m[3][2] - player->hands[handnum].prevmtx.m[3][2]) / g_Vars.lvupdate60freal;
			}
		} else {
			if (weaponnum == WEAPON_REAPER) {
				casing->speed.x = -(RANDOMFRAC() * 0.41666666f * 0.125f + 0.41666666f);
				casing->speed.y = RANDOMFRAC() * 3.3333333f * 0.125f + 3.3333333f;
			} else {
				casing->speed.x = -((RANDOMFRAC() * 1.4166666f * 0.125f) + 1.4166666f);
				casing->speed.y = RANDOMFRAC() * 1.6666666f * 0.125f + 1.6666666f;
			}

			casing->speed.z = 0.0f;

			if (weaponnum == WEAPON_DY357MAGNUM || weaponnum == WEAPON_DY357LX) {
				casing->speed.x = 0.0f;
				casing->speed.y = 0.0f;
				casing->speed.z = -1.0f;
			}

			mtx4RotateVecInPlace(mtx, &casing->speed);

			if (weaponnum == WEAPON_REAPER) {
				spa4.x = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;
				spa4.y = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;
				spa4.z = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;

				mtx4LoadRotation(&spa4, &sp64);
				mtx4RotateVecInPlace(&sp64, &casing->speed);
			}

			spa4.x = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;
			spa4.y = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;
			spa4.z = 2.0f * RANDOMFRAC() * M_BADTAU * 0.015625f - 0.09815914f;

			mtx4LoadRotation(&spa4, &sp64);
			mtx4ToMtx3(&sp64, spc8);

			for (i = 0; i < 3; i++) {
				for (j = 0; j < 3; j++) {
					casing->rotspeed[i][j] = spc8[i][j] * 4096.0f;
				}
			}

			sp4c = ((s32) ((rngRandom() >> 24) * magic) >> 10) + magic;
			f0 = (rngRandom() % sp4c) / PALUP((f32) (OS_CPU_COUNTER / 60));

			newyspeed = casing->speed.y - f0 * 0.2777778f;

			casing->pos.y += f0 * (casing->speed.y + newyspeed) * 0.5f;
			casing->pos.x += f0 * casing->speed.x;
			casing->pos.z += f0 * casing->speed.z;

			casing->speed.y = newyspeed;

			if (g_Vars.lvupdate240 > 0) {
				casing->speed.x += (player->hands[handnum].posmtx.m[3][0] - player->hands[handnum].prevmtx.m[3][0]) / g_Vars.lvupdate60freal;
				casing->speed.y += (player->hands[handnum].posmtx.m[3][1] - player->hands[handnum].prevmtx.m[3][1]) / g_Vars.lvupdate60freal;
				casing->speed.z += (player->hands[handnum].posmtx.m[3][2] - player->hands[handnum].prevmtx.m[3][2]) / g_Vars.lvupdate60freal;
			}
		}
	}
}

void casingRender(struct casing *casing, Gfx **gdlptr)
{
	Gfx *gdl = *gdlptr;
	struct modeldef *modeldef = casing->modeldef;
	Mtxf *matrices = gfxAllocate(modeldef->nummatrices * sizeof(Mtxf));
	struct model model;
	struct modelrenderdata renderdata = { NULL, true, 3 };
	Mtxf mtx;
	s32 i;
	s32 j;
	bool render = true;

	modelAllocateRwData(modeldef);
	modelInit(&model, modeldef, NULL, true);

	model.matrices = matrices;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			mtx.m[i][j] = casing->rot[i][j] * (1.0f / 4096.0f);
		}
	}

	mtx.m[3][0] = 0.0f;
	mtx.m[3][1] = 0.0f;
	mtx.m[3][2] = 0.0f;

	mtx.m[0][3] = 0.0f;
	mtx.m[1][3] = 0.0f;
	mtx.m[2][3] = 0.0f;
	mtx.m[3][3] = 1.0f;

	mtx00015f04(0.1000000089407f, &mtx);
	mtx4SetTranslation(&casing->pos, &mtx);
	mtx00015be4(camGetWorldToScreenMtxf(), &mtx, model.matrices);

	// Check if any coordinate is out of range
	for (i = 0; i < 3; i++) {
		if (model.matrices[0].m[3][i] > 30000) {
			render = false;
		} else if (model.matrices[0].m[3][i] < -30000) {
			render = false;
		}
	}

	if (render) {
		renderdata.zbufferenabled = 1;
		renderdata.gdl = gdl;
		renderdata.unk10 = matrices;
		renderdata.unk30 = 4;
		renderdata.envcolour = g_Vars.currentplayer->gunshadecol[0] << 24
			| g_Vars.currentplayer->gunshadecol[1] << 16
			| g_Vars.currentplayer->gunshadecol[2] << 8
			| g_Vars.currentplayer->gunshadecol[3];

		modelRender(&renderdata, &model);

		*gdlptr = renderdata.gdl;

		mtxF2LBulk(matrices, modeldef->nummatrices);
	}
}

void casingsRender(Gfx **gdlptr)
{
	if (g_CasingsActive) {
		struct casing *end = g_Casings + ARRAYCOUNT(g_Casings);
		struct casing *casing = g_Casings;

		while (casing < end) {
			if (casing->modeldef) {
				casingRender(casing, gdlptr);
			}

			casing++;
		}
	}
}

s32 boltbeamFindByProp(struct prop *prop)
{
	s32 result = -1;
	s32 i = 0;

	for (; i < 8 && result == -1; i++) {
		if (g_BoltBeams[i].unk00_prop == prop) {
			result = i;
		}
	}

	return result;
}

s32 boltbeamCreate(struct prop *prop)
{
	s32 beamnum = boltbeamFindByProp((struct prop *) -1);

	if (beamnum >= 0) {
		g_BoltBeams[beamnum].tickmode = BOLTBEAMTICKMODE_MANUAL;
		g_BoltBeams[beamnum].unk00_prop = prop;
	}

	return beamnum;
}

void boltbeamFree(struct prop *prop)
{
	s32 beamnum = boltbeamFindByProp(prop);

	if (beamnum != -1) {
		g_BoltBeams[beamnum].unk00 = -1;
	}
}

void boltbeamSetHeadPos(s32 beamnum, struct coord *pos)
{
	g_BoltBeams[beamnum].headpos.x = pos->x;
	g_BoltBeams[beamnum].headpos.y = pos->y;
	g_BoltBeams[beamnum].headpos.z = pos->z;
}

void boltbeamSetTailPos(s32 beamnum, struct coord *pos)
{
	g_BoltBeams[beamnum].tailpos.x = pos->x;
	g_BoltBeams[beamnum].tailpos.y = pos->y;
	g_BoltBeams[beamnum].tailpos.z = pos->z;
}

void boltbeamIncrementHeadPos(s32 beamnum, f32 arg1, bool arg2)
{
	f32 dist;

#define DIFF(i) (g_BoltBeams[beamnum].tailpos.f[i] - g_BoltBeams[beamnum].headpos.f[i])
#define SQDIFF(i) (DIFF(i) * DIFF(i))

	dist = sqrtf(SQDIFF(0) + SQDIFF(1) + SQDIFF(2));

	if (dist > arg1 && !arg2) {
		f32 tmp[3];

		if (1);

		tmp[0] = (g_BoltBeams[beamnum].headpos.f[0] - g_BoltBeams[beamnum].tailpos.f[0]) / dist;
		tmp[1] = (g_BoltBeams[beamnum].headpos.f[1] - g_BoltBeams[beamnum].tailpos.f[1]) / dist;
		tmp[2] = (g_BoltBeams[beamnum].headpos.f[2] - g_BoltBeams[beamnum].tailpos.f[2]) / dist;

		g_BoltBeams[beamnum].headpos.f[0] = g_BoltBeams[beamnum].tailpos.f[0] + tmp[0] * arg1;
		g_BoltBeams[beamnum].headpos.f[1] = g_BoltBeams[beamnum].tailpos.f[1] + tmp[1] * arg1;
		g_BoltBeams[beamnum].headpos.f[2] = g_BoltBeams[beamnum].tailpos.f[2] + tmp[2] * arg1;
	}
}

void boltbeamSetAutomatic(s32 beamnum, f32 speed)
{
	g_BoltBeams[beamnum].tickmode = BOLTBEAMTICKMODE_AUTOMATIC;
	g_BoltBeams[beamnum].unk00 = 0;
	g_BoltBeams[beamnum].speed = speed;
}

Gfx *boltbeamsRender(Gfx *gdl)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_BoltBeams); i++) {
		if (g_BoltBeams[i].unk00 != -1) {
			gdl = beamRenderGeneric(gdl, g_TexLaserConfigs, 1, &g_BoltBeams[i].headpos, 0xafafff00, 2, &g_BoltBeams[i].tailpos, 0xafafff7f);
		}
	}

	return gdl;
}

void boltbeamsTick(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_BoltBeams); i++) {
		if (g_BoltBeams[i].unk00 != -1 && g_BoltBeams[i].tickmode == BOLTBEAMTICKMODE_AUTOMATIC) {
			f32 length = sqrtf(
					(g_BoltBeams[i].tailpos.f[0] - g_BoltBeams[i].headpos.f[0]) * (g_BoltBeams[i].tailpos.f[0] - g_BoltBeams[i].headpos.f[0]) +
					(g_BoltBeams[i].tailpos.f[1] - g_BoltBeams[i].headpos.f[1]) * (g_BoltBeams[i].tailpos.f[1] - g_BoltBeams[i].headpos.f[1]) +
					(g_BoltBeams[i].tailpos.f[2] - g_BoltBeams[i].headpos.f[2]) * (g_BoltBeams[i].tailpos.f[2] - g_BoltBeams[i].headpos.f[2]));

			length -= g_BoltBeams[i].speed * LVUPDATE60FREAL() / 60.0f;

			if (length < 0) {
				g_BoltBeams[i].unk00 = -1;
			} else {
				boltbeamIncrementHeadPos(i, length, false);
			}
		}
	}
}

/**
 * Return true if a lasersight with the given ID exists, or false if not.
 *
 * Additionally, populate the index pointer with the index of the lasersight
 * if it exists, or any free slot if it doesn't.
 */
bool lasersightExists(s32 id, s32 *index)
{
	s32 fallback = -1;
	s32 exact = -1;
	s32 i = 0;

	for (; i < 4 && exact == -1; i++) {
		if (g_LaserSights[i].id == id) {
			exact = i;
		}

		if (g_LaserSights[i].id == -1) {
			fallback = i;
		}
	}

	if (exact == -1) {
		*index = fallback;
		return false;
	}

	*index = exact;
	return true;
}

Gfx *lasersightRenderDot(Gfx *gdl)
{
	Mtxf *mtx;
	f32 f0;
	f32 f20;
	struct player *player = g_Vars.currentplayer;
	Mtxf sp1b0;
	struct coord campos;
	Mtxf sp164;
	Mtxf sp124;
	s32 i;

	static u32 sp1 = 800;
#ifndef PLATFORM_N64
	// laser fades out farther away
	static u32 sp2 = 7000 * 3;
	static u32 sp3 = 9000 * 3;
#else
	static u32 sp2 = 7000;
	static u32 sp3 = 9000;
#endif
	static u32 spb = 24;
	static u32 spi = 6;

	mainOverrideVariable("sp1", &sp1);
	mainOverrideVariable("sp2", &sp2);
	mainOverrideVariable("sp3", &sp3);
	mainOverrideVariable("spb", &spb);
	mainOverrideVariable("spi", &spi);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetCombineMode(gdl++, G_CC_BLENDIA, G_CC_BLENDIA);

	if (f20);

	mtx4LoadIdentity(&sp164);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp164);
	mtx4LoadIdentity(&sp124);
	mtx00015be0(camGetProjectionMtxF(), &sp124);

	sp124.m[3][0] = sp124.m[3][1] = sp124.m[3][2] = 0.0f;

	mtx4LoadIdentity(&sp1b0);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp1b0);

	campos.x = player->cam_pos.x;
	campos.y = player->cam_pos.y;
	campos.z = player->cam_pos.z;

	sp1b0.m[3][0] = 0.0f;
	sp1b0.m[3][1] = 0.0f;
	sp1b0.m[3][2] = 0.0f;

	mtx00015f88(0.2f, &sp1b0);

	mtx = gfxAllocateMatrix();
	mtxF2L(&sp1b0, mtx);

	gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	for (i = 0; i < ARRAYCOUNT(g_LaserSights); i++) {
		if (g_LaserSights[i].id != -1) {
			struct coord pos;
			struct coord rot;
			Col *colours;
			Vtx *vertices;

			pos.x = g_LaserSights[i].dotpos.x;
			pos.y = g_LaserSights[i].dotpos.y;
			pos.z = g_LaserSights[i].dotpos.z;

			rot.x = g_LaserSights[i].dotrot.x;
			rot.y = g_LaserSights[i].dotrot.y;
			rot.z = g_LaserSights[i].dotrot.z;

			colours = gfxAllocateColours(2);

			colours[0].word = PD_BE32(0xff00005f);
			colours[1].word = PD_BE32(0xff00000f);

			gSPColor(gdl++, osVirtualToPhysical(colours), 2);

			if (g_LaserSights[i].unk28 > 0.0f) {
				if (!(g_LaserSights[i].dotpos.f[0] < 0.0000001f) || !(g_LaserSights[i].dotpos.f[0] > -0.000001f)
						|| !(g_LaserSights[i].dotpos.f[1] < 0.0000001f) || !(g_LaserSights[i].dotpos.f[1] > -0.000001f)
						|| !(g_LaserSights[i].dotpos.f[2] < 0.0000001f) || !(g_LaserSights[i].dotpos.f[2] > -0.000001f)) {
					f32 spcc;
					f32 spc8;
					f32 spc4;
					f32 f22;
					f32 f24;
					f32 f26;
					f32 f28;
					f32 f30;
					f32 f2;
					f32 f00;

					f20 = spi;

					pos.x = (pos.x - campos.f[0]) * 5.0f;
					pos.y = (pos.y - campos.f[1]) * 5.0f;
					pos.z = (pos.z - campos.f[2]) * 5.0f;

					f0 = sqrtf(pos.f[0] * pos.f[0] + pos.f[1] * pos.f[1] + pos.f[2] * pos.f[2]);

					spcc = sp1;

					if (f0 > spcc) {
						spc8 = sp2;

						if (f0 > spc8) {
							spc4 = sp3;

							if (f0 > spc4) {
								f20 = 0.1f;
							} else {
								f32 tmp1 = spb + f20;
								f32 tmp2 = (f0 - spc8) / (spc4 - spc8);
								f20 = tmp1 - tmp1 * tmp2;
							}
						} else {
							f20 = f20 + (f0 - spcc) * (spb / (spc8 - spcc));
						}
					}

					texSelect(&gdl, &g_TexGeneralConfigs[4], 4, 0, 2, true, NULL);

					if (rot.f[0] == 0.0f && rot.f[2] == 0.0f) {
						spcc = 0.0f;
						spc8 = 0.0f;
						spc4 = 1.0f;
						f22 = 1.0f;
						f24 = 0.0f;
						f00 = 0;
					} else {
						f32 f0 = sqrtf(rot.f[0] * rot.f[0] + rot.f[1] * rot.f[1] + rot.f[2] * rot.f[2]);

						f26 = rot.f[0] / f0;
						f30 = rot.f[1] / f0;
						f28 = rot.f[2] / f0;

						f0 = sqrtf(f26 * f26 + f28 * f28);
						f2 = f26 / f0;

						f22 = f28 / f0;
						spcc = f30 * f2;
						spc8 = -f0;
						spc4 = f30 * f22;
						f24 = -f2;
						f00 = 0;
					}

					vertices = gfxAllocateVertices(4);

					vertices[3].colour = 0;
					vertices[2].colour = 0;
					vertices[1].colour = 0;
					vertices[0].colour = 0;

					vertices[0].s = 0;
					vertices[0].t = 0;
					vertices[1].s = 512;
					vertices[1].t = 0;
					vertices[2].s = 512;
					vertices[2].t = 512;
					vertices[3].s = 0;
					vertices[3].t = 512;

					vertices[0].x = pos.f[0] + (-f20 * f22) + (f20 * spcc);
					vertices[0].y = pos.f[1] + (-f20 * f00) + (f20 * spc8);
					vertices[0].z = pos.f[2] + (-f20 * f24) + (f20 * spc4);

					vertices[1].x = pos.f[0] + (f20 * f22) + (f20 * spcc);
					vertices[1].y = pos.f[1] + (f20 * f00) + (f20 * spc8);
					vertices[1].z = pos.f[2] + (f20 * f24) + (f20 * spc4);

					vertices[2].x = pos.f[0] + (f20 * f22) + (-f20 * spcc);
					vertices[2].y = pos.f[1] + (f20 * f00) + (-f20 * spc8);
					vertices[2].z = pos.f[2] + (f20 * f24) + (-f20 * spc4);

					vertices[3].x = pos.f[0] + (-f20 * f22) + (-f20 * spcc);
					vertices[3].y = pos.f[1] + (-f20 * f00) + (-f20 * spc8);
					vertices[3].z = pos.f[2] + (-f20 * f24) + (-f20 * spc4);

					gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

					gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
				}
			}
		}
	}

	return gdl;
}

Gfx *lasersightRenderBeam(Gfx *gdl)
{
	u32 stack;
	struct player *player = g_Vars.currentplayer;
	Mtxf *mtx;
	s32 i;
	Mtxf sp198;
	struct coord campos;
	Mtxf sp14c;
	Mtxf sp10c;

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetCombineMode(gdl++, G_CC_BLENDIA, G_CC_BLENDIA);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, &g_TexGeneralConfigs[3], 4, 0, 2, 1, NULL);
	mtx4LoadIdentity(&sp14c);

	mtx00015be0(camGetWorldToScreenMtxf(), &sp14c);
	mtx4LoadIdentity(&sp10c);
	mtx00015be0(camGetProjectionMtxF(), &sp10c);

	sp10c.m[3][1] = 0;
	sp10c.m[3][0] = 0;
	sp10c.m[3][2] = 0;

	mtx4LoadIdentity(&sp198);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp198);

	campos.x = player->cam_pos.x;
	campos.y = player->cam_pos.y;
	campos.z = player->cam_pos.z;

	sp198.m[3][0] = 0;
	sp198.m[3][1] = 0;
	sp198.m[3][2] = 0;

	mtx00015f88(0.2f, &sp198);
	mtx = gfxAllocateMatrix();
	mtxF2L(&sp198, mtx);

	gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	for (i = 0; i < ARRAYCOUNT(g_LaserSights); i++) {
		if (g_LaserSights[i].id != -1) {
			Col *colours;
			struct coord spcc;
			struct coord spc0;
			struct coord spb4;
			struct coord spa8;
			Vtx *vertices;
			struct coord sp98;

			sp98.x = g_LaserSights[i].beamnear.x;
			sp98.y = g_LaserSights[i].beamnear.y;
			sp98.z = g_LaserSights[i].beamnear.z;

			mtx4TransformVecInPlace(&sp14c, &sp98);

			spa8.x = sp98.f[0] < 0.0f ? 1.0f : -1.0f;
			spa8.y = 2.0f;
			spa8.z = 0.0f;

			guNormalize(&spa8.x, &spa8.y, &spa8.z);

			mtx4RotateVecInPlace(&sp10c, &spa8);

			spcc.x = g_LaserSights[i].beamnear.x;
			spcc.y = g_LaserSights[i].beamnear.y;
			spcc.z = g_LaserSights[i].beamnear.z;

			spcc.x = (spcc.x - campos.x) * 5.0f;
			spcc.y = (spcc.y - campos.y) * 5.0f;
			spcc.z = (spcc.z - campos.z) * 5.0f;

			spc0.x = g_LaserSights[i].beamfar.x;
			spc0.y = g_LaserSights[i].beamfar.y;
			spc0.z = g_LaserSights[i].beamfar.z;

			spc0.x = (spc0.x - campos.f[0]) * 5.0f;
			spc0.y = (spc0.y - campos.f[1]) * 5.0f;
			spc0.z = (spc0.z - campos.f[2]) * 5.0f;

			spb4.x = spc0.f[0] - spcc.x;
			spb4.y = spc0.f[1] - spcc.y;
			spb4.z = spc0.f[2] - spcc.z;

			guNormalize(&spb4.x, &spb4.y, &spb4.z);

			colours = gfxAllocateColours(2);

			colours[0].word = PD_BE32(0xff00005f);
			colours[1].word = PD_BE32(0xff00000f);

			gSPColor(gdl++, osVirtualToPhysical(colours), 2);

			vertices = gfxAllocateVertices(6);

			vertices[0].colour = 0;
			vertices[1].colour = 0;
			vertices[2].colour = 0;
			vertices[3].colour = 0;
			vertices[4].colour = 4;
			vertices[5].colour = 4;

			vertices[0].s = 0;
			vertices[0].t = 0;
			vertices[1].s = 0;
			vertices[1].t = 256;
			vertices[2].s = 32;
			vertices[2].t = 0;
			vertices[3].s = 32;
			vertices[3].t = 256;
			vertices[4].s = 0;
			vertices[4].t = 0;
			vertices[5].s = 0;
			vertices[5].t = 256;

			vertices[0].x = spcc.f[0] - spa8.f[0] * 15.0f;
			vertices[0].y = spcc.f[1] - spa8.f[1] * 15.0f;
			vertices[0].z = spcc.f[2] - spa8.f[2] * 15.0f;

			vertices[1].x = spcc.f[0] + spa8.f[0] * 15.0f;
			vertices[1].y = spcc.f[1] + spa8.f[1] * 15.0f;
			vertices[1].z = spcc.f[2] + spa8.f[2] * 15.0f;

			vertices[2].x = spcc.f[0] + (200 * spb4.f[0]) - (spa8.f[0] * 15.0f);
			vertices[2].y = spcc.f[1] + (200 * spb4.f[1]) - (spa8.f[1] * 15.0f);
			vertices[2].z = spcc.f[2] + (200 * spb4.f[2]) - (spa8.f[2] * 15.0f);

			vertices[3].x = spcc.f[0] + (200 * spb4.f[0]) + (spa8.f[0] * 15.0f);
			vertices[3].y = spcc.f[1] + (200 * spb4.f[1]) + (spa8.f[1] * 15.0f);
			vertices[3].z = spcc.f[2] + (200 * spb4.f[2]) + (spa8.f[2] * 15.0f);

			vertices[4].x = spcc.f[0] + (400 * spb4.f[0]) - (spa8.f[0] * 15.0f);
			vertices[4].y = spcc.f[1] + (400 * spb4.f[1]) - (spa8.f[1] * 15.0f);
			vertices[4].z = spcc.f[2] + (400 * spb4.f[2]) - (spa8.f[2] * 15.0f);

			vertices[5].x = spcc.f[0] + (400 * spb4.f[0]) + (spa8.f[0] * 15.0f);
			vertices[5].y = spcc.f[1] + (400 * spb4.f[1]) + (spa8.f[1] * 15.0f);
			vertices[5].z = spcc.f[2] + (400 * spb4.f[2]) + (spa8.f[2] * 15.0f);

			gSPVertex(gdl++, osVirtualToPhysical(vertices), 6, 0);

			gSPTri4(gdl++, 0, 1, 2, 2, 3, 1, 2, 3, 5, 2, 5, 4);
		}
	}

	return gdl;
}

void lasersightSetBeam(s32 id, s32 arg1, struct coord *near, struct coord *far)
{
	s32 i;

	if (!lasersightExists(id, &i)) {
		if (i == -1) {
			return;
		}

		g_LaserSights[i].id = id;
	}

	g_LaserSights[i].unk04.x = near->x;
	g_LaserSights[i].unk04.y = near->y;
	g_LaserSights[i].unk04.z = near->z;

	g_LaserSights[i].beamnear.x = near->x;
	g_LaserSights[i].beamnear.y = near->y;
	g_LaserSights[i].beamnear.z = near->z;

	g_LaserSights[i].beamfar.x = far->x;
	g_LaserSights[i].beamfar.y = far->y;
	g_LaserSights[i].beamfar.z = far->z;

	g_LaserSights[i].unk44 = arg1;
	g_LaserSights[i].unk28 = 0;
}

void lasersightSetDot(s32 arg0, struct coord *pos, struct coord *rot)
{
	s32 i;

	if (lasersightExists(arg0, &i)) {
		g_LaserSights[i].unk28 += 1.0f;

		g_LaserSights[i].dotpos.x = pos->x;
		g_LaserSights[i].dotpos.y = pos->y;
		g_LaserSights[i].dotpos.z = pos->z;

		g_LaserSights[i].dotrot.x = rot->x;
		g_LaserSights[i].dotrot.y = rot->y;
		g_LaserSights[i].dotrot.z = rot->z;
	}
}

void lasersightFree(s32 arg0)
{
	s32 i;

	if (lasersightExists(arg0, &i)) {
		g_LaserSights[i].id = -1;
	}
}
