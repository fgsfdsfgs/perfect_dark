#include <ultra64.h>
#include "constants.h"
#include "game/player.h"
#include "game/healthbar.h"
#include "game/game_1531a0.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "bss.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#endif

struct marker {
	f32 x1;
	f32 y1;
	f32 x2;
	f32 y2;
	f32 frac;
};

s32 healthbarMaybeInsertMarker(struct marker *markers, s32 *indexes, s32 maxlen, f32 fillfrac)
{
	s32 len = 0;
	s32 i;
	s32 j;

	if (fillfrac < 0.0f) {
		fillfrac = 0.0f;
	}

	if (fillfrac > 1.0f) {
		fillfrac = 1.0f;
	}

	for (i = 0; i < maxlen; i++) {
		if (indexes[i] > len) {
			len = indexes[i];
		}
	}

	len++;

	for (i = 0; i < len; i++) {
		s32 index1 = indexes[i];
		s32 index2 = indexes[i + 1];
		f32 x1;
		f32 x2;
		f32 y1;
		f32 y2;
		f32 tmp1;
		f32 tmp2;

		if (markers[index1].frac < fillfrac && markers[index2].frac > fillfrac) {
			x1 = markers[index2].x1 - markers[index1].x1;
			y1 = markers[index2].y1 - markers[index1].y1;
			x2 = markers[index2].x2 - markers[index1].x2;
			y2 = markers[index2].y2 - markers[index1].y2;
			tmp1 = markers[index2].frac - markers[index1].frac;
			tmp2 = fillfrac - markers[index1].frac;

			markers[len].x1 = markers[index1].x1 + tmp2 / tmp1 * x1;
			markers[len].y1 = markers[index1].y1 + tmp2 / tmp1 * y1;
			markers[len].x2 = markers[index1].x2 + tmp2 / tmp1 * x2;
			markers[len].y2 = markers[index1].y2 + tmp2 / tmp1 * y2;
			markers[len].frac = fillfrac;

			for (j = len - 1; j > i; j--) {
				indexes[j + 1] = indexes[j];
			}

			indexes[i + 1] = len;
			return 1;
		}
	}

	return 0;
}

u32 healthbarChooseColour(u32 fillcol, u32 bgcol, f32 fillexcfade, f32 fillincfade, f32 frac)
{
	f32 mult;
	u32 r;
	u32 g;
	u32 b;
	u32 a;

	if (frac >= fillincfade) {
		return bgcol;
	}

	if (frac <= fillexcfade) {
		return fillcol;
	}

	mult = (frac - fillexcfade) / (fillincfade - fillexcfade);

	r = (fillcol >> 24) & 0xff;
	g = (fillcol >> 16) & 0xff;
	b = (fillcol >> 8) & 0xff;
	a = fillcol & 0xff;

	r += (s32) ((s32) (((bgcol >> 24) & 0xff) - r) * mult);
	g += (s32) ((s32) (((bgcol >> 16) & 0xff) - g) * mult);
	b += (s32) ((s32) (((bgcol >> 8) & 0xff) - b) * mult);
	a += (s32) ((s32) ((bgcol & 0xff) - a) * mult);

	return r << 24 | g << 16 | b << 8 | a;
}

/**
 * Terminology used in this function:
 *
 * armour - is the green part of the main health bar
 * trauma - is the red part of the main health bar
 * shield - is the green shield bar underneath the health bar
 * health - refers to armour + trauma collectively
 *
 * The health bar determines what shapes to draw by utilising markers. A marker
 * is a pair of vertices at a certain fraction (0.0 to 1.0) across a healthbar
 * element. Each marker's vertices are used to allocate proper graphics vertices
 * and are connected in order.
 *
 * For example, the green health part of the bar has a single marker on the left
 * at frac 0, then the next marker on the right side where the curve starts at
 * 0.9, then several markers in close proximity between 0.9 and 1.0 to implement
 * the rounded end.
 *
 * If the player's health falls between two markers, up to two two additional
 * markers will be created dynamically for colouring purposes. The coordinates
 * of these dynamic markers are tweened from the two neighbours. There are two
 * of these because the colour change uses a fade rather than a hard edge.
 */
