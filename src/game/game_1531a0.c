#include <ultra64.h>
#include "constants.h"
#include "game/game_006900.h"
#include "game/gfxmemory.h"
#include "game/savebuffer.h"
#include "game/game_1531a0.h"
#include "game/file.h"
#include "game/lang.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "platform.h"
#include "ext_tex.h"

#define SPACE_WIDTH 5

#define BLENDTYPE_DIAGONAL   0x01
#define BLENDTYPE_VERTICAL   0x02
#define BLENDTYPE_WAVE       0x04
#define BLENDTYPE_MENU       0x08
#define BLENDTYPE_HORIZONTAL 0x10

struct blendsettings {
	/*0x00*/ u8 types;
	/*0x04*/ u32 colour04;
	/*0x08*/ u32 colour08;
	/*0x0c*/ s32 diagrefx;
	/*0x10*/ s32 diagrefy;
	/*0x14*/ f32 diagtimer;
	/*0x18*/ u8 diagmode;
	/*0x1c*/ s32 backupdiagrefx;
	/*0x20*/ s32 backupdiagrefy;
	/*0x24*/ f32 backupdiagtimer;
	/*0x28*/ u8 backupdiagmode;
	/*0x29*/ u8 backupdiagtypes;
	/*0x2a*/ u8 backuptypes;
	/*0x2c*/ s32 vertrefy1;
	/*0x30*/ s32 vertrefy2;
	/*0x34*/ s32 vert34;
	/*0x38*/ s32 horizrefx1;
	/*0x3c*/ s32 horizrefx2;
	/*0x40*/ s32 horiz40;
	/*0x44*/ u32 colour44;
	/*0x48*/ u32 colour48;
	/*0x4c*/ s32 wave4c;
	/*0x50*/ s32 wave50;
	/*0x54*/ s32 wave54;
	/*0x58*/ u32 wavecolour1;
	/*0x5c*/ u32 wavecolour2;
	/*0x60*/ u32 menuweight;
};

#if VERSION == VERSION_JPN_FINAL
s32 g_JpnKerning[13 * 13];
#endif

struct blendsettings g_Blend;
Gfx *var800a4634;
Gfx *var800a4638;
#if VERSION == VERSION_JPN_FINAL
struct fontchar g_TmpJpnChar;
#endif
u32 var800a463c;

#if VERSION == VERSION_JPN_FINAL
s32 var800800f0jf = 0;
s32 g_ScaleX = 1;
bool var80080104jf = false;
s32 var8007fac4 = 0;
bool g_TextRotated90 = false;
s32 g_WrapIndentCount = 0;
s32 var8007fad0 = 1;
s32 var80080108jf = 1;
#else
s32 g_ScaleX = 1;
s32 var8007fac4 = 0;
bool g_TextRotated90 = false;
s32 g_WrapIndentCount = 0;
s32 var8007fad0 = 1;
#endif

s32 var8007fad4 = -1;
u32 var8007fad8 = 0x00000000;
s32 var8007fadc = 0;
s32 var8007fae0 = 0;
u32 var8007fae4 = 0x00000000;
u32 var8007fae8 = 0x00000000;
u32 var8007faec = 0x00000000;
u32 var8007faf0 = 0x00000000;

struct font *g_FontTahoma2 = NULL;
struct fontchar *g_FontTahoma1 = NULL;
struct font *g_FontNumeric = NULL;
struct fontchar *g_CharsNumeric = NULL;
struct font *g_FontHandelGothicXs = NULL;
struct fontchar *g_CharsHandelGothicXs = NULL;
struct font *g_FontHandelGothicSm = NULL;
struct fontchar *g_CharsHandelGothicSm = NULL;
struct font *g_FontHandelGothicMd = NULL;
struct fontchar *g_CharsHandelGothicMd = NULL;
struct font *g_FontHandelGothicLg = NULL;
struct fontchar *g_CharsHandelGothicLg = NULL;

u32 var8007fb24 = 0x00000000;
u32 var8007fb28 = 0x00000000;
u32 var8007fb2c = 0x00000000;
u32 var8007fb30 = 0x00000000;
u32 var8007fb34 = 0x00000000;
u32 var8007fb38 = 0x00000000;

u16 var8007fb3c[] = {
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
	0xff00, 0xff24, 0xff48, 0xff6c, 0xff90, 0xffb4, 0xffd8, 0xffff,
};

u16 var8007fb5c[] = {
	0xff00, 0xff58, 0xff74, 0xff90, 0xffac, 0xffc8, 0xffe4, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
	0xff00, 0xff18, 0xff30, 0xff5c, 0xff88, 0xffb4, 0xffd8, 0xffff,
};

#if VERSION == VERSION_JPN_FINAL
u16 var800801d8jf[] = {
	0xff00, 0xff11, 0xff22, 0xff33, 0xff44, 0xff55, 0xff66, 0xff77,
	0xff88, 0xff99, 0xffaa, 0xffbb, 0xffcc, 0xffdd, 0xffee, 0xffff,
};
#endif

bool var8007fb9c = false;
s32 var8007fba0 = 0;
s32 var8007fba4 = -1;
u32 var8007fba8 = 0x00000000;
u32 var8007fbac = 0x00000001;
u32 var8007fbb0 = 0x00000064;
u32 var8007fbb4 = 0x0000002c;
u32 var8007fbb8 = 0x00000080;
u32 var8007fbbc = 0x0000003c;
u32 var8007fbc0 = 0x44444400;
u32 var8007fbc4 = 0xffffff00;

#ifndef PLATFORM_N64
s32 g_HudCenter = HUDCENTER_NONE;
u32 g_HudAlignModeL = G_ASPECT_LEFT_EXT;
u32 g_HudAlignModeR = G_ASPECT_RIGHT_EXT;
#endif

void textInit(void)
{
	// empty
}

void text0f1531a8(s32 arg0)
{
	var8007fac4 = -arg0;
}

void textSetRotation90(bool rotated)
{
	g_TextRotated90 = rotated;
}

void textSetWrapIndent(s32 count)
{
	g_WrapIndentCount = count;
}

void text0f1531d0(s32 arg0)
{
	var8007fad4 = arg0;
}

void text0f1531dc(bool arg0)
{
	if (arg0) {
		var8007fad0 = 2;
	} else {
		var8007fad0 = 1;
	}

#if VERSION == VERSION_JPN_FINAL
	var80080108jf = 1;
#endif
}

void textLoadFont(u8 *romstart, u8 *romend, struct font **fontptr, struct fontchar **charsptr, bool monospace)
{
	extern u8 EXT_SEG _fonthandelgothicsmSegmentRomStart;
	extern u8 EXT_SEG _fonthandelgothicxsSegmentRomStart;
	extern u8 EXT_SEG _fonthandelgothicmdSegmentRomStart;

	u32 len;
	s32 maxwidth;
	s32 i;
	struct font *font;
	struct fontchar *chars;

#if VERSION >= VERSION_PAL_BETA
	s32 numchars = 94;

#if PAL
	// PAL has more characters in these fonts
	if (romstart == REF_SEG _fonthandelgothicsmSegmentRomStart
			|| romstart == REF_SEG _fonthandelgothicxsSegmentRomStart
			|| romstart == REF_SEG _fonthandelgothicmdSegmentRomStart) {
		numchars = 135;
	}
#endif
#define NUMCHARS() numchars
#else
#define NUMCHARS() 94
#endif

	len = (romptr_t)romend - (romptr_t)romstart;
	font = mempAlloc(len, MEMPOOL_STAGE);
	chars = font->chars;

	dmaExec(font, (romptr_t) romstart, len);

	// Convert pointers
	for (i = 0; i < NUMCHARS(); i++) {
		chars[i].pixeldata += (uintptr_t)font;
	}

#if VERSION >= VERSION_JPN_FINAL
	if (romstart == REF_SEG _fonthandelgothicsmSegmentRomStart) {
		for (i = 0; i < NUMCHARS(); i++) {
			chars[i].baseline++;
		}
	}
#endif

	// If requested monospace, set all widths to the max, minus 1 for some reason
	if (monospace) {
		maxwidth = 0;

		for (i = 0; i < NUMCHARS(); i++) {
			if (chars[i].width > maxwidth) {
				maxwidth = chars[i].width;
			}
		}

		maxwidth--;

		for (i = 0; i < NUMCHARS(); i++) {
			chars[i].width = maxwidth;
		}
	}

	*fontptr = font;
	*charsptr = chars;

#if PAL
	if (romstart == REF_SEG _fonthandelgothicsmSegmentRomStart
			|| romstart == REF_SEG _fonthandelgothicxsSegmentRomStart
			|| romstart == REF_SEG _fonthandelgothicmdSegmentRomStart) {
		// Increment the baseline of the pipe character.
		// It is suspected that the pipe character is used as a reference to set
		// the size for all other text, so changing this increases the line
		// height of these fonts.
		(*charsptr)['|' - 0x21].baseline++;
	}
#endif
}

void textReset(void)
{
	extern u8 EXT_SEG _fontbankgothicSegmentRomStart,     EXT_SEG _fontbankgothicSegmentRomEnd;
	extern u8 EXT_SEG _fontzurichSegmentRomStart,         EXT_SEG _fontzurichSegmentRomEnd;
	extern u8 EXT_SEG _fonttahomaSegmentRomStart,         EXT_SEG _fonttahomaSegmentRomEnd;
	extern u8 EXT_SEG _fontnumericSegmentRomStart,        EXT_SEG _fontnumericSegmentRomEnd;
	extern u8 EXT_SEG _fonthandelgothicsmSegmentRomStart, EXT_SEG _fonthandelgothicsmSegmentRomEnd;
	extern u8 EXT_SEG _fonthandelgothicxsSegmentRomStart, EXT_SEG _fonthandelgothicxsSegmentRomEnd;
	extern u8 EXT_SEG _fonthandelgothicmdSegmentRomStart, EXT_SEG _fonthandelgothicmdSegmentRomEnd;
	extern u8 EXT_SEG _fonthandelgothiclgSegmentRomStart, EXT_SEG _fonthandelgothiclgSegmentRomEnd;
	extern u8 EXT_SEG _fontocramdSegmentRomStart,         EXT_SEG _fontocramdSegmentRomEnd;
	extern u8 EXT_SEG _fontocralgSegmentRomStart,         EXT_SEG _fontocralgSegmentRomEnd;

	var8007faec = 0;
	g_FontTahoma2 = NULL;
	g_FontNumeric = NULL;
	g_FontHandelGothicXs = NULL;
	g_FontHandelGothicSm = NULL;
	g_FontHandelGothicMd = NULL;
	g_FontHandelGothicLg = NULL;
	var8007fb24 = 0;
	var8007fb2c = 0;
	var8007fb34 = 0;

	var8007faf0 = 0;
	g_FontTahoma1 = NULL;
	g_CharsNumeric = NULL;
	g_CharsHandelGothicXs = NULL;
	g_CharsHandelGothicSm = NULL;
	g_CharsHandelGothicMd = NULL;
	g_CharsHandelGothicLg = NULL;
	var8007fb28 = 0;
	var8007fb30 = 0;
	var8007fb38 = 0;

	var8007fac4 = 0;
	g_TextRotated90 = false;
	g_WrapIndentCount = 0;
	var8007fad4 = -1;
	var8007fad8 = 0;
	var8007fadc = 0;
	var8007fae0 = 0;
	var8007fae4 = 0;
	var8007fae8 = 0;

	if (g_Vars.stagenum == STAGE_TITLE) {
		textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);
		textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);
		textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);
	} else if (g_Vars.stagenum == STAGE_CREDITS) {
		textLoadFont(REF_SEG _fonthandelgothicxsSegmentRomStart, REF_SEG _fonthandelgothicxsSegmentRomEnd, &g_FontHandelGothicXs, &g_CharsHandelGothicXs, false);
		textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);
		textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);
		textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);
	} else {
#if VERSION >= VERSION_JPN_FINAL
		textLoadFont(REF_SEG _fontnumericSegmentRomStart, REF_SEG _fontnumericSegmentRomEnd, &g_FontNumeric, &g_CharsNumeric, false);

		if (g_Vars.normmplayerisrunning) {
			if (IS4MB()) {
				s32 i;

				for (i = 0; i < 169; i++) {
					g_JpnKerning[i] = 0;
				}

				g_FontHandelGothicXs = g_FontHandelGothicSm = (struct font *) g_JpnKerning;
				return;
			}
		}

		textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);

		if (!g_Vars.normmplayerisrunning || IS8MB()) {
			textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);
		}

		if (g_Vars.stagenum == STAGE_TEST_OLD) {
			textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);
		}

		textLoadFont(REF_SEG _fonthandelgothicxsSegmentRomStart, REF_SEG _fonthandelgothicxsSegmentRomEnd, &g_FontHandelGothicXs, &g_CharsHandelGothicXs, false);
#elif VERSION >= VERSION_PAL_BETA
		textLoadFont(REF_SEG _fontnumericSegmentRomStart, REF_SEG _fontnumericSegmentRomEnd, &g_FontNumeric, &g_CharsNumeric, false);
		textLoadFont(REF_SEG _fonthandelgothicxsSegmentRomStart, REF_SEG _fonthandelgothicxsSegmentRomEnd, &g_FontHandelGothicXs, &g_CharsHandelGothicXs, false);
		textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);

		if (!g_Vars.normmplayerisrunning) {
			textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);
		}

		if (g_Vars.stagenum == STAGE_TEST_OLD) {
			textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);
		}
#else
		// This unused GE font exists in NTSC but was removed in the PAL version
		textLoadFont(REF_SEG _fonttahomaSegmentRomStart, REF_SEG _fonttahomaSegmentRomEnd, &g_FontTahoma2, &g_FontTahoma1, false);

		textLoadFont(REF_SEG _fontnumericSegmentRomStart, REF_SEG _fontnumericSegmentRomEnd, &g_FontNumeric, &g_CharsNumeric, false);
		textLoadFont(REF_SEG _fonthandelgothicxsSegmentRomStart, REF_SEG _fonthandelgothicxsSegmentRomEnd, &g_FontHandelGothicXs, &g_CharsHandelGothicXs, false);
		textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);
		textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);

		if (g_Vars.stagenum == STAGE_TEST_OLD) {
			textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);
		}
#endif
	}
}

Gfx *text0f153628(Gfx *gdl)
{
	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);

	if (var8007fad8) {
		gDPSetTextureFilter(gdl++, G_TF_AVERAGE);
	}
#if VERSION == VERSION_JPN_FINAL
	else if (var80080108jf == 2) {
		gDPSetTextureFilter(gdl++, G_TF_POINT);
	}
#endif
	else {
		gDPSetTextureFilter(gdl++, TEX_FILTER_2D);
	}

	return gdl;
}

Gfx *text0f153780(Gfx *gdl)
{
	gDPPipeSync(gdl++);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetTextureLOD(gdl++, G_TL_LOD);

	return gdl;
}

Gfx *textSetPrimColour(Gfx *gdl, u32 colour)
{
	gDPPipeSync(gdl++);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);

	return gdl;
}

Gfx *text0f153838(Gfx *gdl)
{
	gDPSetCombineMode(gdl++, G_CC_CUSTOM_02, G_CC_CUSTOM_02);

	return gdl;
}

Gfx *text0f153858(Gfx *gdl, s32 *x1, s32 *y1, s32 *x2, s32 *y2)
{
	gdl = textSetPrimColour(gdl, 0x00000000);

	gDPFillRectangle(gdl++, *x1, *y1, *x2, *y2);

	gdl = text0f153838(gdl);

	return gdl;
}

