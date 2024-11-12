#include <ultra64.h>
#include "constants.h"
#include "game/game_006900.h"
#include "game/tex.h"
#include "game/stars.h"
#include "game/game_1531a0.h"
#include "game/camera.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

s32 g_StarCount;
s8 *g_StarPositions = NULL;
f32 *g_StarData3;
s32 g_StarGridSize;
s32 *g_StarPosIndexes;

bool g_StarsBelowHorizon = false;

void stars0f135c70(void)
{
	u32 stack[4];
	struct coord coord;
	f32 mult;
	s32 i;
	s32 j;
	s32 k;
	f32 tmp = g_StarGridSize * 0.5f;

	for (i = 0; i < 6; i++) {
		for (j = 0; j <= g_StarGridSize; j++) {
			for (k = 0; k <= g_StarGridSize; k++) {
				s32 index = ((i * (g_StarGridSize + 1) * (g_StarGridSize + 1)) + k + (j * (g_StarGridSize + 1))) * 3;

				switch (i) {
				case 0:
				case 1:
					coord.x = (i == 0 ? -1.0f : 1.0f);
					coord.y = k / tmp - 1;
					coord.z = j / tmp - 1;
					break;
				case 2:
				case 3:
					coord.y = (i == 2 ? -1.0f : 1.0f);
					coord.x = j / tmp - 1;
					coord.z = k / tmp - 1;
					break;
				case 4:
				case 5:
					coord.z = (i == 4 ? -1.0f : 1.0f);
					coord.x = k / tmp - 1;
					coord.y = j / tmp - 1;
					break;
				}

				mult = 1.0f / sqrtf(coord.f[0] * coord.f[0] + coord.f[1] * coord.f[1] + coord.f[2] * coord.f[2]);

				g_StarData3[index + 0] = coord.x * mult;
				g_StarData3[index + 1] = coord.y * mult;
				g_StarData3[index + 2] = coord.z * mult;
			}
		}
	}
}

/**
 * Insert a star position *after* the given index.
 */
void starInsert(s32 index, struct coord *arg1)
{
	s32 i;

	// Shuffle g_StarPositions forward after the insertion point
	for (i = g_StarPosIndexes[g_StarGridSize * 6 * g_StarGridSize] - 1; i >= g_StarPosIndexes[index + 1]; i--) {
		g_StarPositions[i * 3 + 3] = g_StarPositions[i * 3 + 0];
		g_StarPositions[i * 3 + 4] = g_StarPositions[i * 3 + 1];
		g_StarPositions[i * 3 + 5] = g_StarPositions[i * 3 + 2];
	}

	// Write new data
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 0] = arg1->x * 127;
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 1] = arg1->y * 127;
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 2] = arg1->z * 127;

	// Increment indexes after the insertion point
	for (i = index + 1; i <= g_StarGridSize * 6 * g_StarGridSize; i++) {
		g_StarPosIndexes[i]++;
	}
}

#define ABS2(value) ((value) < 0 ? -(value) : (value))

