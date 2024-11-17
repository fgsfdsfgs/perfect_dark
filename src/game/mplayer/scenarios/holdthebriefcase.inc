/**
 * Hold the Briefcase
 *
 * A briefcase is spawned into a random location in the arena. Players must pick
 * up the briefcase and then stay alive for as long as possible. The player
 * scores a point every 30 seconds while the briefcase is held.
 *
 * While holding the briefcase, the player cannot pick up shields. The move
 * slightly slower but still have full access to their weaponry.
 */

struct menuitem g_HtbOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_222, // "One-Hit Kills"
		MPOPTION_ONEHITKILLS,
		menuhandlerMpOneHitKills,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_223, // "Slow Motion"
		0,
		menuhandlerMpSlowMotion,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_224, // "Fast Movement"
		MPOPTION_FASTMOVEMENT,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_225, // "Display Team"
		MPOPTION_DISPLAYTEAM,
		menuhandlerMpDisplayTeam,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_226, // "No Radar"
		MPOPTION_NORADAR,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_227, // "No Auto-Aim"
		MPOPTION_NOAUTOAIM,
		menuhandlerMpCheckboxOption,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Start Armed",
		MPOPTION_SPAWNWITHWEAPON,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"No Drug Blur",
		MPOPTION_NODRUGBLUR,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_OPTIONS_257, // "Friendly Fire"
		MPOPTION_FRIENDLYFIRE,
		menuhandlerMpDisplayTeam,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"No Player on Radar",
		MPOPTION_NOPLAYERONRADAR,
		menuhandlerMpCheckboxOption,
	},
#endif
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_OPTIONS_493, // "Kills Score"
		MPOPTION_KILLSSCORE,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_237, // "Highlight Briefcase"
		MPOPTION_HTB_HIGHLIGHTBRIEFCASE,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_238, // "Show on Radar"
		MPOPTION_HTB_SHOWONRADAR,
		menuhandlerMpCheckboxOption,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_239, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HtbOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_216, // "Briefcase Options"
	g_HtbOptionsMenuItems,
	mpOptionsMenuDialog,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct defaultobj *var800869ec = NULL;

struct weaponobj g_HtbTokenObj;

void htbInit(void)
{
	g_ScenarioData.htb.token = NULL;
}

void htbAddPad(s16 padnum)
{
	struct scenariodata_htb *data = &g_ScenarioData.htb;

#if VERSION >= VERSION_NTSC_1_0
	if (data->nextindex < ARRAYCOUNT(data->padnums))
#endif
	{
		osSyncPrintf("CaptureTheBriefcaseAddBankPad -> Adding New Pad %d  - Pad Id = %d-> Saving Pad\n", data->nextindex, padnum);

		data->padnums[data->nextindex] = padnum;
		data->nextindex++;
	}
}

s32 htbNumProps(void)
{
	return 1;
}

void htbRemoveAmmoCrateAtPad(s16 padnum)
{
	struct prop *prop = g_Vars.activeprops;

	while (prop) {
		if (prop->type == PROPTYPE_OBJ) {
			struct defaultobj *obj = prop->obj;

			if (obj->pad == padnum
					&& (obj->type == OBJTYPE_AMMOCRATE || obj->type == OBJTYPE_MULTIAMMOCRATE)
					&& obj->modelnum == MODEL_MULTI_AMMO_CRATE) {
				obj->hidden |= OBJHFLAG_DELETING;
				obj->hidden2 &= ~OBJH2FLAG_CANREGEN;
				return;
			}
		}

		prop = prop->next;
	}
}

void htbReset(void)
{
	s32 i;

	g_ScenarioData.htb.nextindex = 0;

	for (i = 0; i < ARRAYCOUNT(g_ScenarioData.htb.padnums); i++) {
		g_ScenarioData.htb.padnums[i] = -1;
	}
}

