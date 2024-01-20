#include <ultra64.h>
#include "constants.h"
#include "game/camera.h"
#include "game/game_006900.h"
#include "game/title.h"
#include "game/tex.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/credits.h"
#include "game/game_1531a0.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/lang.h"
#include "bss.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "lib/rng.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"

/**
 * Credits
 *
 * This file handles both the credits sequence and the alternative title screen.
 *
 * For the alternative title screen, title code calls creditsRequestAltTitle
 * before switching the stage to STAGE_CREDITS. For the proper credits, the
 * stage is switched to STAGE_CREDITS without calling creditsRequestAltTitle.
 *
 * Credits are then initialised using creditsReset, then called on each tick
 * using creditsDraw. creditsTick is called from creditsDraw.
 *
 * For the main part of the credits, the scene is composed of:
 * - Two visible background layers
 * - Up to 500 particles that move around on screen
 * - Text shadows/repetitions
 * - Text foreground
 *
 * Once the credits are finished, the second scene begins. The second scene is
 * composed of:
 * - Two visible background layers
 * - Up to 500 particles that move around on screen
 * - The words "PERFECT DARK" scrolling slowly from right to left
 *
 * The second scene loops continuously.
 *
 * For the alternative title, the sequence is started at the second scene.
 *
 * The credits themselves are grouped into slides. Only one slide is shown on
 * screen at a time. Each slide can contain up to four credits, and each credit
 * can contain one or two phrases of text. Credits which contain two phrases can
 * have their phrases displayed side by side or one above the other depending on
 * the credit's configuration.
 */

/**
 * The retain values are transition settings for credits. Most credits only
 * appear on a single slide, so RETAIN_NONE is used for those which makes them
 * transition in and out at the start and end of that slide.
 *
 * For credits that span multiple slides, the credit is repeated on each slide.
 * On the first slide it should use RETAIN OUT, on inner slides use RETAIN_BOTH
 * and on the final slide use RETAIN_IN.
 */

#define RETAIN_NONE 0
#define RETAIN_OUT  1
#define RETAIN_IN   2
#define RETAIN_BOTH 3

#define HDIR_RIGHT 0
#define HDIR_MID   1
#define HDIR_LEFT  2

#define VDIR_BOTTOM 0
#define VDIR_MID    1
#define VDIR_TOP    2

#define MOVETYPE_LINEAR 0
#define MOVETYPE_WAVE   1
#define MOVETYPE_SPIRAL 2
#define MOVETYPE_FLING  3
#define MOVETYPE_TWIST  4
#define MOVETYPE_FALL   99

#define STRETCHTYPE_HORIZONTAL        0
#define STRETCHTYPE_VERTICAL          1
#define STRETCHTYPE_HORIZONTAL_SQUASH 2
#define STRETCHTYPE_VERTICAL_SQUASH   3

#define NUM_CORE_TEAM 16

struct particle {
	f32 x;
	f32 y;
	f32 z;
	f32 rotation;
	u8 movetype;
	u8 confignum;
	u8 unk12;
	u8 size;
	u8 colourindex;
};

struct creditsbglayer {
	s32 type;
	f32 rotspeed;
	f32 panspeed;
	s32 confignum;
};

struct creditsdata {
	/*0x0000*/ u32 unk0000;
	/*0x0004*/ u32 unk0004;
	/*0x0008*/ u32 unk0008;
	/*0x000c*/ struct particle particles[500];
	/*0x2eec*/ u8 unk2eec;
	/*0x2ef0*/ struct menumodel menumodel;
	/*0x34a8*/ u8 unk34a8[0xcac];

	/**
	 * Credit indexes 1 through 16 are randomised. coreteammap stores the chosen
	 * order. The index is the apparent credit index and the value is the real
	 * credit index that it maps to.
	 */
	/*0x4154*/ u32 coreteammap[NUM_CORE_TEAM + 1];

	/**
	 * creditnum is the credit index of the first credit being displayed on the
	 * current slide. It's usually a title.
	 *
	 * However, if creditnum is between 1 and 16 then this value gets mapped
	 * through the coreteammap, so its value won't actually correspond with the
	 * credit being displayed.
	 */
	/*0x4198*/ s32 creditnum;

	/*0x419c*/ u8 numthisslide;
	/*0x41a0*/ f32 slideage; // age in seconds
	/*0x41a4*/ f32 slidelifetime; // in seconds
	/*0x41a8*/ u8 unk41a8[8];
	/*0x41b0*/ s8 unk41b0[2];
	/*0x41b4*/ struct creditsbglayer bglayers[4];
	/*0x41f4*/ u8 slidesenabled;
	/*0x41f5*/ s8 particlecolourindex1;
	/*0x41f6*/ s8 particlecolourindex2;
	/*0x41f8*/ f32 particlecolourweight;
	/*0x41fc*/ u8 particlemovetype;
	/*0x41fd*/ u8 particleconfignum1;
	/*0x41fe*/ u8 particleconfignum2;
	/*0x41ff*/ u8 particlemaxsize;
	/*0x4200*/ u8 particleminsize;
	/*0x4204*/ u32 blacktimer60;
	/*0x4208*/ u8 unk4208;
	/*0x420c*/ u32 unk420c;
	/*0x4210*/ u32 unk4210;
	/*0x4214*/ u32 unk4214;
	/*0x4218*/ u32 unk4218;
	/*0x421c*/ u32 unk421c;
	/*0x4220*/ u32 unk4220;
	/*0x4224*/ u32 unk4224;
	/*0x4228*/ u32 unk4228;
	/*0x422c*/ u32 unk422c;
	/*0x4230*/ u32 unk4230;
	/*0x4234*/ u32 unk4234;
	/*0x4238*/ u32 unk4238;
	/*0x423c*/ u32 unk423c;
};

f32 g_CreditsParticleRotationFrac;
u32 g_CreditsCurFrame;
u32 g_CreditsPrevFrame;
u32 g_CreditsCurFrame2;
struct creditsdata *g_CreditsData;

bool g_CreditsScrollStarted = false;
bool g_CreditsAltTitleRequested = false;
bool g_CreditsUsingAltTitle = false;

void creditsMap4BgVertices(Vtx *vertices, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6)
{
	f32 a = arg2 * sinf(arg1) + arg3 * cosf(arg1);
	f32 b = arg2 * cosf(arg1) - arg3 * sinf(arg1);

	vertices[0].s = (sinf(arg4) + cosf(arg4)) * arg5 + a;
	vertices[0].t = (cosf(arg4) - sinf(arg4)) * arg6 + b;

	vertices[1].s = (sinf(arg4 + 1.5707963705063f) + cosf(arg4 + 1.5707963705063f)) * arg5 + a;
	vertices[1].t = (cosf(arg4 + 1.5707963705063f) - sinf(arg4 + 1.5707963705063f)) * arg6 + b;

	vertices[2].s = (sinf(arg4 + 3.1415927410126f) + cosf(arg4 + 3.1415927410126f)) * arg5 + a;
	vertices[2].t = (cosf(arg4 + 3.1415927410126f) - sinf(arg4 + 3.1415927410126f)) * arg6 + b;

	vertices[3].s = (sinf(arg4 + 4.7123889923096f) + cosf(arg4 + 4.7123889923096f)) * arg5 + a;
	vertices[3].t = (cosf(arg4 + 4.7123889923096f) - sinf(arg4 + 4.7123889923096f)) * arg6 + b;
}

void creditsMap9BgVertices(Vtx *vertices, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6)
{
	f32 a = arg2 * sinf(arg1) + arg3 * cosf(arg1);
	f32 b = arg2 * cosf(arg1) - arg3 * sinf(arg1);

	vertices[0].s = (sinf(arg4) + cosf(arg4)) * arg5 + a;
	vertices[0].t = (cosf(arg4) - sinf(arg4)) * arg6 + b;

	vertices[2].s = (sinf(arg4 + 1.5707963705063f) + cosf(arg4 + 1.5707963705063f)) * arg5 + a;
	vertices[2].t = (cosf(arg4 + 1.5707963705063f) - sinf(arg4 + 1.5707963705063f)) * arg6 + b;

	vertices[8].s = (sinf(arg4 + 3.1415927410126f) + cosf(arg4 + 3.1415927410126f)) * arg5 + a;
	vertices[8].t = (cosf(arg4 + 3.1415927410126f) - sinf(arg4 + 3.1415927410126f)) * arg6 + b;

	vertices[6].s = (sinf(arg4 + 4.7123889923096f) + cosf(arg4 + 4.7123889923096f)) * arg5 + a;
	vertices[6].t = (cosf(arg4 + 4.7123889923096f) - sinf(arg4 + 4.7123889923096f)) * arg6 + b;

	vertices[1].s = (vertices[0].s + vertices[2].s) / 2;
	vertices[1].t = (vertices[0].t + vertices[2].t) / 2;

	vertices[3].s = (vertices[0].s + vertices[6].s) / 2;
	vertices[3].t = (vertices[0].t + vertices[6].t) / 2;

	vertices[5].s = (vertices[8].s + vertices[2].s) / 2;
	vertices[5].t = (vertices[8].t + vertices[2].t) / 2;

	vertices[7].s = (vertices[6].s + vertices[8].s) / 2;
	vertices[7].t = (vertices[6].t + vertices[8].t) / 2;

	vertices[4].s = (vertices[3].s + vertices[5].s) / 2;
	vertices[4].t = (vertices[3].t + vertices[5].t) / 2;
}

/**
 * Initialises the vertices coordinates. The generated x and y coordinates are:
 *
 * 0:  -1800  -1800
 * 1:  0      -1800
 * 2:  1800   -1800
 * 3:  -1800  0
 * 4:  0      0
 * 5:  1800   0
 * 6:  -1800  1800
 * 7:  0      1800
 * 8:  1800   1800
 */