void starsReset(void)
{
	s32 v0;
	s32 v1;
	struct coord spd4;
	struct coord spc8;
	s32 i;
	f32 spc0;
	f32 spbc;
	f32 stack[1];
	s32 count;
	s32 spb0;
	f32 f0;
	s32 tmp;
	s32 tmp1;
	s32 tmp2;

	g_StarPositions = NULL;

	if (PLAYERCOUNT() < 2) {
		g_StarsBelowHorizon = false;
		g_StarGridSize = 3;

		if (g_Vars.stagenum == STAGE_TEST_OLD) {
			g_StarsBelowHorizon = true;
			g_StarCount = 1600;
		} else if (g_Vars.stagenum == STAGE_DEFECTION || g_Vars.stagenum == STAGE_EXTRACTION) {
			g_StarCount = 200;
			g_StarGridSize = 2;
		} else if (g_Vars.stagenum == STAGE_ATTACKSHIP) {
			g_StarsBelowHorizon = true;
			g_StarCount = 1200;
		} else {
			g_StarCount = 200;
			g_StarGridSize = 2;
		}

		tmp = g_StarGridSize + 1;
		g_StarPositions = mempAlloc(ALIGN64(g_StarCount * 3U + tmp * 72 * tmp + 6 * g_StarGridSize * g_StarGridSize * 4U + 4), MEMPOOL_STAGE);

		if (g_StarPositions != NULL) {
			g_StarPosIndexes = (s32 *)(g_StarPositions + g_StarCount * 3);

			for (i = 0; i < (6 * g_StarGridSize * g_StarGridSize + 1); i++) {
				g_StarPosIndexes[i] = 0;
			}

			count = 6 * g_StarGridSize * g_StarGridSize + 1;
			g_StarData3 = (f32 *)(count * sizeof(f32) + (uintptr_t)g_StarPosIndexes);

			stars0f135c70();

			for (i = 0; i < g_StarCount; i++) {
				spd4.f[0] = 2.0f * RANDOMFRAC() - 1.0f;
				spd4.f[1] = g_StarsBelowHorizon ? 2.0f * RANDOMFRAC() - 1.0f : RANDOMFRAC();
				spd4.f[2] = 2.0f * RANDOMFRAC() - 1.0f;

				guNormalize(&spd4.f[0], &spd4.f[1], &spd4.f[2]);

				f0 = (ABS2(spd4.f[0]) > ABS2(spd4.f[1])) ? (ABS2(spd4.f[0]) > ABS2(spd4.f[2]) ? ABS2(spd4.f[0]) : ABS2(spd4.f[2])) : (ABS2(spd4.f[1]) > ABS2(spd4.f[2]) ? ABS2(spd4.f[1]) : ABS2(spd4.f[2]));

				spc8.f[0] = spd4.f[0] / f0;
				spc8.f[1] = spd4.f[1] / f0;
				spc8.f[2] = spd4.f[2] / f0;

				tmp1 = g_StarGridSize * g_StarGridSize;

				if (spc8.f[0] == 1 || spc8.f[0] == -1) {
					spb0 = spc8.f[0] == -1 ? 0 : 1;
					spc0 = spc8.f[1];
					spbc = spc8.f[2];
				} else if (spc8.f[1] == 1 || spc8.f[1] == -1) {
					spb0 = spc8.f[1] == -1 ? 2 : 3;
					spc0 = spc8.f[2];
					spbc = spc8.f[0];
				} else if (spc8.f[2] == 1 || spc8.f[2] == -1) {
					spb0 = spc8.f[2] == -1 ? 4 : 5;
					spc0 = spc8.f[0];
					spbc = spc8.f[1];
				} else {
					// empty
				}

				v0 = (spc0 + 1) / 2 * g_StarGridSize;
				v1 = (spbc + 1) / 2 * g_StarGridSize;

				if (v0 == g_StarGridSize) {
					v0--;
				}

				if (v1 == g_StarGridSize) {
					v1--;
				}

				tmp2 = v0 + g_StarGridSize * v1;

				starInsert(spb0 * tmp1 + tmp2, &spd4);
			}
		}
	}
}

