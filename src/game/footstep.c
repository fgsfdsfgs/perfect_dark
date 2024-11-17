#include <ultra64.h>
#include "constants.h"
#include "game/debug.h"
#include "game/game_006900.h"
#include "game/lv.h"
#include "game/propsnd.h"
#include "game/bg.h"
#include "bss.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

s32 g_FootstepSounds[] = {
	/* none   */ -1,                -1,                -1,                -1,                -1,                -1,                -1,                -1,
	/* wood   */ SFX_FOOTSTEP_80DC, SFX_FOOTSTEP_80DD, SFX_FOOTSTEP_80E0, SFX_FOOTSTEP_80E1, SFX_FOOTSTEP_80DE, SFX_FOOTSTEP_80DF, SFX_FOOTSTEP_80E2, SFX_FOOTSTEP_80E3,
	/* stone  */ SFX_FOOTSTEP_80C4, SFX_FOOTSTEP_80C5, SFX_FOOTSTEP_80C8, SFX_FOOTSTEP_80C9, SFX_FOOTSTEP_80C6, SFX_FOOTSTEP_80C7, SFX_FOOTSTEP_80CA, SFX_FOOTSTEP_80CB,
	/* carpet */ SFX_FOOTSTEP_80E6, SFX_FOOTSTEP_80E7, SFX_FOOTSTEP_80EA, SFX_FOOTSTEP_80EB, SFX_FOOTSTEP_80E8, SFX_FOOTSTEP_80E9, SFX_FOOTSTEP_80EC, SFX_FOOTSTEP_80ED,
	/* metal  */ SFX_FOOTSTEP_80D4, SFX_FOOTSTEP_80D5, SFX_FOOTSTEP_80D8, SFX_FOOTSTEP_80D9, SFX_FOOTSTEP_80D6, SFX_FOOTSTEP_80D7, SFX_FOOTSTEP_80DA, SFX_FOOTSTEP_80DB,
	/* mud    */ SFX_FOOTSTEP_80EE, SFX_FOOTSTEP_80EF, SFX_FOOTSTEP_80F2, SFX_FOOTSTEP_80F3, SFX_FOOTSTEP_80F0, SFX_FOOTSTEP_80F1, SFX_FOOTSTEP_80F4, SFX_FOOTSTEP_80F5,
	/* water  */ SFX_FOOTSTEP_80E4, SFX_FOOTSTEP_80E5, SFX_FOOTSTEP_80E4, SFX_FOOTSTEP_80E5, SFX_FOOTSTEP_80E4, SFX_FOOTSTEP_80E5, SFX_FOOTSTEP_80E4, SFX_FOOTSTEP_80E5,
	/* dirt   */ SFX_FOOTSTEP_80CC, SFX_FOOTSTEP_80CD, SFX_FOOTSTEP_80D0, SFX_FOOTSTEP_80D1, SFX_FOOTSTEP_80CE, SFX_FOOTSTEP_80CF, SFX_FOOTSTEP_80D2, SFX_FOOTSTEP_80D3,
	/* snow   */ SFX_FOOTSTEP_8187, SFX_FOOTSTEP_8188, SFX_FOOTSTEP_818B, SFX_FOOTSTEP_818C, SFX_FOOTSTEP_8189, SFX_FOOTSTEP_818A, SFX_FOOTSTEP_818D, SFX_FOOTSTEP_818E,
};

struct footstepframe {
	u16 animnum;
	u8 frame1;
	u8 frame2;
};

