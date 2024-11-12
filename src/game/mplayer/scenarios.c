#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/dlights.h"
#include "game/game_006900.h"
#include "game/setup.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/atan2f.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/hudmsg.h"
#include "game/menu.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/game_1531a0.h"
#include "game/mplayer/setup.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/botcmd.h"
#include "game/botinv.h"
#include "game/challenge.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "game/propobj.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "data.h"
#include "types.h"

/**
 * There are six multiplayer scenarios:
 *
 * - Combat
 * - Hold the Briefcase (HTB)
 * - Capture the Case (CTC)
 * - Hack that Mac (HTM) - labelled as Hacker Central
 * - King of the Hill (KOH)
 * - Pop a Cap (PAC)
 *
 * Each scenario registers callback functions for certain events. For example,
 * code elsewhere in the game may call scenarioTick, then scenarioTick checks
 * if the current scenario has a tick callback defined (eg. htbTick). If so,
 * that scenario's callback is run.
 */

struct mpscenario {
	struct menudialogdef *optionsdialog;
	void (*initfunc)(void);
	s32 (*numpropsfunc)(void);
	void (*initpropsfunc)(void);
	void (*tickfunc)(void);
	void (*tickchrfunc)(struct chrdata *chr);
	Gfx *(*hudfunc)(Gfx *gdl);
	void (*calcscorefunc)(struct mpchrconfig *mpchr, s32 chrnum, s32 *score, s32 *deaths);
	Gfx *(*radarextrafunc)(Gfx *gdl);
	bool (*radarchrfunc)(Gfx **gdl, struct prop *prop);
	bool (*highlightpropfunc)(struct prop *prop, s32 *colour);
	bool (*spawnfunc)(f32 arg0, struct coord *pos, RoomNum *rooms, struct prop *prop, f32 *arg4);
	s32 (*maxteamsfunc)(void);
	bool (*isroomhighlightedfunc)(RoomNum room);
	void (*highlightroomfunc)(RoomNum room, s32 *arg1, s32 *arg2, s32 *arg3);
	void *unk3c; // never hooked into nor fired
	void (*readsavefunc)(struct savebuffer *buffer);
	void (*writesavefunc)(struct savebuffer *buffer);
};

#if VERSION >= VERSION_JPN_FINAL
char *scenarioRemoveLineBreaks(char *src, s32 stringnum)
{
	static char strings[2][30];
	char *out = strings[stringnum];
	s32 i;

	for (; *src != '\0'; src++) {
		if (*src != '\n') {
			*out = *src;
			out++;
		}
	}

	*out = '\0';

	return strings[stringnum];
}
#endif

struct scenariodata g_ScenarioData;

MenuItemHandlerResult menuhandlerMpDisplayTeam(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
			return false;
		}

		return true;
	}

	return menuhandlerMpCheckboxOption(operation, item, data);
}

MenuItemHandlerResult menuhandlerMpOneHitKills(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED || operation == MENUOP_CHECKHIDDEN) {
		if (challengeIsFeatureUnlocked(MPFEATURE_ONEHITKILLS)) {
			return false;
		}

		return true;
	}

	return menuhandlerMpCheckboxOption(operation, item, data);
}

MenuItemHandlerResult menuhandlerMpSlowMotion(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 labels[] = {
		L_MPMENU_240, // "Off"
		L_MPMENU_241, // "On"
		L_MPMENU_242, // "Smart"
	};

	switch (operation) {
	case MENUOP_CHECKDISABLED:
	case MENUOP_CHECKHIDDEN:
		if (challengeIsFeatureUnlocked(MPFEATURE_SLOWMOTION)) {
			return false;
		}
		return true;
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 3;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t)langGet(labels[data->dropdown.value]);
	case MENUOP_SET:
		g_MpSetup.options &= ~(MPOPTION_SLOWMOTION_ON | MPOPTION_SLOWMOTION_SMART);

		if (data->dropdown.value == SLOWMOTION_ON) {
			g_MpSetup.options |= MPOPTION_SLOWMOTION_ON;
		} else if (data->dropdown.value == SLOWMOTION_SMART) {
			g_MpSetup.options |= MPOPTION_SLOWMOTION_SMART;
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		if (g_MpSetup.options & MPOPTION_SLOWMOTION_SMART) {
			data->dropdown.value = SLOWMOTION_SMART;
		} else if (g_MpSetup.options & MPOPTION_SLOWMOTION_ON) {
			data->dropdown.value = SLOWMOTION_ON;
		} else {
			data->dropdown.value = SLOWMOTION_OFF;
		}
		break;
	}

	return 0;
}

// Include the code files where each scenario implements its callbacks
#include "scenarios/combat.inc"
#include "scenarios/holdthebriefcase.inc"
#include "scenarios/capturethecase.inc"
#include "scenarios/kingofthehill.inc"
#include "scenarios/hackthatmac.inc"
#include "scenarios/popacap.inc"

// Define the scenario callbacks
struct mpscenario g_MpScenarios[] = {
	{
		&g_MpCombatOptionsMenuDialog,
	}, {
		&g_HtbOptionsMenuDialog,
		htbInit,
		htbNumProps,
		htbInitProps,
		htbTick,
		htbTickChr,
		htbRenderHud,
		htbCalculatePlayerScore,
		htbRadarExtra,
		htbRadarChr,
		htbHighlightProp,
	}, {
		&g_HtmOptionsMenuDialog,
		htmInit,
		htmNumProps,
		htmInitProps,
		htmTick,
		htmTickChr,
		htmRenderHud,
		htmCalculatePlayerScore,
		htmRadarExtra,
		htmRadarChr,
		htmHighlightProp,
	}, {
		&g_PacOptionsMenuDialog,
		pacInit,
		NULL,
		pacInitProps,
		pacTick,
		NULL,
		pacRenderHud,
		pacCalculatePlayerScore,
		pacRadarExtra,
		pacRadarChr,
		pacHighlightProp,
	}, {
		&g_KohOptionsMenuDialog,
		kohInit,
		NULL,
		kohInitProps,
		kohTick,
		NULL,
		kohRenderHud,
		kohCalculatePlayerScore,
		kohRadarExtra,
		NULL,
		NULL,
		NULL,
		NULL,
		kohIsRoomHighlighted,
		kohHighlightRoom,
		NULL,
		kohReadSave,
		kohWriteSave
	}, {
		&g_CtcOptionsMenuDialog,
		ctcInit,
		ctcNumProps,
		ctcInitProps,
		ctcTick,
		ctcTickChr,
		NULL,
		ctcCalculatePlayerScore,
		ctcRadarExtra,
		ctcRadarChr,
		ctcHighlightProp,
		ctcChooseSpawnLocation,
		ctcGetMaxTeams,
		ctcIsRoomHighlighted,
		ctcHighlightRoom,
	},
};