Gfx *starsRender(Gfx *gdl)
{
	bool isddtower = false;
	Mtxf mtx;
	f32 viewleft = viGetViewLeft();
	f32 viewright = viewleft + viGetViewWidth();
	f32 viewtop = viGetViewTop();
	f32 viewbottom = viewtop + viGetViewHeight();
	s32 i;
	f32 sp154;
	struct coord sp148;
	f32 screenmidx = g_Vars.currentplayer->c_screenleft + g_Vars.currentplayer->c_halfwidth;
	f32 screenmidy = g_Vars.currentplayer->c_screentop + g_Vars.currentplayer->c_halfheight;
	s32 j;
	s32 k;
	s32 l;
	u32 stack;
	s32 tmp;
	u32 colours[4];

	if (g_StarPositions == NULL) {
		return gdl;
	}

	if (g_Vars.stagenum == STAGE_DEFECTION || g_Vars.stagenum == STAGE_EXTRACTION) {
		isddtower = true;
	}

	colours[0] = colourBlend(0xffffff7f, 0x7777777f, menuGetSinOscFrac(2) * 255);
	colours[1] = colourBlend(0x0000aa7f, 0x2222ff7f, menuGetSinOscFrac(4) * 255);
	colours[2] = colourBlend(0x0000ff7f, 0x5555ff7f, menuGetCosOscFrac(2) * 255);
	colours[3] = colourBlend(0xaaaaff7f, 0x7777ff7f, menuGetCosOscFrac(4) * 255);

	if (isddtower) {
		for (i = 0; i < 3; i++) {
			// Nothing is done with the return value here, so this has no
			// effect. Maybe the original code incorrectly did a comparison
			// instead of an assign? eg. colours[i] == colourBlend(...)
			// Doing this would make the stars more transparent.
			colourBlend(colours[i], colours[i] & 0xff, 0x5f);
		}
	}

	sp154 = cosf(0.017453199252486f * (90.0f - viGetFovY() / viGetAspect() * 0.5f));

	mtx4LoadIdentity(&mtx);
	mtx00015be0(camGetWorldToScreenMtxf(), &mtx);

	mtx.m[3][0] = 0.0f;
	mtx.m[3][1] = 0.0f;
	mtx.m[3][2] = 0.0f;

	mtx00015f88(262.9f, &mtx);

	mtx.m[0][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx.m[1][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx.m[2][1] *= g_Vars.currentplayer->c_recipscaley;

	mtx.m[0][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx.m[1][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx.m[2][0] *= g_Vars.currentplayer->c_recipscalex;

	sp148.f[0] = g_Vars.currentplayer->cam_look.f[0];
	sp148.f[1] = g_Vars.currentplayer->cam_look.f[1];
	sp148.f[2] = g_Vars.currentplayer->cam_look.f[2];

	gdl = textSetPrimColour(gdl, 0xffffffff);

	gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);

	for (i = 0; i < 6; i++) {
		if (g_StarsBelowHorizon || i != 2) {
			f32 f0;
			f32 f0_2;
			bool spd0[4][4];
			struct coord spc4;

			for (j = 0; j <= g_StarGridSize; j++) {
				for (k = 0; k <= g_StarGridSize; k++) {
					tmp = ((g_StarGridSize + 1) * i * (g_StarGridSize + 1) + k + j * (g_StarGridSize + 1)) * 3;
					f0 = sp148.f[0] * g_StarData3[tmp] + sp148.f[1] * g_StarData3[tmp + 1] + sp148.f[2] * g_StarData3[tmp + 2];

					if (f0 <= sp154) {
						spd0[k][j] = true;
					} else {
						spd0[k][j] = false;
					}
				}
			}

			for (j = 0; j < g_StarGridSize; j++) {
				for (k = 0; k < g_StarGridSize; k++) {
					if (spd0[k][j] == 0 || spd0[k + 1][j] == 0 || spd0[k][j + 1] == 0 || spd0[k + 1][j + 1] == 0) {
						s32 tmp = g_StarGridSize * g_StarGridSize * i + k + j * g_StarGridSize;
						s32 colourindex = 0;
						f32 screenpos[2];
						s32 drawpos[2];
						s32 nextgroupstart = g_StarPosIndexes[tmp];
						s32 groupsize = (g_StarPosIndexes[tmp + 1] - g_StarPosIndexes[tmp]) / 4 + 1;
						s8 *pos = &g_StarPositions[g_StarPosIndexes[tmp] * 3];

						for (l = g_StarPosIndexes[tmp]; l < g_StarPosIndexes[tmp + 1]; l++) {
							if (nextgroupstart == l) {
								gDPSetPrimColorViaWord(gdl++, 0, 0, colours[colourindex]);

								colourindex++;
								nextgroupstart += groupsize;
							}

							spc4.f[0] = pos[0];
							spc4.f[1] = pos[1];
							spc4.f[2] = pos[2];
							pos += 3;

							f0_2 = 1.0f / (mtx.m[0][2] * spc4.f[0] + mtx.m[1][2] * spc4.f[1] + mtx.m[2][2] * spc4.f[2]);
							screenpos[1] = screenmidy + (mtx.m[0][1] * spc4.f[0] + mtx.m[1][1] * spc4.f[1] + mtx.m[2][1] * spc4.f[2]) * f0_2;

							if (screenpos[1] > viewtop && screenpos[1] < viewbottom) {
								screenpos[0] = screenmidx - (mtx.m[0][0] * spc4.f[0] + mtx.m[1][0] * spc4.f[1] + mtx.m[2][0] * spc4.f[2]) * f0_2;

								if (screenpos[0] > viewleft && screenpos[0] < viewright) {
									drawpos[0] = screenpos[0];
									drawpos[1] = screenpos[1];

									gDPFillRectangle(gdl++, drawpos[0], drawpos[1], drawpos[0] + 1, drawpos[1] + 1);
								}
							}
						}
					}
				}
			}
		}
	}

	gdl = text0f153838(gdl);

	return gdl;
}