void htbCreateToken(void)
{
	struct weaponobj template = {
		256,                    // extrascale
		0,                      // hidden2
		OBJTYPE_WEAPON,         // type
		MODEL_CHRBRIEFCASE,     // modelnum
		0,                      // pad
		OBJFLAG_FALL | OBJFLAG_INVINCIBLE | OBJFLAG_FORCENOBOUNCE,
		OBJFLAG2_IMMUNETOGUNFIRE | OBJFLAG2_IMMUNETOEXPLOSIONS,
		0,                      // flags3
		NULL,                   // prop
		NULL,                   // model
		1, 0, 0,                // realrot
		0, 1, 0,
		0, 0, 1,
		0,                      // hidden
		NULL,                   // geo
		NULL,                   // projectile
		0,                      // damage
		1000,                   // maxdamage
		0xff, 0xff, 0xff, 0x00, // shadecol
		0xff, 0xff, 0xff, 0x00, // nextcol
		0x0fff,                 // floorcol
		0,                      // tiles
		WEAPON_BRIEFCASE2,      // weaponnum
		0,                      // unk5d
		0,                      // unk5e
		FUNC_PRIMARY,           // gunfunc
		0,                      // fadeouttimer60
		-1,                     // dualweaponnum
		-1,                     // timer240
		NULL,                   // dualweapon
	};

	struct prop *prop = g_Vars.activeprops;
	struct defaultobj *obj;
	s32 count = 0;
	struct defaultobj *candidates[20];

	// Build a list of candidate objects to replace. Consider only ammocrates.
	// NTSC beta doesn't check the prop type, so it could potentially replace a
	// player, bot, explosion or smoke.
	while (prop && count < 20) {
#if VERSION >= VERSION_NTSC_1_0
		if (prop->type == PROPTYPE_OBJ)
#endif
		{
			obj = prop->obj;

			if (obj->type == OBJTYPE_MULTIAMMOCRATE) {
				candidates[count] = obj;
				count++;
			}
		}

		prop = prop->next;
	}

	// Choose the candidate and remove it
	if (count > 0) {
		count = rngRandom() % count;
		var800869ec = candidates[count];
		g_ScenarioData.htb.tokenpad = var800869ec->pad;
		var800869ec->hidden |= OBJHFLAG_DELETING;
		var800869ec->hidden2 |= OBJH2FLAG_CANREGEN;
	} else if (g_ScenarioData.htb.nextindex > 0) {
		g_ScenarioData.htb.tokenpad = g_ScenarioData.htb.padnums[rngRandom() % g_ScenarioData.htb.nextindex];
	} else {
		g_ScenarioData.htb.tokenpad = 0;
	}

	// Set up the token
	g_HtbTokenObj = template;
	g_HtbTokenObj.base.pad = g_ScenarioData.htb.tokenpad;

	setupPlaceWeapon(&g_HtbTokenObj, 999);

	g_HtbTokenObj.base.hidden2 &= ~OBJH2FLAG_CANREGEN;

	g_ScenarioData.htb.token = g_HtbTokenObj.base.prop;

	if (g_ScenarioData.htb.token) {
		g_ScenarioData.htb.token->forcetick = true;
	}
}

void htbInitProps(void)
{
	var800869ec = NULL;
	htbCreateToken();
}