Gfx *text0f1538e4(Gfx *gdl, s32 *x1, s32 *y1, s32 *x2, s32 *y2)
{
	gdl = textSetPrimColour(gdl, 0x00000000);

	gDPFillRectangleScaled(gdl++, *x1, *y1, *x2, *y2);

	gdl = text0f153838(gdl);

	return gdl;
}

#if VERSION >= VERSION_NTSC_1_0
Gfx *text0f153990(Gfx *gdl, s32 left, s32 top, s32 width, s32 height)
{
	gdl = textSetPrimColour(gdl, 0x00000000);

#if VERSION >= VERSION_JPN_FINAL
	gDPFillRectangle(gdl++, left - 1, top - 1, width * var8007fad0 + left + 1, top + height * var80080108jf + 1);
#else
	gDPFillRectangle(gdl++, left - 1, top - 1, width * var8007fad0 + left + 1, top + height + 1);
#endif

	gdl = text0f153838(gdl);

	return gdl;
}
#endif

Gfx *text0f153a34(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, u32 colour)
{
	gdl = textSetPrimColour(gdl, colour);

	gDPFillRectangle(gdl++, x1, y1, x2, y2);

	gdl = text0f153838(gdl);

	return gdl;
}

Gfx *text0f153ab0(Gfx *gdl)
{
	Gfx *allocation;

	var8007fb9c = true;

	allocation = gfxAllocate(sizeof(Gfx) * 530);

	var800a4634 = allocation;
	var800a4638 = allocation + 530;

	gSPDisplayList(gdl++, var800a4634);

	gdl = func0f0d4c80(gdl);

	var800a4634 = func0f0d4a3c(var800a4634, 0);
	var8007fba4 = -1;

	return gdl;
}

void text0f153b40(void)
{
	var8007fb9c = false;

	gSPEndDisplayList(var800a4634++);
}

void text0f153b6c(s32 arg0)
{
	if (arg0 != var8007fba4) {
		f32 tmp = g_Blend.diagtimer * g_Blend.diagtimer - (f32)((arg0 - g_Blend.diagrefy) * (arg0 - g_Blend.diagrefy));

		if (tmp > 0.0f) {
			var8007fba0 = sqrtf(tmp) + g_Blend.diagrefx;
		} else {
			var8007fba0 = 0;
		}

		var8007fba4 = arg0;
	}
}

void textSetDiagonalBlend(s32 x, s32 y, f32 timer, u8 mode)
{
	g_Blend.types |= BLENDTYPE_DIAGONAL;
	g_Blend.diagrefx = x;
	g_Blend.diagrefy = y;
	g_Blend.diagtimer = timer;
	g_Blend.diagmode = mode;
}

void textBackupDiagonalBlendSettings(void)
{
	g_Blend.backupdiagrefx = g_Blend.diagrefx;
	g_Blend.backupdiagrefy = g_Blend.diagrefy;
	g_Blend.backupdiagtimer = g_Blend.diagtimer;
	g_Blend.backupdiagmode = g_Blend.diagmode;
	g_Blend.backupdiagtypes = g_Blend.types & BLENDTYPE_DIAGONAL;
}

void textRestoreDiagonalBlendSettings(void)
{
	g_Blend.diagrefx = g_Blend.backupdiagrefx;
	g_Blend.diagrefy = g_Blend.backupdiagrefy;
	g_Blend.diagtimer = g_Blend.backupdiagtimer;
	g_Blend.diagmode = g_Blend.backupdiagmode;
	g_Blend.types |= g_Blend.backupdiagtypes;
}

void textSetVerticalBlend(s32 y1, s32 y2, u32 arg2)
{
	g_Blend.types |= BLENDTYPE_VERTICAL;
	g_Blend.vertrefy1 = y1;
	g_Blend.vertrefy2 = y2;
	g_Blend.vert34 = arg2;
}

void textSetHorizontalBlend(s32 x1, s32 x2, u32 arg2)
{
	g_Blend.types |= BLENDTYPE_HORIZONTAL;
	g_Blend.horizrefx1 = x1;
	g_Blend.horizrefx2 = x2;
	g_Blend.horiz40 = arg2;
}

void textResetBlends2(void)
{
	g_Blend.types = 0;
}

void textResetBlends3(void)
{
	g_Blend.types = 0;
}

void textBackupAndResetBlends(void)
{
	g_Blend.backuptypes = g_Blend.types;
	g_Blend.types = 0;
}

void textRestoreBlends(void)
{
	g_Blend.types = g_Blend.backuptypes;
}

void textSetWaveBlend(s32 arg0, s32 arg1, s32 cthresh)
{
	g_Blend.types |= BLENDTYPE_WAVE;
	g_Blend.wave4c = arg0;
	g_Blend.wave50 = arg1;
	g_Blend.wave54 = cthresh;
	g_Blend.wavecolour1 = 0x44444400;
	g_Blend.wavecolour2 = 0xffffff00;
}

void textSetMenuBlend(f32 arg0)
{
	g_Blend.types |= BLENDTYPE_MENU;
	g_Blend.menuweight = arg0 * arg0 * 110.0f;
}

void textSetWaveColours(u32 colour1, u32 colour2)
{
	g_Blend.wavecolour1 = colour1;
	g_Blend.wavecolour2 = colour2;
}

void textResetBlends(void)
{
	g_Blend.types = 0;
}

bool textHasDiagonalBlend(void)
{
	return (g_Blend.types & BLENDTYPE_DIAGONAL)
		&& (g_Blend.diagmode == DIAGMODE_FADEIN || g_Blend.diagmode == DIAGMODE_FADEOUT);
}