struct mpscenariooverview g_MpScenarioOverviews[] = {
	// name, short name, require feature, team only
	{ L_MPMENU_246, L_MPMENU_253, 0,                      false }, // "Combat", "Combat"
	{ L_MPMENU_247, L_MPMENU_254, MPFEATURE_SCENARIO_HTB, false }, // "Hold the Briefcase", "Briefcase"
	{ L_MPMENU_248, L_MPMENU_255, MPFEATURE_SCENARIO_HTM, false }, // "Hacker Central", "Hacker"
	{ L_MPMENU_249, L_MPMENU_256, MPFEATURE_SCENARIO_PAC, false }, // "Pop a Cap", "Pop"
	{ L_MPMENU_250, L_MPMENU_257, MPFEATURE_SCENARIO_KOH, true  }, // "King of the Hill", "Hill"
	{ L_MPMENU_251, L_MPMENU_258, MPFEATURE_SCENARIO_CTC, true  }, // "Capture the Case", "Capture"
};

/**
 * While the options dialog is open, check if another player has changed the
 * scenario to a different one. If so, replace this dialog with the new one.
 */
MenuDialogHandlerResult mpOptionsMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog->definition != g_MpScenarios[g_MpSetup.scenario].optionsdialog) {
			s32 i;
			s32 end = ARRAYCOUNT(g_MpScenarios);

			for (i = 0; i < end; i++) {
				if (g_Menus[g_MpPlayerNum].curdialog->definition == g_MpScenarios[i].optionsdialog) {
					break;
				}
			}

			if (i < end) {
				menuPopDialog();
				menuPushDialog(g_MpScenarios[g_MpSetup.scenario].optionsdialog);
			}
		}
	}

	return 0;
}

char *mpMenuTextScenarioShortName(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s\n", langGet(g_MpScenarioOverviews[g_MpSetup.scenario].shortname));
	return g_StringPointer;
}

char *mpMenuTextScenarioName(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s\n", langGet(g_MpScenarioOverviews[g_MpSetup.scenario].name));
	return g_StringPointer;
}

struct scenariogroup {
	s32 startindex;
	u16 textid;
};

MenuItemHandlerResult scenarioScenarioMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct scenariogroup groups[] = {
		{ 0, L_MPMENU_244 }, // "Free for All!"
		{ 4, L_MPMENU_245 }, // "-Teamwork-"
	};

	s32 i;
	s32 count = 0;
	bool teamgame = true;

	if (item->param) {
		if (g_Vars.mpquickteam == MPQUICKTEAM_PLAYERSONLY || g_Vars.mpquickteam == MPQUICKTEAM_PLAYERSANDSIMS) {
			teamgame = false;
		}
	}

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		for (i = 0; i < ARRAYCOUNT(g_MpScenarioOverviews); i++) {
			if (challengeIsFeatureUnlocked(g_MpScenarioOverviews[i].requirefeature)
					&& (teamgame || g_MpScenarioOverviews[i].teamonly == false)) {
				count++;
			}
		}

		data->list.value = count;
		break;
	case MENUOP_GETOPTIONTEXT:
		for (i = 0; i < ARRAYCOUNT(g_MpScenarioOverviews); i++) {
			if (challengeIsFeatureUnlocked(g_MpScenarioOverviews[i].requirefeature)
					&& (teamgame || g_MpScenarioOverviews[i].teamonly == false)) {
				if (count == data->list.value) {
					return (uintptr_t)langGet(g_MpScenarioOverviews[i].name);
				}

				count++;
			}
		}

		break;
	case MENUOP_SET:
		for (i = 0; i < ARRAYCOUNT(g_MpScenarioOverviews); i++) {
			if (challengeIsFeatureUnlocked(g_MpScenarioOverviews[i].requirefeature)
					&& (teamgame || g_MpScenarioOverviews[i].teamonly == false)) {
				if (count == data->list.value) {
					g_MpSetup.scenario = i;
					break;
				}

				count++;
			}
		}

		scenarioInit();
		break;
	case MENUOP_GETSELECTEDINDEX:
		for (i = 0; i < ARRAYCOUNT(g_MpScenarioOverviews); i++) {
			if (challengeIsFeatureUnlocked(g_MpScenarioOverviews[i].requirefeature)
					&& (teamgame || g_MpScenarioOverviews[i].teamonly == false)) {
				if (i == g_MpSetup.scenario) {
					data->list.value = count;
					break;
				}

				count++;
			}
		}

		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 2;

		if (!teamgame || (!challengeIsFeatureUnlocked(MPFEATURE_SCENARIO_KOH) && !challengeIsFeatureUnlocked(MPFEATURE_SCENARIO_CTC))) {
			data->list.value--;
		}
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return (uintptr_t)langGet(groups[data->list.value].textid);
	case MENUOP_GETGROUPSTARTINDEX:
		for (i = 0; i < groups[data->list.value].startindex; i++) {
			if (challengeIsFeatureUnlocked(g_MpScenarioOverviews[i].requirefeature)
					&& (teamgame || g_MpScenarioOverviews[i].teamonly == false)) {
				count++;
			}
		}

		data->list.groupstartindex = count;
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpOpenOptions(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPushDialog(g_MpScenarios[g_MpSetup.scenario].optionsdialog);
	}

	return 0;
}

/**
 * Allow a callback to read data from the setup file's save file buffer.
 *
 * There is one byte available to be read. If not handled, the byte will be
 * consumed and ignored.
 *
 * Used by KOH to read the mphilltime.
 */