void creditsInitBgVertices(Vtx *vertices, s32 z)
{
	s32 i;

	for (i = 0; i < 9; i++) {
		vertices[i].x = (i % 3) * 1800 - 1800;
		vertices[i].y = (i / 3) * 1800 - 1800;
		vertices[i].z = z;
	}
}

struct bgconfig {
	/*0x00*/ f32 unk00;
	/*0x04*/ u8 s[9];
	/*0x10*/ u32 colours[3][3];
};

struct bgconfig g_CreditsBgConfigs[] = {
	{ 0.20, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x000064, 0x000064, 0x000000 }, { 0x003d84, 0x003d84, 0x000000 }, { 0x36003d, 0x36003d, 0x000000 } } },
	{ 1.00, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x000037, 0x0079a2, 0x36003d }, { 0x0079a2, 0x36003d, 0x000037 }, { 0x36003d, 0x000037, 0x0079a2 } } },
	{ 0.20, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x36003d, 0x51003f, 0x000037 }, { 0x51003f, 0x000037, 0x36003d }, { 0x0000a6, 0x36003d, 0x51003f } } },
	{ 0.20, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x003d84, 0x003d84, 0x000064 }, { 0x000064, 0x003d84, 0x003d84 }, { 0x36003d, 0x003d84, 0x36003d } } },
	{ 0.20, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x004345, 0x004345, 0x51003f }, { 0x000037, 0x000037, 0x004345 }, { 0x0079a2, 0x004345, 0x0079a2 } } },
	{ 0.20, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x0000a6, 0x0000a6, 0x000000 }, { 0x0000a6, 0x0000a6, 0x000000 }, { 0x0000a6, 0x0000a6, 0x000000 } } },
	{ 0.01, { 0, 1, 0, 1, 2, 1, 0, 1, 0 }, { { 0x00ffff, 0x0000ff, 0x000000 }, { 0x00ffff, 0x0000ff, 0x000000 }, { 0x00ffff, 0x0000ff, 0x000000 } } },
	{ 0.01, { 2, 1, 2, 1, 0, 1, 2, 1, 2 }, { { 0x00ffff, 0x0000ff, 0x000000 }, { 0xffffff, 0xffffff, 0xffffff }, { 0xffffff, 0xffffff, 0xffffff } } },
};

void creditsChooseBgColours(Vtx *vertices, Col *colours, s32 confignum, s32 alpha, s32 arg4)
{
#if PAL
	s32 iVar1 = (s32)(g_CreditsBgConfigs[confignum].unk00 * g_CreditsCurFrame2 / (10.0f / 3.0f) + arg4) % 180;
#else
	s32 iVar1 = (s32)(g_CreditsBgConfigs[confignum].unk00 * g_CreditsCurFrame2 * 0.25f + arg4) % 180;
#endif
	s32 colour1index = iVar1 / 60;
	s32 colour2index;
	f32 weightfrac;
	u32 weight;
	s32 i;

	if (colour1index > 2) {
		colour1index = 0;
	}

	colour2index = colour1index + 1;

	if (colour2index > 2) {
		colour2index = 0;
	}

	for (i = 0; i < ARRAYCOUNT(g_CreditsBgConfigs[confignum].s);) {
		vertices[i].colour = g_CreditsBgConfigs[confignum].s[i] * 4;
		i++;
	}

	weightfrac = (iVar1 % 60) / 60.0f;
	weight = 255.0f * weightfrac;

	colours[0].word = PD_BE32(colourBlend(
			g_CreditsBgConfigs[confignum].colours[colour2index][0] << 8,
			g_CreditsBgConfigs[confignum].colours[colour1index][0] << 8, weight) | alpha);

	colours[1].word = PD_BE32(colourBlend(
			g_CreditsBgConfigs[confignum].colours[colour2index][1] << 8,
			g_CreditsBgConfigs[confignum].colours[colour1index][1] << 8, weight) | alpha);

	colours[2].word = PD_BE32(colourBlend(
			g_CreditsBgConfigs[confignum].colours[colour2index][2] << 8,
			g_CreditsBgConfigs[confignum].colours[colour1index][2] << 8, weight) | alpha);
}

struct creditsbgtype {
	s16 unk00;
	s16 unk02;
	s16 unk04;
	s32 texturenum;
	f32 unk0c;
	f32 unk10;
};

struct creditsbgtype g_CreditsBgTypes[] = {
	{ 10000, 10000, -200, 0x04, 120, 0 },
	{ 1000,  1000,  -200, 0x05, 220, 0 },
	{ 1000,  10000, -50,  0x06, 220, 0 },
	{ 1000,  10000, -200, 0x26, 220, 0 },
	{ 500,   10000, -200, 0x26, 520, 0 },
	{ 1000,  20000, -50,  0x26, 120, 0 },
	{ 10000, 10000, -200, 0x2a, 120, 0 },
	{ 10000, 10000, -200, 0x2b, 80,  0 },
	{ 10000, 3000,  -200, 0x2b, 80,  0 },
	{ 3000,  3000,  -200, 0x2d, 120, 0 },
	{ 10000, 10000, -200, 0x07, 80,  0 },
	{ 10000, 1000,  -200, 0x2d, 220, 0 },
};

Gfx *creditsDrawBackgroundLayer(Gfx *gdl, u8 type, u8 layernum, f32 arg3, u32 alpha, s32 arg5)
{
	Vtx *vertices;
	Col *colours;
	u32 stack;
	f32 pan;
	f32 b;
	f32 c;
	f32 rotation;

	gdl = menugfx0f0e2498(gdl);

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEI, G_CC_MODULATEI);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, &g_TexGeneralConfigs[g_CreditsBgTypes[type].texturenum], 1, 1, 2, 1, NULL);

	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	colours = gfxAllocateColours(3);
	vertices = gfxAllocateVertices(9);

	creditsInitBgVertices(vertices, (s32)((g_CreditsBgTypes[type].unk04 + 2000) * arg3) - 2000);

#if PAL
	rotation = g_CreditsData->bglayers[layernum].rotspeed * g_CreditsCurFrame2 / (10.0f / 3.0f);
	pan = g_CreditsData->bglayers[layernum].panspeed * g_CreditsCurFrame2 / (10.0f / 3.0f);
#else
	rotation = g_CreditsData->bglayers[layernum].rotspeed * g_CreditsCurFrame2 * 0.25f;
	pan = g_CreditsData->bglayers[layernum].panspeed * g_CreditsCurFrame2 * 0.25f;
#endif
	b = g_CreditsBgTypes[type].unk0c * 32.0f;
	c = g_CreditsBgTypes[type].unk10 * 32.0f;

	creditsMap9BgVertices(vertices, pan, b, c, rotation, g_CreditsBgTypes[type].unk00, g_CreditsBgTypes[type].unk02);

	if (g_CreditsBgTypes[type].texturenum == 4) {
		alpha = alpha * 2 / 3;
	}

	if (g_CreditsBgTypes[type].texturenum == 0x26) {
		alpha = alpha * 2 / 3;
	}

	creditsChooseBgColours(vertices, colours, g_CreditsData->bglayers[layernum].confignum, alpha, arg5 * 90);

	gSPColor(gdl++, osVirtualToPhysical(colours), 3);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 9, 0);

	gSPTri4(gdl++, 0, 1, 3, 1, 4, 3, 1, 2, 5, 5, 4, 1);
	gSPTri4(gdl++, 3, 4, 7, 7, 6, 3, 4, 5, 7, 5, 8, 7);

	return gdl;
}

void creditsCopyBackgroundLayer(s32 srcindex, s32 dstindex, bool move)
{
	g_CreditsData->bglayers[dstindex].type = g_CreditsData->bglayers[srcindex].type;
	g_CreditsData->bglayers[dstindex].rotspeed = g_CreditsData->bglayers[srcindex].rotspeed;
	g_CreditsData->bglayers[dstindex].panspeed = g_CreditsData->bglayers[srcindex].panspeed;
	g_CreditsData->bglayers[dstindex].confignum = g_CreditsData->bglayers[srcindex].confignum;

	if (move) {
		g_CreditsData->bglayers[srcindex].type = -1;
	}
}

Gfx *creditsDrawBackground(Gfx *gdl)
{
	s32 s1[4] = {-1, -1, -1, -1};
	u32 s5[4];
	f32 s4[4];
	s32 s2[4];

	u32 stack;
	f32 tmp = g_CreditsCurFrame2 / (PAL ? 1000.0f : 1200.0f);
	f32 bestvalue;
	s32 bestindex;
	s32 len = 0;
	s32 i;

	for (i = 0; i < 2; i++) {
		f32 value = i * 0.5f + tmp;
		value = value - (s32)value;

		if (value > 0.8f) {
			u32 uVar14;
			s32 val = i;

			if (g_CreditsData->bglayers[i + 2].type != -1) {
				val = i + 2;
				g_CreditsData->unk41b0[i] = 1;
			} else {
				g_CreditsData->unk41b0[i] = 2;
			}

			uVar14 = ((value - 0.8f) / 0.2f) * 255.0f;

			s1[len + 0] = val;
			s2[len + 0] = uVar14;
			s4[len + 0] = value - 1.0f;
			s5[len + 0] = i;

			s1[len + 1] = i;
			s2[len + 1] = 255 - (s32)uVar14;
			s4[len + 1] = value;
			s5[len + 1] = i;

			len += 2;
		} else {
			if (g_CreditsData->unk41b0[i] == 1) {
				creditsCopyBackgroundLayer(i + 2, i, true);
			}

			g_CreditsData->unk41b0[i] = 0;

			s1[len] = i;
			s2[len] = 255;
			s4[len] = value;
			s5[len] = i;

			len++;
		}
	}

	do {
		bestvalue = 10.0f;
		bestindex = -1;

		for (i = 0; i < len; i++) {
			if (s1[i] >= 0 && s4[i] < bestvalue) {
				bestindex = i;
				bestvalue = s4[i];
			}
		}

		if (bestindex >= 0) {
			gdl = creditsDrawBackgroundLayer(gdl, g_CreditsData->bglayers[s1[bestindex]].type,
					s1[bestindex], s4[bestindex], s2[bestindex], s5[bestindex]);

			s1[bestindex] = -1;
		}
	} while (bestindex != -1);

	return gdl;
}