u32 textApplyProjectionColour(s32 x, s32 y, u32 colour)
{
	u32 stack[3];
	u32 result = colour;

	if (g_Blend.types & BLENDTYPE_DIAGONAL) {
		f32 weightf;
		f32 f12;
		f32 f14;
		f32 f16;
		f32 f18;

		if (x - g_Blend.diagrefx > -3000 && x - g_Blend.diagrefx < 3000
				&& y - g_Blend.diagrefy > -3000 && y - g_Blend.diagrefy < 3000) {
			f12 = sqrtf((x - g_Blend.diagrefx) * (x - g_Blend.diagrefx) + (y - g_Blend.diagrefy) * (y - g_Blend.diagrefy));
		} else {
			f12 = 3000.0f;
		}

		f14 = var8007fbac;
		f18 = var8007fbb0;
		f16 = var8007fbb4;

		if (g_Blend.diagmode == 0) {
			if (g_Blend.diagtimer < f12) {
				result = 0;
			} else if (g_Blend.diagtimer - f14 < f12) {
				u32 intensity;
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				intensity = 255 - (u32) weightf;
				result = intensity << 8 | intensity | intensity << 16 | intensity << 24;
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				result = (((colour & 0xff) + 0xff) >> 1) | (colour & 0xffffff00);
			} else if ((g_Blend.diagtimer - (f14 + f18 + f16)) < f12) {
				u32 colour2 = (((colour & 0xff) + 0xff) / 2) | (colour & 0xffffff00);
				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				result = colourBlend(colour, colour2, 0xff - (u32) weightf);
			}
		} else if (g_Blend.diagmode == 2) {
			f16 = 0.0f;

			if (g_Blend.diagtimer < f12) {
				result = 0x00000000;
			} else if (g_Blend.diagtimer - f14 < f12) {
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				result = colourBlend(0x00000000, colour & 0xff, weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				result = colour & 0xff;
			} else if ((g_Blend.diagtimer - (f14 + f18 + f16)) < f12) {
				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				result = colourBlend(colour & 0xff, colour, weightf);
			}
		}
	}

	return result;
}

u32 text0f1543ac(s32 x, s32 y, u32 colourarg)
{
	f32 f14;
	f32 f18;
	f32 f16;
	u32 colour = colourarg;

	if (g_Blend.types & BLENDTYPE_MENU) {
		colour = (colourBlend(0x00000000, colour, g_Blend.menuweight) & 0xffffff00) | (colour & 0xff);
	}

	if (g_Blend.types & BLENDTYPE_VERTICAL) {
		s32 v0 = y - g_Blend.vertrefy1;
		s32 v1 = y - g_Blend.vertrefy2;

		if (v0 < 0) {
			v0 = -v0;
		}

		if (v1 < 0) {
			v1 = -v1;
		}

		if (v1 < v0) {
			v0 = v1;
		}

		if (g_Blend.vert34 >= v0) {
			colour = colourBlend(colour, 0x00000000, v0 * 255 / g_Blend.vert34);
		}
	}

	if (g_Blend.types & BLENDTYPE_HORIZONTAL) {
		s32 v0 = x - g_Blend.horizrefx1;
		s32 v1 = x - g_Blend.horizrefx2;

		if (v0 < 0) {
			v0 = 0;
		}

		if (v1 < 0) {
			v1 = -v1;
		}

		if (v1 < v0) {
			v0 = v1;
		}

		if (g_Blend.horiz40 >= v0) {
			colour = colourBlend(colour, 0x00000000, v0 * 255 / g_Blend.horiz40);
		}
	}

	if (g_Blend.types & BLENDTYPE_DIAGONAL) {
		f32 f12;
		u32 stack[3];
		f32 weightf;

		if (x - g_Blend.diagrefx > -3000 && x - g_Blend.diagrefx < 3000
				&& y - g_Blend.diagrefy > -3000 && y - g_Blend.diagrefy < 3000) {
			f12 = sqrtf((x - g_Blend.diagrefx) * (x - g_Blend.diagrefx) + (y - g_Blend.diagrefy) * (y - g_Blend.diagrefy));
		} else {
			f12 = 3000.0f;
		}

		f14 = var8007fbac;
		f18 = var8007fbb0;
		f16 = var8007fbb4;

		if (g_Blend.diagmode == 0) {
			if (g_Blend.diagtimer < f12) {
				colour = 0;
			} else if (g_Blend.diagtimer - f14 < f12) {
				u32 intensity;
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				intensity = 255 - (u32) weightf;
				colour = intensity << 8 | intensity | intensity << 16 | intensity << 24;
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				colour = 0xffffffff;
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				u32 add;
				u32 mult;

				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				add = (u32) weightf * 255;
				mult = 255 - (u32) weightf;

				colour = ((((colour >> 24) & 0xff) * mult + add) >> 8) << 24
					| ((((colour >> 16) & 0xff) * mult + add) >> 8) << 16
					| ((((colour >> 8) & 0xff) * mult + add) >> 8) << 8
					| ((colour & 0xff) * mult + add) >> 8;
			}
		} else if (g_Blend.diagmode == 2) {
			f14 = 0.00f;
			f18 = 66.0f;
			f16 = 0.0f;

			if (g_Blend.diagtimer < f12) {
				colour = 0x00000000;
			} else if (g_Blend.diagtimer - f14 < f12) {
				f32 weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				colour = colourBlend(0x00000000, colour & 0xff, weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				colour &= 0xff;
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				f32 weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				colour = colourBlend(0x00000000, colour, weightf);
			}
		} else {
			u32 alpha[4];

			static s32 burncol = 0xffffff00;

			alpha[0] = colour & 0xff;
			f18 = 50.0f;
			f16 = 22.0f;

			mainOverrideVariable("burncol", &burncol);

			if (g_Blend.diagtimer < f12) {
				colour = colourBlend(alpha[0], colour, 110);
			} else if (g_Blend.diagtimer - f14 < f12) {
				f32 weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				colour = colourBlend(
						colourBlend(burncol | (colour & 0xff), colour, 0xc0),
						colourBlend(alpha[0], colour, 110),
						255 - (u32) weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				u32 stack;
				colour = colourBlend(burncol | (colour & 0xff), colour, 0xc0);
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				f32 weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				colour = colourBlend(
						colour,
						colourBlend(burncol | (colour & 0xff), colour, 0xc0),
						255 - (u32) weightf);
			}
		}
	}

	if (g_Blend.types & BLENDTYPE_WAVE) {
		u32 stack[2];
		f32 f0 = (s32)(g_Blend.wave4c - x + g_Blend.wave50 - y + 800);
		f0 = 4.0f * f0 / g_Blend.wave54;
		f0 -= (s32) (f0 * 0.25f) * 4.0f;
		f0 -= 1.0f;

		if (f0 > 1.0f) { \
			f0 = 2.0f - f0;
		}

		if (f0 < 0.0f) {
			s32 weight = var8007fbbc * (0 - f0);
			colour = colourBlend(g_Blend.wavecolour1 | (colour & 0xff), colour, weight);
		} else {
			s32 weight = var8007fbb8 * f0;
			colour = colourBlend(g_Blend.wavecolour2 | (colour & 0xff), colour, weight);
		}
	}

	return colour;
}

Gfx *text0f154ecc(Gfx *gdl, u32 arg1, u32 arg2)
{
	u32 colour = text0f1543ac(arg1, arg2, g_Blend.colour04);

	if (colour != g_Blend.colour44) {
		gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	}

	g_Blend.colour44 = colour;

	return gdl;
}

#if VERSION >= VERSION_PAL_BETA
void textMapCodeUnitToChar2(u8 *c)
{
	switch (*c) {
	case 0xc0:
	case 0xc1:
	case 0xc4:
		*c = 0x41;
		break;
	case 0xc8:
	case 0xc9:
		*c = 0x45;
		break;
	case 0xcc:
	case 0xcd:
		*c = 0x49;
		break;
	case 0xd2:
	case 0xd3:
	case 0xd6:
		*c = 0x4f;
		break;
	case 0xd9:
	case 0xda:
	case 0xdc:
		*c = 0x55;
		break;
	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe4:
		*c = 0x61;
		break;
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
		*c = 0x65;
		break;
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xef:
		*c = 0x69;
		break;
	case 0xf2:
	case 0xf3:
	case 0xf4:
	case 0xf6:
		*c = 0x6f;
		break;
	case 0xf9:
	case 0xfa:
	case 0xfb:
	case 0xfc:
		*c = 0x75;
		break;
	case 0xd1:
		*c = 0x4e;
		break;
	case 0xf1:
		*c = 0x6e;
		break;
	case 0xe7:
		*c = 0x63;
		break;
	case 0xdf:
		*c = 0x42;
		break;
	case 0xa1:
	case 0xaa:
	case 0xb0:
	case 0xbf:
		*c = 0x20;
		break;
	}
}
#endif

#if VERSION >= VERSION_JPN_FINAL
s32 func0f154784jf(u16 arg0)
{
	u8 result = 0;
	u16 u16val = arg0;
	u8 u8val = arg0;

	if (arg0 >= 0x10 && arg0 < 0x1a) {
		result = u16val;
		result += 0x20;
	}

	if (arg0 >= 0x1a && arg0 < 0x34) {
		result = u16val;
		result += 0x27;
	}

	if (arg0 >= 0x95 && arg0 < 0xaf) {
		result = u16val;
		result -= 0x34;
	}

	if (arg0 == 0x3fe || arg0 == 0x3ff) {
		result = 0x2d;
	}

	if (arg0 < 0xff) {
		switch (u8val) {
		case 0x00: result = 0x20; break;
		case 0x01: result = 0x2c; break;
		case 0x02: result = 0x24; break;
		case 0x03: result = 0x28; break;
		case 0x04: result = 0x29; break;
		case 0x05: result = 0x2e; break;
		case 0x06: result = 0x25; break;
		case 0x07: result = 0x5b; break;
		case 0x08: result = 0x5d; break;
		case 0x09: result = 0x22; break;
		case 0x0a: result = 0x3c; break;
		case 0x0b: result = 0x3e; break;
		case 0x0c: result = 0x26; break;
		case 0x0d: result = 0x7e; break;
		case 0x0e: result = 0x2e; break;
		case 0x0f: result = 0x20; break;
		case 0x34: result = 0x21; break;
		case 0x35: result = 0x22; break;
		case 0x36: result = 0x23; break;
		case 0x37: result = 0x22; break;
		case 0x38: result = 0x2a; break;
		case 0x39: result = 0x2b; break;
		case 0x3a: result = 0x2c; break;
		case 0x3b: result = 0x2d; break;
		case 0x3c: result = 0x2e; break;
		case 0x3d: result = 0x2f; break;
		case 0x3e: result = 0x3a; break;
		case 0x3f: result = 0x3d; break;
		case 0x40: result = 0x3f; break;
		case 0x41: result = 0x40; break;
		}
	}

	return result;
}
#endif

#if VERSION >= VERSION_JPN_FINAL
u16 func0f154968jf(u8 value)
{
	u16 result = 0;

	if (value >= 0x30 && value < 0x3a) {
		result = value - 0x20;
	}

	if (value >= 0x41 && value < 0x5b) {
		result = value - 0x27;
	}

	if (value >= 0x61 && value < 0x7b) {
		result = value + 0x34;
	}

	switch (value) {
	case 0x20: result = 0x00; break;
	case 0x2c: result = 0x01; break;
	case 0x24: result = 0x02; break;
	case 0x28: result = 0x03; break;
	case 0x29: result = 0x04; break;
	case 0x2e: result = 0x05; break;
	case 0x25: result = 0x06; break;
	case 0x5b: result = 0x07; break;
	case 0x5d: result = 0x08; break;
	case 0x22: result = 0x09; break;
	case 0x3c: result = 0x0a; break;
	case 0x3e: result = 0x0b; break;
	case 0x26: result = 0x0c; break;
	case 0x7e: result = 0x0d; break;
	case 0x21: result = 0x34; break;
	case 0x23: result = 0x36; break;
	case 0x2a: result = 0x38; break;
	case 0x2b: result = 0x39; break;
	case 0x2d: result = 0x3b; break;
	case 0x2f: result = 0x3d; break;
	case 0x3a: result = 0x3e; break;
	case 0x3d: result = 0x3f; break;
	case 0x3f: result = 0x40; break;
	case 0x40: result = 0x41; break;
	}

	return result;
}
#endif

void textMapCodeUnitToChar(char **text, struct fontchar **arg1, struct fontchar **arg2, struct fontchar *chars, u8 *prevchar);

#if VERSION == VERSION_JPN_FINAL
#if MATCHING
GLOBAL_ASM(
glabel textMapCodeUnitToChar
/*  f154b00:	27bdffd0 */ 	addiu	$sp,$sp,-48
/*  f154b04:	afbf001c */ 	sw	$ra,0x1c($sp)
/*  f154b08:	afb00018 */ 	sw	$s0,0x18($sp)
/*  f154b0c:	afa50034 */ 	sw	$a1,0x34($sp)
/*  f154b10:	afa60038 */ 	sw	$a2,0x38($sp)
/*  f154b14:	afa7003c */ 	sw	$a3,0x3c($sp)
/*  f154b18:	8c8e0000 */ 	lw	$t6,0x0($a0)
/*  f154b1c:	00808025 */ 	move	$s0,$a0
/*  f154b20:	3c047f1c */ 	lui	$a0,%hi(var7f1b8068jf)
/*  f154b24:	91c20000 */ 	lbu	$v0,0x0($t6)
/*  f154b28:	240a000b */ 	li	$t2,0xb
/*  f154b2c:	24848068 */ 	addiu	$a0,$a0,%lo(var7f1b8068jf)
/*  f154b30:	28410080 */ 	slti	$at,$v0,0x80
/*  f154b34:	10200032 */ 	beqz	$at,.JF0f154c00
/*  f154b38:	00401825 */ 	move	$v1,$v0
/*  f154b3c:	10e00004 */ 	beqz	$a3,.JF0f154b50
/*  f154b40:	3c188008 */ 	lui	$t8,%hi(var800800f0jf)
/*  f154b44:	8f1800f0 */ 	lw	$t8,%lo(var800800f0jf)($t8)
/*  f154b48:	53000016 */ 	beqzl	$t8,.JF0f154ba4
/*  f154b4c:	2404000c */ 	li	$a0,0xc
.JF0f154b50:
/*  f154b50:	3c03800a */ 	lui	$v1,%hi(g_TmpJpnChar)
/*  f154b54:	24635008 */ 	addiu	$v1,$v1,%lo(g_TmpJpnChar)
/*  f154b58:	2405000c */ 	li	$a1,0xc
/*  f154b5c:	304400ff */ 	andi	$a0,$v0,0xff
/*  f154b60:	a4600000 */ 	sh	$zero,0x0($v1)
/*  f154b64:	a0600002 */ 	sb	$zero,0x2($v1)
/*  f154b68:	a0650003 */ 	sb	$a1,0x3($v1)
/*  f154b6c:	a0650004 */ 	sb	$a1,0x4($v1)
/*  f154b70:	a4600006 */ 	sh	$zero,0x6($v1)
/*  f154b74:	0fc5525a */ 	jal	func0f154968jf
/*  f154b78:	ac600008 */ 	sw	$zero,0x8($v1)
/*  f154b7c:	8fab0034 */ 	lw	$t3,0x34($sp)
/*  f154b80:	3c03800a */ 	lui	$v1,%hi(g_TmpJpnChar)
/*  f154b84:	24635008 */ 	addiu	$v1,$v1,%lo(g_TmpJpnChar)
/*  f154b88:	24590080 */ 	addiu	$t9,$v0,0x80
/*  f154b8c:	a4790000 */ 	sh	$t9,0x0($v1)
/*  f154b90:	ad630000 */ 	sw	$v1,0x0($t3)
/*  f154b94:	8fac0038 */ 	lw	$t4,0x38($sp)
/*  f154b98:	10000011 */ 	b	.JF0f154be0
/*  f154b9c:	ad830000 */ 	sw	$v1,0x0($t4)
/*  f154ba0:	2404000c */ 	li	$a0,0xc
.JF0f154ba4:
/*  f154ba4:	00640019 */ 	multu	$v1,$a0
/*  f154ba8:	8fae003c */ 	lw	$t6,0x3c($sp)
/*  f154bac:	8fb90034 */ 	lw	$t9,0x34($sp)
/*  f154bb0:	00006812 */ 	mflo	$t5
/*  f154bb4:	01ae7821 */ 	addu	$t7,$t5,$t6
/*  f154bb8:	25f8fe74 */ 	addiu	$t8,$t7,-396
/*  f154bbc:	af380000 */ 	sw	$t8,0x0($t9)
/*  f154bc0:	8fab0040 */ 	lw	$t3,0x40($sp)
/*  f154bc4:	8fb90038 */ 	lw	$t9,0x38($sp)
/*  f154bc8:	916c0000 */ 	lbu	$t4,0x0($t3)
/*  f154bcc:	01840019 */ 	multu	$t4,$a0
/*  f154bd0:	00006812 */ 	mflo	$t5
/*  f154bd4:	01ae7821 */ 	addu	$t7,$t5,$t6
/*  f154bd8:	25f8fe74 */ 	addiu	$t8,$t7,-396
/*  f154bdc:	af380000 */ 	sw	$t8,0x0($t9)
.JF0f154be0:
/*  f154be0:	8e0b0000 */ 	lw	$t3,0x0($s0)
/*  f154be4:	8fad0040 */ 	lw	$t5,0x40($sp)
/*  f154be8:	916c0000 */ 	lbu	$t4,0x0($t3)
/*  f154bec:	a1ac0000 */ 	sb	$t4,0x0($t5)
/*  f154bf0:	8e0e0000 */ 	lw	$t6,0x0($s0)
/*  f154bf4:	25cf0001 */ 	addiu	$t7,$t6,0x1
/*  f154bf8:	10000040 */ 	b	.JF0f154cfc
/*  f154bfc:	ae0f0000 */ 	sw	$t7,0x0($s0)
.JF0f154c00:
/*  f154c00:	3c03800a */ 	lui	$v1,%hi(g_TmpJpnChar)
/*  f154c04:	24635008 */ 	addiu	$v1,$v1,%lo(g_TmpJpnChar)
/*  f154c08:	a4600000 */ 	sh	$zero,0x0($v1)
/*  f154c0c:	a0600002 */ 	sb	$zero,0x2($v1)
/*  f154c10:	a06a0003 */ 	sb	$t2,0x3($v1)
/*  f154c14:	a06a0004 */ 	sb	$t2,0x4($v1)
/*  f154c18:	a4600006 */ 	sh	$zero,0x6($v1)
/*  f154c1c:	ac600008 */ 	sw	$zero,0x8($v1)
/*  f154c20:	8e020000 */ 	lw	$v0,0x0($s0)
/*  f154c24:	3c058008 */ 	lui	$a1,%hi(ope)
/*  f154c28:	24a50228 */ 	addiu	$a1,$a1,%lo(ope)
/*  f154c2c:	90460000 */ 	lbu	$a2,0x0($v0)
/*  f154c30:	24580001 */ 	addiu	$t8,$v0,0x1
/*  f154c34:	ae180000 */ 	sw	$t8,0x0($s0)
/*  f154c38:	93070000 */ 	lbu	$a3,0x0($t8)
/*  f154c3c:	27190001 */ 	addiu	$t9,$t8,0x1
/*  f154c40:	30cb007f */ 	andi	$t3,$a2,0x7f
/*  f154c44:	ae190000 */ 	sw	$t9,0x0($s0)
/*  f154c48:	000b61c0 */ 	sll	$t4,$t3,0x7
/*  f154c4c:	30ed007f */ 	andi	$t5,$a3,0x7f
/*  f154c50:	018d4825 */ 	or	$t1,$t4,$t5
/*  f154c54:	a7a9002a */ 	sh	$t1,0x2a($sp)
/*  f154c58:	afa90024 */ 	sw	$t1,0x24($sp)
/*  f154c5c:	0c0036b4 */ 	jal	mainOverrideVariable
/*  f154c60:	a3a00029 */ 	sb	$zero,0x29($sp)
/*  f154c64:	3c0e8008 */ 	lui	$t6,%hi(ope)
/*  f154c68:	8dce0228 */ 	lw	$t6,%lo(ope)($t6)
/*  f154c6c:	3c03800a */ 	lui	$v1,%hi(g_TmpJpnChar)
/*  f154c70:	24635008 */ 	addiu	$v1,$v1,%lo(g_TmpJpnChar)
/*  f154c74:	93a80029 */ 	lbu	$t0,0x29($sp)
/*  f154c78:	11c00006 */ 	beqz	$t6,.JF0f154c94
/*  f154c7c:	8fb00024 */ 	lw	$s0,0x24($sp)
/*  f154c80:	0fc551e1 */ 	jal	func0f154784jf
/*  f154c84:	3204ffff */ 	andi	$a0,$s0,0xffff
/*  f154c88:	3c03800a */ 	lui	$v1,%hi(g_TmpJpnChar)
/*  f154c8c:	24635008 */ 	addiu	$v1,$v1,%lo(g_TmpJpnChar)
/*  f154c90:	304800ff */ 	andi	$t0,$v0,0xff
.JF0f154c94:
/*  f154c94:	11000004 */ 	beqz	$t0,.JF0f154ca8
/*  f154c98:	01001025 */ 	move	$v0,$t0
/*  f154c9c:	8fa5003c */ 	lw	$a1,0x3c($sp)
/*  f154ca0:	2404000c */ 	li	$a0,0xc
/*  f154ca4:	14a0000d */ 	bnez	$a1,.JF0f154cdc
.JF0f154ca8:
/*  f154ca8:	32181fff */ 	andi	$t8,$s0,0x1fff
/*  f154cac:	2b010400 */ 	slti	$at,$t8,0x400
/*  f154cb0:	14200002 */ 	bnez	$at,.JF0f154cbc
/*  f154cb4:	24190002 */ 	li	$t9,0x2
/*  f154cb8:	a7b9002a */ 	sh	$t9,0x2a($sp)
.JF0f154cbc:
/*  f154cbc:	97ab002a */ 	lhu	$t3,0x2a($sp)
/*  f154cc0:	8fad0034 */ 	lw	$t5,0x34($sp)
/*  f154cc4:	256c0080 */ 	addiu	$t4,$t3,0x80
/*  f154cc8:	a46c0000 */ 	sh	$t4,0x0($v1)
/*  f154ccc:	ada30000 */ 	sw	$v1,0x0($t5)
/*  f154cd0:	8fae0038 */ 	lw	$t6,0x38($sp)
/*  f154cd4:	10000009 */ 	b	.JF0f154cfc
/*  f154cd8:	adc30000 */ 	sw	$v1,0x0($t6)
.JF0f154cdc:
/*  f154cdc:	00440019 */ 	multu	$v0,$a0
/*  f154ce0:	8fab0034 */ 	lw	$t3,0x34($sp)
/*  f154ce4:	00007812 */ 	mflo	$t7
/*  f154ce8:	01e5c021 */ 	addu	$t8,$t7,$a1
/*  f154cec:	2719fe74 */ 	addiu	$t9,$t8,-396
/*  f154cf0:	ad790000 */ 	sw	$t9,0x0($t3)
/*  f154cf4:	8fac0038 */ 	lw	$t4,0x38($sp)
/*  f154cf8:	ad830000 */ 	sw	$v1,0x0($t4)
.JF0f154cfc:
/*  f154cfc:	8fbf001c */ 	lw	$ra,0x1c($sp)
/*  f154d00:	8fb00018 */ 	lw	$s0,0x18($sp)
/*  f154d04:	27bd0030 */ 	addiu	$sp,$sp,0x30
/*  f154d08:	03e00008 */ 	jr	$ra
/*  f154d0c:	00000000 */ 	nop
);

u32 ope = 0;

const char var7f1b8068jf[] = "ope";
#else
void textMapCodeUnitToChar(char **text, struct fontchar **arg1, struct fontchar **arg2, struct fontchar *chars, u8 *prevchar)
{
	u16 c;
	u8 c1;
	u8 c2;
	u16 sp2a;
	u8 sp29;

	static u32 ope = 0;

	c = **text;

	if (c < 0x80) {
		if (chars == NULL || var800800f0jf) {
			g_TmpJpnChar.index = 0;
			g_TmpJpnChar.baseline = 0;
			g_TmpJpnChar.height = 12;
			g_TmpJpnChar.width = 12;
			g_TmpJpnChar.kerningindex = 0;
			g_TmpJpnChar.pixeldata = NULL;

			g_TmpJpnChar.index = 0x80 + func0f154968jf(c);

			*arg1 = &g_TmpJpnChar;
			*arg2 = &g_TmpJpnChar;
		} else {
			*arg1 = &chars[c - 0x21];
			*arg2 = &chars[*prevchar - 0x21];
		}

		*prevchar = **text;
		*text += 1;
		return;
	}

	g_TmpJpnChar.index = 0;
	g_TmpJpnChar.baseline = 0;
	g_TmpJpnChar.height = 11;
	g_TmpJpnChar.width = 11;
	g_TmpJpnChar.kerningindex = 0;
	g_TmpJpnChar.pixeldata = NULL;

	c1 = **text;
	*text = *text + 1;
	c2 = **text;
	*text = *text + 1;

	sp2a = ((c1 & 0x7f) << 7) | (c2 & 0x7f);
	sp29 = 0;

	mainOverrideVariable("ope", &ope);

	if (ope) {
		sp29 = func0f154784jf(sp2a);
	}

	if (sp29 == 0 || chars == NULL) {
		if ((sp2a & 0x1fff) >= 0x400) {
			sp2a = 2;
		}

		g_TmpJpnChar.index = sp2a + 0x80;

		*arg1 = &g_TmpJpnChar;
		*arg2 = &g_TmpJpnChar;
	} else {
		*arg1 = &chars[sp29 - 0x21];
		*arg2 = &g_TmpJpnChar;
	}
}
#endif
#elif VERSION >= VERSION_PAL_BETA
void textMapCodeUnitToChar(char **text, struct fontchar **arg1, struct fontchar **arg2, struct fontchar *chars, u8 *prevchar)
{
	u8 c;
	u8 index;

	if (g_Jpn) {
		if (**text < 0x80) {
			*arg1 = &chars[**text - 0x21];
			*arg2 = &chars[*prevchar - 0x21];

			*prevchar = **text;
			*text += 1;
			return;
		}

		*arg1 = &chars[*prevchar - 0x21];
		*arg2 = &chars[*prevchar - 0x21];
		return;
	}

	index = 0;
	c = **text;

	if (chars == g_CharsHandelGothicSm || chars == g_CharsHandelGothicMd || chars == g_CharsHandelGothicXs) {
		switch (c) {
		case 0xc4: index = 0x5e; break;
		case 0xc1: index = 0x66; break;
		case 0xc0: index = 0x70; break;
		case 0xc9: index = 0x67; break;
		case 0xc8: index = 0x71; break;
		case 0xcd: index = 0x68; break;
		case 0xcc: index = 0x72; break;
		case 0xd6: index = 0x5f; break;
		case 0xd3: index = 0x69; break;
		case 0xd2: index = 0x73; break;
		case 0xdc: index = 0x60; break;
		case 0xda: index = 0x6a; break;
		case 0xd9: index = 0x74; break;
		case 0xe4: index = 0x61; break;
		case 0xe1: index = 0x6b; break;
		case 0xe0: index = 0x75; break;
		case 0xe2: index = 0x7a; break;
		case 0xea: index = 0x7b; break;
		case 0xe9: index = 0x6c; break;
		case 0xeb: index = 0x62; break;
		case 0xe8: index = 0x76; break;
		case 0xec: index = 0x77; break;
		case 0xef: index = 0x63; break;
		case 0xed: index = 0x6d; break;
		case 0xee: index = 0x7c; break;
		case 0xf4: index = 0x7d; break;
		case 0xf6: index = 0x64; break;
		case 0xf3: index = 0x6e; break;
		case 0xf2: index = 0x78; break;
		case 0xf9: index = 0x79; break;
		case 0xfc: index = 0x65; break;
		case 0xfa: index = 0x6f; break;
		case 0xfb: index = 0x7e; break;
		case 0xd1: index = 0x7f; break;
		case 0xf1: index = 0x80; break;
		case 0xe7: index = 0x81; break;
		case 0xdf: index = 0x82; break;
		case 0xbf: index = 0x83; break;
		case 0xa1: index = 0x84; break;
		case 0xb0: index = 0x85; break;
		case 0xaa: index = 0x86; break;
		}
	} else {
		textMapCodeUnitToChar2(&c);
	}

	if (index > 0) {
		*arg1 = &chars[index];
	} else {
		if (c < 0x21) {
			c = 0x21;
		}

		if (c > 0x7e) {
			c = 0x21;
		}

		*arg1 = &chars[c - 0x21];
	}

	*arg2 = &chars[*prevchar - 0x21];

	if (index > 0) {
		textMapCodeUnitToChar2(&c);
	}

	*prevchar = c;
	*text += 1;
}
#endif

Gfx *text0f154f38(Gfx *gdl, s32 *arg1, struct fontchar *curchar, struct fontchar *prevchar,
		struct font *font, f32 widthscale, f32 heightscale, f32 x, f32 y)
{
	s32 tmp1;
	s32 tmp2;
	s16 sp3e;
	s16 sp3c;
	s16 sp3a;
	s16 sp38;
	s16 sp36;
	s16 sp34;
	s16 sp32;
	s16 sp30;
	Vtx *vertices;
	Col *colours;

	tmp2 = font->kerning[prevchar->kerningindex * 13 + curchar->kerningindex] + var8007fac4;
	*arg1 = *arg1 - tmp2 + 1;

#if VERSION >= VERSION_JPN_FINAL
	if (curchar->pixeldata == NULL) {
		curchar->pixeldata = (void *) langGetJpnCharPixels(curchar->index - 0x80);
	}

	if (curchar->index >= 0x80) {
		if (!var80080104jf) {
			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var800801d8jf));
			var80080104jf = true;
			gDPLoadSync(gdl++);
			gDPLoadTLUTCmd(gdl++, 6, 15);
		}
	} else {
		if (var80080104jf) {
			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var8007fb3c));
			var80080104jf = false;
			gDPLoadSync(gdl++);
			gDPLoadTLUTCmd(gdl++, 6, 15);
		}
	}
