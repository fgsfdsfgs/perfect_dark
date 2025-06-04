#include <ultra64.h>
#include "constants.h"
#include "game/game_006900.h"
#include "game/title.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/savebuffer.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/filemgr.h"
#include "game/lv.h"
#include "game/mplayer/ingame.h"
#include "game/challenge.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/mplayer/setup.h"
#include "game/options.h"
#include "bss.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

struct menudialogdef g_MpEndscreenChallengeCompletedMenuDialog;
struct menudialogdef g_MpEndscreenIndGameOverMenuDialog;
struct menudialogdef g_MpEndscreenTeamGameOverMenuDialog;

#if VERSION >= VERSION_NTSC_1_0
struct menudialogdef g_MpEndscreenSavePlayerMenuDialog;
#endif

MenuItemHandlerResult mpStatsForPlayerDropdownHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct mpchrconfig *mpchr;
	s32 v0;
	s32 v1;
	s32 a1;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = 0;

		for (v0 = 0; v0 < MAX_MPCHRS; v0++) {
			if (g_MpSetup.chrslots & (1 << v0)) {
				data->list.value++;
			}
		}
		break;
	case MENUOP_GETOPTIONTEXT:
		v0 = 0;

		for (a1 = 0; a1 < MAX_MPCHRS; a1++) {
			if (g_MpSetup.chrslots & (1 << a1)) {
				mpchr = MPCHR(a1);

				if (v0 == data->list.value) {
					return (uintptr_t) mpchr->name;
				}

				v0++;
			}
		}

		return (uintptr_t) "";
	case MENUOP_SET:
		v0 = 0;

		for (a1 = 0; a1 < MAX_MPCHRS; a1++) {
			if (g_MpSetup.chrslots & (1 << a1)) {
				if (v0);

				if (data->list.value == v0) {
					g_MpSelectedPlayersForStats[g_MpPlayerNum] = a1;
				}

				v0++;
			}
		}

		break;
	case MENUOP_GETSELECTEDINDEX:
		v0 = 0;

		for (v1 = 0; v1 < MAX_MPCHRS; v1++) {
			if (g_MpSetup.chrslots & (1 << v1)) {
				if (v0);

				if (g_MpSelectedPlayersForStats[g_MpPlayerNum] == v1) {
					data->list.value = v0;
				}

				v0++;
			}
		}

		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpEndGame(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Vars.currentplayer->aborted = true;
		mainEndStage();
	}

	return 0;
}

/**
 * This is something near the top of the "End Game" dialog during gameplay.
 */
MenuItemHandlerResult menuhandler00178018(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_BossFile.locktype != MPLOCKTYPE_CHALLENGE) {
			return true;
		}
	}

	return 0;
}

char *mpMenuTextInGameLimit(struct menuitem *item)
{
	*g_StringPointer = 0;

	switch (item->param) {
	case 0:
		sprintf(g_StringPointer, _("%d Min\n"), g_MpSetup.timelimit + 1);
		break;
	case 1:
		sprintf(g_StringPointer, _("%d\n"), g_MpSetup.scorelimit + 1);
		break;
	case 2:
		sprintf(g_StringPointer, _("%d\n"), mpCalculateTeamScoreLimit() + 1);
		break;
	}

	return g_StringPointer;
}

MenuItemHandlerResult menuhandlerMpInGameLimitLabel(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		switch (item->param) {
		case 0: if (g_MpSetup.timelimit == 60) return true; break;
		case 1: if (g_MpSetup.scorelimit == 100) return true; break;
		case 2: if (g_MpSetup.teamscorelimit == 400) return true; break;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpPause(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (mpIsPaused()) {
			mpSetPaused(MPPAUSEMODE_UNPAUSED);
		} else {
			mpSetPaused(MPPAUSEMODE_PAUSED);
		}
	}

	if (operation == MENUOP_CHECKHIDDEN) {
		if (PLAYERCOUNT() == 1) {
			return true;
		}
	}

	if (operation == MENUOP_CHECKPREFOCUSED) {
		if (item->param == 1) {
			return true;
		}
	}

	return 0;
}

char *menutextPauseOrUnpause(s32 arg0)
{
	if (mpIsPaused()) {
		return _("Unpause\n"); // "Unpause"
	}

	return _("Pause\n"); // "Pause"
}

char *menutextMatchTime(s32 arg0)
{
#if PAL
	formatTime(g_StringPointer, lvGetStageTime60() * 60 / 50, TIMEPRECISION_SECONDS);
#else
	formatTime(g_StringPointer, lvGetStageTime60(), TIMEPRECISION_SECONDS);
#endif

	return g_StringPointer;
}

struct menuitem g_MpEndGameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		gettext_noop("Are you sure?\n"), // "Are you sure?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		gettext_noop("_"),// previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("End Game\n"), // "End Game"
		0,
		menuhandlerMpEndGame,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEndGameMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("End Game\n"), // "End Game"
	g_MpEndGameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpPauseControlMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		&mpMenuTextChallengeName,
		0,
		menuhandler00178018,
	},
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		&mpMenuTextScenarioName,
		0,
		NULL,
	},