u32 var8007f3dc = 0x00000000;
u32 var8007f3e0 = 0x00000000;
u32 var8007f3e4 = 0x00000000;
u32 var8007f3e8 = 0x00000001;
u32 var8007f3ec = 0x00000000;
u32 var8007f3f0 = 0x00000000;
u32 var8007f3f4 = 0x010301f4;
u32 var8007f3f8 = 0x008888ff;
u32 var8007f3fc = 0x00ff88ff;
u32 var8007f400 = 0x00ff8888;
u32 var8007f404 = 0x006666ff;
u32 var8007f408 = 0x41f00000;
u32 var8007f40c = 0x41f00000;

u32 g_CreditColourPalette[][4] = {
	{ 0x0000ffff, 0x0044ffff, 0x0088ffff, 0x00ffffff },
	{ 0x8888ff7f, 0xff88ff7f, 0xff88887f, 0x6666ff7f },
	{ 0xff00007f, 0xffffff7f, 0xff00004f, 0xffffff4f },
	{ 0xffffff7f, 0xffffff5f, 0xffffff3f, 0xffffff1f },
};

struct g_CreditParticleConfigs {
	s32 texturenum;
	s16 s1;
	s16 t1;
	s16 s2;
	s16 t2;
};

struct g_CreditParticleConfigs g_CreditParticleConfigs[] = {
	{ 0x2a, 0, 0, 32, 32 },
	{ 0x2b, 0, 0, 32, 32 },
};

/**
 * Generate a random float between -range and range,
 * without being within 0.0001f of zero.
 *
 * This is only called with max = 0.00223f.
 */
f32 creditsRandInRange(f32 range)
{
	f32 value;

	do {
		value = RANDOMFRAC() * range;
		value = (value + value) - range;
	} while (value < 0.0001f && value > -0.0001f);

	return value;
}

void creditsCreatePendingBgLayers(u32 mask)
{
	s32 i;

	for (i = 0; i < 2; i++) {
		s32 shift = i * 8;

		if (g_CreditsData->unk41b0[i] == 0) {
			g_CreditsData->bglayers[i + 2].type = random() % 12;
			g_CreditsData->bglayers[i + 2].rotspeed = creditsRandInRange(0.00223f);
			g_CreditsData->bglayers[i + 2].panspeed = creditsRandInRange(0.00223f);

			if (((u8)(mask >> shift) & 2)) {
				g_CreditsData->bglayers[i + 2].confignum = random() % 6;
			}
		}
	}
}

Gfx *creditsFillFramebuffer(Gfx *gdl, u32 colour)
{
	gSPDisplayList(gdl++, &var800613a0);

	gdl = textSetPrimColour(gdl, colour);

	gDPFillRectangle(gdl++, 0, 0, viGetWidth(), viGetHeight());

	gdl = text0f153838(gdl);

	return gdl;
}

/**
 * An unused function. Renders a white line between two 3D coordinates.
 * The line would be 10 units high, though I'm not sure if that's big or not.
 */
Gfx *creditsRenderLine(Gfx *gdl, struct coord *from, struct coord *to)
{
	Vtx *vertices;
	Col *colours;

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(4);

	vertices[0].x = from->x;
	vertices[0].y = from->y;
	vertices[0].z = from->z;

	vertices[1].x = from->x;
	vertices[1].y = from->y + 10.0f;
	vertices[1].z = from->z;

	vertices[2].x = to->x;
	vertices[2].y = to->y + 10.0f;
	vertices[2].z = to->z;

	vertices[3].x = to->x;
	vertices[3].y = to->y;
	vertices[3].z = to->z;

	vertices[0].colour = 0;
	vertices[1].colour = 4;
	vertices[2].colour = 4;
	vertices[3].colour = 0;

	colours[0].word = 0xffffffff;
	colours[1].word = 0xffffffff;

	gSPColor(gdl++, osVirtualToPhysical(colours), 2);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

	gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);

	return gdl;
}

void creditsResetParticles(void)
{
	f32 tmp;
	s32 i;

	g_CreditsData->particlecolourindex1 = -1;
	g_CreditsData->particlecolourindex2 = 0;
	g_CreditsData->particlemovetype = MOVETYPE_LINEAR;
	g_CreditsData->particleconfignum1 = 0;
	g_CreditsData->particleconfignum2 = 0;
	g_CreditsData->particleminsize = 0;
	g_CreditsData->particlemaxsize = 7;

	for (i = 0; i < ARRAYCOUNT(g_CreditsData->particles); i++) {
		tmp = RANDOMFRAC();
		g_CreditsData->particles[i].x = (tmp + tmp) * 3000.0f - 3000.0f;

		tmp = RANDOMFRAC();
		g_CreditsData->particles[i].y = (tmp + tmp) * 3000.0f - 3000.0f;

		g_CreditsData->particles[i].unk12 = random() % 4;
		g_CreditsData->particles[i].colourindex = random() % 4;
		g_CreditsData->particles[i].rotation = RANDOMFRAC() * M_BADTAU;

		if (g_CreditsData->particleminsize < g_CreditsData->particlemaxsize) {
			g_CreditsData->particles[i].size = g_CreditsData->particleminsize
				+ (random() % (g_CreditsData->particlemaxsize - g_CreditsData->particleminsize));
		} else {
			g_CreditsData->particles[i].size = g_CreditsData->particleminsize;
		}

		g_CreditsData->particles[i].movetype = g_CreditsData->particlemovetype;

		if (random() % 2 == 1) {
			g_CreditsData->particles[i].confignum = g_CreditsData->particleconfignum1;
		} else {
			g_CreditsData->particles[i].confignum = g_CreditsData->particleconfignum2;
		}

		g_CreditsData->particles[i].z = RANDOMFRAC() * -8000.0f;
	}
}

void creditsTickParticles(void)
{
	s32 i;
	f32 amount;
	f32 zspeed = 30.0f;

	if (g_CreditsData->particlecolourindex1 >= 0) {
#if VERSION >= VERSION_PAL_BETA
		g_CreditsData->particlecolourweight += g_Vars.diffframe240freal / 720.0f;
#else
		g_CreditsData->particlecolourweight += g_Vars.diffframe240f / 720.0f;
#endif

		if (g_CreditsData->particlecolourweight > 1.0f) {
			g_CreditsData->particlecolourindex2 = g_CreditsData->particlecolourindex1;
			g_CreditsData->particlecolourindex1 = -1;
		}
	} else {
#if VERSION >= VERSION_NTSC_1_0
		if (RANDOMFRAC() < 0.007f && joyGetButtons(0, R_TRIG) == 0) {
			g_CreditsData->particlecolourindex1 = random() % 4;
			g_CreditsData->particlecolourweight = 0;
		}
#else
		if (RANDOMFRAC() < 0.007f) {
			g_CreditsData->particlecolourindex1 = random() % 4;
			g_CreditsData->particlecolourweight = 0;
		}
#endif
	}

#if VERSION >= VERSION_NTSC_1_0
	if (RANDOMFRAC() < 0.002f && joyGetButtons(0, R_TRIG) == 0) {
		g_CreditsData->particlemovetype = random() % 5;
	}
#else
	if (RANDOMFRAC() < 0.002f) {
		g_CreditsData->particlemovetype = random() % 5;
	}
#endif

#if VERSION >= VERSION_NTSC_1_0
	if (joyGetButtonsPressedThisFrame(0, R_TRIG)) {
		g_CreditsData->particlemovetype = random() % 5;

		if (g_CreditsData->particlecolourindex1 < 0) {
			g_CreditsData->particlecolourindex1 = random() % 4;
			g_CreditsData->particlecolourweight = 0;
		}
	}
#endif

	if (RANDOMFRAC() < 0.007f) {
		g_CreditsData->particleconfignum1 = random() % 2;
		g_CreditsData->particleconfignum2 = random() % 2;
	}

	for (i = 0; i < ARRAYCOUNT(g_CreditsData->particles); i++) {
		// Increment the rotation
		amount = ((i & 7) + 1) * 0.01f;

		if ((i & 8) == 0) {
			amount = -amount;
		}

		g_CreditsData->particles[i].rotation += amount;

		// Move the particle closer to the camera
#if VERSION >= VERSION_PAL_BETA
		amount = g_Vars.diffframe240freal * zspeed * 0.25f;
#else
		amount = g_Vars.diffframe240f * zspeed * 0.25f;
#endif
		g_CreditsData->particles[i].z += amount;

		// If the particle has gone behind the camera, reset it
		if (g_CreditsData->particles[i].z > 0.0f) {
			g_CreditsData->particles[i].x = RANDOMFRAC() * 2.0f * 3000.0f - 3000.0f;
			g_CreditsData->particles[i].y = RANDOMFRAC() * 2.0f * 3000.0f - 3000.0f;
			g_CreditsData->particles[i].unk12 = random() % 4;
			g_CreditsData->particles[i].colourindex = random() % 4;
			g_CreditsData->particles[i].rotation = RANDOMFRAC() * M_BADTAU;

			if (g_CreditsData->particleminsize < g_CreditsData->particlemaxsize) {
				g_CreditsData->particles[i].size = g_CreditsData->particleminsize
					+ random() % (g_CreditsData->particlemaxsize - g_CreditsData->particleminsize);
			} else {
				g_CreditsData->particles[i].size = g_CreditsData->particleminsize;
			}

			g_CreditsData->particles[i].movetype = g_CreditsData->particlemovetype;

			if (random() % 2 == 1) {
				g_CreditsData->particles[i].confignum = g_CreditsData->particleconfignum1;
			} else {
				g_CreditsData->particles[i].confignum = g_CreditsData->particleconfignum2;
			}

			g_CreditsData->particles[i].z += -8000;
		}

		if (zspeed);
	}
}