struct footstepframe g_FootstepAnims[] = {
	{ ANIM_002B,               0x08, 0x19 },
	{ ANIM_0029,               0x05, 0x0e },
	{ ANIM_006B,               0x08, 0x19 },
	{ ANIM_0028,               0x1b, 0x08 },
	{ ANIM_RUNNING_TWOHANDGUN, 0x12, 0x06 },
	{ ANIM_0052,               0x08, 0x19 },
	{ ANIM_0053,               0x19, 0x08 },
	{ ANIM_0054,               0x19, 0x08 },
	{ ANIM_0055,               0x07, 0x12 },
	{ ANIM_0056,               0x07, 0x12 },
	{ ANIM_0057,               0x12, 0x07 },
	{ ANIM_0058,               0x0f, 0x05 },
	{ ANIM_RUNNING_ONEHANDGUN, 0x08, 0x14 },
	{ ANIM_005A,               0x06, 0x0f },
	{ ANIM_006C,               0x19, 0x08 },
	{ ANIM_006D,               0x19, 0x08 },
	{ ANIM_006E,               0x08, 0x13 },
	{ ANIM_006F,               0x15, 0x08 },
	{ ANIM_0070,               0x0f, 0x05 },
	{ ANIM_0071,               0x0f, 0x05 },
	{ ANIM_0072,               0x17, 0x08 },
	{ ANIM_0073,               0x08, 0x13 },
	{ ANIM_0093,               0x17, 0x0a },
	{ ANIM_0094,               0x0f, 0x05 },
	{ ANIM_005F,               0x0e, 0x01 },
	{ ANIM_0016,               0x1d, 0x0a },
	{ ANIM_0018,               0x18, 0x2e },
	{ ANIM_001B,               0x0a, 0x1c },
	{ ANIM_001D,               0x0d, 0x02 },
	{ ANIM_001E,               0x0c, 0x01 },
	{ ANIM_005C,               0x13, 0x2a },
	{ ANIM_005D,               0x0f, 0x05 },
	{ ANIM_005E,               0x04, 0x0c },
	{ ANIM_0392,               0x05, 0x14 },
	{ ANIM_SKEDAR_RUNNING,     0x00, 0x00 },
};

bool footstepIsRunning(s32 animnum)
{
	switch (animnum) {
	case ANIM_001D:
	case ANIM_001E:
	case ANIM_0029:
	case ANIM_RUNNING_TWOHANDGUN:
	case ANIM_0055:
	case ANIM_0056:
	case ANIM_0057:
	case ANIM_0058:
	case ANIM_RUNNING_ONEHANDGUN:
	case ANIM_005A:
	case ANIM_005D:
	case ANIM_005E:
	case ANIM_005F:
	case ANIM_006E:
	case ANIM_006F:
	case ANIM_0070:
	case ANIM_0071:
	case ANIM_0073:
	case ANIM_0093:
	case ANIM_0094:
	case ANIM_SKEDAR_RUNNING:
		return true;
	}

	return false;
}

s32 footstepChooseSound(struct chrdata *chr, s32 footstepindex)
{
	s32 floortype;
	s32 running;
	s32 rand;
	s32 index;

	if (chr->footstep == 0) {
		return 0;
	}

	floortype = chr->floortype <= FLOORTYPE_SNOW ? chr->floortype : 0;

	if (floortype == -1) {
		return 0;
	}

	if (CHRRACE(chr) == RACE_SKEDAR && chr->bodynum != BODY_MINISKEDAR) {
		u32 result;
		chr->lastfootsample ^= 1;

		if (floortype == FLOORTYPE_METAL) {
			if (chr->lastfootsample) {
				result = SFX_FOOTSTEP_8191;
			} else {
				result = SFX_FOOTSTEP_8192;
			}
		} else {
			if (chr->lastfootsample) {
				result = SFX_FOOTSTEP_818F;
			} else {
				result = SFX_FOOTSTEP_8190;
			}
		}

		return result;
	}

	running = footstepIsRunning(g_FootstepAnims[footstepindex].animnum);

	do {
		rand = rngRandom() % 8;
		index = (running ? 2 : 0) + (rand & 5) + floortype * 8;
	} while (index == chr->lastfootsample);

	chr->lastfootsample = index;

	return g_FootstepSounds[index];
}

/**
 * Check if a footstep sound should play for the given chr and play it if so.
 *
 * It is assumed that the chr is moving normally and not via magic.
 */