Gfx *healthbarDraw(Gfx *gdl, struct chrdata *chr, s32 offyarg, f32 heightfracarg)
{
	struct marker shieldmarkers[12];
	struct marker armourmarkers[8];
	struct marker traumamarkers[8];
	s32 shieldmarkerindexes[12];
	s32 armourmarkerindexes[8];
	s32 traumamarkerindexes[8];
	Vtx *vertices;
	Col *colours;
	Col *shieldcolours;
	Col *armourcolours;
	Col *traumacolours;
	Vtx *shieldvertices;
	Vtx *armourvertices;
	Vtx *traumavertices;
	f32 shieldfillincfade;
	f32 shieldfillexcfade;
	f32 armourfillincfade;
	f32 armourfillexcfade;
	f32 traumafillincfade;
	f32 traumafillexcfade;
	struct marker *marker;
	s32 numshieldmarkers;
	s32 numtraumamarkers;
	s32 numarmourmarkers;
	f32 shieldfrac;
	f32 armourfrac;
	f32 traumafrac;
	f32 healthfrac;
	f32 heightfrac;
	f32 fovoffset;
	u32 colour;
	s32 index;
	s32 i;
	s32 numverts;
	s32 numvertsremaining;
	f32 len0 = 0.0f;
	s32 coloursize;

	static s32 radmax = 30; // outer radius of the shield
	static s32 radmed = 18; // inner radius of the shield
	static s32 radmin = 12; // radius of the health bar
	static s32 len1 = 170; // x pos of right side radius centre
	static s32 len2 = 47;  // x pos of the left side of armour
	static s32 len3 = 40;  // x pos of the right side of trauma (left side is 0)
	static u32 shieldcol = 0x10500090;
	static u32 armourcol = 0x00c00060;
	static u32 traumacol = 0xff000060;
	static u32 bgcol = 0x00000080;
	static s32 offx = -85;
	static s32 offy = -185;
	static s32 shieldfade = 100;
	static s32 armourfade = 100;
	static s32 traumafade = 200;
	static s32 shielddir = 1;
	static s32 underleft = 95;
	static s32 undertop = 18;
	static s32 underright = 225;
	static s32 underbottom = 46;
	static u32 undercol = 0x00000000;

	vertices = gfxAllocateVertices(56);
	colours = gfxAllocateColours(56);

	// 12 markers (24 vertices) for shield
	// 8 markers (16 vertices) for armour
	// 8 markers (16 vertices) for trauma
	shieldvertices = vertices;
	armourvertices = vertices + 24;
	traumavertices = vertices + 40;

	shieldcolours = colours;
	armourcolours = colours + 24;
	traumacolours = colours + 40;

	mainOverrideVariable("radmax", &radmax);
	mainOverrideVariable("radmed", &radmed);
	mainOverrideVariable("radmin", &radmin);
	mainOverrideVariable("len1", &len1);
	mainOverrideVariable("len2", &len2);
	mainOverrideVariable("len3", &len3);
	mainOverrideVariable("offx", &offx);
	mainOverrideVariable("offy", &offy);
	mainOverrideVariable("shieldcol", &shieldcol);
	mainOverrideVariable("armourcol", &armourcol);
	mainOverrideVariable("traumacol", &traumacol);
	mainOverrideVariable("bgcol", &bgcol);
	mainOverrideVariable("shieldfade", &shieldfade);
	mainOverrideVariable("armourfade", &armourfade);
	mainOverrideVariable("traumafade", &traumafade);
	mainOverrideVariable("shielddir", &shielddir);
	mainOverrideVariable("underleft", &underleft);
	mainOverrideVariable("undertop", &undertop);
	mainOverrideVariable("underright", &underright);
	mainOverrideVariable("underbottom", &underbottom);
	mainOverrideVariable("undercol", &undercol);

	if (chr == NULL || offyarg <= 0 || heightfracarg <= 0.0f) {
		// Use the player's health
		shieldfrac = g_Vars.currentplayer->apparentarmour;
		armourfrac = (g_Vars.currentplayer->apparenthealth - 0.25f) / 0.75f;
		traumafrac = (0.25f - g_Vars.currentplayer->apparenthealth) * 4.0f;
		heightfrac = playerGetHealthBarHeightFrac();
		fovoffset = 0;
#ifndef PLATFORM_N64 // dynamically adjust health bar depending on fov
		if (videoGetPlayerFovY() > 60.0f) {
			fovoffset = (videoGetPlayerFovY() - 60.0f) * -2.4f;
		} else if (videoGetPlayerFovY() < 60.0f) {
			fovoffset = (60.0f - videoGetPlayerFovY()) * 1.8f;
		}
#endif
	} else {
		// Use the given chr's health
		healthfrac = (chr->maxdamage - chr->damage) / chr->maxdamage;
		shieldfrac = chr->cshield * 0.125f;
		armourfrac = (healthfrac - 0.25f) / 0.75f;
		traumafrac = (0.25f - healthfrac) * 4.0f;
		heightfrac = heightfracarg;
		fovoffset = 0;
		offy += offyarg;
	}

	armourfrac = armourfrac > 0 ? armourfrac : 0;
	traumafrac = traumafrac > 0 ? traumafrac : 0;

	for (i = 0; i < 12; i++) {
		shieldmarkerindexes[i] = i < 10 ? i : -1;

		if (i < 8) {
			armourmarkerindexes[i] = i < 6 ? i : -1;
			traumamarkerindexes[i] = i < 6 ? i : -1;
		}
	}

	/**
	 * Shield markers
	 *
	 * This is actually defined and drawn right to left rather than left to
	 * right, but is coloured the other way around using the shielddir variable.
	 *
	 * len0 - is the X position of the left side, and is defined as 0.
	 *        A variable must be used in some places for this to match, and a
	 *        literal 0 in others. It's even required in a Y value for a match.
	 * len1 - is the X position of the right side.
	 */
	shieldmarkers[0].x1 = len1 + radmax * 1.08f;
	shieldmarkers[0].y1 = 0 + fovoffset;
	shieldmarkers[0].x2 = len1 + (f32)radmed;
	shieldmarkers[0].y2 = 0 + fovoffset;
	shieldmarkers[0].frac = 0;

	shieldmarkers[1].x1 = len1 + radmax * 0.924f * 1.04f;
	shieldmarkers[1].y1 = heightfrac * radmax * 0.383f + fovoffset;
	shieldmarkers[1].x2 = len1 + radmed * 0.924f;
	shieldmarkers[1].y2 = heightfrac * radmed * 0.383f + fovoffset;
	shieldmarkers[1].frac = 0.05f;

	shieldmarkers[2].x1 = len1 + radmax * 0.707f * 1.02f;
	shieldmarkers[2].y1 = heightfrac * radmax * 0.707f + fovoffset;
	shieldmarkers[2].x2 = len1 + radmed * 0.707f;
	shieldmarkers[2].y2 = heightfrac * radmed * 0.707f + fovoffset;
	shieldmarkers[2].frac = 0.1f;

	shieldmarkers[3].x1 = len1 + radmax * 0.383f;
	shieldmarkers[3].y1 = heightfrac * radmax * 0.924f + fovoffset;
	shieldmarkers[3].x2 = len1 + radmed * 0.383f;
	shieldmarkers[3].y2 = heightfrac * radmed * 0.924f + fovoffset;
	shieldmarkers[3].frac = 0.15f;

	shieldmarkers[4].x1 = len1;
	shieldmarkers[4].y1 = heightfrac * radmax + fovoffset;
	shieldmarkers[4].x2 = len1;
	shieldmarkers[4].y2 = heightfrac * radmed + fovoffset;
	shieldmarkers[4].frac = 0.2f;

	shieldmarkers[5].x1 = len0;
	shieldmarkers[5].y1 = heightfrac * radmax + fovoffset;
	shieldmarkers[5].x2 = len0;
	shieldmarkers[5].y2 = heightfrac * radmed + fovoffset;
	shieldmarkers[5].frac = 0.8f;

	shieldmarkers[6].x1 = len0 - radmax * 0.383f;
	shieldmarkers[6].y1 = heightfrac * radmax * 0.924f + fovoffset;
	shieldmarkers[6].x2 = len0 - radmed * 0.383f;
	shieldmarkers[6].y2 = heightfrac * radmed * 0.924f + fovoffset;
	shieldmarkers[6].frac = 0.85f;

	shieldmarkers[7].x1 = 0.0f - radmax * 0.707f * 1.02f;
	shieldmarkers[7].y1 = heightfrac * radmax * 0.707f + fovoffset;
	shieldmarkers[7].x2 = 0.0f - radmed * 0.707f;
	shieldmarkers[7].y2 = heightfrac * radmed * 0.707f + fovoffset;
	shieldmarkers[7].frac = 0.9f;

	shieldmarkers[8].x1 = len0 - radmax * 0.924f * 1.04f;
	shieldmarkers[8].y1 = heightfrac * radmax * 0.383f + fovoffset;
	shieldmarkers[8].x2 = len0 - radmed * 0.924f;
	shieldmarkers[8].y2 = heightfrac * radmed * 0.383f + fovoffset;
	shieldmarkers[8].frac = 0.95f;

	shieldmarkers[9].x1 = len0 - radmax * 1.08f;
	shieldmarkers[9].y1 = len0 + fovoffset;
	shieldmarkers[9].x2 = len0 - radmed;
	shieldmarkers[9].y2 = len0 + fovoffset;
	shieldmarkers[9].frac = 1;

	/**
	 * Armour markers
	 *
	 * Drawn left to right as expected.
	 *
	 * len2 - is the X position of the left side.
	 * len1 - is the X position of the right side.
	 */
	armourmarkers[0].x1 = len2;
	armourmarkers[0].y1 = heightfrac * radmin + fovoffset;
	armourmarkers[0].x2 = len2;
	armourmarkers[0].y2 = -heightfrac * radmin + fovoffset;
	armourmarkers[0].frac = 0;

	armourmarkers[1].x1 = len1;
	armourmarkers[1].y1 = heightfrac * radmin + fovoffset;
	armourmarkers[1].x2 = len1;
	armourmarkers[1].y2 = -heightfrac * radmin + fovoffset;
	armourmarkers[1].frac = 0.9f;

	armourmarkers[2].x1 = len1 + radmin * 0.342f;
	armourmarkers[2].y1 = heightfrac * radmin * 0.94f + fovoffset;
	armourmarkers[2].x2 = len1 + radmin * 0.342f;
	armourmarkers[2].y2 = -heightfrac * radmin * 0.94f + fovoffset;
	armourmarkers[2].frac = 0.94f;

	armourmarkers[3].x1 = len1 + radmin * 0.643f;
	armourmarkers[3].y1 = heightfrac * radmin * 0.766f + fovoffset;
	armourmarkers[3].x2 = len1 + radmin * 0.643f;
	armourmarkers[3].y2 = -heightfrac * radmin * 0.766f + fovoffset;
	armourmarkers[3].frac = 0.97f;

	armourmarkers[4].x1 = len1 + radmin * 0.866f;
	armourmarkers[4].y1 = heightfrac * radmin * 0.5f + fovoffset;
	armourmarkers[4].x2 = len1 + radmin * 0.866f;
	armourmarkers[4].y2 = -heightfrac * radmin * 0.5f + fovoffset;
	armourmarkers[4].frac = 0.99f;

	armourmarkers[5].x1 = len1 + radmin * 0.985f;
	armourmarkers[5].y1 = heightfrac * radmin * 0.174f + fovoffset;
	armourmarkers[5].x2 = len1 + radmin * 0.985f;
	armourmarkers[5].y2 = -heightfrac * radmin * 0.174f + fovoffset;
	armourmarkers[5].frac = 1;

	/**
	 * Trauma markers
	 *
	 * Drawn right to left because it fills right to left.
	 *
	 * len0 - is the X position of the left side. Like the shield markers, this
	 *        is defined as 0 and must only be used sometimes.
	 * len3 - is the X position of the right side.
	 *
	 * Note that the trailing 0 on 0.7070f is absolutely required for a match.
	 *
	 * Fun fact: There are also trauma markers on my brain after matching this
	 * code.
	 */
	traumamarkers[0].x1 = len3;
	traumamarkers[0].y1 = heightfrac * radmin + fovoffset;
	traumamarkers[0].x2 = len3;
	traumamarkers[0].y2 = -heightfrac * radmin + fovoffset;
	traumamarkers[0].frac = 0;

	traumamarkers[1].x1 = 0.0f;
	traumamarkers[1].y1 = heightfrac * radmin + fovoffset;
	traumamarkers[1].x2 = 0.0f;
	traumamarkers[1].y2 = -heightfrac * radmin + fovoffset;
	traumamarkers[1].frac = 0.8f;

	traumamarkers[2].x1 = 0.0f - radmin * 0.383f;
	traumamarkers[2].y1 = heightfrac * radmin * 0.924f + fovoffset;
	traumamarkers[2].x2 = 0.0f - radmin * 0.383f;
	traumamarkers[2].y2 = -heightfrac * radmin * 0.924f + fovoffset;
	traumamarkers[2].frac = 0.85f;

	traumamarkers[3].x1 = 0.0f - radmin * 0.707f;
	traumamarkers[3].y1 = heightfrac * radmin * 0.7070f + fovoffset;
	traumamarkers[3].x2 = 0.0f - radmin * 0.707f;
	traumamarkers[3].y2 = -heightfrac * radmin * 0.7070f + fovoffset;
	traumamarkers[3].frac = 0.9f;

	traumamarkers[4].x1 = len0 - radmin * 0.924f;
	traumamarkers[4].y1 = heightfrac * radmin * 0.383f + fovoffset;
	traumamarkers[4].x2 = len0 - radmin * 0.924f;
	traumamarkers[4].y2 = -heightfrac * radmin * 0.383f + fovoffset;
	traumamarkers[4].frac = 0.95f;

	traumamarkers[5].x1 = len0 - radmin;
	traumamarkers[5].y1 = 0.0f + fovoffset;
	traumamarkers[5].x2 = len0 - radmin;
	traumamarkers[5].y2 = 0.0f + fovoffset;
	traumamarkers[5].frac = 1;

	// Build shield graphics data
	if (shielddir != 0) {
		shieldfrac = 1.0f - shieldfrac;
	}

	shieldfillincfade = shieldfade * 0.001f;
	shieldfillincfade = (1.0f + shieldfillincfade) * shieldfrac;
	shieldfillexcfade = shieldfillincfade - shieldfade * 0.001f;

	numshieldmarkers = 10;
	numshieldmarkers += healthbarMaybeInsertMarker(shieldmarkers, shieldmarkerindexes, ARRAYCOUNT(shieldmarkers), shieldfillexcfade);
	numshieldmarkers += healthbarMaybeInsertMarker(shieldmarkers, shieldmarkerindexes, ARRAYCOUNT(shieldmarkers), shieldfillincfade);

	for (i = 0; i < numshieldmarkers; i++) {
		index = shieldmarkerindexes[i];
		marker = &shieldmarkers[index];

		if (shielddir != 0) {
			colour = healthbarChooseColour(bgcol, shieldcol, shieldfillexcfade, shieldfillincfade, marker->frac);
		} else {
			colour = healthbarChooseColour(shieldcol, bgcol, shieldfillexcfade, shieldfillincfade, marker->frac);
		}

		shieldvertices->x = (s32)marker->x1 + offx;
		shieldvertices->y = 0;
		shieldvertices->z = (s32)marker->y1 + offy + fovoffset;
		shieldvertices->colour = (i + i) << 2;
		shieldvertices++;

		shieldcolours->word = PD_BE32(colour);
		shieldcolours++;

		shieldvertices->x = (s32)marker->x2 + offx;
		shieldvertices->y = 0;
		shieldvertices->z = (s32)marker->y2 + offy + fovoffset;
		shieldvertices->colour = (i + i + 1) << 2;
		shieldvertices++;

		shieldcolours->word = PD_BE32(colour);
		shieldcolours++;
	}

	// Build armour graphics data
	armourfillincfade = armourfade * 0.001f;
	armourfillincfade = (1.0f + armourfillincfade) * armourfrac;
	armourfillexcfade = armourfillincfade - armourfade * 0.001f;

	numarmourmarkers = 6;
	numarmourmarkers += healthbarMaybeInsertMarker(armourmarkers, armourmarkerindexes, ARRAYCOUNT(armourmarkers), armourfillexcfade);
	numarmourmarkers += healthbarMaybeInsertMarker(armourmarkers, armourmarkerindexes, ARRAYCOUNT(armourmarkers), armourfillincfade);

	for (i = 0; i < numarmourmarkers; i++) {
		index = armourmarkerindexes[i];
		marker = &armourmarkers[index];

		colour = healthbarChooseColour(armourcol, bgcol, armourfillexcfade, armourfillincfade, marker->frac);

		armourvertices->x = (s32)marker->x1 + offx;
		armourvertices->y = 0;
		armourvertices->z = (s32)marker->y1 + offy + fovoffset;
		armourvertices->colour = (i + i) << 2;
		armourvertices++;

		armourcolours->word = PD_BE32(colour);
		armourcolours++;

		armourvertices->x = (s32)marker->x2 + offx;
		armourvertices->y = 0;
		armourvertices->z = (s32)marker->y2 + offy + fovoffset;
		armourvertices->colour = (i + i + 1) << 2;
		armourvertices++;

		armourcolours->word = PD_BE32(colour);
		armourcolours++;
	}

	// Build trauma graphics data
	traumafillincfade = traumafade * 0.001f;
	traumafillincfade = (1.0f + traumafillincfade) * traumafrac;
	traumafillexcfade = traumafillincfade - traumafade * 0.001f;

	numtraumamarkers = 6;
	numtraumamarkers += healthbarMaybeInsertMarker(traumamarkers, traumamarkerindexes, ARRAYCOUNT(traumamarkers), traumafillexcfade);
	numtraumamarkers += healthbarMaybeInsertMarker(traumamarkers, traumamarkerindexes, ARRAYCOUNT(traumamarkers), traumafillincfade);

	for (i = 0; i < numtraumamarkers; i++) {
		index = traumamarkerindexes[i];
		marker = &traumamarkers[index];

		colour = healthbarChooseColour(traumacol, bgcol, traumafillexcfade, traumafillincfade, marker->frac);

		traumavertices->x = (s32)marker->x1 + offx;
		traumavertices->y = 0;
		traumavertices->z = (s32)marker->y1 + offy + fovoffset;
		traumavertices->colour = (i + i) << 2;
		traumavertices++;

		traumacolours->word = PD_BE32(colour);
		traumacolours++;

		traumavertices->x = (s32)marker->x2 + offx;
		traumavertices->y = 0;
		traumavertices->z = (s32)marker->y2 + offy + fovoffset;
		traumavertices->colour = (i + i + 1) << 2;
		traumavertices++;

		traumacolours->word = PD_BE32(colour);
		traumacolours++;
	}

	gdl = text0f153628(gdl);
	gdl = text0f153a34(gdl, underleft, undertop, underright, underbottom, undercol);
	gdl = text0f153780(gdl);

	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);

	// Same as gSPColor but casts the sizeof to an s32, which is required for a match.