void creditsGetParticlePos(struct coord *pos, struct particle *particle, s32 particlenum)
{
	f32 sp4c;
	f32 sp48;
	f32 sp44;
	f32 sp40;
	f32 sp3c;
	f32 sp38;
	f32 sp34;
	f32 sp30;
	f32 sp2c;

	switch (particle->movetype) {
	case MOVETYPE_WAVE:
		sp44 = g_CreditsParticleRotationFrac * M_BADTAU;
		sp4c = particle->x;
		sp48 = -400.0f + (sinf(particle->x / 300.0f) + cosf(particle->z / 300.0f)) * 100.0f;

		if ((particlenum & 1) == 1) {
			sp48 += 800.0f;
		}

		pos->z = particle->z;
		pos->x = sinf(sp44) * sp4c + cosf(sp44) * sp48;
		pos->y = cosf(sp44) * sp4c - sinf(sp44) * sp48;
		break;
	case MOVETYPE_SPIRAL:
		pos->x = particle->x + (cosf(particle->y / 300.0f) + sinf(particle->z / 300.0f)) * 100.0f;
		pos->y = particle->y + (sinf(particle->x / 300.0f) + cosf(particle->z / 300.0f)) * 100.0f;
		pos->z = particle->z;
		break;
	case MOVETYPE_FLING:
		sp40 = particle->x / 5.0f;
		sp3c = particle->y / 5.0f;
		sp38 = particle->z / -8000.0f;
		sp38 = sp38 * sp38 * M_BADTAU;

		if (sp40 < 0.0f) {
			sp40 -= 420.0f;
		} else {
			sp40 += 420.0f;
		}

		if (sp3c < 0.0f) {
			sp3c -= 420.0f;
		} else {
			sp3c += 420.0f;
		}

		pos->x = sinf(sp38) * sp40 + cosf(sp38) * sp3c;
		pos->y = cosf(sp38) * sp40 - sinf(sp38) * sp3c;
		pos->z = particle->z;
		break;
	case MOVETYPE_TWIST:
		sp34 = particle->x / 5.0f;
		sp30 = 400.0f;
		sp2c = particle->z / -8000.0f;

		if ((particlenum & 1) == 1) {
			sp30 = -sp30;
		}

		sp2c = sp2c * sp2c * M_BADTAU + g_CreditsParticleRotationFrac * M_BADTAU;

		pos->x = sinf(sp2c) * sp34 + cosf(sp2c) * sp30;
		pos->y = cosf(sp2c) * sp34 - sinf(sp2c) * sp30;
		pos->z = particle->z;
		break;
	case MOVETYPE_FALL:
		pos->y = -(particle->z - -4000.0f);
		pos->x = particle->x;
		pos->z = particle->y;
		break;
	case MOVETYPE_LINEAR:
	default:
		pos->x = particle->x;
		pos->y = particle->y;
		pos->z = particle->z;
		break;
	}
}

Gfx *creditsDrawParticles(Gfx *gdl)
{
	Col *colours;
	u32 colour;
	s32 i;
	s32 j;
	s32 confignum;
	f32 invsine;
	f32 invcosine;

	static u32 tload = 0x25;
	static u32 dump = 0;

	mainOverrideVariable("tload", &tload);
	mainOverrideVariable("dump", &dump);

	colours = gfxAllocateColours(60);

	for (i = 0; i < 4; i++) {
		if (g_CreditsData->particlecolourindex1 >= 0) {
			u32 weight = g_CreditsData->particlecolourweight * 255.0f;
			colour = colourBlend(g_CreditColourPalette[g_CreditsData->particlecolourindex1][i], g_CreditColourPalette[g_CreditsData->particlecolourindex2][i], weight);
		} else {
			colour = g_CreditColourPalette[g_CreditsData->particlecolourindex2][i];
		}

		if (tload == 0x28) {
			colour = 0xffffffff;
		}

		for (j = 0; j < 15; j++) {
			colours[i + j * 4].word = PD_BE32((colour & 0xffffff00) | ((colour & 0xff) * (15 - j) / 15));
		}
	}

	gSPColor(gdl++, osVirtualToPhysical(colours), 60);

	for (confignum = 0; confignum < ARRAYCOUNT(g_CreditParticleConfigs); confignum++) {
		bool donetexture = false;

		for (i = 0; i < ARRAYCOUNT(g_CreditsData->particles); i++) {
			if (g_CreditsData->particles[i].confignum == confignum) {
				s32 offset;
				u32 stack[3];
				struct coord pos;
				Vtx *vertices;
				f32 sine = sinf(g_CreditsData->particles[i].rotation);
				f32 cosine = cosf(g_CreditsData->particles[i].rotation);
				f32 radius = g_CreditsData->particles[i].size * 10.0f + 25.0f;
				f32 frac;

				if (!donetexture) {
					texSelect(&gdl, &g_TexGeneralConfigs[g_CreditParticleConfigs[confignum].texturenum], 2, 1, 2, 1, 0);

					gDPSetCycleType(gdl++, G_CYC_1CYCLE);
					gDPSetColorDither(gdl++, G_CD_DISABLE);
					gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
					gDPSetAlphaCompare(gdl++, G_AC_NONE);
					gDPSetTextureLOD(gdl++, G_TL_TILE);
					gDPSetTextureConvert(gdl++, G_TC_FILT);
					gDPSetCombineLERP(gdl++,
							0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0,
							0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0);
					gDPSetTextureFilter(gdl++, G_TF_BILERP);
					gDPSetTexturePersp(gdl++, G_TP_PERSP);

					donetexture = true;
				}

				creditsGetParticlePos(&pos, &g_CreditsData->particles[i], i);

				frac = 1.0f - (pos.f[2] - -4500.0f) / 3500.0f;

				if (frac < 0.0f) {
					frac = 0.0f;
				}

				if (frac > 1.0f) {
					frac = 1.0f;
				}

				invsine = -sine;
				invcosine = -cosine;

				vertices = gfxAllocateVertices(4);

				vertices[0].x = pos.f[0] + radius * (sine + cosine);
				vertices[0].y = pos.f[1] + radius * (cosine + -sine);
				vertices[0].z = pos.f[2];

				vertices[1].x = pos.f[0] + radius * (sine - cosine);
				vertices[1].y = pos.f[1] + radius * (cosine - invsine);
				vertices[1].z = pos.f[2];

				vertices[2].x = pos.f[0] + radius * (invsine - cosine);
				vertices[2].y = pos.f[1] + radius * (invcosine - invsine);
				vertices[2].z = pos.f[2];

				vertices[3].x = pos.f[0] + radius * (invsine + cosine);
				vertices[3].y = pos.f[1] + radius * (invcosine + invsine);
				vertices[3].z = pos.f[2];

				vertices[0].s = g_CreditParticleConfigs[confignum].s1 * 32;
				vertices[0].t = g_CreditParticleConfigs[confignum].t1 * 32;
				vertices[1].s = g_CreditParticleConfigs[confignum].s1 * 32;
				vertices[1].t = g_CreditParticleConfigs[confignum].t2 * 32;
				vertices[2].s = g_CreditParticleConfigs[confignum].s2 * 32;
				vertices[2].t = g_CreditParticleConfigs[confignum].t2 * 32;
				vertices[3].s = g_CreditParticleConfigs[confignum].s2 * 32;
				vertices[3].t = g_CreditParticleConfigs[confignum].t1 * 32;

				if (1);
				offset = (g_CreditsData->particles[i].colourindex + ((s32) (frac * 14.99f) * 4)) * 4;

				vertices[0].colour = offset;
				vertices[1].colour = offset;
				vertices[2].colour = offset;
				vertices[3].colour = offset;

				gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

				gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
			}
		}
	}

	return gdl;
}

Gfx *creditsDrawBackgroundText(Gfx *gdl, s32 x, s32 y, char *text, struct fontchar *chars, struct font *font, f32 opacity, f32 speed, f32 spread)
{
	f32 sp98;
	u32 colour1;
	u32 colour2;
	u32 colour3;
	s32 i;

	sp98 = ((y / 2) % 100) * M_BADTAU / 100.0f;

	colour1 = colourBlend(0x00ffff00, 0xffffff00, 255.0f * spread);
	colour2 = colourBlend(0x0000af00, 0xffffff00, 255.0f * spread);

	for (i = 4; i >= 0; i--) {
		f32 f24 = i * 0.2f + g_CreditsParticleRotationFrac * 10.0f * M_BADTAU + sp98;
		f32 f26 = i * 0.1f * (spread + 0.5f) + 1.0f;

		if ((random() % 256) == 1) {
			f26 *= 10.0f;
		}

		colour3 = colourBlend(colour2, colour1, 63 * i);
		colour3 |= (u32)((63.0f + ((4 - i) * 16)) * opacity);

		if (colour3 & 0x000000ff) {
			f32 mult = (i + 1) * 4.4f * speed;
			f32 value = (i * 0.3f * (spread + 0.5f)) + 1.0f;
			u32 stack[2];

			gdl = text0f1552d4(gdl, x + sinf(f24) * mult, y + cosf(f24) * mult, f26, value, text, chars, font, colour3, 0, 1);
		}
	}

	return gdl;
}