#endif

	gDPLoadSync(gdl++);
	gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((curchar->height * 8 + 17) >> 1) - 1, 2048);
	gDPPipeSync(gdl++);

	tmp1 = var8007fadc;
	tmp2 = var8007fae0;

	sp3e = (var8007fadc + *arg1 * 4) * widthscale * 10.0f + 40.0f * x;
	sp3c = (var8007fae0 + curchar->baseline * 4) * heightscale * 10.0f + 40.0f * y;
	sp3a = (var8007fadc + (*arg1 + (curchar->width + 1)) * 4) * widthscale * 10.0f + 40.0f * x;
	sp38 = (var8007fae0 + (curchar->baseline + (curchar->height + 1)) * 4) * heightscale * 10.0f + 40.0f * y;

	sp36 = 1;
	sp32 = 1;
	sp34 = curchar->width;
	sp30 = curchar->height;
	sp34 = sp36 + ((sp34 + 1) << 6);
	sp30 = sp32 + ((sp30 + 1) << 6);

	vertices = gfxAllocateVertices(4);

	colours = gfxAllocateColours(1);
	colours[0].word = 0xff0000ff;

	vertices[0].z = -10;
	vertices[1].z = -10;
	vertices[2].z = -10;
	vertices[3].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 0;
	vertices[3].colour = 0;

	vertices[0].x = sp3e / 4;
	vertices[0].y = sp3c / 4;
	vertices[0].s = sp36;
	vertices[0].t = sp32;
	vertices[1].x = sp3a / 4;
	vertices[1].y = sp3c / 4;
	vertices[1].s = sp34;
	vertices[1].t = sp32;
	vertices[2].x = sp3a / 4;
	vertices[2].y = sp38 / 4;
	vertices[2].s = sp34;
	vertices[2].t = sp30;
	vertices[3].x = sp3e / 4;
	vertices[3].y = sp38 / 4;
	vertices[3].s = sp36;
	vertices[3].t = sp30;

	gSPColor(gdl++, colours, 1);
	gSPVertex(gdl++, vertices, 4, 0);

	gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);

	*arg1 += curchar->width;

	return gdl;
}

Gfx *text0f1552d4(Gfx *gdl, f32 x, f32 y, f32 widthscale, f32 heightscale,
		char *text, struct fontchar *chars, struct font *font, u32 colour, s32 hdir, s32 vdir)
{
	s32 totalheight;
	u8 prevchar;
	s32 textwidth;
	s32 textheight;
	s32 lineheight;
	s32 relx;
	f32 *ptr;
	f32 fx;
	f32 fy;

	totalheight = 0;
	prevchar = 'H';
	relx = 0;

#if VERSION >= VERSION_JPN_FINAL
	if (1);
	lineheight = 13;
#else
	lineheight = chars['['].height + chars['['].baseline;

	if (g_Jpn && lineheight < 14) {
		lineheight = 14;
	}
#endif

	textMeasure(&textheight, &textwidth, text, chars, font, 0);

	ptr = &x;
	fx = *ptr - (widthscale - 1.0f) * textwidth * 0.5f * hdir;
	fy = y - (heightscale - 1.0f) * lineheight * 0.5f * vdir;

	if (fx);
	if (fy);

	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);

#if VERSION >= VERSION_JPN_FINAL
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var800801d8jf));
	var80080104jf = true;
#else
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var8007fb3c));
#endif

	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 15);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPPipeSync(gdl++);

#if VERSION >= VERSION_PAL_BETA
	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				relx = relx + var8007fad0 * 5;
				prevchar = 'H';
				text += 1;
			} else if (*text == '\n') {
				if (var8007fad4 >= 0 && relx == 0) {
					totalheight += var8007fad4;
					relx = 0;
				} else {
					relx = 0;
#if VERSION >= VERSION_JPN_FINAL
					totalheight = totalheight + lineheight * var80080108jf;
#else
					totalheight += lineheight;
#endif
				}

				prevchar = 'H';
				text += 1;
			} else {
				struct fontchar *sp84;
				struct fontchar *sp80;

				textMapCodeUnitToChar(&text, &sp84, &sp80, chars, &prevchar);
				gdl = text0f154f38(gdl, &relx, sp84, sp80, font, widthscale, heightscale, fx, fy);
			}
		}
	}
#else
	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				prevchar = 'H';
				text += 1;
				relx = relx + var8007fad0 * 5;
			} else if (*text == '\n') {
				prevchar = 'H';
				text += 1;

				if (var8007fad4 >= 0 && relx == 0) {
					totalheight += var8007fad4;
					relx = 0;
				} else {
					totalheight += lineheight;
					relx = 0;
				}
			} else if (*text < 0x80) {
				u8 fontID = extTexFontID(font);
				gDPSetTextureInfoEXT(gdl++, G_TEXTYPE_FONT, fontID, *text - 0x21, 0);
				gdl = text0f154f38(gdl, &relx, &chars[*text - 0x21], &chars[prevchar - 0x21], font,
						widthscale, heightscale, fx, fy);
				prevchar = *text;
				text += 1;
			} else {
				u16 codepoint = (text[0] & 0x7f) << 7 | (text[1] & 0x7f);
				struct fontchar tmpchar = {0, 0, 12, 11};

				if (1);

				if (codepoint & 0x2000) {
					tmpchar.width = 15;
					tmpchar.height = 16;
				}

				if ((codepoint & 0x1fff) >= 0x3c8) {
					codepoint = 2;
				}

				tmpchar.index = codepoint + 0x80;
				tmpchar.pixeldata = (void *) langGetJpnCharPixels(codepoint);

				text += 2;
			}
		}
	}
#endif

	return gdl;
}

Gfx *text0f15568c(Gfx *gdl, s32 *x, s32 *y, struct fontchar *curchar, struct fontchar *prevchar,
		struct font *font, s32 savedx, s32 savedy, s32 width, s32 height, s32 arg10)
{
#if VERSION >= VERSION_JPN_FINAL
	s32 tmp;
	s32 sp90;
	s32 xscale = var8007fad0;
	s32 yscale = var80080108jf;

	savedy -= 2;

	if (g_TextRotated90) {
		xscale = 1;
	}

	sp90 = *y + arg10;
	tmp = var8007fac4 + font->kerning[prevchar->kerningindex * 13 + curchar->kerningindex];
	*x -= (tmp - 1) * xscale;
	width *= xscale;
	height *= yscale;

	if (g_TextRotated90 || (*x > 0 && *x <= viGetWidth() && sp90 + curchar->baseline <= viGetHeight())) {
		if (savedx + width >= *x
				&& savedy + height >= curchar->baseline + sp90
				&& *x >= savedx
				&& curchar->baseline + sp90 + curchar->height >= savedy) {
			if (curchar->pixeldata == NULL) {
				curchar->pixeldata = (void *)langGetJpnCharPixels(curchar->index - 0x80);
			}

			if (curchar->index >= 0x80) {
				if (!var80080104jf) {
					gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var800801d8jf));

					var80080104jf = true;

					gDPLoadSync(gdl++);
					gDPLoadTLUTCmd(gdl++, 6, 15);
				}
			} else {
				if (var80080104jf) {
					gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var8007fb3c));

					var80080104jf = false;

					gDPLoadSync(gdl++);
					gDPLoadTLUTCmd(gdl++, 6, 15);
				}
			}

			gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, curchar->pixeldata);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((curchar->height * 8 + 17) >> 1) - 1, 2048);
			gDPPipeSync(gdl++);

			if (g_Blend.types) {
				gdl = text0f154ecc(gdl, *x / g_ScaleX, *y + arg10);
			}

			if (1);

			if (*x + xscale * curchar->width <= savedx + width) {
				if (savedy <= curchar->baseline * yscale + sp90) {
					if (curchar->baseline * yscale + sp90 + curchar->height * yscale <= savedy + height) {
						if (g_TextRotated90) {
							gSPTextureRectangleFlip(gdl++,
									(sp90 - curchar->baseline - curchar->height * var8007fad0) * 4 + var8007fae0,
									*x * 4 + var8007fadc,
									(sp90 - curchar->baseline) * 4 + var8007fae0,
									(*x + curchar->width * var80080108jf) * 4 + var8007fadc,
									G_TX_RENDERTILE,
									var8007fae8 + 32,
									((curchar->height - 1) << 5) + var8007fae4 + 32,
									1024 / var80080108jf,
									65536 - 1024 / var8007fad0);
						} else {
							gSPTextureRectangle(gdl++,
									*x * 4 + var8007fadc,
									(sp90 + curchar->baseline * var80080108jf) * 4 + var8007fae0,
									(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
									(sp90 + curchar->baseline * var80080108jf + curchar->height * var80080108jf) * 4 + var8007fae0 - (var80080108jf - 1) * 4,
									G_TX_RENDERTILE,
									var8007fae4 + 32,
									var8007fae8 + 32,
									1024 / var8007fad0,
									1024 / var80080108jf);

							if (var8007fb9c) {
								text0f153b6c(*y + arg10);

								if (var8007fba0 >= *x / g_ScaleX && *x / g_ScaleX + curchar->width * var8007fad0 >= var8007fba0) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline * var80080108jf + sp90,
											var8007fba0,
											curchar->baseline * var80080108jf + sp90 + curchar->height * var80080108jf,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}

								if (var8007fba0 - 3 >= *x / g_ScaleX && *x / g_ScaleX + curchar->width * var8007fad0 >= var8007fba0 - 3) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline * var80080108jf + sp90,
											var8007fba0,
											curchar->baseline * var80080108jf + sp90 + curchar->height * var80080108jf,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}
							}
						}
					} else if (savedy + height >= curchar->baseline * var80080108jf + sp90) {
						gSPTextureRectangle(gdl++,
								*x * 4 + var8007fadc,
								(sp90 + curchar->baseline * var80080108jf) * 4 + var8007fae0,
								(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
								(savedy + height) * 4 + var8007fae0,
								G_TX_RENDERTILE,
								var8007fae4 + 32,
								var8007fae8 + 32,
								1024 / var8007fad0,
								1024 / var80080108jf);
					}
				} else {
					if (curchar->baseline * var80080108jf + sp90 + curchar->height * var80080108jf >= savedy) {
						gSPTextureRectangle(gdl++,
								*x * 4 + var8007fadc,
								savedy * 4 + var8007fae0,
								(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
								(curchar->baseline * var80080108jf + sp90 + curchar->height * var80080108jf) * 4 + var8007fae0,
								G_TX_RENDERTILE,
								var8007fae4 + 32,
								((savedy - sp90 - curchar->baseline * var80080108jf) << 5) + var8007fae8 + 32,
								1024 / var8007fad0,
								1024 / var80080108jf);
					}
				}
			}
		}
	}