#endif
	{
		MENUITEMTYPE_LABEL,
		0,
#if VERSION == VERSION_JPN_FINAL
		MENUITEMFLAG_LESSLEFTPADDING,
#else
		MENUITEMFLAG_SMALLFONT,
#endif
		gettext_noop("Time Limit:\n"), // "Time Limit:"
		(uintptr_t) &mpMenuTextInGameLimit,
		menuhandlerMpInGameLimitLabel,
	},
	{
		MENUITEMTYPE_LABEL,
		1,
#if VERSION == VERSION_JPN_FINAL
		MENUITEMFLAG_LESSLEFTPADDING,
#else
		MENUITEMFLAG_SMALLFONT,
#endif
		gettext_noop("Score Limit:\n"), // "Score Limit:"
		(uintptr_t) &mpMenuTextInGameLimit,
		menuhandlerMpInGameLimitLabel,
	},
	{
		MENUITEMTYPE_LABEL,
		2,
#if VERSION == VERSION_JPN_FINAL
		MENUITEMFLAG_LESSLEFTPADDING,
#else
		MENUITEMFLAG_SMALLFONT,
#endif
		gettext_noop("Team Score Limit:\n"), // "Team Score Limit:"
		(uintptr_t) &mpMenuTextInGameLimit,
		menuhandlerMpInGameLimitLabel,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		gettext_noop("_"), // previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
#if VERSION == VERSION_JPN_FINAL
		MENUITEMFLAG_LESSLEFTPADDING,
#else
		0,
#endif
		gettext_noop("Game Time:\n"), // "Game Time:"
		(uintptr_t)&menutextMatchTime,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		&menutextPauseOrUnpause,
		0,
		menuhandlerMpPause,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("End Game\n"), // "End Game"
		0,
		(void *)&g_MpEndGameMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPauseControlMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
#if VERSION >= VERSION_JPN_FINAL
	(uintptr_t)&mpMenuTextScenarioName,
#else
	gettext_noop("Control\n"), // "Control"
#endif
	g_MpPauseControlMenuItems,
	NULL,
	0,
	NULL,
};

#if VERSION >= VERSION_JPN_FINAL
char *mpMenuTextWeaponDescription(struct menuitem *item)
{
	struct weapon *weapondef = weaponFindById(g_Menus[g_MpPlayerNum].training.weaponnum);

	if (weapondef != NULL) {
		if (g_Menus[g_MpPlayerNum].training.weaponnum == WEAPON_EYESPY) {
			if (g_Vars.currentplayer->eyespy != NULL) {
				if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
					return langGet(L_GUN_237);
				}

				if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_BOMBSPY) {
					return langGet(L_GUN_236);
				}
			}
		}

		if (g_Menus[g_MpPlayerNum].training.weaponnum == WEAPON_NECKLACE && g_Vars.stagenum == STAGE_ATTACKSHIP) {
			if (lvGetDifficulty() >= DIFF_PA) {
				u8 username[] = {
					'C' + 9 * 1,
					'D' + 9 * 2,
					'V' + 9 * 3,
					'7' + 9 * 4,
					'8' + 9 * 5,
					'0' + 9 * 6,
					'3' + 9 * 7,
					'2' + 9 * 8,
					'2' + 9 * 9,
					'\0' + 9 * 10,
				};

				u8 password[] = {
					'I' + 4 * 1,
					'8' + 4 * 2,
					'M' + 4 * 3,
					'O' + 4 * 4,
					'Z' + 4 * 5,
					'Y' + 4 * 6,
					'M' + 4 * 7,
					'8' + 4 * 8,
					'N' + 4 * 9,
					'D' + 4 * 10,
					'I' + 4 * 11,
					'8' + 4 * 12,
					'5' + 4 * 13,
					'\0' + 4 * 14,
				};

				s32 i;

				for (i = 0; i < ARRAYCOUNT(username); i++) {
					username[i] -= i * 9 + 9;
				}

				for (i = 0; i < ARRAYCOUNT(password); i++) {
					password[i] -= i * 4 + 4;
				}

				sprintf(g_StringPointer, langGet(L_GUN_239), username, password);

				return g_StringPointer;
			}
		}

		return langGet(weapondef->description);
	}

	return langGet(L_OPTIONS_003); // ""
}
#else
char *mpMenuTextWeaponDescription(struct menuitem *item)
{
	struct weapon *weapon = weaponFindById(g_Menus[g_MpPlayerNum].mppause.weaponnum);

	if (weapon) {
		return langGet(weapon->description);
	}

	return "\n";
}
#endif