void htbTick(void)
{
	s32 i;
	u32 prevplayernum = g_Vars.currentplayernum;
	struct prop *prop;

	if (var800869ec && var800869ec->prop) {
		if (g_ScenarioData.htb.token == NULL || g_ScenarioData.htb.token->type != PROPTYPE_WEAPON) {
			var800869ec = NULL;
		} else {
			var800869ec->prop->timetoregen = TICKS(1200);
		}
	}

	g_ScenarioData.htb.token = NULL;

	// Check if briefcase is on the ground
	prop = g_Vars.activeprops;

	while (prop) {
		if (prop->type == PROPTYPE_WEAPON) {
			struct weaponobj *weapon = prop->weapon;

			if (weapon->weaponnum == WEAPON_BRIEFCASE2) {
				g_ScenarioData.htb.token = prop;
			}
		}

		prop = prop->next;
	}

	// Check if a player is holding it
	if (g_ScenarioData.htb.token == NULL) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			setCurrentPlayerNum(i);

			if (invHasBriefcase()) {
				g_ScenarioData.htb.token = g_Vars.currentplayer->prop;
				break;
			}
		}
	}

	setCurrentPlayerNum(prevplayernum);

	// Check if a simulant is holding it
	if (g_ScenarioData.htb.token == NULL) {
		for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
#if VERSION >= VERSION_NTSC_1_0
			if (g_MpAllChrPtrs[i]->prop && g_MpAllChrPtrs[i]->aibot->hasbriefcase)
#else
			if (g_MpAllChrPtrs[i]->aibot->hasbriefcase)
#endif
			{
				g_ScenarioData.htb.token = g_MpAllChrPtrs[i]->prop;
				break;
			}
		}
	}

	if (g_ScenarioData.htb.token == NULL) {
		htbCreateToken();
	}

	if (g_ScenarioData.htb.token == NULL) {
		g_ScenarioData.htb.pos.x = 0;
		g_ScenarioData.htb.pos.y = 0;
		g_ScenarioData.htb.pos.z = 0;
	} else {
		struct coord *pos = &g_ScenarioData.htb.pos;
		pos->x = g_ScenarioData.htb.token->pos.x;
		pos->y = g_ScenarioData.htb.token->pos.y;
		pos->z = g_ScenarioData.htb.token->pos.z;
	}
}

void htbTickChr(struct chrdata *chr)
{
	if (chr) {
		if (chr->aibot->hasbriefcase) {
			chr->aibot->htbheldtimer60 += g_Vars.lvupdate240;

			if (chr->aibot->htbheldtimer60 >= TICKS(7200)) {
				sndStart(var80095200, SFX_MP_SCOREPOINT, NULL, -1, -1, -1, -1, -1);
				g_MpAllChrConfigPtrs[mpPlayerGetIndex(chr)]->numpoints++;
				chr->aibot->htbheldtimer60 = 0;
			}
		} else {
			chr->aibot->htbheldtimer60 = 0;
		}
	} else {
		if (invHasBriefcase()) {
			g_Vars.currentplayerstats->tokenheldtime += g_Vars.lvupdate240;

			if (g_Vars.currentplayerstats->tokenheldtime >= TICKS(7200)) {
				sndStart(var80095200, SFX_MP_SCOREPOINT, NULL, -1, -1, -1, -1, -1);
				g_MpAllChrConfigPtrs[g_Vars.currentplayernum]->numpoints++;
#if VERSION >= VERSION_JPN_FINAL
				hudmsgCreateWithFlags(langGet(L_MPWEAPONS_024), HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP); // "1 Point!"
#else
				hudmsgCreateWithFlags(langGet(L_MPWEAPONS_024), HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE); // "1 Point!"
#endif
				g_Vars.currentplayerstats->tokenheldtime = 0;
			}
		} else {
			g_Vars.currentplayerstats->tokenheldtime = 0;
		}
	}
}

/**
 * @bug: In NTSC Final, the calculation of mins and subsequent subtraction from
 * time240 should use 14400 instead of 7200. This has no noticeable
 * effect unless the score duration is increased to above 30 seconds.
 *
 * PAL recognises that mins will always be 0 and simplifies the calculation.
 */