void footstepCheckDefault(struct chrdata *chr)
{
	if (debugIsFootstepsEnabled() && PLAYERCOUNT() == 1 && chr) {
		chr->footstep = 0;
		chr->magicanim = -1;
		chr->magicframe = 0;

		if (chr->model && chr->model->anim) {
			s32 soundnum;
			f32 prevframe = chr->oldframe;
			f32 frame = chr->model->anim->frame;
			s32 i;

			chr->oldframe = frame;

			for (i = 0; i < ARRAYCOUNT(g_FootstepAnims); i++) {
				if (modelGetAnimNum(chr->model) == g_FootstepAnims[i].animnum) {
					if (CHRRACE(chr) == RACE_SKEDAR && g_FootstepAnims[i].animnum == ANIM_SKEDAR_RUNNING) {
						if ((frame >= 2 && prevframe < 2) || (frame >= 17 && prevframe < 17)) { \
							chr->footstep = 1;
						} else if ((frame >= 10 && prevframe < 10) || (frame >= 25 && prevframe < 25)) {
							chr->footstep = 2;
						}
					} else {
#ifndef PLATFORM_N64
						// fix spamming footsteps at >= 60fps
						if (g_Vars.lvupdate240 < 4) {
							const s32 roundedframe = (s32)(frame * (4.f / (f32)g_Vars.lvupdate240));
							if (roundedframe == (g_FootstepAnims[i].frame1*2) && prevframe < g_FootstepAnims[i].frame1) {
								chr->footstep = 1;
							} else if (roundedframe == (g_FootstepAnims[i].frame2*2) && prevframe < g_FootstepAnims[i].frame2) {
								chr->footstep = 2;
							}
						} else
#endif
						if (frame >= g_FootstepAnims[i].frame1 && prevframe < g_FootstepAnims[i].frame1) {
							chr->footstep = 1;
						} else if (frame >= g_FootstepAnims[i].frame2 && prevframe < g_FootstepAnims[i].frame2) {
							chr->footstep = 2;
						}
					}

					soundnum = footstepChooseSound(chr, i);

					if (soundnum != -1 && chr->footstep != 0) {
#if VERSION >= VERSION_NTSC_1_0
						psCreate(NULL, chr->prop, soundnum, -1, -1, PSFLAG_0400, 0, PSTYPE_FOOTSTEP, NULL, -1, NULL, -1, -1, -1, -1);
#else
						psCreate(NULL, chr->prop, soundnum, -1, -1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
#endif
					}

					chr->magicanim = i;
#if VERSION >= VERSION_PAL_BETA
					chr->magicspeed = chr->model->anim->speed;
#else
					chr->magicspeed = chr->model->anim->speed * 0.25f;
#endif
					return;
				}
			}
		}
	}
}

/**
 * Assuming the given chr is moving using magic, check if a footstep sound
 * should play and play it if so.
 *
 * The "magic" method of movement applies when the chr is off-screen.
 * It uses a computationally simple approach to movement.
 */
void footstepCheckMagic(struct chrdata *chr)
{
	s32 index;
	f32 frame;
	f32 prevframe;
	struct prop *playerprop = g_Vars.currentplayer->prop;
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;
	s32 soundnum;

	if (debugIsFootstepsEnabled() && PLAYERCOUNT() == 1 && chr->magicanim >= 0) {
#if VERSION >= VERSION_PAL_BETA
		chr->magicframe += g_Vars.lvupdate60freal * chr->magicspeed;
#else
		chr->magicframe += g_Vars.lvupdate240 * chr->magicspeed;
#endif

		if (chr->prop) {
			xdiff = playerprop->pos.x - chr->prop->pos.x;
			ydiff = playerprop->pos.y - chr->prop->pos.y;

			if (ydiff < 0.0f) {
				ydiff = -ydiff;
			}

			zdiff = playerprop->pos.z - chr->prop->pos.z;

			frame = chr->magicframe;
			index = chr->magicanim;
			prevframe = chr->oldframe;

			chr->footstep = 0;
			chr->oldframe = frame;

			if (ydiff < 250 && xdiff * xdiff + zdiff * zdiff < 3000000) {
				if (CHRRACE(chr) == RACE_SKEDAR && g_FootstepAnims[index].animnum == ANIM_SKEDAR_RUNNING) {
					if ((frame >= 2 && prevframe < 2) || (frame >= 17 && prevframe < 17)) {
						chr->footstep = 1;
					} else if ((frame >= 10 && prevframe < 10) || (frame >= 25 && prevframe < 25)) {
						chr->footstep = 2;
					}
				} else {
					if (frame >= g_FootstepAnims[index].frame1 && prevframe < g_FootstepAnims[index].frame1) {
						chr->footstep = 1;
					} else if (frame >= g_FootstepAnims[index].frame2 && prevframe < g_FootstepAnims[index].frame2) {
						chr->footstep = 2;
					}
				}

				soundnum = footstepChooseSound(chr, index);

				if (soundnum != -1 && chr->footstep != 0) {
#if VERSION >= VERSION_NTSC_1_0
					psCreate(NULL, chr->prop, soundnum, -1,
							-1, PSFLAG_0400, 0, PSTYPE_FOOTSTEP, NULL, -1, NULL, -1, -1, -1, -1);
#else
					psCreate(NULL, chr->prop, soundnum, -1,
							-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
#endif
				}
			}

			if (footstepIsRunning(g_FootstepAnims[index].animnum)) {
				if (chr->magicframe > 22) {
					chr->magicframe -= 22;
				}
			} else {
				if (chr->magicframe > 34) {
					chr->magicframe -= 34;
				}
			}
		}
	}
}