Gfx *creditsDrawForegroundText(Gfx *gdl, s32 x, s32 y, char *text, struct fontchar *chars, struct font *font, f32 transfrac, s32 hdir, s32 vdir, s32 stretchtype)
{
	f32 widthscale;
	f32 heightscale;
	f32 opacityfrac;
	u32 colour;

	opacityfrac = 2.0f - 2.0f * transfrac;

	switch (stretchtype) {
	case STRETCHTYPE_HORIZONTAL:
		widthscale = 1.0f + 15.0f * transfrac;
		heightscale = 1.0f + 4.0f * transfrac;
		break;
	case STRETCHTYPE_VERTICAL:
		widthscale = 1.0f + 2.0f * transfrac;
		heightscale = 1.0f + 75.0f * transfrac;
		break;
	case STRETCHTYPE_HORIZONTAL_SQUASH:
		widthscale = 1.0f + 15.0f * transfrac;
		heightscale = 1.0f - (transfrac - 0.34f);

		if (heightscale > 1.0f) {
			heightscale = 1.0f;
		}
		break;
	case STRETCHTYPE_VERTICAL_SQUASH:
		heightscale = 1.0f + 55.0f * transfrac;
		widthscale = 1.0f - (transfrac - 0.34f);

		if (widthscale > 1.0f) {
			widthscale = 1.0f;
		}
		break;
	}

	if (opacityfrac > 1.0f) {
		opacityfrac = 1.0f;
	}

	colour = 0xffffff00 | (u32) (255.0f * opacityfrac);

	gdl = text0f1552d4(gdl, x, y, widthscale, heightscale, text, chars, font, colour, hdir, vdir);

	return gdl;
}

void creditsResetSlides(void)
{
	u8 pool[NUM_CORE_TEAM];
	s32 i;

	// Shuffle the core team slides
	for (i = 0; i < NUM_CORE_TEAM; i++) {
		pool[i] = i + 1;
	}

	for (i = 0; i < NUM_CORE_TEAM; i++) {
		s32 index = random() % (NUM_CORE_TEAM - i);

		g_CreditsData->coreteammap[i + 1] = pool[index];

#ifdef AVOID_UB
		// don't overflow `pool`
		while (index < NUM_CORE_TEAM - i - 1) {
#else
		while (index < NUM_CORE_TEAM - i) {
#endif
			pool[index] = pool[index + 1];
			index++;
		}
	}

	g_CreditsData->creditnum = 0;
	g_CreditsData->unk41a8[0] = 0xf0;
	g_CreditsData->numthisslide = 1;
	g_CreditsData->slideage = 0;
}

struct credit g_Credits[] = {
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_002,  L_TITLE_000 }, // "perfect dark", ""

	// The order of these items are randomised
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_003,  L_TITLE_004 }, // "beau ner chesluk", "guns and visual orgasms"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_005,  L_TITLE_006 }, // "jonathan nasty mummery", "mover and shaker"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_007,  L_TITLE_008 }, // "darkmark", "reality engineer"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_009,  L_TITLE_010 }, // "chris tilston", "designs on the future"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_011,  L_TITLE_012 }, // "chris darling", "weapons specialist"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_013,  L_TITLE_014 }, // "duncan botwood", "grey area"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_015,  L_TITLE_016 }, // "b jones", "bodybuilder"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_017,  L_TITLE_018 }, // "steve malpass", "perfect locations and vox"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_019,  L_TITLE_020 }, // "brian marshall", "-aqham-"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_021,  L_TITLE_022 }, // "russel irwin", "well packed man"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_023,  L_TITLE_024 }, // "jamie 'evo' williams", "manic welsh designer"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_025,  L_TITLE_026 }, // "ross bury", "bionic backgrounds"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_027,  L_TITLE_028 }, // "martin penny", "sound geezer"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_029,  L_TITLE_030 }, // "grant kirkhope", "play that music maestro"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_031,  L_TITLE_032 }, // "david clynick", "bangin hardcore choonz"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM, L_TITLE_033,  L_TITLE_034 }, // "keith 'bunny' rabbette", "dinomic backgrounds"

	//load next credit for same slide
	//|               duration: 0=4s, 1=8s, 2=12s, 3=16s
	//|               |
	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING1,    L_TITLE_035, L_TITLE_000 }, // "dd snipers", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_036, L_TITLE_037 }, // "kevin bayliss", "dean smith"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_038, L_TITLE_001 }, // "graham smith", "\n"

	{ 1, RETAIN_BOTH, 0, CREDITSTYLE_HEADING1,    L_TITLE_035, L_TITLE_000 }, // "dd snipers", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_039, L_TITLE_040 }, // "mike 'curry' currington", "tony wong"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_041, L_TITLE_001 }, // "simon farmer", "\n"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING1,    L_TITLE_035, L_TITLE_000 }, // "dd snipers", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_042, L_TITLE_043 }, // "leigh loveday", "rob harrison"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_044, L_TITLE_001 }, // "steven hurst", "\n"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING1,    L_TITLE_045, L_TITLE_000 }, // "fearsome foursome", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_046, L_TITLE_047 }, // "phil dunne", "ricky berwick"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_048, L_TITLE_049 }, // "gareth glover", "jonathan ambrose"

	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING1,    L_TITLE_050, L_TITLE_000 }, // "trent's henchmen", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_051, L_TITLE_052 }, // "lee musgrave", "johnni christensen"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_053, L_TITLE_001 }, // "mark betteridge", "\n"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING1,    L_TITLE_050, L_TITLE_000 }, // "trent's henchmen", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_054, L_TITLE_055 }, // "chris marlow", "robin beanland"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_056, L_TITLE_057 }, // "neil gallagher", "feargal plant"

	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING1,    L_TITLE_058, L_TITLE_000 }, // "voices in the dark", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_059, L_TITLE_060 }, // "eveline fischer", "chris sutherland"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_061, L_TITLE_062 }, // "chris seavor", "john silke"

	{ 1, RETAIN_BOTH, 0, CREDITSTYLE_HEADING1,    L_TITLE_058, L_TITLE_000 }, // "voices in the dark", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_063, L_TITLE_064 }, // "ben cullum", "louise tilston"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_065, L_TITLE_066 }, // "alistair", "lindsay"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING1,    L_TITLE_058, L_TITLE_000 }, // "voices in the dark", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_068, L_TITLE_067 }, // "beau chesluk", "b jones"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_069, L_TITLE_001 }, // "steve malpass", "\n"

	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING2,    L_TITLE_093, L_TITLE_000 }, // "rare exterminators", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_094, L_TITLE_095 }, // "huw ward", "adam munton"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_096, L_TITLE_097 }, // "david wong", "luke munton"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_098, L_TITLE_109 }, // "gary phelps", "andrew wilson"

	{ 1, RETAIN_BOTH, 0, CREDITSTYLE_HEADING2,    L_TITLE_093, L_TITLE_000 }, // "rare exterminators", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_099, L_TITLE_100 }, // "john silke", "matthew carter"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_101, L_TITLE_102 }, // "gavin price", "gareth stevenson"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_103, L_TITLE_001 }, // "bushbaby", "\n"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING2,    L_TITLE_093, L_TITLE_000 }, // "rare exterminators", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_104, L_TITLE_105 }, // "stephen stamper", "ross bullimore"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_106, L_TITLE_107 }, // "justin cook", "dale murchie"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_108, L_TITLE_001 }, // "roger smith", "\n"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_070, L_TITLE_000 }, // "ci techs", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_071, L_TITLE_072 }, // "richard 'force 9' gale", "mark green"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_073, L_TITLE_074 }, // "alex zoro", "mark wilson"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_075, L_TITLE_000 }, // "hard and wary support", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_076, L_TITLE_077 }, // "pete 'hardrom' cox", "maul 'max' mikell"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_078, L_TITLE_000 }, // "doug crouch", ""

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_083, L_TITLE_000 }, // "motion capture", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_084, L_TITLE_085 }, // "alan tippertronic", "des easen"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_086, L_TITLE_000 }, // "jim 'love' ballard", ""

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_087, L_TITLE_000 }, // "going through the motions", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_088, L_TITLE_089 }, // "duncan 'bot' botwood", "michelle tipper"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_090, L_TITLE_091 }, // "sue 'falling' fell", "doug 'crouch' crouch"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_092, L_TITLE_000 }, // "ross bury", ""

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_118, L_TITLE_000 }, // "testing (rare miami)", ""
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_119, L_TITLE_000 }, // "keith coll", ""

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING1,    L_TITLE_113, L_TITLE_000 }, // "rare in the sun", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_114, L_TITLE_115 }, // "joel hochberg", ""
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_116, L_TITLE_117 }, // "scott hochberg", ""

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING1,    L_TITLE_110, L_TITLE_000 }, // "dark commanders", ""
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_111, L_TITLE_112 }, // "bis", "wombat"

	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING2,    L_TITLE_120, L_TITLE_000 }, // "testing (noa)", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_121, L_TITLE_122 }, // "michael kelbaugh", "tim bechtel"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_123, L_TITLE_124 }, // "tom hertzog", "melvin 'sherwood' forrest"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING2,    L_TITLE_120, L_TITLE_000 }, // "testing (noa)", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_125, L_TITLE_126 }, // "dougall campbell", "roger harrison"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_127, L_TITLE_001 }, // "jeff kalles", "\n"

	{ 0, RETAIN_NONE, 0, CREDITSTYLE_CORETEAM,    L_TITLE_128, L_TITLE_129 }, // "the perfect core", "-darker than you"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_130, L_TITLE_000 }, // "treehouse", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_131, L_TITLE_132 }, // "armond williams junior", "henry sterchi"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_133, L_TITLE_000 }, // "ed ridgeway", ""