#else
	s32 tmp;
	s32 sp90;
	s32 xscale = var8007fad0;

#if VERSION >= VERSION_PAL_BETA
	savedy -= 2;
#endif

	if (g_TextRotated90) {
		xscale = 1;
	}

	sp90 = *y + arg10;
	tmp = var8007fac4 + font->kerning[prevchar->kerningindex * 13 + curchar->kerningindex];
	*x -= (tmp - 1) * xscale;
	width *= xscale;

	if (g_TextRotated90 || (*x > 0 && *x <= viGetWidth() && sp90 + curchar->baseline <= viGetHeight())) {
		if (savedx + width >= *x
				&& savedy + height >= curchar->baseline + sp90
				&& *x >= savedx
				&& curchar->baseline + sp90 + curchar->height >= savedy) {
			gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, curchar->pixeldata);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((curchar->height * 8 + 17) >> 1) - 1, 2048);
			gDPPipeSync(gdl++);

			if (g_Blend.types) {
				gdl = text0f154ecc(gdl, *x / g_ScaleX, *y + arg10);
			}

			if (1);

			if (*x + xscale * curchar->width <= savedx + width) {
				if (savedy <= curchar->baseline + sp90) {
					if (curchar->baseline + sp90 + curchar->height <= savedy + height) {
						if (g_TextRotated90) {
							gSPTextureRectangleFlip(gdl++,
									(sp90 - curchar->baseline - curchar->height * var8007fad0) * 4 + var8007fae0,
									*x * 4 + var8007fadc,
									(sp90 - curchar->baseline) * 4 + var8007fae0,
									(*x + curchar->width) * 4 + var8007fadc,
									G_TX_RENDERTILE,
									var8007fae8 + 32,
									((curchar->height) << 5) + var8007fae4 + 32,
									1024,
									65536 - 1024 / var8007fad0);
						} else {
							gSPTextureRectangle(gdl++,
									*x * 4 + var8007fadc,
									(sp90 + curchar->baseline) * 4 + var8007fae0,
									(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
									(sp90 + curchar->baseline + curchar->height) * 4 + var8007fae0,
									G_TX_RENDERTILE,
									var8007fae4 + 32,
									var8007fae8 + 32,
									1024 / var8007fad0,
									1024);

							if (var8007fb9c) {
								text0f153b6c(*y + arg10);

								if (var8007fba0 >= *x / g_ScaleX && *x / g_ScaleX + curchar->width * var8007fad0 >= var8007fba0) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline + sp90,
											var8007fba0,
											curchar->baseline + sp90 + curchar->height,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}

								if (var8007fba0 - 3 >= *x / g_ScaleX && *x / g_ScaleX + curchar->width * var8007fad0 >= var8007fba0 - 3) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline + sp90,
											var8007fba0,
											curchar->baseline + sp90 + curchar->height,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}
							}
						}
					} else if (savedy + height >= curchar->baseline + sp90) {
						gSPTextureRectangle(gdl++,
								*x * 4 + var8007fadc,
								(sp90 + curchar->baseline) * 4 + var8007fae0,
								(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
								(savedy + height) * 4 + var8007fae0,
								G_TX_RENDERTILE,
								var8007fae4 + 32,
								var8007fae8 + 32,
								1024 / var8007fad0,
								1024);
					}
				} else {
					if (curchar->baseline + sp90 + curchar->height >= savedy) {
						gSPTextureRectangle(gdl++,
								*x * 4 + var8007fadc,
								savedy * 4 + var8007fae0,
								(*x + curchar->width * var8007fad0) * 4 + var8007fadc,
								(curchar->baseline + sp90 + curchar->height) * 4 + var8007fae0,
								G_TX_RENDERTILE,
								var8007fae4 + 32,
								((savedy - sp90 - curchar->baseline) << 5) + var8007fae8 + 32,
								1024 / var8007fad0,
								1024);
					}
				}
			}
		}
	}
#endif

	*x += curchar->width * xscale;

	return gdl;
}

u32 var8007fbd8 = 0;

void text0f156024(s32 arg0)
{
	var8007fbd8 = arg0;
}

void text0f156030(u32 colour)
{
	var800a463c = colour;
}

Gfx *textRenderProjected(Gfx *gdl, s32 *x, s32 *y, char *text, struct fontchar *chars, struct font *font,
		s32 colour, s32 width, s32 height, s32 arg9, s32 lineheight)
{
	s32 savedx;
	s32 savedy;
	u8 prevchar;
	s32 spb0;
	u32 colour2;
	u32 tmpcolour;
	s32 newx;
	s32 newy;
#if VERSION >= VERSION_JPN_FINAL
	s32 savedmode;
	s32 savedtypes;
#endif
	f32 alpha;

	static u32 sbrd = 0x00000000;

	spb0 = var8007fad0;

	if (g_TextRotated90) {
		*y *= g_ScaleX;
		spb0 = 1;
	} else {
		*x *= g_ScaleX;
	}

	if (var8007fbd8) {
#if VERSION >= VERSION_JPN_FINAL
		menuGetSinOscFrac(40);
		newx = *x / g_ScaleX;
		newy = *y;
		tmpcolour = var800a463c;
		savedmode = g_Blend.diagmode;
		savedtypes = g_Blend.types;

		if (!g_TextRotated90) {
			newy--;
		}

		mainOverrideVariable("sbrd", &sbrd);

		if (sbrd != 0) {
			tmpcolour = sbrd;
		}

		g_Blend.types &= ~BLENDTYPE_WAVE;

		if (g_Blend.diagmode != DIAGMODE_FADEIN) {
			g_Blend.types &= ~BLENDTYPE_DIAGONAL;
		} else {
			g_Blend.diagmode = DIAGMODE_FADEOUT;
		}

		var8007fbd8 = 0;

		newx = *x / g_ScaleX + 1;
		newy = *y - 1;
		gdl = textRenderProjected(gdl, &newx, &newy, text, chars, font, tmpcolour, width, height, arg9, lineheight);

		newx = *x / g_ScaleX + 1;
		newy = *y + 1;
		gdl = textRenderProjected(gdl, &newx, &newy, text, chars, font, tmpcolour, width, height, arg9, lineheight);

		newx = *x / g_ScaleX - 1;
		newy = *y - 1;
		gdl = textRenderProjected(gdl, &newx, &newy, text, chars, font, tmpcolour, width, height, arg9, lineheight);

		newx = *x / g_ScaleX - 1;
		newy = *y + 1;
		gdl = textRenderProjected(gdl, &newx, &newy, text, chars, font, tmpcolour, width, height, arg9, lineheight);

		var8007fbd8 = 1;

		g_Blend.types = savedtypes;
		g_Blend.diagmode = savedmode;
#else
		alpha = (1.0f - menuGetSinOscFrac(40.0f)) * 100.0f + 150.0f;
		newx = *x / g_ScaleX;
		newy = *y;
		tmpcolour = var800a463c;
		colour2 = (colour & 0xffffff00) | (u32) alpha;

		mainOverrideVariable("sbrd", &sbrd);

		if (sbrd) {
			tmpcolour = sbrd;
		}

		gdl = textRender(gdl, &newx, &newy, text, chars, font, colour2, tmpcolour, width, height, arg9, lineheight);
#endif
	}

#if VERSION >= VERSION_JPN_FINAL
	if (!g_TextRotated90) {
		*y = *y - 1;
	}
#endif

	mainOverrideVariable("lalpha", &var8007fbac);
	mainOverrideVariable("subtlety", &var8007fbb8);
	mainOverrideVariable("subtletx", &var8007fbbc);
	mainOverrideVariable("coly", &var8007fbc4);
	mainOverrideVariable("colx", &var8007fbc0);
	mainOverrideVariable("lfade", &var8007fbb0);
	mainOverrideVariable("llimbo", &var8007fbb4);

	savedx = *x;
	savedy = *y;
	prevchar = 'H';

#if VERSION >= VERSION_JPN_FINAL
	if (lineheight == -1) {
		lineheight = chars['['].baseline + chars['['].height;
	} else {
		lineheight = 13;
	}
#else
	if (lineheight == 0) {
		lineheight = chars['['].height + chars['['].baseline;
	}

	if (g_Jpn && lineheight < 14) {
		lineheight = 14;
	}
#endif

	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);

#if VERSION >= VERSION_JPN_FINAL
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var800801d8jf));
	var80080104jf = true;
#else
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(var8007fb3c));
#endif

	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 15);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPPipeSync(gdl++);

	g_Blend.colour04 = colour;
	g_Blend.colour44 = colour;

#if VERSION >= VERSION_PAL_BETA
	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				*x += spb0 * 5;
				prevchar = 'H';
				text++;
			} else if (*text == '\n') {
				if (var8007fad4 >= 0 && savedx == *x) {
					*y += var8007fad4;
				} else {
					*y += lineheight;
				}

				*x = savedx;
				prevchar = 'H';
				text++;
			} else {
				struct fontchar *sp78;
				struct fontchar *sp74;

				textMapCodeUnitToChar(&text, &sp78, &sp74, chars, &prevchar);

				gdl = text0f15568c(gdl, x, y, sp78, sp74, font, savedx, savedy, width, height, arg9);
			}
		}
	}
#else
	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				prevchar = 'H';
				*x += spb0 * 5;
				text++;
			} else if (*text == '\n') {
				prevchar = 'H';
				text++;

				if (var8007fad4 >= 0 && savedx == *x) {
					*y += var8007fad4;
				} else {
					*y += lineheight;
				}

				*x = savedx;
			} else if (*text < 0x80) {
				u8 fontID = extTexFontID(font);
				gDPSetTextureInfoEXT(gdl++, G_TEXTYPE_FONT, fontID, *text - 0x21, 0);
				gdl = text0f15568c(gdl, x, y, &chars[*text - 0x21], &chars[prevchar - 0x21], font, savedx, savedy, width, height, arg9);
				prevchar = *text;
				text++;
			} else {
				u16 codepoint = ((*text & 0x7f) << 7) | (text[1] & 0x7f);
				struct fontchar tmpchar = {0, 0, 12, 11};

				if (codepoint & 0x2000) {
					tmpchar.width = 15;
					tmpchar.height = 16;
				}

				if ((codepoint & 0x1fff) >= 0x3c8) {
					codepoint = 2;
				}

				tmpchar.index = codepoint + 0x80;
				tmpchar.pixeldata = (void *)langGetJpnCharPixels(codepoint);

				gdl = text0f15568c(gdl, x, y, &tmpchar, &tmpchar, font, savedx, savedy, width, height, arg9);

				text += 2;
			}
		}
	}
#endif

	if (g_TextRotated90) {
		*y = *y / g_ScaleX;
	} else {
		*x = *x / g_ScaleX;
	}

	return gdl;
}

Gfx *text0f1566cc(Gfx *gdl, u32 arg1, u32 arg2)
{
	u32 colour = text0f1543ac(arg1, arg2, g_Blend.colour04);

	if (colour != g_Blend.colour44) {
		gDPSetColor(gdl++, G_SETENVCOLOR, colour);
	}

	g_Blend.colour44 = colour;

	colour = (g_Blend.colour08 & 0xffffff00) | (text0f1543ac(arg1, arg2, g_Blend.colour08) & 0xff);

	if (colour != g_Blend.colour48) {
		gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	}

	g_Blend.colour48 = colour;

	return gdl;
}

Gfx *textRenderChar(Gfx *gdl, s32 *x, s32 *y, struct fontchar *char1, struct fontchar *char2,
		struct font *font, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10)
{
	s32 tmp;
	s32 sp38;

	sp38 = *y + arg10;

	tmp = font->kerning[char2->kerningindex * 13 + char1->kerningindex] + var8007fac4;
	*x -= (tmp - 1) * var8007fad0;

	if (*x > 0
			&& *x <= viGetWidth()
			&& sp38 + char1->baseline <= viGetHeight()
			&& *x <= arg6 + arg8
			&& char1->baseline + sp38 <= arg7 + arg9
			&& *x >= arg6
			&& sp38 + char1->baseline + char1->height >= arg7) {
#if VERSION >= VERSION_JPN_FINAL
		if (char1->pixeldata == NULL) {
			char1->pixeldata = (void *)langGetJpnCharPixels(char1->index - 0x80);
		}
#else
		if (g_Blend.types) {
			gdl = text0f1566cc(gdl, *x / g_ScaleX, *y + arg10);
		}
#endif

		gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, char1->pixeldata);
		gDPLoadSync(gdl++);
		gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((char1->height * 8 + 17) >> 1) - 1, 2048);
		gDPPipeSync(gdl++);

		gdl = text0f156a24(gdl, *x - var8007fad0, sp38 - 1, char1, arg6, arg7 - 1, arg8, arg9);
	}

	*x += char1->width * var8007fad0;

	return gdl;
}

Gfx *text0f156a24(Gfx *gdl, s32 x, s32 y, struct fontchar *char1, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
	if (arg4 + arg6 >= char1->width + x + 2) {
		if (y + char1->baseline >= arg5) {
			if (arg5 + arg7 >= y + char1->baseline + char1->height + 2) {
				if (g_TextRotated90) {
					gSPTextureRectangleFlip(gdl++,
							((y - char1->baseline) - ((char1->height + 2) * var8007fad0)) * 4,
							x * 4,
							(y - char1->baseline) * 4,
							(x + char1->width + 2) * 4,
							G_TX_RENDERTILE,
							0,
							(char1->height + 1) << 5,
							1024 / var8007fad0,
							-1024);
				} else {
					gSPTextureRectangle(gdl++,
							x * 4,
							(y + char1->baseline) * 4,
							(x + char1->width * var8007fad0 + 2) * 4,
							(y + char1->baseline + char1->height + 2) * 4,
							G_TX_RENDERTILE,
							0,
							0,
							1024 / var8007fad0,
							1024);
				}
			} else {
				if (arg5 + arg7 >= y + char1->baseline) {
					gSPTextureRectangle(gdl++,
							x * 4,
							(y + char1->baseline) * 4,
							(x + char1->width * var8007fad0 + 2) * 4,
							(arg5 + arg7) * 4,
							G_TX_RENDERTILE,
							0,
							0,
							1024 / var8007fad0,
							1024);
				}
			}
		} else {
			if (y + char1->baseline + char1->height + 2 >= arg5) {
				gSPTextureRectangle(gdl++,
						x * 4,
						arg5 * 4,
						(x + char1->width * var8007fad0 + 2) * 4,
						(y + char1->baseline + char1->height + 2) * 4,
						G_TX_RENDERTILE,
						0,
						(arg5 - char1->baseline - y) << 5,
						1024 / var8007fad0,
						1024);
			}
		}
	}

	return gdl;
}

Gfx *textRender(Gfx *gdl, s32 *x, s32 *y, char *text,
		struct fontchar *chars, struct font *font, u32 arg6, u32 colour,
		s32 width, s32 height, u32 arg10, s32 lineheight)
{
	s32 savedx;
	s32 savedy;
#if VERSION >= VERSION_PAL_BETA
	u8 prevchar;
#else
	s32 prevchar;
#endif

	*x *= g_ScaleX;

	savedx = *x;
	savedy = *y;
	prevchar = 'H';

#if VERSION >= VERSION_JPN_FINAL
	if (lineheight == -1) {
		lineheight = chars['['].baseline + chars['['].height;
	} else {
		lineheight = 13;
	}
#else
	if (lineheight == 0) {
		lineheight = chars['['].height + chars['['].baseline;
	}

	if (g_Jpn && lineheight < 14) {
		lineheight = 14;
	}
#endif

	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, osVirtualToPhysical(&var8007fb5c));
	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 31);

