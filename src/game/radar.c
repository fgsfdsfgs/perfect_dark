#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/atan2f.h"
#include "game/game_0b2150.h"
#include "game/tex.h"
#include "game/game_152fa0.h"
#include "game/game_1531a0.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/mtx.h"
#include "data.h"
#include "gbiex.h"
#include "types.h"

u32 g_RadarX;
u32 g_RadarY;

bool g_RadarYIndicatorsEnabled = true;

u32 g_TeamColours[] = {
	0xff000000, // Red
	0xffff0000, // Yellow
	0x0000ff00, // Blue
	0xff00ff00, // Magenta
	0x00ffff00, // Cyan
	0xff885500, // Orange
	0x8800ff00, // Pink
	0x88445500, // Brown
};

u32 var80087ce4[] = {
	0xf801f801,
	0xffc1ffc1,
	0x003f003f,
	0xf83ff83f,
	0x07ff07ff,
	0xfc55fc55,
	0xfc63fc63,
	0x8a158a15,
};

void radarSetYIndicatorsEnabled(bool enable)
{
	g_RadarYIndicatorsEnabled = enable;
}

Gfx *radarRenderBackground(Gfx *gdl, struct textureconfig *tconfig, s32 arg2, s32 arg3, s32 arg4)
{
	f32 spb0[2];
	f32 spa8[2];

	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
	gDPSetPrimColorViaWord(gdl++, 0, 0, 0x00000000);

	gDPFillRectangle(gdl++,
			arg2 * g_ScaleX,
			arg3,
			(arg2 + tconfig->width) * g_ScaleX,
			arg3 + tconfig->width);

	spb0[0] = arg2 * g_ScaleX;
	spb0[1] = arg3;
	spa8[0] = arg4 * g_ScaleX;
	spa8[1] = arg4;

	texSelect(&gdl, tconfig, 2, 0, 0, 1, NULL);
	func0f0b278c(&gdl, spb0, spa8, tconfig->width, tconfig->height,
			0, 0, 1, 0, 0xff, 0, 40, tconfig->level > 0, 0);

	gDPPipeSync(gdl++);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_LOD);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);

	return gdl;
}

s32 radarGetTeamIndex(s32 team)
{
	s32 index = 0;

	if (team & 1) {
		index = 0;
	} else if (team & 0x02) {
		index = 1;
	} else if (team & 0x04) {
		index = 2;
	} else if (team & 0x08) {
		index = 3;
	} else if (team & 0x10) {
		index = 4;
	} else if (team & 0x20) {
		index = 5;
	} else if (team & 0x40) {
		index = 6;
	} else if (team & 0x80) {
		index = 7;
	}

	return index;
}

Gfx *radarDrawDot(Gfx *gdl, struct prop *prop, struct coord *dist, u32 colour1, u32 colour2, bool swapcolours)
{
	s32 x;
	s32 y;
	u32 shiftamount;
	f32 sqdist;
	f32 spcc;

	osSyncPrintf("RadarDrawDot : Prop=%x", prop);

	spcc = (atan2f(dist->x, dist->z) * 180.0f) / M_PI + g_Vars.currentplayer->vv_theta + 180.0f;
	sqdist = sqrtf(dist->z * dist->z + dist->x * dist->x) * (1.0f / 250.0f);

	if (sqdist < 16.0f) {
		shiftamount = 0;
	} else {
		sqdist = 16.0f;
		shiftamount = 1;
	}

	x = g_RadarX + (s32)(sinf(spcc * 0.017453292384744f) * sqdist);
	y = g_RadarY + (s32)PALUPF(cosf(spcc * 0.017453292384744f) * sqdist);

	if (swapcolours) {
		if (prop == g_Vars.currentplayer->prop) {
			// Box
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 2, y + 2, x + 1, y + 3);
			gDPFillRectangleScaled(gdl++, x - 3, y - 1, x + 2, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 1, y + 1, x + 0, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y + 0, x + 1, y + 1);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = text0f153838(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y < -250) {
			// Up triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 3, y - 1, x + 2, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 2, y + 0, x + 1, y + 1);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = text0f153838(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y > 250) {
			// Down triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 3, y - 2, x + 2, y + 1);
			gDPFillRectangleScaled(gdl++, x - 2, y + 1, x + 1, y + 2);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 1, x + 1, y + 0);
			gDPFillRectangleScaled(gdl++, x - 1, y + 0, x + 0, y + 1);
			gdl = text0f153838(gdl);
		} else {
			// Dot
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 2, y + 2);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 1, y + 1);
			gdl = text0f153838(gdl);
		}
	} else {
		if (prop == g_Vars.currentplayer->prop) {
			// Box
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 2, y + 2, x + 1, y + 3);
			gDPFillRectangleScaled(gdl++, x - 3, y - 1, x + 2, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 1, y + 1, x + 0, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y + 0, x + 1, y + 1);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = text0f153838(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y < -250) {
			// Up triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 3, y - 1, x + 2, y + 2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 2, y + 0, x + 1, y + 1);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = text0f153838(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y > 250) {
			// Down triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 3, y - 2, x + 2, y + 1);
			gDPFillRectangleScaled(gdl++, x - 2, y + 1, x + 1, y + 2);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 2, y - 1, x + 1, y + 0);
			gDPFillRectangleScaled(gdl++, x - 1, y + 0, x + 0, y + 1);
			gdl = text0f153838(gdl);
		} else {
			// Dot
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gDPFillRectangleScaled(gdl++, x - 2, y - 2, x + 2, y + 2);
			gdl = text0f153838(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gDPFillRectangleScaled(gdl++, x - 1, y - 1, x + 1, y + 1);
			gdl = text0f153838(gdl);
		}
	}

	return gdl;
}