#if VERSION == VERSION_JPN_FINAL
	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING1,    L_TITLE_184, L_TITLE_000 }, // "ncl staff", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_185, L_TITLE_186 }, // "s. miyamoto", "k. miki"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_187, L_TITLE_188 }, // "s. kojoh", "y. nakano"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING1,    L_TITLE_184, L_TITLE_000 }, // "ncl staff", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_189, L_TITLE_190 }, // "k.yamaguchi", "k. terasaki"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_191, L_TITLE_192 }, // "m. goto", "super mario club"
#endif

#if VERSION == VERSION_PAL_FINAL
	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING2,    L_TITLE_151, L_TITLE_000 }, // "testing (noe)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_SUPERVISOR,  L_TITLE_152, L_TITLE_153 }, // "supervisor", "kai 'jellybean' neumann"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_SUPERVISOR,  L_TITLE_154, L_TITLE_155 }, // "deputy supervisor", "maurice 'pathfinder' tisdale"

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING2,    L_TITLE_151, L_TITLE_000 }, // "testing (noe)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_SUPERVISOR,  L_TITLE_156, L_TITLE_157 }, // "coordinators", "patrick 'capricorn' thieret"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_158, L_TITLE_001 }, // "andreas 'brennero' dietz", "\n"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_162, L_TITLE_000 }, // "localization (french)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_159, L_TITLE_163 }, // "editor", "julien 'sexy boy' bardakoff"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_161, L_TITLE_164 }, // "assistant editors", "jean-baptiste 'ours noir' fleury"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_165, L_TITLE_166 }, // "nicolas 'darth lapinou' gourio", "nicolas 'el betal' robert"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_167, L_TITLE_000 }, // "localization (german)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_159, L_TITLE_168 }, // "editor", "micky 'scorpio's movement' auer"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_160, L_TITLE_169 }, // "assistant editor", "jan 'ian' peitzmeier"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_170, L_TITLE_000 }, // "localization (italian)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_159, L_TITLE_171 }, // "editor", "elenor isbitish"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_160, L_TITLE_172 }, // "assistant editor", "calimero tiiiun"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_173, L_TITLE_000 }, // "localization (spanish)"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_159, L_TITLE_174 }, // "editor", "antonio 'toro' greppi"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_161, L_TITLE_175 }, // "assistant editors", "carlos 'yoshio' montilla"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_176, L_TITLE_000 }, // "susa & natalia"

	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_177, L_TITLE_000 }, // "localization"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_178, L_TITLE_179 }, // "product coordinator", "andy 'fiedl' fey"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_EDITOR,      L_TITLE_180, L_TITLE_181 }, // "product supervisor", "tanja 'personalized sarcasm' baar"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_ASSISTANT,   L_TITLE_182, L_TITLE_183 }, // "product localization manager", "kai '0 to 100' zeh"
#endif

#if VERSION >= VERSION_NTSC_1_0
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_HEADING2,    L_TITLE_134, L_TITLE_000 }, // "perfect spelling", ""
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME2,       L_TITLE_135, L_TITLE_000 }, // "teresa lillygren", ""
#endif

	{ 1, RETAIN_OUT,  0, CREDITSTYLE_HEADING1,    L_TITLE_136, L_TITLE_000 }, // "nintendo", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_137, L_TITLE_138 }, // "mr arakawa", ""
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_139, L_TITLE_140 }, // "don james", ""

	{ 1, RETAIN_IN,   0, CREDITSTYLE_HEADING1,    L_TITLE_136, L_TITLE_000 }, // "nintendo", ""
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_141, L_TITLE_142 }, // "howard lincoln", "ken lobb"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_NAME1,       L_TITLE_143, L_TITLE_144 }, // "jacqualee story", "gail tilden"

	{ 1, RETAIN_NONE, 1, CREDITSTYLE_RAREDESIGNS, L_TITLE_148, L_TITLE_149 }, // "rare", "designs on the future"
	{ 1, RETAIN_NONE, 0, CREDITSTYLE_COPYRIGHT,   L_TITLE_001, L_TITLE_145 }, // "\n", "copyright rare 2000"
	{ 0, RETAIN_NONE, 0, CREDITSTYLE_COPYRIGHT,   L_TITLE_146, L_TITLE_147 }, // "perfect dark and the pd device", "are trademarks"

	{ 0, RETAIN_NONE, 0, CREDITSTYLE_COPYRIGHT,   L_TITLE_150, L_TITLE_000 }, // "perfect dark is forever", ""

	{ 0, RETAIN_NONE, 0, CREDITSTYLE_TERMINATOR,  L_TITLE_000, L_TITLE_000 }, // "", ""
};

/**
 * Get a credit to display, based on its row index on the current slide.
 *
 * Row 0 will usually be a title.
 * Row 1 will usually be the first two names and so on.
 *
 * The first slide, "perfect dark", is always first, then the next 16 are the
 * core team and are in a random order. The rest are in the same order as
 * defined in the g_Credits array.
 */
struct credit *creditGetByRow(s32 row)
{
	// If core team, use the mapping
	if (g_CreditsData->creditnum > 0 && g_CreditsData->creditnum < ARRAYCOUNT(g_CreditsData->coreteammap)) {
		if (row > 0) {
			return NULL;
		}

		return &g_Credits[g_CreditsData->coreteammap[g_CreditsData->creditnum]];
	}

	return &g_Credits[g_CreditsData->creditnum + row];
}

void creditsTickSlide(void)
{
	struct credit *credit = creditGetByRow(0);
	s32 i;
	f32 durations[] = {4, 8, 12, 16}; // in seconds
	f32 loadat = durations[credit->durationindex] + 2.0f + 0.142f * (g_CreditsData->numthisslide * 2 - 1);
	f32 seconds;

	g_CreditsData->slidelifetime = durations[credit->durationindex] + 1.0f;
#if VERSION >= VERSION_PAL_BETA
	seconds = g_Vars.diffframe240freal / 240.0f;
#else
	seconds = g_Vars.diffframe240f / 240.0f;
#endif
	g_CreditsData->slideage += seconds;

	if (g_CreditsData->slideage > loadat) {
		// Load new slide
		g_CreditsData->slideage = 0;
		g_CreditsData->creditnum += g_CreditsData->numthisslide;
		g_CreditsData->numthisslide = 1;

		do {
			credit = creditGetByRow(g_CreditsData->numthisslide - 1);

			if (credit->more) {
				g_CreditsData->numthisslide++;
			}

			if (credit && credit->style == CREDITSTYLE_TERMINATOR && g_CreditsData->numthisslide == 1) {
				// Reached the end of the slides list
				g_CreditsData->creditnum = 0;

				credit = NULL;

				g_CreditsData->numthisslide = 1;
				g_CreditsData->slideage = 0;
				g_CreditsData->slidesenabled = false;
				g_CreditsData->blacktimer60 = 0;
				g_CreditsData->unk4208 = 0;

				musicEndMenu();
				musicStartPrimaryWithReason(0,1);
			}
		} while (credit && credit->more && g_CreditsData->numthisslide < 4);

		for (i = 0; i < ARRAYCOUNT(g_CreditsData->unk41a8); i++) {
			g_CreditsData->unk41a8[i] = random() % 3 | random() % 3 << 2 | random() % 16 << 4;
		}

		creditsCreatePendingBgLayers(0xffffffff);
	}
}

