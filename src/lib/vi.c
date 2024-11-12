#include <ultra64.h>
#include "constants.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/file.h"
#include "game/zbuf.h"
#include "game/gfxmemory.h"
#include "game/menu.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#include "platform.h"
#endif

#define TO_U16_A(x) ((u16)(x))
#define TO_U16_B(x) ((x) & 0xffff)
#define TO_U16_C(x) ((u16)((x) & 0xffff))

#define ADD_LOW_AND_HI_16_TRUNCATE(reg, add) ((TO_U16_B(TO_U16_A((reg) >> 16) + (add)) << 16) | TO_U16_B(TO_U16_A(reg) + (add)))
#define ADD_LOW_AND_HI_16_MOD(reg, add)      ((((((reg >> 16) & 0xffff) + (add)) % 0xffff) << 16) | (((((reg >> 0) & 0xffff) + (add)) % 0xffff) << 0))

Mtxf var80092830;
Mtx *var80092870;
u16 g_ViPerspScale;
u8 g_ViFrontIndex;
u8 g_ViBackIndex;

struct rend_vidat g_ViDataArray[NUM_GFXTASKS] = {
	{
		0, 0, 0, 0,
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // x and y
		60,                                     // fovy
		(f32) FBALLOC_WIDTH_LO / (f32) FBALLOC_HEIGHT_LO, // aspect
		30,                                     // znear
		10000,                                  // zfar
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // bufx and bufy
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // viewx and viewy
		0, 0,                                   // viewleft and viewtop
		true,                                   // usezbuf
		0,
	}, {
		0, 0, 0, 0,
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // x and y
		60,                                     // fovy
		(f32) FBALLOC_WIDTH_LO / (f32) FBALLOC_HEIGHT_LO, // aspect
		30,                                     // znear
		10000,                                  // zfar
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // bufx and bufy
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // viewx and viewy
		0, 0,                                   // viewleft and viewtop
		true,                                   // usezbuf
		0,
	},
};

s32 g_ViTargetHStart = 0;
s32 g_ViTargetVStart = 0;
struct rend_vidat *g_ViFrontData = &g_ViDataArray[0];
struct rend_vidat *g_ViBackData = &g_ViDataArray[0];
bool g_ViIs16Bit = true;
bool g_ViReconfigured = false;
s32 g_ViSlot = 0;

void viConfigureForLogos(void)
{
	g_ViFrontIndex = 0;
	g_ViBackIndex = 1;

	g_ViFrontData = g_ViDataArray + g_ViFrontIndex;
	g_ViBackData = g_ViDataArray + g_ViBackIndex;

#if VERSION >= VERSION_PAL_FINAL
	if (IS4MB()) {
		g_ViDataArray[0].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].viewy = FBALLOC_HEIGHT_LO;

		g_ViDataArray[1].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[1].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[1].viewy = FBALLOC_HEIGHT_LO;

		g_ViTargetHStart = 0;
		g_ViTargetVStart = 0;
	} else {
		g_ViTargetHStart = 0;
		g_ViTargetVStart = VERSION >= VERSION_JPN_FINAL ? 0 : 12;
	}
#else
	g_ViTargetHStart = 0;
	g_ViTargetVStart = 0;

	if (IS4MB()) {
		g_ViDataArray[0].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].viewy = FBALLOC_HEIGHT_LO;

		g_ViDataArray[1].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[1].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[1].viewy = FBALLOC_HEIGHT_LO;
	}
#endif
}

/**
 * Configure the VI to display the copyright or accessingpak texture.
 *
 * The texture is the first thing displayed after power on.
 *
 * Both textures are 507 x 48, but the framebuffer width is 576.
 */