Gfx *htbRenderHud(Gfx *gdl)
{
	s32 time240;
	s32 mins;
	s32 secs;
	s32 textwidth;
	s32 textheight;
	s32 x;
	s32 y;
	char text[64];

	if (invHasBriefcase()) {
		x = viGetViewLeft() + viGetViewWidth() / 2;
		y = viGetViewTop() + 10;

		time240 = TICKS(7200) - g_Vars.currentplayerstats->tokenheldtime;

#if VERSION >= VERSION_PAL_BETA
		secs = (time240 + TICKS(240) - 1) / TICKS(240);
		sprintf(text, "%d:%02d", 0, secs);
#else
		mins = time240 / TICKS(7200);
		time240 -= TICKS(7200) * mins;
		secs = (time240 + TICKS(240) - 1) / TICKS(240);
		sprintf(text, "%d:%02d", mins, secs);
#endif

#ifndef PLATFORM_N64
		const s32 playercount = PLAYERCOUNT();
		if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
			gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
		}
#endif

		gdl = text0f153628(gdl);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		x -= textwidth / 2;
		textwidth += x;
		textheight += y;

#if VERSION >= VERSION_NTSC_1_0
		gdl = text0f153990(gdl, x, y, textwidth, textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, 0x00ff00a0, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
#else
		gdl = text0f153858(gdl, &x, &y, &textwidth, &textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, 0x00ff00a0, 0x88, viGetWidth(), viGetHeight(), 0, 0);
#endif

		gdl = text0f153780(gdl);

#ifndef PLATFORM_N64
		gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);
#endif
	}

	return gdl;
}

void htbCalculatePlayerScore(struct mpchrconfig *mpchr, s32 mpchrnum, s32 *score, s32 *deaths)
{
	struct mpchrconfig *loopmpchr;
	s32 i;

	*score = 0;
	*score += mpchr->numpoints;

	if (g_MpSetup.options & MPOPTION_KILLSSCORE) {
		for (i = 0; i != MAX_MPCHRS; i++) {
			if (i == mpchrnum) {
				*score -= mpchr->killcounts[i];
			} else if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
				loopmpchr = MPCHR(i);

				if (loopmpchr->team == mpchr->team) {
					*score -= mpchr->killcounts[i];
				} else {
					*score += mpchr->killcounts[i];
				}
			} else {
				*score += mpchr->killcounts[i];
			}
		}
	}

	*deaths = mpchr->numdeaths;
}

Gfx *htbRadarExtra(Gfx *gdl)
{
	if ((g_MpSetup.options & MPOPTION_HTB_SHOWONRADAR) &&
			g_ScenarioData.htb.token != NULL &&
			g_ScenarioData.htb.token->type != PROPTYPE_PLAYER &&
			g_ScenarioData.htb.token->type != PROPTYPE_CHR) {
		struct coord dist;
		dist.x = g_ScenarioData.htb.pos.x - g_Vars.currentplayer->prop->pos.x;
		dist.y = g_ScenarioData.htb.pos.y - g_Vars.currentplayer->prop->pos.y;
		dist.z = g_ScenarioData.htb.pos.z - g_Vars.currentplayer->prop->pos.z;
		gdl = radarDrawDot(gdl, g_ScenarioData.htb.token, &dist, 0x00ff0000, 0, 1);
	}

	return gdl;
}

bool htbRadarChr(Gfx **gdl, struct prop *prop)
{
	if ((g_MpSetup.options & MPOPTION_HTB_SHOWONRADAR) &&
			g_ScenarioData.htb.token &&
			prop == g_ScenarioData.htb.token) {
		if (prop->type == PROPTYPE_PLAYER || prop->type == PROPTYPE_CHR) {
			struct coord dist;
			dist.x = prop->pos.x - g_Vars.currentplayer->prop->pos.x;
			dist.y = prop->pos.y - g_Vars.currentplayer->prop->pos.y;
			dist.z = prop->pos.z - g_Vars.currentplayer->prop->pos.z;

			if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
				u32 colour = g_TeamColours[radarGetTeamIndex(prop->chr->team)];
				*gdl = radarDrawDot(*gdl, g_ScenarioData.htb.token, &dist, colour, 0, 1);
			} else {
				*gdl = radarDrawDot(*gdl, g_ScenarioData.htb.token, &dist, 0x00ff0000, 0, 1);
			}

			return true;
		}
	}

	return false;
}

bool htbHighlightProp(struct prop *prop, s32 *colour)
{
	if ((g_MpSetup.options & MPOPTION_HTB_HIGHLIGHTBRIEFCASE) && prop == g_ScenarioData.htb.token) {
		colour[0] = 0;
		colour[1] = 0xff;
		colour[2] = 0;
		colour[3] = 0x40;

		return true;
	}

	return false;
}