Gfx *creditsDrawSlide(Gfx *gdl)
{
	s32 i;
	struct credit *credits[4];
	char *texts[8];
	struct fontchar *chars[8];
	struct font *fonts[8];
	s32 extray;
	s32 x[8];
	s32 y[8];
	s32 textwidths[8];
	s32 textheights[8];
	s32 cury = 0;
	s32 prevstyle = -1;
	s32 index;
	s32 hdir;
	s32 vdir;
	s32 stretchtype;
	bool settled;
	f32 age;
	f32 mult = 1.0f;

	// Iterate the credits on this slide and populate the arrays
	for (i = 0; i < g_CreditsData->numthisslide; i++) {
		index = i * 2;
		credits[i] = creditGetByRow(i);

		if (credits[i] == NULL) {
			g_CreditsData->creditnum = 0;
			g_CreditsData->numthisslide = 1;
			g_CreditsData->slideage = 0.0f;
			return gdl;
		}

		texts[index + 0] = langGet(credits[i]->text1);
		texts[index + 1] = langGet(credits[i]->text2);

		// Choose first font
		switch (credits[i]->style) {
		case CREDITSTYLE_CORETEAM:
		case CREDITSTYLE_HEADING1:
		case CREDITSTYLE_NAME1:
		case CREDITSTYLE_HEADING2:
		case CREDITSTYLE_UNUSED_08:
		case CREDITSTYLE_RAREDESIGNS:
			chars[index + 0] = g_CharsHandelGothicLg;
			fonts[index + 0] = g_FontHandelGothicLg;
			break;
		case CREDITSTYLE_ASSISTANT:
		case CREDITSTYLE_UNUSED_06:
		case CREDITSTYLE_NAME2:
		case CREDITSTYLE_SUPERVISOR:
#if VERSION >= VERSION_PAL_FINAL
		case CREDITSTYLE_EDITOR:
#endif
			chars[index + 0] = g_CharsHandelGothicMd;
			fonts[index + 0] = g_FontHandelGothicMd;
			break;
		default:
			chars[index + 0] = g_CharsHandelGothicSm;
			fonts[index + 0] = g_FontHandelGothicSm;
			break;
		}

		// Choose second font
		switch (credits[i]->style) {
		case CREDITSTYLE_UNUSED_06:
			chars[index + 1] = g_CharsHandelGothicSm;
			fonts[index + 1] = g_FontHandelGothicSm;
			break;
		case CREDITSTYLE_CORETEAM:
		case CREDITSTYLE_RAREDESIGNS:
			chars[index + 1] = g_CharsHandelGothicMd;
			fonts[index + 1] = g_FontHandelGothicMd;
			break;
		default:
			chars[index + 1] = chars[index + 0];
			fonts[index + 1] = fonts[index + 0];
			break;
		}

		if (1);
		if (1);

		if (prevstyle == CREDITSTYLE_HEADING1 || prevstyle == CREDITSTYLE_COPYRIGHT || prevstyle == CREDITSTYLE_HEADING2) {
			cury += 10.0f;
		}

		prevstyle = credits[i]->style;

#if VERSION >= VERSION_JPN_FINAL
		textMeasure(&textheights[index + 0], &textwidths[index + 0], texts[index + 0], chars[index + 0], fonts[index + 0], -1);
		textMeasure(&textheights[index + 1], &textwidths[index + 1], texts[index + 1], chars[index + 1], fonts[index + 1], -1);
#else
		textMeasure(&textheights[index + 0], &textwidths[index + 0], texts[index + 0], chars[index + 0], fonts[index + 0], 0);
		textMeasure(&textheights[index + 1], &textwidths[index + 1], texts[index + 1], chars[index + 1], fonts[index + 1], 0);
#endif

		// Choose first position
		switch (credits[i]->style) {
		case CREDITSTYLE_CORETEAM:
		case CREDITSTYLE_HEADING1:
		case CREDITSTYLE_NAME1:
		case CREDITSTYLE_ASSISTANT:
		case CREDITSTYLE_COPYRIGHT:
		case CREDITSTYLE_UNUSED_06:
		case CREDITSTYLE_RAREDESIGNS:
#if VERSION >= VERSION_PAL_FINAL
		case CREDITSTYLE_EDITOR:
#endif
			x[index + 0] = 160 - textwidths[index + 0] / 2;
			break;
		case CREDITSTYLE_UNUSED_08:
		case CREDITSTYLE_NAME2:
			x[index + 0] = 270 - textwidths[index + 0];
			break;
		case CREDITSTYLE_SUPERVISOR:
		default:
			x[index + 0] = 50;
			break;
		}

		y[index + 0] = cury;
		cury += textheights[index + 0];

		// Choose second position
		switch (credits[i]->style) {
		case CREDITSTYLE_CORETEAM:
			x[index + 1] = x[index + 0] + textwidths[index + 0] - textwidths[index + 1];
			break;
		case CREDITSTYLE_HEADING1:
		case CREDITSTYLE_NAME1:
		case CREDITSTYLE_ASSISTANT:
		case CREDITSTYLE_COPYRIGHT:
		case CREDITSTYLE_UNUSED_06:
		case CREDITSTYLE_RAREDESIGNS:
#if VERSION >= VERSION_PAL_FINAL
		case CREDITSTYLE_EDITOR:
#endif
			x[index + 1] = 160 - textwidths[index + 1] / 2;
			break;
		case CREDITSTYLE_UNUSED_08:
		case CREDITSTYLE_NAME2:
		case CREDITSTYLE_SUPERVISOR:
			x[index + 1] = 270 - textwidths[index + 1];
			break;
		default:
			x[index + 1] = 50;
			break;
		}

		y[index + 1] = cury;
		cury += textheights[index + 1];

#if VERSION >= VERSION_PAL_FINAL
		if (credits[i]->style == CREDITSTYLE_EDITOR) {
			cury += 10;
		}
#endif
	}

	gdl = text0f153628(gdl);

	// Draw text repetitions
	for (i = 0; i < g_CreditsData->numthisslide * 2; i++) {
		f32 opacity = 0.0f;
		f32 speed = 1.0f;
		f32 spread = 1.0f;

		age = g_CreditsData->slideage - i * 0.142f;

		if ((age > 0.0f || (credits[i / 2]->retain & RETAIN_IN) == RETAIN_IN)
				&& (age < g_CreditsData->slidelifetime + 1.0f || (credits[i / 2]->retain & RETAIN_OUT) == RETAIN_OUT)) {
			bool settled = false;
			extray = 120 - cury / 2;

			if (age < 1.0f) {
				if ((credits[i / 2]->retain & RETAIN_IN) == RETAIN_IN) {
					settled = true;
				}
			} else {
				if (age > g_CreditsData->slidelifetime) {
					if ((credits[i / 2]->retain & RETAIN_OUT) == RETAIN_OUT) {
						settled = true;
					} else {
						opacity = 1.0f - (age - g_CreditsData->slidelifetime) * speed;
					}
				} else {
					if ((credits[i / 2]->retain & RETAIN_IN) == RETAIN_IN) {
						settled = true;
					} else {
						opacity = 1.0f;
						speed = (age - 1.0f) / (g_CreditsData->slidelifetime - 1.0f);
						spread = speed * 5.0f;
					}
				}
			}

			if (settled) {
				opacity = 1.0f;
				speed = 1.0f;
				spread = 1.0f;
			}

			if (opacity > 1.0f) {
				opacity = 1.0f;
			}

			if (spread > 1.0f) {
				spread = 1.0f;
			}

			if (speed > 1.0f) {
				speed = 1.0f;
			}

			gdl = creditsDrawBackgroundText(gdl, x[i], y[i] + extray, texts[i], chars[i], fonts[i], opacity, speed, spread);
		}
	}

	// Draw text properly
	for (i = 0; i < g_CreditsData->numthisslide * 2; i++) {
#if VERSION >= VERSION_PAL_FINAL
		f32 transfrac = 0;
#else
		f32 transfrac = 0.0f;
#endif

		age = g_CreditsData->slideage - i * 0.142f;

		if (age > 0.0f || (credits[i / 2]->retain & RETAIN_IN) == RETAIN_IN) {
			if (age < g_CreditsData->slidelifetime + 1.0f || (credits[i / 2]->retain & RETAIN_OUT) == RETAIN_OUT) {
				u32 a2 = g_CreditsData->creditnum;
				settled = false;
				stretchtype = STRETCHTYPE_HORIZONTAL;
				vdir = (g_CreditsData->unk41a8[i] >> 2) & 3;
				hdir = g_CreditsData->unk41a8[i] & 3;

				// b74
				if (((g_CreditsData->unk41a8[i] >> 4) & 1) == 1 && credits[i / 2]->style != CREDITSTYLE_NAME2) {
					stretchtype = STRETCHTYPE_VERTICAL;

					// This is a fake match. The asm sets vdir to 1 (VDIR_MID)
					// unconditionally, but we need to load and discard
					// g_CreditsData->creditnum, which we do here by putting it
					// in a u32 then checking if it's >= 0, which always passes.
					vdir = a2 >= 0;
				}

				if (age < 1.0f) {
					if ((credits[i / 2]->retain & RETAIN_IN) != RETAIN_IN) {
						transfrac = (1.0f - age) * mult;
					} else {
						settled = true;
					}
				} else {
					if (age > g_CreditsData->slidelifetime) {
						bool distort = true;

						if ((credits[i / 2]->retain & RETAIN_OUT) != RETAIN_OUT) {
							transfrac = (age - g_CreditsData->slidelifetime) * mult;

							switch (credits[i / 2]->style) {
							case CREDITSTYLE_HEADING1:
							case CREDITSTYLE_HEADING2:
								distort = false;
								break;
							case CREDITSTYLE_CORETEAM:
								if ((i % 2) == 0) {
									distort = false;
								}
								break;
							}

							if (credits[i / 2]->retain == RETAIN_NONE && distort) {
								stretchtype = STRETCHTYPE_HORIZONTAL_SQUASH;
								vdir = VDIR_MID;
							}
						} else {
							settled = true;
						}
					} else {
						settled = true;
					}
				}

				switch (credits[i / 2]->style) {
				case CREDITSTYLE_HEADING2:
					hdir = HDIR_RIGHT;
					break;
				case CREDITSTYLE_UNUSED_08:
				case CREDITSTYLE_NAME2:
				case CREDITSTYLE_SUPERVISOR:
					hdir = HDIR_LEFT;
					break;
				case CREDITSTYLE_HEADING1:
					hdir = HDIR_MID;
					break;
				}

				// Apply random flicker
				if (settled && random() % (g_CreditsData->numthisslide * 16) == 1) {
					transfrac = RANDOMFRAC() * 0.05f;
					hdir = random() % 3;
					vdir = random() % 3;
				}

				extray = 120 - cury / 2;
				gdl = creditsDrawForegroundText(gdl, x[i], y[i] + extray, texts[i], chars[i], fonts[i],
						transfrac, hdir, vdir, stretchtype);
			}
		}
	}

	gdl = text0f153780(gdl);

	return gdl;
}

void credits0f13ae04(Mtxf *mtx)
{
	// empty
}