void scenarioReadSave(struct savebuffer *buffer)
{
	if (g_MpScenarios[g_MpSetup.scenario].readsavefunc) {
		g_MpScenarios[g_MpSetup.scenario].readsavefunc(buffer);
	} else {
		savebufferReadBits(buffer, 8);
	}
}

/**
 * Allow a callback to write data to the setup file's save file buffer.
 *
 * There is one byte available to be written. If not handled, the byte will be
 * written as 0.
 *
 * Used by KOH to write the mphilltime.
 */
void scenarioWriteSave(struct savebuffer *buffer)
{
	if (g_MpScenarios[g_MpSetup.scenario].writesavefunc) {
		g_MpScenarios[g_MpSetup.scenario].writesavefunc(buffer);
	} else {
		savebufferOr(buffer, 0, 8);
	}
}

/**
 * Called whenever a scenario is selected/applied in the match settings.
 *
 * The callback should initialise all the properties in g_ScenarioData.
 */
void scenarioInit(void)
{
	if (g_MpScenarios[g_MpSetup.scenario].initfunc) {
		g_MpScenarios[g_MpSetup.scenario].initfunc();
	}
}

/**
 * Return the number of additional props that will be created, such as
 * briefcases and uplinks.
 */
s32 scenarioNumProps(void)
{
	s32 result = 0;

	if (g_MpScenarios[g_MpSetup.scenario].numpropsfunc) {
		result = g_MpScenarios[g_MpSetup.scenario].numpropsfunc();
	}

	return result;
}

/**
 * Create the additional props, such as briefcases and uplinks.
 */
void scenarioInitProps(void)
{
	if (g_MpScenarios[g_MpSetup.scenario].initpropsfunc) {
		g_MpScenarios[g_MpSetup.scenario].initpropsfunc();
	}
}

/**
 * At the start of each match, a hud message appears for all players containing
 * the challenge name if it's a challenge, or the scenario name if not.
 */
void scenarioCreateMatchStartHudmsgs(void)
{
	s32 i;
	s32 prevplayernum = g_Vars.currentplayernum;
	char challengename[60];
	char scenarioname[60];

	if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
#if VERSION >= VERSION_JPN_FINAL
		sprintf(challengename, "%s\n", challengeGetNameBySlot(challengeGetCurrent()));
#else
		sprintf(challengename, "%s:\n", challengeGetNameBySlot(challengeGetCurrent()));
#endif
	}

	sprintf(scenarioname, "%s\n", langGet(g_MpScenarioOverviews[g_MpSetup.scenario].name));

	for (i = 0; i < g_MpNumChrs; i++) {
		if (g_MpAllChrPtrs[i]->aibot == NULL) {
			setCurrentPlayerNum(i);

			if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
#if VERSION >= VERSION_JPN_FINAL
				hudmsgCreateWithFlags(challengename, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
#else
				hudmsgCreateWithFlags(challengename, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE);
#endif
			}

#if VERSION >= VERSION_JPN_FINAL
			hudmsgCreateWithFlags(scenarioname, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
#else
			hudmsgCreateWithFlags(scenarioname, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE);
#endif
		}
	}

	setCurrentPlayerNum(prevplayernum);
}

/**
 * Called on every frame during a match, including while paused.
 */
void scenarioTick(void)
{
	if (g_Vars.normmplayerisrunning) {
		if (g_Vars.lvframenum == 5) {
			scenarioCreateMatchStartHudmsgs();
		}

		if (g_MpScenarios[g_MpSetup.scenario].tickfunc) {
			g_MpScenarios[g_MpSetup.scenario].tickfunc();
		}
	}
}

/**
 * Tick a single chr.
 *
 * If chr is provided then it will be a bot.
 * If chr is NULL then the handler should tick the current player instead.
 */
void scenarioTickChr(struct chrdata *chr)
{
	if (g_Vars.normmplayerisrunning && g_MpScenarios[g_MpSetup.scenario].tickchrfunc) {
		g_MpScenarios[g_MpSetup.scenario].tickchrfunc(chr);
	}
}

/**
 * Render any HUD information such as timers.
 */