void viConfigureForCopyright(u16 *texturedata)
{
	s32 i;

	for (i = 0; i < NUM_GFXTASKS; i++) {
		g_FrameBuffers[i] = texturedata;

		g_ViDataArray[i].x = 576;
		g_ViDataArray[i].bufx = 576;
		g_ViDataArray[i].viewx = (VERSION >= VERSION_NTSC_1_0 ? 576 : 480);

		g_ViDataArray[i].y = 48;
		g_ViDataArray[i].bufy = 48;
		g_ViDataArray[i].viewy = 48;
	}

	g_ViFrontData->fb = g_FrameBuffers[g_ViFrontIndex];
	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	g_ViReconfigured = true;
	g_Vars.fourmeg2player = false;
}

/**
 * Configure the VI to display the legal screen.
 *
 * It's also used for the "check controllers" message
 * if controller 1 is not connected.
 */
void viConfigureForLegal(void)
{
	s32 i;

	for (i = 0; i < NUM_GFXTASKS; i++) {
		g_ViDataArray[i].x = FBALLOC_WIDTH_LO;
		g_ViDataArray[i].bufx = FBALLOC_WIDTH_LO;
		g_ViDataArray[i].viewx = FBALLOC_WIDTH_LO;

		g_ViDataArray[i].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[i].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[i].viewy = FBALLOC_HEIGHT_LO;
	}

	g_Vars.fourmeg2player = false;

#if PAL
	playerResetLoResIf4Mb();
#endif
}

const s16 g_ViModeWidths[]  = {FBALLOC_WIDTH_LO,  FBALLOC_WIDTH_LO,  SCREEN_320 * 2};
const s16 g_ViModeHeights[] = {FBALLOC_HEIGHT_LO, FBALLOC_HEIGHT_LO, (PAL ? 252 : 220) * 2};

/**
 * Allocate the colour framebuffers for the given stage.
 *
 * Regardless of whether hi-res is being used or not, the buffers are allocated
 * for hi-res. This is because hi-res can be changed mid-stage, and the engine
 * cannot reallocate the frame buffer without clearing the stage's entire memory
 * pool.
 *
 * The same is probably true for wide and cinema modes.
 */
void viReset(s32 stagenum)
{
	s32 i;
	s32 fbsize;
	u8 *ptr;
	u8 *fb0;
	u8 *fb1;

	g_Vars.fourmeg2player = false;

	if (stagenum == STAGE_TITLE || stagenum == STAGE_TEST_OLD) {
		if (IS4MB()) {
			viSetMode(VIMODE_HI);
			fbsize = (FBALLOC_WIDTH_LO * 2) * (FBALLOC_HEIGHT_LO * 2) * NUM_FRAMEBUFFERS;
		} else {
			viSetMode(VIMODE_HI);
			fbsize = g_ViModeWidths[2] * g_ViModeHeights[2] * NUM_FRAMEBUFFERS;
		}
	} else {
		viSetMode(VIMODE_LO);

		if (1);

		fbsize = IS4MB()
			? FBALLOC_WIDTH_LO * FBALLOC_HEIGHT_LO * NUM_FRAMEBUFFERS
			: FBALLOC_WIDTH_HI * FBALLOC_HEIGHT_HI * NUM_FRAMEBUFFERS;

		if (IS4MB() && PLAYERCOUNT() == 2) {
			// 4MB 2-player: The viewports are 110px tall
#if VERSION >= VERSION_NTSC_1_0
			fbsize = FBALLOC_WIDTH_LO * (FBALLOC_HEIGHT_LO / 2) * NUM_FRAMEBUFFERS;
#else
			fbsize = SCREEN_320 * (SCREEN_240 / 2) * NUM_FRAMEBUFFERS;
#endif
			g_Vars.fourmeg2player = true;
		} else if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && PLAYERCOUNT() == 2) {
			// PAL is using its correct size
			fbsize = SCREEN_WIDTH_LO * SCREEN_HEIGHT_LO * NUM_FRAMEBUFFERS;
		}
	}

	ptr = mempAlloc(fbsize * sizeof(u16) + 0x40, MEMPOOL_STAGE);