#if VERSION >= VERSION_JPN_FINAL
	// Use clamp
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, 1, 1, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
#else
	// Use wrap
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, 1, 1, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
#endif

	gDPSetTileSize(gdl++, 1, 0, 0, 0x007c, 0x007c);
	gDPSetCycleType(gdl++, G_CYC_2CYCLE);
	gDPSetCombineLERP(gdl++,
			ENVIRONMENT, PRIMITIVE, TEXEL1_ALPHA, PRIMITIVE, 0, 0, 0, TEXEL0,
			0, 0, 0, COMBINED, COMBINED, 0, ENVIRONMENT, 0);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPSetEnvColorViaWord(gdl++, arg6);
	gDPPipeSync(gdl++);

	g_Blend.colour08 = colour;
	g_Blend.colour48 = colour;
	g_Blend.colour04 = arg6;
	g_Blend.colour44 = arg6;

#if VERSION >= VERSION_PAL_BETA
	while (*text != '\0') {
		if (*text == ' ') {
			*x += var8007fad0 * 5;
			prevchar = 'H';
			text++;
		} else if (*text == '\n') {
			*x = savedx;
			*y += lineheight;
			prevchar = 'H';
			text++;
		} else {
			struct fontchar *sp78;
			struct fontchar *sp74;

			textMapCodeUnitToChar(&text, &sp78, &sp74, chars, &prevchar);

			gdl = textRenderChar(gdl, x, y, sp78, sp74,
					font, savedx, savedy, width * var8007fad0, height, arg10);
		}
	}
#else
	while (*text != '\0') {
		if (*text == ' ') {
			*x += var8007fad0 * 5;
			prevchar = 'H';
			text++;
		} else if (*text == '\n') {
			*x = savedx;
			*y += lineheight;
			prevchar = 'H';
			text++;
		} else if (*text < 0x80) {
			u8 fontID = extTexFontID(font);
			gDPSetTextureInfoEXT(gdl++, G_TEXTYPE_FONT, fontID, *text - 0x21, MASK_FONT_OUTLINE);
			gdl = textRenderChar(gdl, x, y, &chars[*text - 0x21], &chars[prevchar - 0x21],
					font, savedx, savedy, width * var8007fad0, height, arg10);
			prevchar = *text;
			text++;
		} else {
			u16 codepoint = ((*text & 0x7f) << 7) | (text[1] & 0x7f);
			struct fontchar sp74 = {0, 0, 12, 11};

			if (codepoint & 0x2000) {
				sp74.width = 15;
				sp74.height = 16;
			}

			if ((codepoint & 0x1fff) >= 0x3c8) {
				codepoint = 2;
			}

			sp74.index = codepoint + 0x80;
			sp74.pixeldata = (void *)langGetJpnCharPixels(codepoint);

			u8 fontID = extTexFontID(font);
			gDPSetTextureInfoEXT(gdl++, G_TEXTYPE_FONT, fontID, *text - 0x21, MASK_FONT_OUTLINE);
			gdl = textRenderChar(gdl, x, y, &sp74, &sp74, font, savedx, savedy, width * var8007fad0, height, arg10);

			text += 2;
		}
	}
#endif

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

	*x = *x / g_ScaleX;

	return gdl;
}

#if VERSION == VERSION_JPN_FINAL
Gfx *func0f1574d0jf(Gfx *gdl, s32 *x, s32 *y, char *text, struct fontchar *font1, struct font *font2, s32 colour, u32 colour2, s32 width, s32 height, s32 arg9, u32 arg10)
{
	return textRenderProjected(gdl, x, y, text, font1, font2, colour, width, height, arg9, arg10);
}
#endif

#if VERSION < VERSION_PAL_BETA && MATCHING
GLOBAL_ASM(
glabel textMeasure
/*  f1572f8:	27bdffe8 */ 	addiu	$sp,$sp,-24
/*  f1572fc:	afb10008 */ 	sw	$s1,0x8($sp)
/*  f157300:	8fb1002c */ 	lw	$s1,0x2c($sp)
/*  f157304:	afb40014 */ 	sw	$s4,0x14($sp)
/*  f157308:	afb30010 */ 	sw	$s3,0x10($sp)
/*  f15730c:	afb2000c */ 	sw	$s2,0xc($sp)
/*  f157310:	afb00004 */ 	sw	$s0,0x4($sp)
/*  f157314:	ac800000 */ 	sw	$zero,0x0($a0)
/*  f157318:	00808025 */ 	or	$s0,$a0,$zero
/*  f15731c:	24030048 */ 	addiu	$v1,$zero,0x48
/*  f157320:	00001025 */ 	or	$v0,$zero,$zero
/*  f157324:	16200004 */ 	bnez	$s1,.L0f157338
/*  f157328:	aca00000 */ 	sw	$zero,0x0($a1)
/*  f15732c:	90ee0446 */ 	lbu	$t6,0x446($a3)
/*  f157330:	80ef0445 */ 	lb	$t7,0x445($a3)
/*  f157334:	01cf8821 */ 	addu	$s1,$t6,$t7
.L0f157338:
/*  f157338:	3c188008 */ 	lui	$t8,%hi(g_Jpn)
/*  f15733c:	8f184120 */ 	lw	$t8,%lo(g_Jpn)($t8)
/*  f157340:	2a21000e */ 	slti	$at,$s1,0xe
/*  f157344:	13000004 */ 	beqz	$t8,.L0f157358
/*  f157348:	00000000 */ 	nop
/*  f15734c:	10200002 */ 	beqz	$at,.L0f157358
/*  f157350:	00000000 */ 	nop
/*  f157354:	2411000e */ 	addiu	$s1,$zero,0xe
.L0f157358:
/*  f157358:	10c00059 */ 	beqz	$a2,.L0f1574c0
/*  f15735c:	00000000 */ 	nop
/*  f157360:	90c40000 */ 	lbu	$a0,0x0($a2)
/*  f157364:	3c148008 */ 	lui	$s4,%hi(var8007fac4)
/*  f157368:	2694fac4 */ 	addiu	$s4,$s4,%lo(var8007fac4)
/*  f15736c:	10800054 */ 	beqz	$a0,.L0f1574c0
/*  f157370:	00804025 */ 	or	$t0,$a0,$zero
/*  f157374:	2413000d */ 	addiu	$s3,$zero,0xd
/*  f157378:	2412000c */ 	addiu	$s2,$zero,0xc
/*  f15737c:	240c000a */ 	addiu	$t4,$zero,0xa
/*  f157380:	240b0020 */ 	addiu	$t3,$zero,0x20
/*  f157384:	8fa90028 */ 	lw	$t1,0x28($sp)
.L0f157388:
/*  f157388:	1568000a */ 	bne	$t3,$t0,.L0f1573b4
/*  f15738c:	00000000 */ 	nop
/*  f157390:	90d90001 */ 	lbu	$t9,0x1($a2)
/*  f157394:	24030048 */ 	addiu	$v1,$zero,0x48
/*  f157398:	11990004 */ 	beq	$t4,$t9,.L0f1573ac
/*  f15739c:	00000000 */ 	nop
/*  f1573a0:	8cae0000 */ 	lw	$t6,0x0($a1)
/*  f1573a4:	25cf0005 */ 	addiu	$t7,$t6,0x5
/*  f1573a8:	acaf0000 */ 	sw	$t7,0x0($a1)
.L0f1573ac:
/*  f1573ac:	10000041 */ 	b	.L0f1574b4
/*  f1573b0:	24c60001 */ 	addiu	$a2,$a2,0x1
.L0f1573b4:
/*  f1573b4:	1588000c */ 	bne	$t4,$t0,.L0f1573e8
/*  f1573b8:	29010080 */ 	slti	$at,$t0,0x80
/*  f1573bc:	8ca40000 */ 	lw	$a0,0x0($a1)
/*  f1573c0:	0044082a */ 	slt	$at,$v0,$a0
/*  f1573c4:	50200003 */ 	beqzl	$at,.L0f1573d4
/*  f1573c8:	aca00000 */ 	sw	$zero,0x0($a1)
/*  f1573cc:	00801025 */ 	or	$v0,$a0,$zero
/*  f1573d0:	aca00000 */ 	sw	$zero,0x0($a1)
.L0f1573d4:
/*  f1573d4:	8e180000 */ 	lw	$t8,0x0($s0)
/*  f1573d8:	24c60001 */ 	addiu	$a2,$a2,0x1
/*  f1573dc:	0311c821 */ 	addu	$t9,$t8,$s1
/*  f1573e0:	10000034 */ 	b	.L0f1574b4
/*  f1573e4:	ae190000 */ 	sw	$t9,0x0($s0)
.L0f1573e8:
/*  f1573e8:	5020001d */ 	beqzl	$at,.L0f157460
/*  f1573ec:	290100c0 */ 	slti	$at,$t0,0xc0
/*  f1573f0:	308800ff */ 	andi	$t0,$a0,0xff
/*  f1573f4:	01120019 */ 	multu	$t0,$s2
/*  f1573f8:	24c60001 */ 	addiu	$a2,$a2,0x1
/*  f1573fc:	00007012 */ 	mflo	$t6
/*  f157400:	00ee5021 */ 	addu	$t2,$a3,$t6
/*  f157404:	8d4ffe78 */ 	lw	$t7,-0x188($t2)
/*  f157408:	00720019 */ 	multu	$v1,$s2
/*  f15740c:	000fc080 */ 	sll	$t8,$t7,0x2
/*  f157410:	0138c821 */ 	addu	$t9,$t1,$t8
/*  f157414:	00007012 */ 	mflo	$t6
/*  f157418:	00ee7821 */ 	addu	$t7,$a3,$t6
/*  f15741c:	8df8fe78 */ 	lw	$t8,-0x188($t7)
/*  f157420:	03130019 */ 	multu	$t8,$s3
/*  f157424:	00007012 */ 	mflo	$t6
/*  f157428:	000e7880 */ 	sll	$t7,$t6,0x2
/*  f15742c:	032fc021 */ 	addu	$t8,$t9,$t7
/*  f157430:	8f0e0000 */ 	lw	$t6,0x0($t8)
/*  f157434:	8e990000 */ 	lw	$t9,0x0($s4)
/*  f157438:	9158fe77 */ 	lbu	$t8,-0x189($t2)
/*  f15743c:	8caf0000 */ 	lw	$t7,0x0($a1)
/*  f157440:	01d92021 */ 	addu	$a0,$t6,$t9
/*  f157444:	2484ffff */ 	addiu	$a0,$a0,-1
/*  f157448:	01f87021 */ 	addu	$t6,$t7,$t8
/*  f15744c:	01c4c823 */ 	subu	$t9,$t6,$a0
/*  f157450:	acb90000 */ 	sw	$t9,0x0($a1)
/*  f157454:	10000017 */ 	b	.L0f1574b4
/*  f157458:	90c3ffff */ 	lbu	$v1,-0x1($a2)
/*  f15745c:	290100c0 */ 	slti	$at,$t0,0xc0
.L0f157460:
/*  f157460:	5020000c */ 	beqzl	$at,.L0f157494
/*  f157464:	8d380000 */ 	lw	$t8,0x0($t1)
/*  f157468:	8d2f0000 */ 	lw	$t7,0x0($t1)
/*  f15746c:	8e980000 */ 	lw	$t8,0x0($s4)
/*  f157470:	8cae0000 */ 	lw	$t6,0x0($a1)
/*  f157474:	24c60002 */ 	addiu	$a2,$a2,0x2
/*  f157478:	01f82021 */ 	addu	$a0,$t7,$t8
/*  f15747c:	2484ffff */ 	addiu	$a0,$a0,-1
/*  f157480:	01c4c823 */ 	subu	$t9,$t6,$a0
/*  f157484:	272f000b */ 	addiu	$t7,$t9,0xb
/*  f157488:	1000000a */ 	b	.L0f1574b4
/*  f15748c:	acaf0000 */ 	sw	$t7,0x0($a1)
/*  f157490:	8d380000 */ 	lw	$t8,0x0($t1)
.L0f157494:
/*  f157494:	8e8e0000 */ 	lw	$t6,0x0($s4)
/*  f157498:	8cb90000 */ 	lw	$t9,0x0($a1)
/*  f15749c:	24c60002 */ 	addiu	$a2,$a2,0x2
/*  f1574a0:	030e2021 */ 	addu	$a0,$t8,$t6
/*  f1574a4:	2484ffff */ 	addiu	$a0,$a0,-1
/*  f1574a8:	03247823 */ 	subu	$t7,$t9,$a0
/*  f1574ac:	25f8000f */ 	addiu	$t8,$t7,0xf
/*  f1574b0:	acb80000 */ 	sw	$t8,0x0($a1)
.L0f1574b4:
/*  f1574b4:	90c40000 */ 	lbu	$a0,0x0($a2)
/*  f1574b8:	1480ffb3 */ 	bnez	$a0,.L0f157388
/*  f1574bc:	00804025 */ 	or	$t0,$a0,$zero
.L0f1574c0:
/*  f1574c0:	3c0e8008 */ 	lui	$t6,%hi(g_ScaleX)
/*  f1574c4:	8dcefac0 */ 	lw	$t6,%lo(g_ScaleX)($t6)
/*  f1574c8:	24010001 */ 	addiu	$at,$zero,0x1
/*  f1574cc:	3c0f8008 */ 	lui	$t7,%hi(var8007fad0)
/*  f1574d0:	55c10008 */ 	bnel	$t6,$at,.L0f1574f4
/*  f1574d4:	8cae0000 */ 	lw	$t6,0x0($a1)
/*  f1574d8:	8cb90000 */ 	lw	$t9,0x0($a1)
/*  f1574dc:	8deffad0 */ 	lw	$t7,%lo(var8007fad0)($t7)
/*  f1574e0:	032f0019 */ 	multu	$t9,$t7
/*  f1574e4:	0000c012 */ 	mflo	$t8
/*  f1574e8:	acb80000 */ 	sw	$t8,0x0($a1)
/*  f1574ec:	00000000 */ 	nop
/*  f1574f0:	8cae0000 */ 	lw	$t6,0x0($a1)
.L0f1574f4:
/*  f1574f4:	01c2082a */ 	slt	$at,$t6,$v0
/*  f1574f8:	50200003 */ 	beqzl	$at,.L0f157508
/*  f1574fc:	8fb00004 */ 	lw	$s0,0x4($sp)
/*  f157500:	aca20000 */ 	sw	$v0,0x0($a1)
/*  f157504:	8fb00004 */ 	lw	$s0,0x4($sp)
.L0f157508:
/*  f157508:	8fb10008 */ 	lw	$s1,0x8($sp)
/*  f15750c:	8fb2000c */ 	lw	$s2,0xc($sp)
/*  f157510:	8fb30010 */ 	lw	$s3,0x10($sp)
/*  f157514:	8fb40014 */ 	lw	$s4,0x14($sp)
/*  f157518:	03e00008 */ 	jr	$ra
/*  f15751c:	27bd0018 */ 	addiu	$sp,$sp,0x18
);
#else
// Mismatch: Regalloc
void textMeasure(s32 *textheight, s32 *textwidth, char *text, struct fontchar *font1, struct font *font2, s32 lineheight)
{
	char prevchar;
	char thischar;
	s32 longest;
#if VERSION == VERSION_JPN_FINAL
	s32 overlap = 0;
	struct fontchar *tmp2 = font1;
	struct fontchar *sp54;
	struct fontchar *sp50;
#endif
	s32 tmp;

	prevchar = 'H';
	thischar = '\0';
	longest = 0;
	*textheight = 0;
	*textwidth = 0;

#if VERSION == VERSION_JPN_FINAL
	if (lineheight == -1) {
		lineheight = font1['['].baseline + font1['['].height;
	} else {
		lineheight = 13;
	}
#else
	if (lineheight == 0) {
		lineheight = font1['['].baseline + font1['['].height;
	}

	if (g_Jpn && lineheight < 14) {
		lineheight = 14;
	}
#endif

	if (text) {
		while (*text != '\0') {
			if (*text == ' ') {
				// Space
				if (text[1] != '\n') {
					*textwidth += 5;
				}

				prevchar = 'H';
				text++;
			} else if (*text == '\n') {
				// Line break
#if VERSION >= VERSION_JPN_FINAL
				*textwidth -= overlap;
				overlap = 0;
#endif

				if (*textwidth > longest) {
					longest = *textwidth;
				}

				*textwidth = 0;
				*textheight += lineheight;
				text++;
			} else {
#if VERSION >= VERSION_JPN_FINAL
				textMapCodeUnitToChar(&text, &sp54, &sp50, font1, &prevchar);

				overlap = 0;

				if (sp54->index == 0xbe) {
					overlap = 4;
				}

				if (sp54->index == 0x84) {
					overlap = 7;
				}

				if (font2) {
					tmp = font2->kerning[sp50->kerningindex * 13 + sp54->kerningindex] + var8007fac4 - 1;
				} else {
					tmp = 0;
				}

				*textwidth = *textwidth + sp54->width - tmp;
#elif VERSION >= VERSION_PAL_BETA
				struct fontchar *sp50;
				struct fontchar *sp4c;

				textMapCodeUnitToChar(&text, &sp50, &sp4c, font1, &prevchar);

				tmp = font2->kerning[sp4c->kerningindex * 13 + sp50->kerningindex] + var8007fac4 - 1;
				*textwidth = *textwidth + sp50->width - tmp;
#else
				if (*text < 0x80) {
					// Normal single-byte character
					thischar = *text;
					tmp = font2->kerning[font1[prevchar - 0x21].kerningindex * 13 + font1[thischar - 0x21].kerningindex] + var8007fac4 - 1;
					*textwidth = font1[thischar - 0x21].width + *textwidth - tmp;

					prevchar = *text;
					text++;
				} else if (*text < 0xc0) {
					// Multi-byte character
					tmp = font2->kerning[0] + var8007fac4 - 1;
					*textwidth = *textwidth - tmp + 11;
					text += 2;
				} else {
					// Multi-byte character
					tmp = font2->kerning[0] + var8007fac4 - 1;
					*textwidth = *textwidth - tmp + 15;
					text += 2;
				}
#endif
			}
		}
	}

	// @bug? Shouldn't this go at the very end of the function?
	if (g_ScaleX == 1) {
		*textwidth *= var8007fad0;
	}

#if VERSION >= VERSION_JPN_FINAL
	*textwidth -= overlap;
#endif

	if (longest > *textwidth) {
		*textwidth = longest;
	}
}
#endif