char *mpMenuTitleStatsFor(struct menudialogdef *dialogdef)
{
	struct mpchrconfig *mpchr = MPCHR(g_MpSelectedPlayersForStats[g_MpPlayerNum]);

	// "Stats for %s"
	sprintf(g_StringPointer, _("Stats for %s"), mpchr->name);
	return g_StringPointer;
}

MenuItemHandlerResult func0f178440(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation);

	return 0;
}

char *mpMenuTextWeaponOfChoiceName(struct menuitem *item)
{
	return mpPlayerGetWeaponOfChoiceName(g_Menus[g_MpPlayerNum].playernum, 0);
}

char *mpMenuTextAward1(struct menuitem *item)
{
	return g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->award1;
}

char *mpMenuTextAward2(struct menuitem *item)
{
	return g_Vars.players[g_Menus[g_MpPlayerNum].playernum]->award2;
}

struct menuitem g_Mp2PMissionInventoryMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		gettext_noop("_"), // previous: 0x00000078,
		0x00000042,
		menuhandlerInventoryList,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&mpMenuTextWeaponDescription,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPauseInventoryMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Inventory\n"), // "Inventory"
	g_Mp2PMissionInventoryMenuItems,
	NULL,
	0,
	&g_MpPauseControlMenuDialog,
};

struct menudialogdef g_2PMissionInventoryHMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Inventory\n"), // "Inventory"
	g_Mp2PMissionInventoryMenuItems,
	NULL,
	0,
	&g_2PMissionOptionsHMenuDialog,
};

struct menudialogdef g_2PMissionInventoryVMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Inventory\n"), // "Inventory"
	g_Mp2PMissionInventoryMenuItems,
	NULL,
	0,
	&g_2PMissionOptionsVMenuDialog,
};

struct menuitem g_MpInGamePlayerStatsMenuItems[] = {
	{
		MENUITEMTYPE_PLAYERSTATS,
		0,
		0,
		0,
		0,
		mpStatsForPlayerDropdownHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPausePlayerStatsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTitleStatsFor,
	g_MpInGamePlayerStatsMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	&g_MpPauseInventoryMenuDialog,
};

struct menudialogdef g_MpEndscreenPlayerStatsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTitleStatsFor,
	g_MpInGamePlayerStatsMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	NULL,
};

struct menuitem g_MpPlayerRankingMenuItems[] = {
	{
		MENUITEMTYPE_RANKING,
		0,
		0,
		0,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPausePlayerRankingMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Ranking\n"), // "Player Ranking"
	g_MpPlayerRankingMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	&g_MpPausePlayerStatsMenuDialog,
};

struct menudialogdef g_MpEndscreenPlayerRankingMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Ranking\n"), // "Player Ranking"
	g_MpPlayerRankingMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	&g_MpEndscreenPlayerStatsMenuDialog,
};

struct menuitem g_MpTeamRankingsMenuItems[] = {
	{
		MENUITEMTYPE_RANKING,
		0,
		0,
		gettext_noop("_"), // previous: 0x00000001,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPauseTeamRankingsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Team Ranking\n"), // "Team Ranking"
	g_MpTeamRankingsMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	&g_MpPausePlayerRankingMenuDialog,
};

struct menudialogdef g_MpEndscreenTeamRankingMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Team Ranking\n"), // "Team Ranking"
	g_MpTeamRankingsMenuItems,
	NULL,
	VERSION >= VERSION_JPN_FINAL ? MENUDIALOGFLAG_1000 : 0,
	&g_MpEndscreenPlayerRankingMenuDialog,
};