#ifdef PLATFORM_64BIT
	ptr = (u8*)(((uintptr_t)ptr + 0x3f) & 0xffffffffffffffc0);
#else
	ptr = (u8 *)(((uintptr_t) ptr + 0x3f) & 0xffffffc0);
#endif

	g_FrameBuffers[0] = (u16 *) ptr;
	g_FrameBuffers[1] = (u16 *) (fbsize + ptr);

	g_ViFrontData->fb = g_FrameBuffers[g_ViFrontIndex];
	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	fb0 = (u8 *) g_FrameBuffers[0];
	fb1 = (u8 *) g_FrameBuffers[1];

	for (i = 0; i < fbsize; i++) {
		fb0[i] = 0;
		fb1[i] = 0;
	}

	g_ViReconfigured = true;
}

/**
 * If black is true, set the video output to black indefinitely.
 * If black is false, unblack once all the framebuffers have been cycled through.
 *
 * The g_ViUnblackTimer value only ticks down when it's 2 or less,
 * so passing true to this function makes it not tick.
 */
void viBlack(bool black)
{
	black += NUM_FRAMEBUFFERS;
	g_ViUnblackTimer = black;
}

void viHandleRetrace(void)
{
	s32 prevmask;
	s32 offset;
	s32 reg;

	if (g_ViShakeTimer != 0) {
		g_ViShakeTimer--;

		if (g_ViShakeTimer == 0) {
			g_ViShakeIntensity = 0;
		}
	}

	offset = g_ViShakeDirection * g_ViShakeIntensity;
	g_ViShakeDirection = -g_ViShakeDirection;

#if VERSION >= VERSION_NTSC_1_0
	prevmask = osSetIntMask(1);
#endif

	reg = g_ViCurVStart0;
	var8008dd60[1 - var8005ce74]->fldRegs[0].vStart = ADD_LOW_AND_HI_16_TRUNCATE(reg, offset);

	reg = g_ViCurVStart1;
	var8008dd60[1 - var8005ce74]->fldRegs[1].vStart = ADD_LOW_AND_HI_16_TRUNCATE(reg, offset);

#if VERSION >= VERSION_NTSC_1_0
	osSetIntMask(prevmask);
#endif

#ifndef PLATFORM_N64
	videoSetWindowOffset(0, offset);
#endif

	osViSetMode(var8008dd60[1 - var8005ce74]);
	osViBlack(g_ViUnblackTimer);
	osViSetXScale(g_ViXScalesBySlot[1 - var8005ce74]);
	osViSetYScale(g_ViYScalesBySlot[1 - var8005ce74]);
	osViSetSpecialFeatures(OS_VI_GAMMA_OFF | OS_VI_DITHER_FILTER_ON);
}