Gfx *radarRender(Gfx *gdl)
{
	s32 stack;
	s32 stack2;
	s32 playercount;
	s32 playernum;
	struct textureconfig *tconfig;
	struct coord pos;
	u32 colour;
	s32 i;

	tconfig = g_TexRadarConfigs;
	playernum = g_Vars.currentplayernum;
	playercount = PLAYERCOUNT();

	if (g_Vars.mplayerisrunning) {
		if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_NORADAR)) {
			return gdl;
		}

		if ((g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.displayoptions & 0x00000004) == 0) {
			return gdl;
		}
	} else if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_RTRACKER) == 0) {
		if (!g_MissionConfig.iscoop || !g_Vars.coopradaron) {
			return gdl;
		}
	}

	if (g_Vars.currentplayer->mpmenuon || g_Vars.currentplayer->isdead) {
		return gdl;
	}

#if PAL
	g_ScaleX = 1;
#else
	if (g_ViRes == VIRES_HI) {
		g_ScaleX = 2;
	} else {
		g_ScaleX = 1;
	}
#endif

	g_RadarX = (viGetViewLeft() + viGetViewWidth()) / g_ScaleX - 41;

	if (playercount == 2) {
		if (IS4MB() || optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			if (playernum == 0) {
				g_RadarX += 16;
			}

			if (IS4MB()) {
				g_RadarX -= 4;
			}
		} else {
			g_RadarX -= 7;
		}
	} else if (playercount >= 3) {
		if ((playernum & 1) == 0) {
			g_RadarX += 7;
		} else {
			g_RadarX -= 7;
		}
	}

	g_RadarY = viGetViewTop() + (PAL ? 29 : 26);

	if (playercount == 2) {
		if (IS4MB()) {
			g_RadarY -= 6;
		} else if (optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && playernum == 1) {
			g_RadarY -= 8;
		}
	} else if (playercount >= 3) {
		if (playernum >= 2) {
			g_RadarY -= 8;
		} else {
			g_RadarY -= 2;
		}
	} else {
		if (optionsGetEffectiveScreenSize() != SCREENSIZE_FULL) {
			g_RadarY -= 6;
		}
#ifndef PLATFORM_N64
		gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, G_ASPECT_RIGHT_EXT);