Gfx *scenarioRenderHud(Gfx *gdl)
{
	s32 viewleft;
	s32 viewright;
	s32 viewtop;
	s32 viewheight;
	s32 cplayernum;
	s32 playercount;
	struct chrdata *chr;
	u32 colour;

	if (g_Vars.normmplayerisrunning) {
		if (g_MpScenarios[g_MpSetup.scenario].hudfunc) {
#if VERSION >= VERSION_NTSC_1_0
			if (g_MpSetup.paused != MPPAUSEMODE_GAMEOVER && g_NumReasonsToEndMpMatch == 0) {
				gDPSetTextureFilter(gdl++, G_TF_POINT);
				gDPSetColorDither(gdl++, G_CD_DISABLE);
				gSPClearGeometryMode(gdl++, G_ZBUFFER);
				gDPPipeSync(gdl++);
				gDPSetTexturePersp(gdl++, G_TP_NONE);
				gDPSetCycleType(gdl++, G_CYC_FILL);
				gDPSetRenderMode(gdl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
				gDPPipelineMode(gdl++, G_PM_1PRIMITIVE);

				gdl = g_MpScenarios[g_MpSetup.scenario].hudfunc(gdl);
			}
#else
			gdl = g_MpScenarios[g_MpSetup.scenario].hudfunc(gdl);
#endif
		}

		playercount = PLAYERCOUNT();

		if ((g_MpSetup.options & MPOPTION_TEAMSENABLED)
				&& (g_MpSetup.options & MPOPTION_DISPLAYTEAM)
				&& playercount >= 2) {
			chr = g_Vars.currentplayer->prop->chr;
			cplayernum = g_Vars.currentplayernum;

			gDPSetTextureFilter(gdl++, G_TF_POINT);
			gDPSetColorDither(gdl++, G_CD_DISABLE);
			gSPClearGeometryMode(gdl++, G_ZBUFFER);
			gDPPipeSync(gdl++);
			gDPSetTexturePersp(gdl++, G_TP_NONE);
			gDPSetCycleType(gdl++, G_CYC_FILL);
			gDPSetRenderMode(gdl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
			gDPPipelineMode(gdl++, G_PM_1PRIMITIVE);

			colour = var80087ce4[radarGetTeamIndex(chr->team)];
			gDPSetFillColor(gdl++, colour);

			viewleft = viGetViewLeft();
			viewright = viewleft + viGetViewWidth();
			viewtop = viGetViewTop();
			viewheight = viGetViewHeight();

			if (playercount >= 3) {
				if (cplayernum <= 1) {
					// Player on top quarter - draw line at bottom of viewport
					gDPFillRectangle(gdl++, viewleft, viewheight + viewtop - 1, viewright - 1, viewheight + viewtop - 1);
				} else {
					// Player on bottom quarter - draw line at top of viewport
					gDPFillRectangle(gdl++, viewleft, viewtop, viewright - 1, viewtop);
				}
			}
#if VERSION >= VERSION_NTSC_1_0
			else if (g_Vars.fourmeg2player) {
				// Draw line at bottom of viewport
				gDPFillRectangle(gdl++, viewleft, viewheight + viewtop - 2, viewright - 1, viewheight + viewtop - 2);
			}
#endif
			else {
				// @bug: No consideration is made for vertical splits here,
				// however when using a vertical split neither line is visible.
				// It's suspected that a screen border is drawn over the top of it.
				if (cplayernum == 0) {
					// Player on top half - draw line at bottom of viewport
					gDPFillRectangle(gdl++, viewleft, viewheight + viewtop - 1, viewright - 1, viewheight + viewtop - 1);
				} else {
					// Player on bottom half - draw line at top of viewport
					gDPFillRectangle(gdl++, viewleft, viewtop, viewright - 1, viewtop);
				}
			}
		}
	}

	return gdl;
}

/**
 * Calculate the score and number of deaths for the given mpchr.
 *
 * If no callback is registered, the default calculation below will apply.
 */
void scenarioCalculatePlayerScore(struct mpchrconfig *mpchr, s32 chrnum, s32 *score, s32 *deaths)
{
	struct mpchrconfig *othermpchr;
	s32 i;

	if (g_MpScenarios[g_MpSetup.scenario].calcscorefunc) {
		g_MpScenarios[g_MpSetup.scenario].calcscorefunc(mpchr, chrnum, score, deaths);
	} else {
		*score = 0;

		for (i = 0; i < MAX_MPCHRS; i++) {
			if (i == chrnum) {
				*score -= mpchr->killcounts[i];
			} else if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
				othermpchr = MPCHR(i);

				if (othermpchr->team == mpchr->team) {
					*score -= mpchr->killcounts[i];
				} else {
					*score += mpchr->killcounts[i];
				}
			} else {
				*score += mpchr->killcounts[i];
			}
		}

		*deaths = mpchr->numdeaths;
	}
}

/**
 * Draw anything extra to the radar, such as props or the king of the hill room.
 */
Gfx *scenarioRadarExtra(Gfx *gdl)
{
	if (g_Vars.normmplayerisrunning && g_MpScenarios[g_MpSetup.scenario].radarextrafunc) {
		return g_MpScenarios[g_MpSetup.scenario].radarextrafunc(gdl);
	}

	return gdl;
}

/**
 * Render a player or bot on the radar.
 *
 * Return true if handled or false if the generic radar code should render it.
 */
bool scenarioRadarChr(Gfx **gdl, struct prop *prop)
{
	if (g_Vars.normmplayerisrunning && g_MpScenarios[g_MpSetup.scenario].radarchrfunc) {
		return g_MpScenarios[g_MpSetup.scenario].radarchrfunc(gdl, prop);
	}

	return false;
}

/**
 * Set the highlight colour of the given prop.
 *
 * The prop may be a chr, weapon or object.
 */
bool scenarioHighlightProp(struct prop *prop, s32 *colour)
{
	if (g_MpScenarios[g_MpSetup.scenario].highlightpropfunc) {
		if (g_MpScenarios[g_MpSetup.scenario].highlightpropfunc(prop, colour)) {
			return true;
		}
	}

	if (prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_DOOR || prop->type == PROPTYPE_WEAPON) {
		struct defaultobj *obj = prop->obj;

		if ((g_MpSetup.scenario != MPSCENARIO_COMBAT || (g_MpSetup.options & MPOPTION_NOPICKUPHIGHLIGHT) == 0)
				&& (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.displayoptions & MPDISPLAYOPTION_HIGHLIGHTPICKUPS)) {
			switch (obj->type) {
			case OBJTYPE_AMMOCRATE:
			case OBJTYPE_WEAPON:
			case OBJTYPE_LINKGUNS:
			case OBJTYPE_MULTIAMMOCRATE:
			case OBJTYPE_SHIELD:
				colour[0] = 0;
				colour[1] = 0xcd;
				colour[2] = 0xff;
				colour[3] = menuGetSinOscFrac(20) * 255;
				return true;
			}
		}
	} else if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
		bool pulse = false;
		bool isunselectedbot = false;
		bool useblue = false;
		bool useteamcolour = false;

		if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
			struct chrdata *botchr = currentPlayerGetCommandingAibot();

			if (botchr) {
				if (botchr == prop->chr) {
					pulse = true;
					useteamcolour = true;
				} else {
					isunselectedbot = true;
				}
			}
		}

		if (!pulse && !isunselectedbot
				&& (g_MpSetup.scenario != MPSCENARIO_COMBAT || (g_MpSetup.options & MPOPTION_NOPLAYERHIGHLIGHT) == 0)) {
			if ((g_MpSetup.options & MPOPTION_TEAMSENABLED)
					&& (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.displayoptions & MPDISPLAYOPTION_HIGHLIGHTTEAMS)) {
				useteamcolour = true;
			} else if (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.displayoptions & MPDISPLAYOPTION_HIGHLIGHTPLAYERS) {
				useblue = true;
			}
		}

		if (useteamcolour) {
			u32 tmp = g_TeamColours[radarGetTeamIndex(prop->chr->team)];

			colour[0] = tmp >> 24 & 0xff;
			colour[1] = tmp >> 16 & 0xff;
			colour[2] = tmp >> 8 & 0xff;
			colour[3] = pulse ? (s32)(menuGetSinOscFrac(20) * 128) : 75;
			return true;
		}

		if (useblue) {
			colour[0] = 0;
			colour[1] = 0xcd;
			colour[2] = 0xff;
			colour[3] = menuGetSinOscFrac(20) * 205;
			return true;
		}
	}

	return false;
}