void viUpdateMode(void)
{
	struct rend_vidat *prevdata;
	f32 x;
	f32 y;
	s32 reg;
	s32 v1;
	s32 tmp;
	s32 slot;
	s32 hstart;
	s32 vstart;

	if (g_ViFrontData->mode != g_ViBackData->mode) {
		switch (g_ViBackData->mode) {
		case VIMODE_NONE:
			osViSetYScale(1.0f);
			osViBlack(true);
			break;
		case VIMODE_LO:
			break;
		case VIMODE_HI:
			break;
		}
	}

	x = (f32) g_ViBackData->x / (f32) g_ViBackData->bufx;
	y = (f32) g_ViBackData->y / (f32) g_ViBackData->bufy;

	if (g_ViBackData->mode == VIMODE_NONE) {
		y = 1.0f; \
	} \
	slot = g_ViSlot;

	if (g_ViBackData->mode == 1);
	if (1);

	g_ViXScalesBySlot[slot] = x;
	g_ViYScalesBySlot[slot] = y;

	if (g_ViBackData->mode == VIMODE_LO) {
#if PAL
		if (g_ViIs16Bit) {
			var8008dcc0[slot] = osViModeTable[OS_VI_PAL_LAN1];
		} else {
			var8008dcc0[slot] = osViModeTable[OS_VI_PAL_LAN2];
		}
#else
		if (g_ViIs16Bit) {
			if (osTvType == OS_TV_MPAL) {
				var8008dcc0[slot] = osViModeTable[OS_VI_MPAL_LAN1];
			} else {
				var8008dcc0[slot] = osViModeTable[OS_VI_NTSC_LAN1];
			}
		} else {
			if (osTvType == OS_TV_MPAL) {
				if (g_ViIs16Bit && g_ViIs16Bit && g_ViIs16Bit);
				var8008dcc0[slot] = osViModeTable[OS_VI_MPAL_LAN2];
			} else {
				var8008dcc0[slot] = osViModeTable[OS_VI_NTSC_LAN2];
			}
		}
#endif

		var8008dcc0[slot].comRegs.width = g_ViBackData->bufx;
		var8008dcc0[slot].comRegs.xScale = g_ViBackData->bufx * 1024 / 640;
		var8008dcc0[slot].fldRegs[0].origin = g_ViBackData->bufx * 2;
		var8008dcc0[slot].fldRegs[1].origin = g_ViBackData->bufx * 2;

#if PAL
		var8008dcc0[slot].fldRegs[0].yScale = 1024;
		var8008dcc0[slot].fldRegs[1].yScale = 1024;
#else
		if (IS4MB()) {
			var8008dcc0[slot].fldRegs[0].yScale = 1024;
			var8008dcc0[slot].fldRegs[1].yScale = 1024;
		} else {
			var8008dcc0[slot].fldRegs[0].yScale = g_ViBackData->bufy * 2048 / 440;
			var8008dcc0[slot].fldRegs[1].yScale = g_ViBackData->bufy * 2048 / 440;
		}
#endif

		hstart = var8008dcc0[slot].comRegs.hStart;
		var8008de08 = var8008dcc0[slot].comRegs.hStart = ADD_LOW_AND_HI_16_MOD(hstart, g_ViTargetHStart);

		v1 = g_ViBackData->bufy;
		v1 = v1 * 1024 / var8008dcc0[slot].fldRegs[0].yScale;

		if (v1 > 300) {
			v1 >>= 1;
		}

		tmp = ((PAL ? 320 : 277) - v1);
		vstart = ((tmp + 2) << 16) | (tmp + ((v1 - 2) << 1) + 2);

		g_ViCurVStart0 = var8008dcc0[slot].fldRegs[0].vStart = ADD_LOW_AND_HI_16_MOD(vstart, g_ViTargetVStart);
		g_ViCurVStart1 = var8008dcc0[slot].fldRegs[1].vStart = ADD_LOW_AND_HI_16_MOD(vstart, g_ViTargetVStart);

		g_SchedViModesPending[slot] = true;
	} else if (g_ViBackData->mode == VIMODE_HI) {
#if PAL
		var8008dcc0[slot] = osViModeTable[OS_VI_PAL_HAF1];
#else
		if (osTvType == OS_TV_MPAL) {
			var8008dcc0[slot] = osViModeTable[OS_VI_MPAL_HAF1];
		} else {
			var8008dcc0[slot] = osViModeTable[OS_VI_NTSC_HAF1];
		}
#endif

		var8008dcc0[slot].comRegs.width = g_ViBackData->bufx;
		var8008dcc0[slot].comRegs.xScale = g_ViBackData->bufx * 1024 / 640;
		var8008dcc0[slot].fldRegs[0].yScale = 2048;
		var8008dcc0[slot].fldRegs[1].yScale = 2048;
		var8008dcc0[slot].fldRegs[0].origin = g_ViBackData->bufx * 2;
		var8008dcc0[slot].fldRegs[1].origin = g_ViBackData->bufx * 4;

		reg = var8008dcc0[slot].comRegs.hStart;
		var8008de08 = var8008dcc0[slot].comRegs.hStart = ADD_LOW_AND_HI_16_MOD(reg, g_ViTargetHStart);

		reg = var8008dcc0[slot].fldRegs[0].vStart;
		g_ViCurVStart0 = var8008dcc0[slot].fldRegs[0].vStart = ADD_LOW_AND_HI_16_MOD(reg, g_ViTargetVStart);

		reg = var8008dcc0[slot].fldRegs[1].vStart;
		g_ViCurVStart1 = var8008dcc0[slot].fldRegs[1].vStart = ADD_LOW_AND_HI_16_MOD(reg, g_ViTargetVStart);

		if (g_MainIsBooting) {
#if PAL
			g_ViCurVStart0 = var8008dcc0[slot].fldRegs[0].vStart = ((g_ViTargetVStart + 506) % 0xffff) << 16 | (g_ViTargetVStart + 134) % 0xffff;
			g_ViCurVStart1 = var8008dcc0[slot].fldRegs[1].vStart = ((g_ViTargetVStart + 508) % 0xffff) << 16 | (g_ViTargetVStart + 132) % 0xffff;
#else
			g_ViCurVStart0 = var8008dcc0[slot].fldRegs[0].vStart = ((g_ViTargetVStart + 431) % 0xffff) << 16 | (g_ViTargetVStart + 123) % 0xffff;
			g_ViCurVStart1 = var8008dcc0[slot].fldRegs[1].vStart = ((g_ViTargetVStart + 433) % 0xffff) << 16 | (g_ViTargetVStart + 121) % 0xffff;
#endif
		}

		g_SchedViModesPending[slot] = true;
	} else {
		g_SchedViModesPending[slot] = false;
	}

	slot = (slot + 1) % NUM_GFXTASKS;
	g_ViSlot = slot;

	g_RdpCurTask->framebuffer = g_ViIs16Bit ? g_ViBackData->fb : g_FrameBuffers[0];

	prevdata = g_ViBackData;

	g_ViFrontIndex = (g_ViFrontIndex + 1) % NUM_FRAMEBUFFERS;
	g_ViBackIndex = (g_ViBackIndex + 1) % NUM_FRAMEBUFFERS;

	g_ViFrontData = g_ViDataArray + g_ViFrontIndex;
	g_ViBackData = g_ViDataArray + g_ViBackIndex;

	bcopy(prevdata, g_ViBackData, sizeof(struct rend_vidat));

	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	if (g_ViReconfigured) {
		g_ViReconfigured = false;
		viBlack(false);
	}
}