void creditsTick(void)
{
	s32 i;
	static bool flick = false;
	static u32 type = 0xffff;

#if VERSION >= VERSION_NTSC_1_0
	if (joyGetButtonsPressedThisFrame(0, R_TRIG)) {
		creditsCreatePendingBgLayers(0xffffffff);
	}
#endif

	mainOverrideVariable("for", &type);

	if (type < 0xffff) {
		for (i = 0; i < ARRAYCOUNT(g_CreditsData->bglayers); i++) {
			g_CreditsData->bglayers[i].type = type;
		}
	}

	mainOverrideVariable("flick", &flick);

	if (flick) {
		creditsCreatePendingBgLayers(0xffffffff);
		flick = false;
	}

	if (g_CreditsCurFrame2 == 0) {
		if (g_CreditsUsingAltTitle) {
			musicSetStage(STAGE_CREDITS);
			musicStartPrimary(0);
		} else {
			musicSetStage(STAGE_CREDITS);
			musicStartTrackAsMenu(MUSIC_CREDITS);
		}
	}

#if VERSION >= VERSION_NTSC_1_0
	g_CreditsPrevFrame = g_CreditsCurFrame;
	g_CreditsCurFrame += g_Vars.diffframe240;
	g_CreditsCurFrame2 += g_Vars.diffframe240;
	g_CreditsParticleRotationFrac = (g_CreditsCurFrame2 % TICKS(4800)) / TICKS(4800.0f);

	creditsTickParticles();

	if (g_CreditsData->slidesenabled) {
		creditsTickSlide();
	} else if (RANDOMFRAC() < 0.01f && !joyGetButtons(0, R_TRIG)) {
		creditsCreatePendingBgLayers(0xffffffff);
	}

	if (!g_CreditsData->slidesenabled && g_CreditsData->blacktimer60 < (PAL ? 1150 : 1360)) {
		g_CreditsData->blacktimer60 += g_Vars.diffframe60;
	}
#else
	if (joyGetButtons(0, R_TRIG) == 0) {
		g_CreditsPrevFrame = g_CreditsCurFrame;
		g_CreditsCurFrame += g_Vars.diffframe240;
		g_CreditsCurFrame2 += g_Vars.diffframe240;
	}

	joyGetButtonsPressedThisFrame(0, Z_TRIG);

	g_CreditsParticleRotationFrac = (g_CreditsCurFrame2 % TICKS(4800)) / TICKS(4800.0f);

	if (joyGetButtons(0, R_TRIG) == 0) {
		creditsTickParticles();

		if (g_CreditsData->slidesenabled) {
			creditsTickSlide();
		} else if (RANDOMFRAC() < 0.01f) {
			creditsCreatePendingBgLayers(0xffffffff);
		}

		if (!g_CreditsData->slidesenabled && g_CreditsData->blacktimer60 < TICKS(1360)) {
			g_CreditsData->blacktimer60 += g_Vars.diffframe60;
		}
	}
#endif
}

Gfx *creditsDraw(Gfx *gdl)
{
	Mtxf sp68;
	Mtxf *matrix = gfxAllocateMatrix();
	struct coord sp58[1];
	struct coord sp4c[1];

	static u32 scrolltimer240 = 0;

	text0f1531dc(false);

#if VERSION >= VERSION_JPN_FINAL
	var800800f0jf = 0;
#endif

	g_ScaleX = 1;

#ifndef PLATFORM_N64
	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);
#endif

	gdl = viPrepareZbuf(gdl);
	gdl = vi0000b1d0(gdl);
	gdl = creditsFillFramebuffer(gdl, 0x000000ff);

	gDPSetScissorFrac(gdl++, G_SC_NON_INTERLACE, 0, 120, viGetWidth() * 4.0f, (viGetHeight() - 30) * 4.0f);

	creditsTick();

	// blacktimer60 is set to 0 when slides are finished, then ticks up
	// * 0 to 60 is fading to black
	// * 60 to 1200 is solid black
	// * 1200 to 1360 is fading to the scrolling title
	// So this is effectively checking if the screen is not solid black
	if (g_CreditsData->slidesenabled
			|| g_CreditsData->blacktimer60 < TICKS(60)
			|| g_CreditsData->blacktimer60 > TICKS(1200)) {
		u32 stack;

		if (!g_CreditsData->slidesenabled && g_CreditsData->blacktimer60 > TICKS(1200)) {
			// Render "PERFECT DARK" scrolling text
			if (!g_CreditsScrollStarted) {
				g_CreditsScrollStarted = true;
				scrolltimer240 = 0;

				menuUnsetModel(&g_CreditsData->menumodel);
			}

			scrolltimer240 += g_Vars.diffframe240;

			if (scrolltimer240 > TICKS(14400)) {
				scrolltimer240 = 0;
			}

			g_CreditsData->menumodel.newroty = 0;
			g_CreditsData->menumodel.curroty = 0;

			g_CreditsData->menumodel.currotx = g_CreditsData->menumodel.newrotx = -0.26175770163536;
			g_CreditsData->menumodel.currotz = g_CreditsData->menumodel.newrotz = 0;

			g_CreditsData->menumodel.newposx = 833.0f - (scrolltimer240 / TICKS(14400.0f)) * 2413.0f;
			g_CreditsData->menumodel.newposy = VERSION == VERSION_PAL_FINAL ? 65.86 : 70.86;
			g_CreditsData->menumodel.newposz = -2050;

			g_CreditsData->menumodel.newscale = 1.467;
			g_CreditsData->menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(FILE_PPDMENU);
			g_CreditsData->menumodel.drawbehinddialog = true;
			g_CreditsData->menumodel.partvisibility = NULL;

			gdl = menuRenderModel(gdl, &g_CreditsData->menumodel, MENUMODELTYPE_CREDITSLOGO);

			gSPMatrix(gdl++, osVirtualToPhysical(matrix), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

			gdl = creditsFillFramebuffer(gdl, 0x000000d8);
		}

		mtx4LoadIdentity(&sp68);
		mtxF2L(&sp68, matrix);
		camSetWorldToScreenMtxf(&sp68);

		gSPMatrix(gdl++, osVirtualToPhysical(matrix), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

		sp58[0].x = -1000;
		sp58[0].y = -1000;
		sp58[0].z = -1000;

		sp4c[0].x = 0;
		sp4c[0].y = 0;
		sp4c[0].z = -100;

		gdl = creditsDrawBackground(gdl);
		gdl = creditsDrawParticles(gdl);

		if (g_CreditsData->slidesenabled) {
			gdl = creditsDrawSlide(func0f0d479c(gdl));
		}

		if (!g_CreditsData->slidesenabled) {
			u32 alpha = 0;

			// If fading from credits to black
			if (g_CreditsData->blacktimer60 < TICKS(60)) {
				alpha = g_CreditsData->blacktimer60 * 0xff / TICKS(60);
			}

			// If fading from black to scroll
			if (g_CreditsData->blacktimer60 > TICKS(1200) && g_CreditsData->blacktimer60 < TICKS(1260)) {
				alpha = 0xff - (g_CreditsData->blacktimer60 - TICKS(1200)) * 0xff / TICKS(60);
			}

			if (alpha) {
				gdl = creditsFillFramebuffer(gdl, alpha);
			}
		}
	}

	// Exit the alternative title if a button is pressed (other than L or R)
#if VERSION >= VERSION_NTSC_1_0
	if (g_CreditsUsingAltTitle && joyGetButtonsPressedThisFrame(0, 0xffcf))
#else
	if (g_CreditsUsingAltTitle && joyGetButtons(0, 0xffff))
#endif
	{
		g_TitleNextStage = STAGE_CITRAINING;

		setNumPlayers(1);
		mainChangeToStage(g_TitleNextStage);

		g_Vars.bondplayernum = 0;
		g_Vars.coopplayernum = -1;
		g_Vars.antiplayernum = -1;

		lvSetDifficulty(DIFF_A);
		viBlack(true);
	}

	return gdl;
}

/**
 * Initialise the credits structures and variables.
 */
void creditsReset(void)
{
	g_CreditsCurFrame = 0;
	g_CreditsCurFrame2 = 0;
	g_CreditsScrollStarted = false;

	g_CreditsData = mempAlloc(sizeof(struct creditsdata), MEMPOOL_STAGE);

	g_CreditsData->unk2eec = 0;
	g_CreditsData->unk41b0[0] = 0;
	g_CreditsData->unk41b0[1] = 0;

	creditsCreatePendingBgLayers(0xffffffff);

	// Move layers 2 and 3 to 0 and 1
	creditsCopyBackgroundLayer(2, 0, true);
	creditsCopyBackgroundLayer(3, 1, true);

	g_CreditsData->slidesenabled = true;

	creditsResetParticles();
	creditsResetSlides();
	menuResetModel(&g_CreditsData->menumodel, 0x25800, true);

	g_CreditsData->menumodel.removingpiece = false;

	menuConfigureModel(&g_CreditsData->menumodel, 0, 0, 0, 0, 0, 0, 1, MENUMODELFLAG_HASSCALE | MENUMODELFLAG_HASROTATION);

	g_CreditsData->menumodel.newposy = 90;
	g_CreditsData->menumodel.curposx = 8.2;
	g_CreditsData->menumodel.curposy = -4.1;
	g_CreditsData->menumodel.curscale = 0.00393;
	g_CreditsData->menumodel.curroty = g_CreditsData->menumodel.newroty = 0;
	g_CreditsData->menumodel.rottimer60 = 60;

	g_CreditsUsingAltTitle = g_CreditsAltTitleRequested;

	if (g_CreditsAltTitleRequested) {
		g_CreditsAltTitleRequested = false;

		g_CreditsData->slidesenabled = false;
		g_CreditsData->blacktimer60 = TICKS(1140);
	}

	playerConfigureVi();
}

/**
 * Instruct the credits system to display the alternative title
 * instead of the regular credits.
 */
void creditsRequestAltTitle(void)
{
	g_CreditsAltTitleRequested = true;
}