/**
 * Choose a spawn location for a chr.
 *
 * CTC uses this to ensure the chrs spawn near their base.
 */
f32 scenarioChooseSpawnLocation(f32 chrradius, struct coord *pos, RoomNum *rooms, struct prop *prop)
{
	f32 result;

	if (g_Vars.normmplayerisrunning && g_MpScenarios[g_MpSetup.scenario].spawnfunc &&
			g_MpScenarios[g_MpSetup.scenario].spawnfunc(chrradius, pos, rooms, prop, &result)) {
		return result;
	}

	return playerChooseGeneralSpawnLocation(chrradius, pos, rooms, prop);
}

/**
 * Called when starting a match.
 *
 * The function clears anything in the g_ScenarioData struct that is
 * stage specific such as pad and room numbers, then re-reads the setup
 * file to reload that information.
 */
void scenarioReset(void)
{
	s32 i;
	s32 j;
	s32 *cmd = g_StageSetup.intro;

	switch (g_MpSetup.scenario) {
	case MPSCENARIO_KINGOFTHEHILL:
		g_ScenarioData.koh.hillcount = 0;
		break;
	case MPSCENARIO_CAPTURETHECASE:
		for (i = 0; i < ARRAYCOUNT(g_ScenarioData.ctc.spawnpadsperteam); i++) {
			g_ScenarioData.ctc.spawnpadsperteam[i].homepad = -1;
			g_ScenarioData.ctc.spawnpadsperteam[i].numspawnpads = 0;

			for (j = 0; j < ARRAYCOUNT(g_ScenarioData.ctc.spawnpadsperteam[i].spawnpads); j++) {
				g_ScenarioData.ctc.spawnpadsperteam[i].spawnpads[j] = -1;
			}
		}

		for (i = 0; i < ARRAYCOUNT(g_ScenarioData.ctc.playercountsperteam); i++) {
			g_ScenarioData.ctc.playercountsperteam[i] = 0;
			g_ScenarioData.ctc.teamindexes[i] = -1;
		}
		break;
	case MPSCENARIO_HACKERCENTRAL:
		htmReset();
		break;
	case MPSCENARIO_HOLDTHEBRIEFCASE:
		htbReset();
		break;
	case MPSCENARIO_POPACAP:
		break;
	}

	if (cmd) {
		while (cmd[0] != INTROCMD_END) {
			switch (cmd[0]) {
			case INTROCMD_SPAWN:
				cmd += 3;
				break;
			case INTROCMD_CASE:
			case INTROCMD_CASERESPAWN:
				if (g_MpSetup.scenario == MPSCENARIO_CAPTURETHECASE) {
					ctcAddPad(cmd);
				} else if (g_MpSetup.scenario == MPSCENARIO_HACKERCENTRAL) {
					htmAddPad(cmd[2]);
				} else if (g_MpSetup.scenario == MPSCENARIO_HOLDTHEBRIEFCASE) {
					htbAddPad(cmd[2]);
				}
				cmd += 3;
				break;
			case INTROCMD_HILL:
				if (g_MpSetup.scenario == MPSCENARIO_KINGOFTHEHILL) {
					kohAddHill(cmd);
				}
				cmd += 2;
				break;
			case INTROCMD_WEAPON:
				cmd += 4;
				break;
			case INTROCMD_AMMO:
				cmd += 4;
				break;
			case INTROCMD_3:
				cmd += 8;
				break;
			case INTROCMD_4:
				cmd += 2;
				break;
			case INTROCMD_OUTFIT:
				cmd += 2;
				break;
			case INTROCMD_6:
				cmd += 10;
				break;
			case INTROCMD_WATCHTIME:
				cmd += 3;
				break;
			case INTROCMD_CREDITOFFSET:
				cmd += 2;
				break;
			default:
				cmd++;
				break;
			}
		}
	}
}

/**
 * Return the maximum number of teams permitted for this scenario.
 *
 * CTC sets this to 4, while the others use the default limit of 8.
 */
s32 scenarioGetMaxTeams(void)
{
	if (g_MpScenarios[g_MpSetup.scenario].maxteamsfunc) {
		return g_MpScenarios[g_MpSetup.scenario].maxteamsfunc();
	}

	return MAX_TEAMS;
}

/**
 * This callback is unused.
 */
bool scenarioIsRoomHighlighted(RoomNum room)
{
	if (g_MpScenarios[g_MpSetup.scenario].isroomhighlightedfunc) {
		return g_MpScenarios[g_MpSetup.scenario].isroomhighlightedfunc(room);
	}

	return false;
}

/**
 * Override the colour for the given room.
 *
 * Used in CTC for the team bases and in KOH for the hill.
 */
void scenarioHighlightRoom(RoomNum room, s32 *arg1, s32 *arg2, s32 *arg3)
{
	if (g_MpScenarios[g_MpSetup.scenario].highlightroomfunc) {
		g_MpScenarios[g_MpSetup.scenario].highlightroomfunc(room, arg1, arg2, arg3);
	}
}

struct menuitem g_MpScenarioMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_WIDE | MENUITEMFLAG_LOCKABLEMINOR,
		0x00000078,
		0x0000004d,
		scenarioScenarioMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpScenarioMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_243, // "Scenario"
	g_MpScenarioMenuItems,
	NULL,
	MENUDIALOGFLAG_CLOSEONSELECT | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpQuickTeamScenarioMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		1,
		MENUITEMFLAG_LIST_WIDE | MENUITEMFLAG_LOCKABLEMINOR,
		0x00000078,
		0x0000004d,
		scenarioScenarioMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickTeamScenarioMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_243, // "Scenario"
	g_MpQuickTeamScenarioMenuItems,
	NULL,
	MENUDIALOGFLAG_CLOSEONSELECT | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