void viShake(f32 intensity)
{
	if (intensity > 14) {
		intensity = 14;
	}

	if (intensity < 0) {
		intensity = 0;
	}

#ifdef PLATFORM_N64
	g_ViShakeIntensity = intensity;
	g_ViShakeTimer = 10;
#else
	g_ViShakeIntensity = intensity * g_ViShakeIntensityMult;
	g_ViShakeTimer = 20;
#endif
}

void viSetMode(s32 mode)
{
	g_ViBackData->mode = mode;

	g_ViBackData->x = g_ViBackData->bufx = g_ViModeWidths[mode];
	g_ViBackData->y = g_ViBackData->bufy = g_ViModeHeights[mode];
}

void viSet16Bit(void)
{
	g_ViIs16Bit = true;
}

void viSet32Bit(void)
{
	g_ViIs16Bit = false;
}

u16 *viGetBackBuffer(void)
{
	return g_ViBackData->fb;
}

u16 *viGetFrontBuffer(void)
{
	return g_ViFrontData->fb;
}

void viSetBackBuffer(u16 *fb)
{
	g_ViBackData->fb = fb;
}

Vp *viGetCurrentPlayerViewport(void)
{
	return &g_Vars.currentplayer->viewport[g_ViBackIndex];
}

u16 viGetPerspScale(void)
{
	return g_ViPerspScale;
}