char *mpMenuTextPlacementWithSuffix(struct menuitem *item)
{
	switch (g_PlayerConfigsArray[g_MpPlayerNum].base.placement) {
		case 0:
			return _("1st\n");
			break;
		case 1:
			return _("2nd\n");
			break;
		case 2:
			return _("3rd\n");
			break;
		case 3:
			return _("4th\n");
			break;
		case 4:
			return _("5th\n");
			break;
		case 5:
			return _("6th\n");
			break;
		case 6:
			return _("7th\n");
			break;
		case 7:
			return _("8th\n");
			break;
		case 8:
			return _("9th\n");
			break;
		case 9:
			return _("10th\n");
			break;
		case 10:
			return _("11th\n");
			break;
		case 11:
			return _("12th\n");
			break;
		default:
			return _("Unknown classement\n");
			break;
	}
}

MenuItemHandlerResult mpPlacementMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_GETCOLOUR) {
		if (g_PlayerConfigsArray[g_MpPlayerNum].base.placement == 0) { // winner
			data->label.colour2 = colourBlend(data->label.colour2, 0xffff00ff, menuGetSinOscFrac(40) * 255);
		}
	}

	return 0;
}

MenuItemHandlerResult mpAwardsMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		bool configured = false;
		s32 x = renderdata->x + renderdata->width - 15;
		s32 i;
		u32 colour;

		for (i = 0; i < MAX_PLAYERS; i++) {
			if (g_PlayerConfigsArray[g_MpPlayerNum].medals & (1 << i)) {
				switch (i) {
				case 0: colour = 0xff7f7fff; break; // killmaster - red
				case 1: colour = 0xbfbf00ff; break; // headshot - yellow
				case 2: colour = 0x00ff00ff; break; // accuracy - green
				case 3: colour = 0x00bfbfff; break; // survivor - blue
				}

				if (!configured) {
					gDPPipeSync(gdl++);
					gDPSetTexturePersp(gdl++, G_TP_NONE);
					gDPSetAlphaCompare(gdl++, G_AC_NONE);
					gDPSetTextureLOD(gdl++, G_TL_TILE);
					gDPSetTextureConvert(gdl++, G_TC_FILT);
					gDPSetTextureFilter(gdl++, G_TF_POINT);

					texSelect(&gdl, &g_TexGeneralConfigs[35], 2, 0, 2, 1, NULL);

					gDPSetCycleType(gdl++, G_CYC_1CYCLE);
					gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
					gDPSetTextureFilter(gdl++, G_TF_POINT);

					gDPSetCombineLERP(gdl++,
							TEXEL0, 0, ENVIRONMENT, 0,
							TEXEL0, 0, ENVIRONMENT, 0,
							TEXEL0, 0, ENVIRONMENT, 0,
							TEXEL0, 0, ENVIRONMENT, 0);

					configured = true;
				}

				gDPSetEnvColorViaWord(gdl++, colour);

#if VERSION == VERSION_JPN_FINAL
				gSPTextureRectangle(gdl++,
						(x << 2) * g_ScaleX,
						(renderdata->y - 6) << 2,
						((x + 11) << 2) * g_ScaleX,
						(renderdata->y + 5) << 2,
						G_TX_RENDERTILE, 0x0010, 0x0150, 1024 / g_ScaleX, -1024);
#else
				gSPTextureRectangle(gdl++,
						(x << 2) * g_ScaleX,
						(renderdata->y - 2) << 2,
						((x + 11) << 2) * g_ScaleX,
						(renderdata->y + 9) << 2,
						G_TX_RENDERTILE, 0x0010, 0x0150, 1024 / g_ScaleX, -1024);
#endif

				x -= 14;
			}
		}

		return (uintptr_t) gdl;
	}

	return 0;
}

MenuItemHandlerResult mpPlayerTitleMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_GETCOLOUR) {
		if (g_PlayerConfigsArray[g_MpPlayerNum].title != g_PlayerConfigsArray[g_MpPlayerNum].newtitle) {
			data->label.colour2 = colourBlend(data->label.colour2, 0xffff00ff, menuGetSinOscFrac(40) * 255);
		}
	}

	return 0;
}