#endif
	}

	gdl = radarRenderBackground(gdl, tconfig, g_RadarX, g_RadarY, 0x10);
	gdl = func0f153134(gdl);

	// Draw dots for human players
	for (i = 0; i < playercount; i++) {
		if (i != playernum) {
			if (g_Vars.players[i]->isdead == false
					&& (g_Vars.players[i]->prop->chr->hidden & CHRHFLAG_CLOAKED) == 0
					&& scenarioRadarChr(&gdl, g_Vars.players[i]->prop) == false) {
				pos.x = g_Vars.players[i]->prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = g_Vars.players[i]->prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = g_Vars.players[i]->prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
					s32 index = g_PlayerConfigsArray[g_Vars.playerstats[i].mpindex].base.team;
					colour = g_TeamColours[index];
				} else {
					colour = 0x00ff0000;
				}

				gdl = radarDrawDot(gdl, g_Vars.players[i]->prop, &pos, colour, 0, 0);
			}
		}
	}

	// Draw dots for coop AI buddies
	if (!g_Vars.normmplayerisrunning && g_MissionConfig.iscoop) {
		for (i = 0; i < g_Vars.numaibuddies && i < ARRAYCOUNT(g_Vars.aibuddies); i++) {
			struct prop *prop = g_Vars.aibuddies[i];

			if (prop
					&& prop->type == PROPTYPE_CHR
					&& prop->chr
					&& prop->chr->actiontype != ACT_DIE
					&& prop->chr->actiontype != ACT_DEAD) {
				pos.x = prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				gdl = radarDrawDot(gdl, prop, &pos, 0x00ff0000, 0, 0);
			}
		}
	}

	// Draw dots for MP simulants
	if (g_Vars.normmplayerisrunning) {
		for (i = 0; i < g_BotCount; i++) {
			if (!chrIsDead(g_MpBotChrPtrs[i])
					&& (g_MpBotChrPtrs[i]->hidden & CHRHFLAG_CLOAKED) == 0
					&& scenarioRadarChr(&gdl, g_MpBotChrPtrs[i]->prop) == false) {
				pos.x = g_MpBotChrPtrs[i]->prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = g_MpBotChrPtrs[i]->prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = g_MpBotChrPtrs[i]->prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
					colour = g_TeamColours[radarGetTeamIndex(g_MpBotChrPtrs[i]->team)];
				} else {
					colour = 0x00ff0000;
				}

				gdl = radarDrawDot(gdl, g_MpBotChrPtrs[i]->prop, &pos, colour, 0, 0);
			}
		}
	}

	gdl = scenarioRadarExtra(gdl);

	// Draw dots for r-tracked props
	if (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_RTRACKER) {
		gdl = radarRenderRTrackedProps(gdl);
	}

	// Draw dot for the current player
	if (scenarioRadarChr(&gdl, g_Vars.currentplayer->prop) == false) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
			s32 index = g_PlayerConfigsArray[g_Vars.playerstats[playernum].mpindex].base.team;
			colour = g_TeamColours[index];
		} else {
			colour = 0x00ff0000;
		}

		gdl = radarDrawDot(gdl, g_Vars.currentplayer->prop, &pos, colour, 0, 0);
	}

#ifndef PLATFORM_N64
	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);
#endif

	g_ScaleX = 1;

	return gdl;
}

Gfx *radarRenderRTrackedProps(Gfx *gdl)
{
	struct prop *prop = g_Vars.activeprops;
	struct coord *playerpos = &g_Vars.currentplayer->prop->pos;
	struct defaultobj *obj;
	struct chrdata *chr;
	u32 stack1;
	struct coord dist1;
	u32 stack2;
	struct coord dist2;

	while (prop) {
		switch (prop->type) {
		case PROPTYPE_OBJ:
		case PROPTYPE_DOOR:
		case PROPTYPE_WEAPON:
			obj = prop->obj;

			if ((obj->flags3 & OBJFLAG3_RTRACKED_YELLOW) ||
					(cheatIsActive(CHEAT_RTRACKER) && (obj->flags3 & OBJFLAG3_RTRACKED_BLUE))) {
				dist1.x = prop->pos.x - playerpos->x;
				dist1.y = prop->pos.y - playerpos->y;
				dist1.z = prop->pos.z - playerpos->z;

				gdl = radarDrawDot(gdl, prop, &dist1,
						(obj->flags3 & OBJFLAG3_RTRACKED_YELLOW) ? 0xffff0000 : 0x0000ff00,
						0, 0);
			}
			break;
		case PROPTYPE_CHR:
			chr = prop->chr;

			if (chr && chr->rtracked
					&& chr->actiontype != ACT_DIE
					&& chr->actiontype != ACT_DEAD
					&& (chr->hidden & CHRHFLAG_CLOAKED) == 0) {
				dist2.x = prop->pos.x - playerpos->x;
				dist2.y = prop->pos.y - playerpos->y;
				dist2.z = prop->pos.z - playerpos->z;
				gdl = radarDrawDot(gdl, prop, &dist2, 0xff000000, 0, 0);
			}
			break;
		}

		prop = prop->next;
	}

	return gdl;
}