Gfx *vi0000ab78(Gfx *gdl)
{
	Mtxf sp110;
	Mtxf spd0;
	Mtxf sp90;
	Mtxf sp50;
	Mtx *sp4c;
	Mtx *sp48;
	u16 sp46;

	guPerspectiveF(sp110.m, &sp46, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar + g_ViBackData->zfar, 1);
	mtx4Copy(camGetWorldToScreenMtxf(), &sp90);

	sp90.m[3][0] = 0;
	sp90.m[3][1] = 0;
	sp90.m[3][2] = 0;

	mtx4MultMtx4(&sp110, &sp90, &spd0);
	sp4c = gfxAllocateMatrix();
	guMtxF2L(spd0.m, sp4c);

	mtx4LoadIdentity(&sp50);
	sp48 = gfxAllocateMatrix();
	guMtxF2L(sp50.m, sp48);

	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(sp4c), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(sp48), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gSPPerspNormalize(gdl++, sp46);

	return gdl;
}

Gfx *vi0000aca4(Gfx *gdl, f32 znear, f32 zfar)
{
	u16 scale;
	Mtxf tmp;
	Mtx *mtx = gfxAllocateMatrix();

	guPerspectiveF(tmp.m, &scale, g_ViBackData->fovy, g_ViBackData->aspect, znear, zfar, 1);
	guMtxF2L(tmp.m, mtx);

	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPPerspNormalize(gdl++, scale);

	return gdl;
}

Gfx *vi0000ad5c(Gfx *gdl, Vp *vp)
{
	vp[g_ViBackIndex].vp.vscale[0] = g_ViBackData->viewx * 2;
	vp[g_ViBackIndex].vp.vtrans[0] = g_ViBackData->viewx * 2 + g_ViBackData->viewleft * 4;

	vp[g_ViBackIndex].vp.vscale[1] = g_ViBackData->viewy * 2;
	vp[g_ViBackIndex].vp.vtrans[1] = g_ViBackData->viewy * 2 + g_ViBackData->viewtop * 4;

	gSPViewport(gdl++, OS_K0_TO_PHYSICAL(&vp[g_ViBackIndex]));

	var80092870 = gfxAllocateMatrix();
	guPerspectiveF(var80092830.m, &g_ViPerspScale, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	guMtxF2L(var80092830.m, var80092870);

	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(var80092870), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPPerspNormalize(gdl++, g_ViPerspScale);

	camSetPerspectiveMtxL(var80092870);
	camSetMtxF1754(&var80092830);

	return gdl;
}

Gfx *vi0000af00(Gfx *gdl, Vp *vp)
{
	vp[g_ViBackIndex].vp.vscale[0] = g_ViBackData->viewx * 2;
	vp[g_ViBackIndex].vp.vtrans[0] = g_ViBackData->viewx * 2 + g_ViBackData->viewleft * 4;

	vp[g_ViBackIndex].vp.vscale[1] = g_ViBackData->viewy * 2;
	vp[g_ViBackIndex].vp.vtrans[1] = g_ViBackData->viewy * 2 + g_ViBackData->viewtop * 4;

	vp[g_ViBackIndex].vp.vscale[2] = 511;
	vp[g_ViBackIndex].vp.vtrans[2] = 511;

	vp[g_ViBackIndex].vp.vscale[3] = 0;
	vp[g_ViBackIndex].vp.vtrans[3] = 0;

	gSPViewport(gdl++, OS_K0_TO_PHYSICAL(&vp[g_ViBackIndex]));

	var80092870 = gfxAllocateMatrix();
	guPerspectiveF(var80092830.m, &g_ViPerspScale, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	guMtxF2L(var80092830.m, var80092870);

	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(var80092870), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPPerspNormalize(gdl++, g_ViPerspScale);

	camSetPerspectiveMtxL(var80092870);
	camSetMtxF1754(&var80092830);

	return gdl;
}

Gfx *vi0000b0e8(Gfx *gdl, f32 fovy, f32 aspect)
{
	Mtxf tmp;
	Mtx *mtx = gfxAllocateMatrix();

	guPerspectiveF(tmp.m, &g_ViPerspScale, fovy, aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	guMtxF2L(tmp.m, mtx);

	gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPPerspNormalize(gdl++, g_ViPerspScale);

	return gdl;
}

Gfx *vi0000b1a8(Gfx *gdl)
{
	return vi0000ad5c(gdl, &g_Vars.currentplayer->viewport[0]);
}

Gfx *vi0000b1d0(Gfx *gdl)
{
	gdl = vi0000b1a8(gdl);

	if (g_ViIs16Bit) {
		gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, g_ViBackData->bufx, OS_K0_TO_PHYSICAL(g_ViBackData->fb));
	} else {
		gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_32b, g_ViBackData->bufx, OS_K0_TO_PHYSICAL(g_FrameBuffers[0]));
	}

	return gdl;
}

Gfx *viPrepareZbuf(Gfx *gdl)
{
	if (g_ViBackData->usezbuf) {
		gdl = zbufConfigureRdp(gdl);
		gdl = zbufClear(gdl);
	}

	return gdl;
}

Gfx *viFillBuffer(Gfx *gdl)
{
	gDPSetCycleType(gdl++, G_CYC_FILL);
	gDPFillRectangle(gdl++, 0, 0, g_ViBackData->bufx - 1, g_ViBackData->bufy - 1);
	gDPPipeSync(gdl++);

	return gdl;
}

Gfx *viRenderViewportEdges(Gfx *gdl)
{
	gDPSetCycleType(gdl++, G_CYC_FILL);
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, viGetWidth(), viGetHeight());
	gDPSetFillColor(gdl++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));