char *mpMenuTextPlayerTitle(s32 arg0)
{
	switch (g_PlayerConfigsArray[g_MpPlayerNum].title) {
		case 0:
			return _("Beginner:21\n");
			break;
		case 1:
			return _("Trainee:20\n");
			break;
		case 2:
			return _("Amateur:19\n");
			break;
		case 3:
			return _("Rookie:18\n");
			break;
		case 4:
			return _("Novice:17\n");
			break;
		case 5:
			return _("Trooper:16\n");
			break;
		case 6:
			return _("Agent:15\n");
			break;
		case 7:
			return _("Star Agent:14\n");
			break;
		case 8:
			return _("Special Agent:13\n");
			break;
		case 9:
			return _("Expert:12\n");
			break;
		case 10:
			return _("Veteran:11\n");
			break;
		case 11:
			return _("Professional:10\n");
			break;
		case 12:
			return _("Dangerous:9\n");
			break;
		case 13:
			return _("Deadly:8\n");
			break;
		case 14:
			return _("Killer:7\n");
			break;
		case 15:
			return _("Assassin:6\n");
			break;
		case 16:
			return _("Lethal:5\n");
			break;
		case 17:
			return _("Elite:4\n");
			break;
		case 18:
			return _("Invincible:3\n");
			break;
		case 19:
			return _("Near Perfect:2\n");
			break;
		case 20:
			return _("Perfect:1\n");
			break;
		default:
			return _("Unknown rank");
			break;
	}
}

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult mpConfirmPlayerNameHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;
	s32 i;

	switch (operation) {
	case MENUOP_GETTEXT:
		i = 0;

		while (g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\n'
				&& g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\0'
				&& i <= 10) {
			name[i] = g_PlayerConfigsArray[g_MpPlayerNum].base.name[i];
			i++;
		}

		while (i <= 10) {
			name[i] = '\0';
			i++;
		}
		break;
	case MENUOP_SETTEXT:
		i = 0;

		while (i <= 10 && name[i] != '\0') {
			g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = name[i];
			i++;
		}

		g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = '\n';
		i++;

		while (i <= 10) {
			g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = '\0';
			i++;
		}
		break;
	case MENUOP_SET:
		filemgrPushSelectLocationDialog(6, FILETYPE_MPPLAYER);
		break;
	}

	return 0;
}
#endif

void mpPushPauseDialog(void)
{
	u32 prevplayernum = g_MpPlayerNum;

#if VERSION >= VERSION_NTSC_1_0
	if (g_MpSetup.paused != MPPAUSEMODE_GAMEOVER && g_MainIsEndscreen == 0)
#endif
	{
		g_MpPlayerNum = g_Vars.currentplayerstats->mpindex;

		if (g_Menus[g_MpPlayerNum].openinhibit == 0) {
			g_Menus[g_MpPlayerNum].playernum = g_Vars.currentplayernum;

			if (g_Vars.normmplayerisrunning) {
				if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
					menuPushRootDialog(&g_MpPauseTeamRankingsMenuDialog, MENUROOT_MPPAUSE);
				} else {
					menuPushRootDialog(&g_MpPausePlayerRankingMenuDialog, MENUROOT_MPPAUSE);
				}
			} else {
				if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
					menuPushRootDialog(&g_2PMissionPauseVMenuDialog, MENUROOT_MPPAUSE);
				} else {
					menuPushRootDialog(&g_2PMissionPauseHMenuDialog, MENUROOT_MPPAUSE);
				}
			}
		}

		g_MpPlayerNum = prevplayernum;
	}
}

void mpPushEndscreenDialog(u32 arg0, u32 playernum)
{
	u32 prevplayernum = g_MpPlayerNum;
	g_MpPlayerNum = playernum;

	g_Menus[g_MpPlayerNum].playernum = arg0;

	if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
		if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
			if (g_CheatsActiveBank0 || g_CheatsActiveBank1) {
				menuPushRootDialog(&g_MpEndscreenChallengeCheatedMenuDialog, MENUROOT_MPENDSCREEN);
			} else if (challengeIsCompleteForEndscreen()) {
				menuPushRootDialog(&g_MpEndscreenChallengeCompletedMenuDialog, MENUROOT_MPENDSCREEN);
			} else {
				menuPushRootDialog(&g_MpEndscreenChallengeFailedMenuDialog, MENUROOT_MPENDSCREEN);
			}
		} else {
			menuPushRootDialog(&g_MpEndscreenTeamGameOverMenuDialog, MENUROOT_MPENDSCREEN);
		}
	} else {
		menuPushRootDialog(&g_MpEndscreenIndGameOverMenuDialog, MENUROOT_MPENDSCREEN);
	}