#define gSPColor2(pkt, v, n) \
	gDma1p(pkt, G_COL, v, (s32)sizeof(u32)*(n),((n)-1)<<2)

	// Draw shield
	// There's between 20 and 24 colours and vertices to load here.
	// The colours are loaded all at once, but the vertices are split into two.
	numverts = numshieldmarkers * 2;

	gSPColor2(gdl++, osVirtualToPhysical(colours), numverts);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 14, 0);

	gSPTri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gSPTri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);
	gSPTri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

	// numvertsremaining will be 8-12
	numvertsremaining = numverts - 12U;

	// This is a macro expansion of gSPVertex but cases the sizeof to an s32,
	// and separates numvertsremaining and numverts - 12.
	// Both are required for a match.
	gDma1p(gdl++, G_VTX,
			osVirtualToPhysical(vertices + 12),
			numvertsremaining * (s32) sizeof(Vtx),
			(numverts - 12 - 1) << 4);

	gSPTri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);

	if (numvertsremaining < 10) {
		gSPTri2(gdl++, 4, 5, 6, 5, 6, 7);
	} else {
		gSPTri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

		if (numvertsremaining >= 12) {
			gSPTri2(gdl++, 8, 9, 10, 9, 10, 11);
		}
	}

	// Draw armour
	numverts = numarmourmarkers * 2;
	coloursize = numverts * 4;

	gSPColor2(gdl++, osVirtualToPhysical(colours + 24), numverts);
	gSPVertex(gdl++, osVirtualToPhysical(vertices + 24), numverts, 0);

	gSPTri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gSPTri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

	if (coloursize < 56) {
		gSPTri2(gdl++, 8, 9, 10, 9, 10, 11);
	} else {
		gSPTri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

		if (coloursize >= 64) {
			gSPTri2(gdl++, 12, 13, 14, 13, 14, 15);
		}
	}

	// Draw trauma
	numverts = numtraumamarkers * 2;
	coloursize = numverts * 4;

	gSPColor2(gdl++, osVirtualToPhysical(colours + 40), numverts);
	gSPVertex(gdl++, osVirtualToPhysical(vertices + 40), numverts, 0);

	gSPTri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gSPTri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

	if (coloursize < 56) {
		gSPTri2(gdl++, 8, 9, 10, 9, 10, 11);
	} else {
		gSPTri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

		if (coloursize >= 64) {
			gSPTri2(gdl++, 12, 13, 14, 13, 14, 15);
		}
	}

	return gdl;
}