#if VERSION >= VERSION_NTSC_1_0
	if (PLAYERCOUNT() == 1
			|| ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
				&& playerHasSharedViewport() && g_Vars.currentplayernum == 0))
#else
	if (PLAYERCOUNT() == 1
			|| ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
				&& (
					(g_InCutscene && !g_MainIsEndscreen) || menuGetRoot() == MENUROOT_COOPCONTINUE
					) && g_Vars.currentplayernum == 0))
#endif
	{
		// Single viewport
		if (viGetViewTop() > 0) {
			// Fill above
			gDPFillRectangle(gdl++, 0, 0, viGetWidth() - 1, viGetViewTop() - 1);
			gDPPipeSync(gdl++);
		}

		if (viGetViewTop() + viGetViewHeight() < viGetHeight()) {
			// Fill below
			gDPFillRectangle(gdl++,
					0, viGetViewTop() + viGetViewHeight(),
					viGetWidth() - 1, viGetHeight() - 1);
			gDPPipeSync(gdl++);
		}
	} else {
		if (g_Vars.currentplayerindex == 0) {
			s32 topplayernum = 0;
			s32 bottomplayernum = 0;
			s32 tmpplayernum = 0;

			if (PLAYERCOUNT() == 2) {
				bottomplayernum = 1;
				tmpplayernum = 1;
			} else if (PLAYERCOUNT() >= 3) {
				bottomplayernum = 2;
				tmpplayernum = 2;
			}

			if (g_Vars.players[topplayernum]->viewtop > 0) {
				// Fill above all viewports - full width
				gDPFillRectangle(gdl++, 0, 0, viGetWidth() - 1, g_Vars.players[topplayernum]->viewtop - 1);
				gDPPipeSync(gdl++);
			}

			if (g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight < viGetHeight()) {
				// Fill below all viewports - full width
				gDPFillRectangle(gdl++,
						0, g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight,
						viGetWidth() - 1, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}

			// Horizontal middle line
			gDPFillRectangle(gdl++,
					0, g_Vars.players[tmpplayernum]->viewtop - 1,
					viGetWidth() - 1, g_Vars.players[tmpplayernum]->viewtop - 1);
			gDPPipeSync(gdl++);

			if (PLAYERCOUNT() >= 3 ||
					(PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || g_Vars.fourmeg2player))) {
				if (PLAYERCOUNT() == 2) {
					tmpplayernum = 0;
				}

				// Vertical middle line
				gDPFillRectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, 0,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}

			if (PLAYERCOUNT() == 3) {
				// Blank square in P4 spot
				gDPFillRectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth + 1, g_Vars.players[tmpplayernum]->viewtop,
						viGetWidth() - 1, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}
		}
	}

	return gdl;
}