#if VERSION >= VERSION_NTSC_1_0
#if VERSION >= VERSION_JPN_FINAL
	if (IS8MB())
#endif
	{
		if ((g_PlayerConfigsArray[g_MpPlayerNum].options & OPTION_ASKEDSAVEPLAYER) == 0
				&& g_PlayerConfigsArray[g_MpPlayerNum].fileguid.fileid == 0
				&& g_PlayerConfigsArray[g_MpPlayerNum].fileguid.deviceserial == 0) {
			g_PlayerConfigsArray[g_MpPlayerNum].options |= OPTION_ASKEDSAVEPLAYER;
			menuPushDialog(&g_MpEndscreenSavePlayerMenuDialog);
		}
	}
#endif

	g_MpPlayerNum = prevplayernum;
}

struct menuitem g_MpGameOverMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		&mpGetCurrentPlayerName,
		(uintptr_t)&mpMenuTextPlacementWithSuffix,
		mpPlacementMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
#if VERSION >= VERSION_JPN_FINAL
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		(uintptr_t)&mpMenuTextPlayerTitle,
		0,
#else
		MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		gettext_noop("Title:\n"), // "Title:"
		(uintptr_t)&mpMenuTextPlayerTitle,
#endif
		mpPlayerTitleMenuHandler,
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
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		gettext_noop("Weapon of Choice:\n"), // "Weapon of Choice:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_ALTCOLOUR,
		&mpMenuTextWeaponOfChoiceName,
		0,
		NULL,
	},
#if VERSION >= VERSION_JPN_FINAL
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002,
		0x00000001,
		0x00000003,
		NULL,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
#if VERSION >= VERSION_JPN_FINAL
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x00000001,
		0x00000002,
		mpAwardsMenuHandler,
	},
#else
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_LIST_CUSTOMRENDER,
		gettext_noop("Awards:\n"), // "Awards:"
		0,
		mpAwardsMenuHandler,
	},
#endif
	{
		MENUITEMTYPE_LABEL,
		0,
#if VERSION >= VERSION_JPN_FINAL
		MENUITEMFLAG_LABEL_ALTCOLOUR,
#else
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_ALTCOLOUR,
#endif
		&mpMenuTextAward1,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
#if VERSION >= VERSION_JPN_FINAL
		MENUITEMFLAG_LABEL_ALTCOLOUR,
#else
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LABEL_ALTCOLOUR,
#endif
		&mpMenuTextAward2,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEndscreenIndGameOverMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Over\n"), // "Game Over"
	g_MpGameOverMenuItems,
	NULL,
	0,
	&g_MpEndscreenPlayerRankingMenuDialog,
};

struct menudialogdef g_MpEndscreenTeamGameOverMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Over\n"), // "Game Over"
	g_MpGameOverMenuItems,
	NULL,
	0,
	&g_MpEndscreenTeamRankingMenuDialog,
};

struct menudialogdef g_MpEndscreenChallengeCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	gettext_noop("Challenge Completed!\n"), // "Challenge Completed!"
	g_MpTeamRankingsMenuItems,
	NULL,
	0,
	&g_MpEndscreenIndGameOverMenuDialog,
};

struct menudialogdef g_MpEndscreenChallengeCheatedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Challenge Cheated!\n"), // "Challenge Cheated!"
	g_MpTeamRankingsMenuItems,
	NULL,
	0,
	&g_MpEndscreenIndGameOverMenuDialog,
};

struct menudialogdef g_MpEndscreenChallengeFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Challenge Failed!\n"), // "Challenge Failed!"
	g_MpTeamRankingsMenuItems,
	NULL,
	0,
	&g_MpEndscreenIndGameOverMenuDialog,
};

#if VERSION >= VERSION_NTSC_1_0
struct menuitem g_MpEndscreenConfirmNameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Confirm player name:\n"), // "Confirm player name:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_KEYBOARD,
		0,
		0,
		0,
		0,
		mpConfirmPlayerNameHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEndscreenConfirmNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Name\n"), // "Player Name"
	g_MpEndscreenConfirmNameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpEndscreenSavePlayerMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Save new player\nand statistics?\n"), // "Save new player and statistics?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Save Now\n"), // "Save Now"
		0,
		(void *)&g_MpEndscreenConfirmNameMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("No Thanks!\n"), // "No Thanks!"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEndscreenSavePlayerMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Save Player\n"), // "Save Player"
	g_MpEndscreenSavePlayerMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};
#endif