#if VERSION == VERSION_JPN_FINAL
bool func0f157768jf(s32 arg0, s32 arg1)
{
	return false;
}
#endif

#if MATCHING && VERSION >= VERSION_JPN_FINAL
GLOBAL_ASM(
glabel textWrap
/*  f157778:	27bdff48 */ 	addiu	$sp,$sp,-184
/*  f15777c:	afbe0040 */ 	sw	$s8,0x40($sp)
/*  f157780:	afb3002c */ 	sw	$s3,0x2c($sp)
/*  f157784:	240e0001 */ 	li	$t6,0x1
/*  f157788:	00c09825 */ 	move	$s3,$a2
/*  f15778c:	00a0f025 */ 	move	$s8,$a1
/*  f157790:	afbf0044 */ 	sw	$ra,0x44($sp)
/*  f157794:	afb7003c */ 	sw	$s7,0x3c($sp)
/*  f157798:	afb60038 */ 	sw	$s6,0x38($sp)
/*  f15779c:	afb50034 */ 	sw	$s5,0x34($sp)
/*  f1577a0:	afb40030 */ 	sw	$s4,0x30($sp)
/*  f1577a4:	afb20028 */ 	sw	$s2,0x28($sp)
/*  f1577a8:	afb10024 */ 	sw	$s1,0x24($sp)
/*  f1577ac:	afb00020 */ 	sw	$s0,0x20($sp)
/*  f1577b0:	afa400b8 */ 	sw	$a0,0xb8($sp)
/*  f1577b4:	afa700c4 */ 	sw	$a3,0xc4($sp)
/*  f1577b8:	afa000b4 */ 	sw	$zero,0xb4($sp)
/*  f1577bc:	afa000a4 */ 	sw	$zero,0xa4($sp)
/*  f1577c0:	afae00a0 */ 	sw	$t6,0xa0($sp)
/*  f1577c4:	0000a825 */ 	move	$s5,$zero
.JF0f1577c8:
/*  f1577c8:	afa000a8 */ 	sw	$zero,0xa8($sp)
/*  f1577cc:	00003825 */ 	move	$a3,$zero
/*  f1577d0:	afa00070 */ 	sw	$zero,0x70($sp)
/*  f1577d4:	00009025 */ 	move	$s2,$zero
/*  f1577d8:	27b40074 */ 	addiu	$s4,$sp,0x74
/*  f1577dc:	93c30000 */ 	lbu	$v1,0x0($s8)
.JF0f1577e0:
/*  f1577e0:	00008825 */ 	move	$s1,$zero
/*  f1577e4:	0000b825 */ 	move	$s7,$zero
/*  f1577e8:	28610080 */ 	slti	$at,$v1,0x80
/*  f1577ec:	10200003 */ 	beqz	$at,.JF0f1577fc
/*  f1577f0:	0000b025 */ 	move	$s6,$zero
/*  f1577f4:	1000000f */ 	b	.JF0f157834
/*  f1577f8:	306400ff */ 	andi	$a0,$v1,0xff
.JF0f1577fc:
/*  f1577fc:	93d00001 */ 	lbu	$s0,0x1($s8)
/*  f157800:	306f007f */ 	andi	$t7,$v1,0x7f
/*  f157804:	000fc1c0 */ 	sll	$t8,$t7,0x7
/*  f157808:	3219007f */ 	andi	$t9,$s0,0x7f
/*  f15780c:	03191025 */ 	or	$v0,$t8,$t9
/*  f157810:	24170001 */ 	li	$s7,0x1
/*  f157814:	3051ffff */ 	andi	$s1,$v0,0xffff
/*  f157818:	3044ffff */ 	andi	$a0,$v0,0xffff
/*  f15781c:	0fc551e1 */ 	jal	func0f154784jf
/*  f157820:	afa70094 */ 	sw	$a3,0x94($sp)
/*  f157824:	8fa70094 */ 	lw	$a3,0x94($sp)
/*  f157828:	16000002 */ 	bnez	$s0,.JF0f157834
/*  f15782c:	304400ff */ 	andi	$a0,$v0,0xff
/*  f157830:	0000b825 */ 	move	$s7,$zero
.JF0f157834:
/*  f157834:	1480001d */ 	bnez	$a0,.JF0f1578ac
/*  f157838:	00801025 */ 	move	$v0,$a0
/*  f15783c:	12e00050 */ 	beqz	$s7,.JF0f157980
/*  f157840:	00008025 */ 	move	$s0,$zero
/*  f157844:	2a210034 */ 	slti	$at,$s1,0x34
/*  f157848:	14200005 */ 	bnez	$at,.JF0f157860
/*  f15784c:	02201025 */ 	move	$v0,$s1
/*  f157850:	2a21004e */ 	slti	$at,$s1,0x4e
/*  f157854:	50200003 */ 	beqzl	$at,.JF0f157864
/*  f157858:	284100af */ 	slti	$at,$v0,0xaf
/*  f15785c:	24100001 */ 	li	$s0,0x1
.JF0f157860:
/*  f157860:	284100af */ 	slti	$at,$v0,0xaf
.JF0f157864:
/*  f157864:	14200004 */ 	bnez	$at,.JF0f157878
/*  f157868:	284100b8 */ 	slti	$at,$v0,0xb8
/*  f15786c:	50200003 */ 	beqzl	$at,.JF0f15787c
/*  f157870:	24010104 */ 	li	$at,0x104
/*  f157874:	24100001 */ 	li	$s0,0x1
.JF0f157878:
/*  f157878:	24010104 */ 	li	$at,0x104
.JF0f15787c:
/*  f15787c:	54410003 */ 	bnel	$v0,$at,.JF0f15788c
/*  f157880:	24010105 */ 	li	$at,0x105
/*  f157884:	24100001 */ 	li	$s0,0x1
/*  f157888:	24010105 */ 	li	$at,0x105
.JF0f15788c:
/*  f15788c:	14410002 */ 	bne	$v0,$at,.JF0f157898
/*  f157890:	00000000 */ 	nop
/*  f157894:	24100001 */ 	li	$s0,0x1
.JF0f157898:
/*  f157898:	16a00002 */ 	bnez	$s5,.JF0f1578a4
/*  f15789c:	00000000 */ 	nop
/*  f1578a0:	24100001 */ 	li	$s0,0x1
.JF0f1578a4:
/*  f1578a4:	10000036 */ 	b	.JF0f157980
/*  f1578a8:	2416000b */ 	li	$s6,0xb
.JF0f1578ac:
/*  f1578ac:	28410021 */ 	slti	$at,$v0,0x21
/*  f1578b0:	14200003 */ 	bnez	$at,.JF0f1578c0
/*  f1578b4:	8faa00c4 */ 	lw	$t2,0xc4($sp)
/*  f1578b8:	10000002 */ 	b	.JF0f1578c4
/*  f1578bc:	24100001 */ 	li	$s0,0x1
.JF0f1578c0:
/*  f1578c0:	00008025 */ 	move	$s0,$zero
.JF0f1578c4:
/*  f1578c4:	00025880 */ 	sll	$t3,$v0,0x2
/*  f1578c8:	01625823 */ 	subu	$t3,$t3,$v0
/*  f1578cc:	000b5880 */ 	sll	$t3,$t3,0x2
/*  f1578d0:	014b6021 */ 	addu	$t4,$t2,$t3
/*  f1578d4:	1600000c */ 	bnez	$s0,.JF0f157908
/*  f1578d8:	9196fe78 */ 	lbu	$s6,-0x188($t4)
/*  f1578dc:	16a0000a */ 	bnez	$s5,.JF0f157908
/*  f1578e0:	00000000 */ 	nop
/*  f1578e4:	10400004 */ 	beqz	$v0,.JF0f1578f8
/*  f1578e8:	24010020 */ 	li	$at,0x20
/*  f1578ec:	50410003 */ 	beql	$v0,$at,.JF0f1578fc
/*  f1578f0:	24010020 */ 	li	$at,0x20
/*  f1578f4:	24100001 */ 	li	$s0,0x1
.JF0f1578f8:
/*  f1578f8:	24010020 */ 	li	$at,0x20
.JF0f1578fc:
/*  f1578fc:	14410002 */ 	bne	$v0,$at,.JF0f157908
/*  f157900:	240d0001 */ 	li	$t5,0x1
/*  f157904:	afad0070 */ 	sw	$t5,0x70($sp)
.JF0f157908:
/*  f157908:	12a0001d */ 	beqz	$s5,.JF0f157980
/*  f15790c:	28410041 */ 	slti	$at,$v0,0x41
/*  f157910:	14200002 */ 	bnez	$at,.JF0f15791c
/*  f157914:	2841005b */ 	slti	$at,$v0,0x5b
/*  f157918:	14200009 */ 	bnez	$at,.JF0f157940
.JF0f15791c:
/*  f15791c:	28410061 */ 	slti	$at,$v0,0x61
/*  f157920:	14200002 */ 	bnez	$at,.JF0f15792c
/*  f157924:	2841007b */ 	slti	$at,$v0,0x7b
/*  f157928:	14200005 */ 	bnez	$at,.JF0f157940
.JF0f15792c:
/*  f15792c:	28410030 */ 	slti	$at,$v0,0x30
/*  f157930:	14200013 */ 	bnez	$at,.JF0f157980
/*  f157934:	2841003a */ 	slti	$at,$v0,0x3a
/*  f157938:	10200011 */ 	beqz	$at,.JF0f157980
/*  f15793c:	00000000 */ 	nop
.JF0f157940:
/*  f157940:	9285ffff */ 	lbu	$a1,-0x1($s4)
/*  f157944:	28a10080 */ 	slti	$at,$a1,0x80
/*  f157948:	1420000d */ 	bnez	$at,.JF0f157980
/*  f15794c:	00000000 */ 	nop
/*  f157950:	9282fffe */ 	lbu	$v0,-0x2($s4)
/*  f157954:	30b8007f */ 	andi	$t8,$a1,0x7f
/*  f157958:	afa70094 */ 	sw	$a3,0x94($sp)
/*  f15795c:	304e007f */ 	andi	$t6,$v0,0x7f
/*  f157960:	000e79c0 */ 	sll	$t7,$t6,0x7
/*  f157964:	01f82025 */ 	or	$a0,$t7,$t8
/*  f157968:	3099ffff */ 	andi	$t9,$a0,0xffff
/*  f15796c:	0fc551e1 */ 	jal	func0f154784jf
/*  f157970:	03202025 */ 	move	$a0,$t9
/*  f157974:	14400002 */ 	bnez	$v0,.JF0f157980
/*  f157978:	8fa70094 */ 	lw	$a3,0x94($sp)
/*  f15797c:	00008025 */ 	move	$s0,$zero
.JF0f157980:
/*  f157980:	1200000d */ 	beqz	$s0,.JF0f1579b8
/*  f157984:	00000000 */ 	nop
/*  f157988:	93ca0000 */ 	lbu	$t2,0x0($s8)
/*  f15798c:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f157990:	26b50001 */ 	addiu	$s5,$s5,0x1
/*  f157994:	26940001 */ 	addiu	$s4,$s4,0x1
/*  f157998:	12e00006 */ 	beqz	$s7,.JF0f1579b4
/*  f15799c:	a28affff */ 	sb	$t2,-0x1($s4)
/*  f1579a0:	93cb0000 */ 	lbu	$t3,0x0($s8)
/*  f1579a4:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f1579a8:	26b50001 */ 	addiu	$s5,$s5,0x1
/*  f1579ac:	26940001 */ 	addiu	$s4,$s4,0x1
/*  f1579b0:	a28bffff */ 	sb	$t3,-0x1($s4)
.JF0f1579b4:
/*  f1579b4:	00f63821 */ 	addu	$a3,$a3,$s6
.JF0f1579b8:
/*  f1579b8:	5600ff89 */ 	bnezl	$s0,.JF0f1577e0
/*  f1579bc:	93c30000 */ 	lbu	$v1,0x0($s8)
/*  f1579c0:	a2800000 */ 	sb	$zero,0x0($s4)
/*  f1579c4:	8fac00c8 */ 	lw	$t4,0xc8($sp)
/*  f1579c8:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f1579cc:	8fa700c4 */ 	lw	$a3,0xc4($sp)
/*  f1579d0:	27a400a4 */ 	addiu	$a0,$sp,0xa4
/*  f1579d4:	27a500a8 */ 	addiu	$a1,$sp,0xa8
/*  f1579d8:	27a60074 */ 	addiu	$a2,$sp,0x74
/*  f1579dc:	0fc55d49 */ 	jal	textMeasure
/*  f1579e0:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f1579e4:	8fad00b4 */ 	lw	$t5,0xb4($sp)
/*  f1579e8:	8fae00a8 */ 	lw	$t6,0xa8($sp)
/*  f1579ec:	8fb800b8 */ 	lw	$t8,0xb8($sp)
/*  f1579f0:	3c088008 */ 	lui	$t0,%hi(g_WrapIndentCount)
/*  f1579f4:	01ae7821 */ 	addu	$t7,$t5,$t6
/*  f1579f8:	030f082a */ 	slt	$at,$t8,$t7
/*  f1579fc:	25080104 */ 	addiu	$t0,$t0,%lo(g_WrapIndentCount)
/*  f157a00:	24090020 */ 	li	$t1,0x20
/*  f157a04:	14200003 */ 	bnez	$at,.JF0f157a14
/*  f157a08:	afaf00b4 */ 	sw	$t7,0xb4($sp)
/*  f157a0c:	10000002 */ 	b	.JF0f157a18
/*  f157a10:	24030001 */ 	li	$v1,0x1
.JF0f157a14:
/*  f157a14:	00001825 */ 	move	$v1,$zero
.JF0f157a18:
/*  f157a18:	93c20000 */ 	lbu	$v0,0x0($s8)
/*  f157a1c:	2401000a */ 	li	$at,0xa
/*  f157a20:	240d000a */ 	li	$t5,0xa
/*  f157a24:	1441001d */ 	bne	$v0,$at,.JF0f157a9c
/*  f157a28:	00000000 */ 	nop
/*  f157a2c:	1460000f */ 	bnez	$v1,.JF0f157a6c
/*  f157a30:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f157a34:	2419000a */ 	li	$t9,0xa
/*  f157a38:	a2790000 */ 	sb	$t9,0x0($s3)
/*  f157a3c:	8d0a0000 */ 	lw	$t2,0x0($t0)
/*  f157a40:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157a44:	19400009 */ 	blez	$t2,.JF0f157a6c
/*  f157a48:	00000000 */ 	nop
/*  f157a4c:	a2690000 */ 	sb	$t1,0x0($s3)
.JF0f157a50:
/*  f157a50:	8d0b0000 */ 	lw	$t3,0x0($t0)
/*  f157a54:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157a58:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157a5c:	024b082a */ 	slt	$at,$s2,$t3
/*  f157a60:	5420fffb */ 	bnezl	$at,.JF0f157a50
/*  f157a64:	a2690000 */ 	sb	$t1,0x0($s3)
/*  f157a68:	00009025 */ 	move	$s2,$zero
.JF0f157a6c:
/*  f157a6c:	1aa00007 */ 	blez	$s5,.JF0f157a8c
/*  f157a70:	27a20074 */ 	addiu	$v0,$sp,0x74
.JF0f157a74:
/*  f157a74:	904c0000 */ 	lbu	$t4,0x0($v0)
/*  f157a78:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157a7c:	24420001 */ 	addiu	$v0,$v0,0x1
/*  f157a80:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157a84:	1655fffb */ 	bne	$s2,$s5,.JF0f157a74
/*  f157a88:	a26cffff */ 	sb	$t4,-0x1($s3)
.JF0f157a8c:
/*  f157a8c:	afa000b4 */ 	sw	$zero,0xb4($sp)
/*  f157a90:	a26d0000 */ 	sb	$t5,0x0($s3)
/*  f157a94:	10000048 */ 	b	.JF0f157bb8
/*  f157a98:	26730001 */ 	addiu	$s3,$s3,0x1
.JF0f157a9c:
/*  f157a9c:	1440001c */ 	bnez	$v0,.JF0f157b10
/*  f157aa0:	00000000 */ 	nop
/*  f157aa4:	1460000f */ 	bnez	$v1,.JF0f157ae4
/*  f157aa8:	afa000a0 */ 	sw	$zero,0xa0($sp)
/*  f157aac:	240e000a */ 	li	$t6,0xa
/*  f157ab0:	a26e0000 */ 	sb	$t6,0x0($s3)
/*  f157ab4:	8d180000 */ 	lw	$t8,0x0($t0)
/*  f157ab8:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157abc:	1b000009 */ 	blez	$t8,.JF0f157ae4
/*  f157ac0:	00000000 */ 	nop
/*  f157ac4:	a2690000 */ 	sb	$t1,0x0($s3)
.JF0f157ac8:
/*  f157ac8:	8d0f0000 */ 	lw	$t7,0x0($t0)
/*  f157acc:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157ad0:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157ad4:	024f082a */ 	slt	$at,$s2,$t7
/*  f157ad8:	5420fffb */ 	bnezl	$at,.JF0f157ac8
/*  f157adc:	a2690000 */ 	sb	$t1,0x0($s3)
/*  f157ae0:	00009025 */ 	move	$s2,$zero
.JF0f157ae4:
/*  f157ae4:	1aa00008 */ 	blez	$s5,.JF0f157b08
/*  f157ae8:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f157aec:	27a20074 */ 	addiu	$v0,$sp,0x74
.JF0f157af0:
/*  f157af0:	90590000 */ 	lbu	$t9,0x0($v0)
/*  f157af4:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157af8:	24420001 */ 	addiu	$v0,$v0,0x1
/*  f157afc:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157b00:	1655fffb */ 	bne	$s2,$s5,.JF0f157af0
/*  f157b04:	a279ffff */ 	sb	$t9,-0x1($s3)
.JF0f157b08:
/*  f157b08:	1000002b */ 	b	.JF0f157bb8
/*  f157b0c:	a2600000 */ 	sb	$zero,0x0($s3)
.JF0f157b10:
/*  f157b10:	14600013 */ 	bnez	$v1,.JF0f157b60
/*  f157b14:	240a000a */ 	li	$t2,0xa
/*  f157b18:	a26a0000 */ 	sb	$t2,0x0($s3)
/*  f157b1c:	8d020000 */ 	lw	$v0,0x0($t0)
/*  f157b20:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157b24:	5840000a */ 	blezl	$v0,.JF0f157b50
/*  f157b28:	8fac00a8 */ 	lw	$t4,0xa8($sp)
/*  f157b2c:	a2690000 */ 	sb	$t1,0x0($s3)
.JF0f157b30:
/*  f157b30:	8d020000 */ 	lw	$v0,0x0($t0)
/*  f157b34:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157b38:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157b3c:	0242082a */ 	slt	$at,$s2,$v0
/*  f157b40:	5420fffb */ 	bnezl	$at,.JF0f157b30
/*  f157b44:	a2690000 */ 	sb	$t1,0x0($s3)
/*  f157b48:	00009025 */ 	move	$s2,$zero
/*  f157b4c:	8fac00a8 */ 	lw	$t4,0xa8($sp)
.JF0f157b50:
/*  f157b50:	00025880 */ 	sll	$t3,$v0,0x2
/*  f157b54:	01625821 */ 	addu	$t3,$t3,$v0
/*  f157b58:	016c6821 */ 	addu	$t5,$t3,$t4
/*  f157b5c:	afad00b4 */ 	sw	$t5,0xb4($sp)
.JF0f157b60:
/*  f157b60:	1aa00007 */ 	blez	$s5,.JF0f157b80
/*  f157b64:	27a20074 */ 	addiu	$v0,$sp,0x74
.JF0f157b68:
/*  f157b68:	904e0000 */ 	lbu	$t6,0x0($v0)
/*  f157b6c:	26520001 */ 	addiu	$s2,$s2,0x1
/*  f157b70:	24420001 */ 	addiu	$v0,$v0,0x1
/*  f157b74:	26730001 */ 	addiu	$s3,$s3,0x1
/*  f157b78:	1655fffb */ 	bne	$s2,$s5,.JF0f157b68
/*  f157b7c:	a26effff */ 	sb	$t6,-0x1($s3)
.JF0f157b80:
/*  f157b80:	93c20000 */ 	lbu	$v0,0x0($s8)
/*  f157b84:	24010020 */ 	li	$at,0x20
/*  f157b88:	8fb80070 */ 	lw	$t8,0x70($sp)
/*  f157b8c:	10410002 */ 	beq	$v0,$at,.JF0f157b98
/*  f157b90:	8faf00b4 */ 	lw	$t7,0xb4($sp)
/*  f157b94:	13000008 */ 	beqz	$t8,.JF0f157bb8
.JF0f157b98:
/*  f157b98:	28410080 */ 	slti	$at,$v0,0x80
/*  f157b9c:	14200002 */ 	bnez	$at,.JF0f157ba8
/*  f157ba0:	25f90005 */ 	addiu	$t9,$t7,0x5
/*  f157ba4:	27de0001 */ 	addiu	$s8,$s8,0x1
.JF0f157ba8:
/*  f157ba8:	afb900b4 */ 	sw	$t9,0xb4($sp)
/*  f157bac:	27de0001 */ 	addiu	$s8,$s8,0x1
/*  f157bb0:	a2690000 */ 	sb	$t1,0x0($s3)
/*  f157bb4:	26730001 */ 	addiu	$s3,$s3,0x1
.JF0f157bb8:
/*  f157bb8:	8faa00a0 */ 	lw	$t2,0xa0($sp)
/*  f157bbc:	24010001 */ 	li	$at,0x1
/*  f157bc0:	5141ff01 */ 	beql	$t2,$at,.JF0f1577c8
/*  f157bc4:	0000a825 */ 	move	$s5,$zero
/*  f157bc8:	8fbf0044 */ 	lw	$ra,0x44($sp)
/*  f157bcc:	8fb00020 */ 	lw	$s0,0x20($sp)
/*  f157bd0:	8fb10024 */ 	lw	$s1,0x24($sp)
/*  f157bd4:	8fb20028 */ 	lw	$s2,0x28($sp)
/*  f157bd8:	8fb3002c */ 	lw	$s3,0x2c($sp)
/*  f157bdc:	8fb40030 */ 	lw	$s4,0x30($sp)
/*  f157be0:	8fb50034 */ 	lw	$s5,0x34($sp)
/*  f157be4:	8fb60038 */ 	lw	$s6,0x38($sp)
/*  f157be8:	8fb7003c */ 	lw	$s7,0x3c($sp)
/*  f157bec:	8fbe0040 */ 	lw	$s8,0x40($sp)
/*  f157bf0:	03e00008 */ 	jr	$ra
/*  f157bf4:	27bd00b8 */ 	addiu	$sp,$sp,0xb8
);
#else
void textWrap(s32 wrapwidth, char *src, char *dst, struct fontchar *chars, struct font *font)
{
#if VERSION >= VERSION_JPN_FINAL
	// JPN mismatch: Regalloc for sp94
	s32 curlinewidth = 0; // b4
	bool itfits;
	s32 i = 0;
	s32 wordwidth; // a8
	s32 wordheight = 0; // a4
	bool more = true; // a0
	bool isvalidchar;
	s32 wordlen;
	s32 sp94 = 0;
	char curword[32]; // 74
	s32 isspace; // 70
	u32 stack;

	while (more == true) {
		wordlen = 0;
		wordwidth = 0;
		sp94 = 0;
		isspace = false;
		isvalidchar = true;

		while (isvalidchar) {
			u16 codepoint = 0;
			bool multibyte = false;
			s32 charwidth = 0;
			s32 c1 = src[0];
			s32 c2;
			u8 c;

			if (c1 < 0x80) {
				c = c1;
			} else {
				c2 = src[1];
				multibyte = true;
				codepoint = ((c1 & 0x7f) << 7) | (c2 & 0x7f);
				c = func0f154784jf(codepoint);

				if (c2 == '\0') {
					multibyte = false;
				}
			}

			if (c == '\0') {
				isvalidchar = false;

				if (multibyte) {
					if (codepoint >= 0x34 && codepoint < 0x4e) {
						isvalidchar = true;
					}

					if (codepoint >= 0xaf && codepoint < 0xb8) {
						isvalidchar = true;
					}

					if (codepoint == 0x104) {
						isvalidchar = true;
					}

					if (codepoint == 0x105) {
						isvalidchar = true;
					}

					if (wordlen == 0) {
						isvalidchar = true;
					}

					charwidth = 11;
				}
			} else {
				if (c > ' ') {
					isvalidchar = true;
				} else {
					isvalidchar = false;
				}

				charwidth = chars[c - 0x21].width;

				if (!isvalidchar && wordlen == 0) {
					if (c != '\0' && c != ' ') {
						isvalidchar = true;
					}

					if (c == ' ') {
						isspace = true;
					}
				}

				if (wordlen != 0
						&& ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
						&& curword[wordlen - 1] >= 0x80) {
					u8 c1 = curword[wordlen - 2];
					u8 c2 = curword[wordlen - 1];
					u16 codepoint = ((c1 & 0x7f) << 7) | (c2 & 0x7f);

					if (func0f154784jf(codepoint) == '\0') {
						isvalidchar = false;
					}
				}
			}

			if (isvalidchar) {
				curword[wordlen] = *src;
				src++;
				wordlen++;

				if (multibyte) {
					curword[wordlen] = *src;
					src++;
					wordlen++;
				}

				sp94 += charwidth;
			}
		}

		curword[wordlen] = '\0';

		textMeasure(&wordheight, &wordwidth, curword, chars, font, 0);

		curlinewidth += wordwidth;

		if (curlinewidth <= wrapwidth) {
			itfits = true;
		} else {
			itfits = false;
		}

		if (*src == '\n') {
			src++;

			if (!itfits) {
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			curlinewidth = 0;

			*dst = '\n';
			dst++;
		} else if (*src == '\0') {
			more = false;

			if (!itfits) {
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			src++;

			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			*dst = '\0';
		} else {
			if (!itfits) {
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}

				curlinewidth = wordwidth + g_WrapIndentCount * SPACE_WIDTH;
			}

			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			if (*src == ' ' || isspace) {
				if (*src >= 0x80) {
					src++;
				}

				src++;

				curlinewidth += SPACE_WIDTH;

				*dst = ' ';
				dst++;
			}
		}
	}
#else
	s32 curlinewidth = 0;
	bool itfits;
	s32 wordlen;
	s32 wordwidth;
	s32 wordheight = 0;
	bool more = true;
	s32 v1;
	s32 i;
	u32 stack;
	char curword[32];

	while (more == true) {
		// Load the next word
		wordwidth = 0;
		wordlen = 0;
		v1 = 0;

		while (*src > ' ') {
			curword[wordlen] = *src;
			v1 += chars[*src - 0x21].width;
			src++;
			wordlen++;

#if VERSION >= VERSION_PAL_FINAL
			if (g_Jpn)
#endif
			{
				if (curword[wordlen - 1] >= 0x80) {
					curword[wordlen] = *src;
					v1 += chars[*src - 0x21].width;
					src++;
					wordlen++;
				}
			}
		}

		curword[wordlen] = '\0';

		textMeasure(&wordheight, &wordwidth, curword, chars, font, 0);

		curlinewidth += wordwidth;

		if (curlinewidth <= wrapwidth) {
			itfits = true;
		} else {
			itfits = false;
		}

		if (*src == '\n') {
			// Write a new line and indent
			if (!itfits) {
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			curlinewidth = 0;

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the original new line that was in src
			*dst = '\n';
			dst++;
		} else if (*src == ' ') {
			if (!itfits) {
				// Write a new line and indent
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}

				curlinewidth = g_WrapIndentCount * SPACE_WIDTH + wordwidth;
			}

			curlinewidth += SPACE_WIDTH;

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the trailing space
			*dst = ' ';
			dst++;
		} else if (*src == '\0') {
			more = false;

			if (!itfits) {
				// Write a new line and indent
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the null terminator
			*dst = '\0';
		}

		src++;
	}
#endif
}
#endif