/**
 * Create a general object.
 *
 * This is a helper function used by HTM to create the terminal.
 */
struct prop *scenarioCreateObj(s32 modelnum, s16 padnum, f32 arg2, u32 flags, u32 flags2, u32 flags3)
{
	struct defaultobj template = {
		256,                    // extrascale
		0,                      // hidden2
		OBJTYPE_BASIC,          // type
		MODEL_A51_CRATE1,       // modelnum
		-1,                     // pad
		0,                      // flags
		0,                      // flags2
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
	};

	struct defaultobj *obj = mempAlloc(ALIGN16(sizeof(struct defaultobj)), MEMPOOL_STAGE);
	*obj = template;

	obj->modelnum = modelnum;
	obj->pad = padnum;
	obj->flags = flags;
	obj->flags2 = flags2;
	obj->flags3 = flags3;
	obj->extrascale = arg2 * 256;
	obj->hidden2 &= ~OBJHFLAG_DELETING;

	setupCreateObject(obj, 123);
	propActivate(obj->prop);
	propEnable(obj->prop);

	return obj->prop;
}

/**
 * Create a HUD message for the given player.
 *
 * This is a helper function used by PAC.
 */
void scenarioCreateHudmsg(s32 playernum, char *message)
{
	if (playernum >= 0 && playernum < PLAYERCOUNT()) {
		s32 prevplayernum = g_Vars.currentplayernum;

		setCurrentPlayerNum(playernum);
#if VERSION >= VERSION_JPN_FINAL
		hudmsgCreateWithFlags(message, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
#else
		hudmsgCreateWithFlags(message, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
#endif
		setCurrentPlayerNum(prevplayernum);
	}
}

/**
 * CHeck if two player numbers are on the same team.
 *
 * This is a helper function used by PAC.
 */
bool scenarioChrsAreSameTeam(s32 playernum1, s32 playernum2)
{
	struct mpchrconfig *achr;
	struct mpchrconfig *bchr;

	if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) && playernum1 >= 0 && playernum2 >= 0) {
		s32 a = func0f18d074(playernum1);
		s32 b = func0f18d074(playernum2);

		if (a >= 0 && b >= 0) {
			achr = MPCHR(a);
			bchr = MPCHR(b);

			return (achr->team == bchr->team) ? true : false;
		}
	}

	return false;
}

/**
 * Handle a player or bot picking up a briefcase, regardless of which scenario
 * it is.
 *
 * The return value is a TICKOP constant.
 */