#if VERSION < VERSION_NTSC_1_0
void viSetVStart(s32 vstart)
{
	g_ViTargetVStart = vstart;
}

s32 viGetVStart(void)
{
	return g_ViTargetVStart;
}

void viSetHStart(s32 hstart)
{
	g_ViTargetHStart = hstart;
}

s32 viGetHStart(void)
{
	return g_ViTargetHStart;
}
#endif

void viSetBufSize(s16 width, s16 height)
{
	g_ViBackData->bufx = width;
	g_ViBackData->bufy = height;
}

s16 viGetBufWidth(void)
{
	return g_ViBackData->bufx;
}

s16 viGetBufHeight(void)
{
	return g_ViBackData->bufy;
}

void viSetSize(s16 width, s16 height)
{
	g_ViBackData->x = width;
	g_ViBackData->y = height;
}

s16 viGetWidth(void)
{
	return g_ViBackData->x;
}

s16 viGetHeight(void)
{
	return g_ViBackData->y;
}

void viSetViewSize(s16 width, s16 height)
{
	g_ViBackData->viewx = width;
	g_ViBackData->viewy = height;

	camSetScreenSize(g_ViBackData->viewx, g_ViBackData->viewy);
	camSetScale();
}

s16 viGetViewWidth(void)
{
	return g_ViBackData->viewx;
}

s16 viGetViewHeight(void)
{
	return g_ViBackData->viewy;
}

void viSetViewPosition(s16 left, s16 top)
{
	g_ViBackData->viewleft = left;
	g_ViBackData->viewtop = top;

	camSetScreenPosition(g_ViBackData->viewleft, g_ViBackData->viewtop);
}

s16 viGetViewLeft(void)
{
	return g_ViBackData->viewleft;
}

s16 viGetViewTop(void)
{
	return g_ViBackData->viewtop;
}

void viSetUseZBuf(bool use)
{
	g_ViBackData->usezbuf = use;
}

void viSetFovY(f32 fovy)
{
	g_ViBackData->fovy = fovy;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

void viSetAspect(f32 aspect)
{
	g_ViBackData->aspect = aspect;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

f32 viGetAspect(void)
{
	return g_ViBackData->aspect;
}

void viSetFovAspectAndSize(f32 fovy, f32 aspect, s16 width, s16 height)
{
	g_ViBackData->fovy = fovy;
	g_ViBackData->aspect = aspect;
	g_ViBackData->viewx = width;
	g_ViBackData->viewy = height;

	camSetScreenSize(g_ViBackData->viewx, g_ViBackData->viewy);
	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

f32 viGetFovY(void)
{
	return g_ViBackData->fovy;
}

void viSetZRange(f32 near, f32 far)
{
	g_ViBackData->znear = near;
	g_ViBackData->zfar = far;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

void viGetZRange(struct zrange *zrange)
{
	zrange->near = g_ViBackData->znear;
	zrange->far = g_ViBackData->zfar;
}

Gfx *viSetFillColour(Gfx *gdl, s32 r, s32 g, s32 b)
{
	if (g_ViIs16Bit) {
		gDPSetFillColor(gdl++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
	} else {
		(gdl++, r << 24 | g << 16 | b << 8 | 0xff);
	}

	return gdl;
}

void viGrabJpg16(void)
{
	// empty
}

void viGrabJpg32(void)
{
	// empty
}

void viGrabRgb16(void)
{
	// empty
}

void viGrabRgb32(void)
{
	// empty
}