s32 scenarioPickUpBriefcase(struct chrdata *chr, struct prop *prop)
{
	struct defaultobj *obj = prop->obj;
	struct weaponobj *weapon = prop->weapon;
	s32 i;
	s32 prevplayernum;
	s32 caseteam;
	char text1[64];
	char text2[64];
	char text3[64];
	struct mpchrconfig *mpchr;

	if (g_MpSetup.scenario == MPSCENARIO_HOLDTHEBRIEFCASE) {
		// Player or bot has picked up the briefcase
		g_ScenarioData.htb.token = chr->prop;

		if (chr->aibot) {
			mpchr = g_MpAllChrConfigPtrs[mpPlayerGetIndex(chr)];
			propPlayPickupSound(prop, weapon->weaponnum);
			chr->aibot->hasbriefcase = true;
			botinvGiveSingleWeapon(chr, WEAPON_BRIEFCASE2);
		} else {
			mpchr = MPCHR(g_Vars.playerstats[g_Vars.currentplayernum].mpindex);

			invGiveSingleWeapon(WEAPON_BRIEFCASE2);
			currentPlayerQueuePickupWeaponHudmsg(WEAPON_BRIEFCASE2, false);
			weaponPlayPickupSound(WEAPON_BRIEFCASE2);
		}

#if VERSION >= VERSION_JPN_FINAL
		// "%shas the Briefcase"
		sprintf(text1, langGet(L_MPWEAPONS_000_2), scenarioRemoveLineBreaks(mpchr->name, 0), bgunGetShortName(WEAPON_BRIEFCASE2));
#elif VERSION >= VERSION_PAL_BETA
		// "%shas the Briefcase"
		sprintf(text1, langGet(L_MPWEAPONS_000_2), mpchr->name, bgunGetShortName(WEAPON_BRIEFCASE2));
#else
		// "%shas the\n%s"
		sprintf(text1, langGet(L_MPWEAPONS_000), mpchr->name, bgunGetShortName(WEAPON_BRIEFCASE2));
#endif

		prevplayernum = g_Vars.currentplayernum;

		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (chr->aibot || i != prevplayernum) {
				setCurrentPlayerNum(i);
#if VERSION >= VERSION_JPN_FINAL
				hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
#else
				hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
#endif
			}
		}

		setCurrentPlayerNum(prevplayernum);

		if (chr->aibot) {
			obj->hidden |= OBJHFLAG_DELETING;
			return TICKOP_NONE;
		}

#if VERSION >= VERSION_NTSC_1_0
		objFree(obj, false, obj->hidden2 & OBJH2FLAG_CANREGEN);
		return TICKOP_FREE;
#else
		return TICKOP_GIVETOPLAYER;
#endif
	} else if (g_MpSetup.scenario == MPSCENARIO_CAPTURETHECASE) {
		if (chr->aibot) {
			mpchr = g_MpAllChrConfigPtrs[mpPlayerGetIndex(chr)];
		} else {
			mpchr = MPCHR(g_Vars.playerstats[g_Vars.currentplayernum].mpindex);
		}

		if (weapon->team == mpchr->team) {
			if ((chr->aibot && chr->aibot->hascase) || (!chr->aibot && invHasBriefcase())) {
				// The chr already has a case and they are "picking up"
				// their home case. ie. A point has just been scored
				mpchr->numpoints++;

				for (i = 0; i < ARRAYCOUNT(g_ScenarioData.ctc.tokens); i++) {
					if (g_ScenarioData.ctc.tokens[i] == chr->prop) {
						break;
					}
				}

				if (chr->aibot) {
					botinvDropOne(chr, WEAPON_BRIEFCASE2);
					chr->aibot->hascase = false;
				} else {
					sndStart(var80095200, SFX_MP_SCOREPOINT, NULL, -1, -1, -1, -1, -1);
					weaponCreateForPlayerDrop(WEAPON_BRIEFCASE2);
					invRemoveItemByNum(WEAPON_BRIEFCASE2);
				}

#if VERSION >= VERSION_JPN_FINAL
				// "You captured the %s Briefcase"
				sprintf(text1, langGet(L_MPWEAPONS_004), scenarioRemoveLineBreaks(g_BossFile.teamnames[i], 0));

				// "%scaptured our Briefcase"
				sprintf(text2, langGet(L_MPWEAPONS_005), scenarioRemoveLineBreaks(mpchr->name, 0));

				// "%scaptured the %s Briefcase"
				sprintf(text3, langGet(L_MPWEAPONS_006), scenarioRemoveLineBreaks(mpchr->name, 0), scenarioRemoveLineBreaks(g_BossFile.teamnames[i], 1));
#elif VERSION >= VERSION_PAL_BETA
				// "You captured the %s Briefcase"
				sprintf(text1, langGet(L_MPWEAPONS_004), g_BossFile.teamnames[i]);

				// "%scaptured our Briefcase"
				sprintf(text2, langGet(L_MPWEAPONS_005), mpchr->name);

				// "%scaptured the %s Briefcase"
				sprintf(text3, langGet(L_MPWEAPONS_006), mpchr->name, g_BossFile.teamnames[i]);
#else
				// "You captured the %s%s"
				sprintf(text1, langGet(L_MPWEAPONS_004), g_BossFile.teamnames[i], bgunGetShortName(WEAPON_BRIEFCASE2));

				// "%scaptured our %s"
				sprintf(text2, langGet(L_MPWEAPONS_005), mpchr->name, bgunGetShortName(WEAPON_BRIEFCASE2));

				// "%scaptured the %s%s"
				sprintf(text3, langGet(L_MPWEAPONS_006), mpchr->name, g_BossFile.teamnames[i], bgunGetShortName(WEAPON_BRIEFCASE2));
#endif

				prevplayernum = g_Vars.currentplayernum;

				caseteam = i;

				if (caseteam);

				for (i = 0; i < PLAYERCOUNT(); i++) {
					setCurrentPlayerNum(i);

#if VERSION >= VERSION_JPN_FINAL
					if (!chr->aibot && i == prevplayernum) {
						hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					} else if (caseteam == g_MpAllChrConfigPtrs[i]->team) {
						hudmsgCreateWithFlags(text2, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					} else {
						hudmsgCreateWithFlags(text3, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					}
#else
					if (!chr->aibot && i == prevplayernum) {
						hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					} else if (caseteam == g_MpAllChrConfigPtrs[i]->team) {
						hudmsgCreateWithFlags(text2, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					} else {
						hudmsgCreateWithFlags(text3, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					}
#endif
				}

				setCurrentPlayerNum(prevplayernum);
			}

			if (chr->aibot) {
				return TICKOP_NONE;
			}

			return TICKOP_NONE;
		} else {
			// Picking up another team's case
			if ((chr->aibot && !chr->aibot->hascase) || (!chr->aibot && !invHasBriefcase())) {
				// And we have no case currently, so go ahead and pick it up
				if (chr->aibot) {
					chr->aibot->hascase = true;
					propPlayPickupSound(prop, weapon->weaponnum);
				}

				g_ScenarioData.ctc.tokens[weapon->team] = chr->prop;

#if VERSION >= VERSION_JPN_FINAL
				// "%shas the %s Briefcase"
				sprintf(text1, langGet(L_MPWEAPONS_000_3), scenarioRemoveLineBreaks(mpchr->name, 0), scenarioRemoveLineBreaks(g_BossFile.teamnames[weapon->team], 1));

				// "%shas our\nBriefcase"
				sprintf(text2, langGet(L_MPWEAPONS_002), scenarioRemoveLineBreaks(mpchr->name, 0));

				// "Got the %s Briefcase"
				sprintf(text3, langGet(L_MPWEAPONS_003), scenarioRemoveLineBreaks(g_BossFile.teamnames[weapon->team], 0));
#elif VERSION >= VERSION_PAL_BETA
				// "%shas the %s Briefcase"
				sprintf(text1, langGet(L_MPWEAPONS_000_3), mpchr->name, g_BossFile.teamnames[weapon->team]);

				// "%shas our\nBriefcase"
				sprintf(text2, langGet(L_MPWEAPONS_002), mpchr->name);

				// "Got the %s Briefcase"
				sprintf(text3, langGet(L_MPWEAPONS_003), g_BossFile.teamnames[weapon->team]);
#else
				// "%shas the %s%s"
				sprintf(text1, langGet(L_MPWEAPONS_001), mpchr->name, g_BossFile.teamnames[weapon->team], bgunGetShortName(WEAPON_BRIEFCASE2));

				// "%shas our %s"
				sprintf(text2, langGet(L_MPWEAPONS_002), mpchr->name, bgunGetShortName(WEAPON_BRIEFCASE2));

				// "Got the %s%s"
				sprintf(text3, langGet(L_MPWEAPONS_003), g_BossFile.teamnames[weapon->team], bgunGetShortName(WEAPON_BRIEFCASE2));
#endif

				prevplayernum = g_Vars.currentplayernum;

				for (i = 0; i < PLAYERCOUNT(); i++) {
					setCurrentPlayerNum(i);

#if VERSION >= VERSION_JPN_FINAL
					if (!chr->aibot && i == prevplayernum) {
						hudmsgCreateWithFlags(text3, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					} else if (weapon->team == g_MpAllChrConfigPtrs[i]->team) {
						hudmsgCreateWithFlags(text2, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					} else {
						hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
					}
#else
					if (!chr->aibot && i == prevplayernum) {
						hudmsgCreateWithFlags(text3, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					} else if (weapon->team == g_MpAllChrConfigPtrs[i]->team) {
						hudmsgCreateWithFlags(text2, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					} else {
						hudmsgCreateWithFlags(text1, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
					}
#endif
				}

				setCurrentPlayerNum(prevplayernum);

				if (chr->aibot) {
					botinvGiveSingleWeapon(chr, WEAPON_BRIEFCASE2);
					obj->hidden |= OBJHFLAG_DELETING;
					return TICKOP_NONE;
				} else {
					invGiveWeaponsByProp(prop);
#if VERSION >= VERSION_NTSC_1_0
					objFree(obj, false, obj->hidden2 & OBJH2FLAG_CANREGEN);
					return TICKOP_FREE;
#else
					return TICKOP_GIVETOPLAYER;
#endif
				}
			} else {
				// Already holding another team's case, so ignore the pickup
				if (chr->aibot) {
					return TICKOP_NONE;
				}

				return TICKOP_NONE;
			}
		}
	} else {
		if (chr->aibot) {
			return TICKOP_NONE;
		}

		return TICKOP_NONE;
	}

	return TICKOP_NONE;
}

/**
 * Handle a token (briefcase) being dropped due to the holder being killed.
 *
 * For CTC, the token is warped back to its home base.
 */
void scenarioHandleDroppedToken(struct chrdata *chr, struct prop *prop)
{
	s32 i;
	struct weaponobj *weapon = prop->weapon;
	struct defaultobj *obj;
	struct pad pad;
	Mtxf mtx;
	RoomNum rooms[2];

	if (g_MpSetup.scenario == MPSCENARIO_CAPTURETHECASE) {
		for (i = 0; i < ARRAYCOUNT(g_ScenarioData.ctc.tokens); i++) {
			if (chr->prop == g_ScenarioData.ctc.tokens[i]) {
				objFreeProjectile(&weapon->base);

				g_ScenarioData.ctc.tokens[i] = prop;
				weapon->team = i;
				obj = prop->obj;

				padUnpack(g_ScenarioData.ctc.spawnpadsperteam[g_ScenarioData.ctc.teamindexes[i]].homepad,
						PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP | PADFIELD_ROOM, &pad);
				mtx00016d58(&mtx, 0, 0, 0, -pad.look.x, -pad.look.y, -pad.look.z, pad.up.x, pad.up.y, pad.up.z);

				if (obj->model) {
					mtx00015f04(obj->model->scale, &mtx);
				}

				rooms[0] = pad.room;
				rooms[1] = -1;

				func0f06a730(obj, &pad.pos, &mtx, rooms, &pad.pos);
			}
		}
	}
}

/**
 * Handle a player or bot picking up a data uplink, regardless of which scenario
 * it is.
 *
 * The return value is a TICKOP constant.
 */
s32 scenarioPickUpUplink(struct chrdata *chr, struct prop *prop)
{
	s32 i;
	char message[64];
	struct mpchrconfig *mpchr;
	u32 playernum;

	if (g_MpSetup.scenario == MPSCENARIO_HACKERCENTRAL) {
#if VERSION >= VERSION_NTSC_1_0
		struct defaultobj *obj = prop->obj;
#endif

		g_ScenarioData.htm.uplink = chr->prop;

		if (chr->aibot) {
			mpchr = g_MpAllChrConfigPtrs[mpPlayerGetIndex(chr)];
		} else {
			mpchr = MPCHR(g_Vars.playerstats[g_Vars.currentplayernum].mpindex);
		}

#if VERSION >= VERSION_JPN_FINAL
		// "%shas the\nData Uplink%s"
		sprintf(message, langGet(L_MPWEAPONS_000), scenarioRemoveLineBreaks(mpchr->name, 0));
#elif VERSION >= VERSION_PAL_BETA
		// "%shas the\nData Uplink%s"
		sprintf(message, langGet(L_MPWEAPONS_000), mpchr->name);
#else
		// "%shas the\n%s"
		sprintf(message, langGet(L_MPWEAPONS_000), mpchr->name, bgunGetShortName(WEAPON_DATAUPLINK));
#endif
		playernum = g_Vars.currentplayernum;

		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (chr->aibot || i != playernum) {
				setCurrentPlayerNum(i);

#if VERSION >= VERSION_JPN_FINAL
				hudmsgCreateWithFlags(message, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE | HUDMSGFLAG_NOWRAP);
#else
				hudmsgCreateWithFlags(message, HUDMSGTYPE_MPSCENARIO, HUDMSGFLAG_ONLYIFALIVE);
#endif
			}
		}

		setCurrentPlayerNum(playernum);

		if (chr->aibot) {
			propPlayPickupSound(prop, WEAPON_DATAUPLINK);
			botinvGiveSingleWeapon(chr, WEAPON_DATAUPLINK);
			chr->aibot->hasuplink = true;

#if VERSION >= VERSION_NTSC_1_0
			obj->hidden |= OBJHFLAG_DELETING;
#else
			prop->obj->hidden |= OBJHFLAG_DELETING;
#endif

			return TICKOP_NONE;
		} else {
			invGiveSingleWeapon(WEAPON_DATAUPLINK);
			currentPlayerQueuePickupWeaponHudmsg(WEAPON_DATAUPLINK, false);
			weaponPlayPickupSound(WEAPON_DATAUPLINK);

#if VERSION >= VERSION_NTSC_1_0
			objFree(obj, false, obj->hidden2 & OBJH2FLAG_CANREGEN);
			return TICKOP_FREE;
#else
			return TICKOP_GIVETOPLAYER;
#endif

		}
	} else if (chr->aibot) {
		return TICKOP_NONE;
	}

	return TICKOP_NONE;
}

/**
 * Handle a terminal being activated with the data uplink.
 */
void scenarioHandleActivatedProp(struct chrdata *chr, struct prop *prop)
{
	if (g_MpSetup.scenario == MPSCENARIO_HACKERCENTRAL) {
		struct defaultobj *obj = prop->obj;

		if (obj->flags3 & OBJFLAG3_HTMTERMINAL) {
			u32 mpindex = mpPlayerGetIndex(chr);

			if ((obj->hidden & OBJHFLAG_ACTIVATED_BY_BOND) == 0) {
				obj->hidden &= 0x0fffffff;
				obj->hidden |= (mpindex << 28) & 0xf0000000;
				obj->hidden |= OBJHFLAG_ACTIVATED_BY_BOND;
			}
		}
	}
}
