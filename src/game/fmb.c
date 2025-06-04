#include <ultra64.h>
#include "constants.h"
#include "game/menu.h"
#include "game/filemgr.h"
#include "game/fmb.h"
#include "game/mainmenu.h"
#include "game/challenge.h"
#include "game/mplayer/mplayer.h"
#include "game/mplayer/scenarios.h"
#include "game/mplayer/setup.h"
#include "bss.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

MenuItemHandlerResult fmbHandleDropOut(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPopDialog();

		if (mpGetNumChrs() == 1) {
			func0f0f820c(&g_MainMenu4MbMenuDialog, MENUROOT_4MBMAINMENU);
		}
	}

	return 0;
}

MenuItemHandlerResult fmdHandleAbortGame(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_Vars.stagenum == STAGE_4MBMENU) {
			func0f0f820c(&g_MainMenu4MbMenuDialog, MENUROOT_4MBMAINMENU);
		} else {
			func0f0f820c(&g_SoloMissionPauseMenuDialog, MENUROOT_MAINMENU);
		}
	}

	return 0;
}

MenuItemHandlerResult fmbHandleAdvancedSetup(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f820c(&g_AdvancedSetup4MbMenuDialog, MENUROOT_4MBMAINMENU);
	}

	return 0;
}

MenuItemHandlerResult fmbHandleAcceptChallenge(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		challengeSetCurrentBySlot(g_Menus[g_MpPlayerNum].main4mb.slotindex);
		func0f0f820c(&g_MpQuickGo4MbMenuDialog, MENUROOT_4MBMAINMENU);
	}

	return 0;
}

void fmbReset(void)
{
	s32 i;
	u32 prevplayernum = g_MpPlayerNum;
	g_MpPlayerNum = 0;

	if (g_FileState != FILESTATE_UNSELECTED) {
		if (var80087260 == 0) {
			g_Vars.mpsetupmenu = MPSETUPMENU_GENERAL;
			menuPushRootDialog(&g_MainMenu4MbMenuDialog, MENUROOT_4MBMAINMENU);
		}
	} else {
		g_FileState = FILESTATE_SELECTED;

		for (i = 0; i != MAX_MPPLAYERCONFIGS; i++) {
			mpPlayerSetDefaults(i, true);
		}

		g_Vars.bondplayernum = 0;
		g_Vars.coopplayernum = -1;
		g_Vars.antiplayernum = -1;

		challengeDetermineUnlockedFeatures();

		menuPushRootDialog(&g_FilemgrFileSelect4MbMenuDialog, MENUROOT_4MBFILEMGR);

#if PAL
		if (g_Vars.language >= 6) {
			menuPushDialog(&g_ChooseLanguageMenuDialog);
		}
#endif
	}

	g_MpPlayerNum = prevplayernum;
}

MenuDialogHandlerResult fmbHandleMainMenu(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Vars.waitingtojoin[0] = false;
		g_Vars.waitingtojoin[1] = false;
		g_Vars.waitingtojoin[2] = false;
		g_Vars.waitingtojoin[3] = false;
	}

	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->definition == &g_MainMenu4MbMenuDialog
			&& operation == MENUOP_TICK) {
		g_Vars.mpsetupmenu = MPSETUPMENU_GENERAL;
		g_Vars.mpquickteam = MPQUICKTEAM_NONE;
		g_Vars.usingadvsetup = false;
		challengeUnsetCurrent();
		challengeRemovePlayerLock();
	}

	return false;
}

struct menuitem g_GameFiles4MbMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Copy:\n"), // "Copy:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Single Player Agent File\n"), // "Single Player Agent File"
		0,
		filemgrOpenCopyFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		gettext_noop("Combat Simulator Player File\n"), // "Combat Simulator Player File"
		0,
		filemgrOpenCopyFileMenuHandler,
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
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Delete:\n"), // "Delete:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Single Player Agent File\n"), // "Single Player Agent File"
		0,
		filemgrOpenDeleteFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		gettext_noop("Combat Simulator Player File\n"), // "Combat Simulator Player File"
		0,
		filemgrOpenDeleteFileMenuHandler,
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
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Delete Game Notes...\n"), // "Delete Game Notes..."
		0,
		(void *)&g_PakChoosePakMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_GameFiles4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Files\n"), // "Game Files"
	g_GameFiles4MbMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

struct menuitem g_FilemgrFileSelect4MbMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Choose Your Reality\n"), // "Choose Your Reality"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"",// previous: 0x000000f5,
		0,
		filemgrChooseAgentListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileSelect4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Perfect Dark\n"), // "Perfect Dark"
	g_FilemgrFileSelect4MbMenuItems,
	filemgrMainMenuDialog,
	MENUDIALOGFLAG_IGNOREBACK,
	&g_GameFiles4MbMenuDialog,
};

struct menuitem g_AudioVideo4MbMenuItems[] = {
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		gettext_noop("Sound\n"), // "Sound"
		0, // ""
		menuhandlerSfxVolume,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		gettext_noop("Music\n"), // "Music"
		0, // ""
		menuhandlerMusicVolume,
	},
#else
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		L_OPTIONS_308, // "Sound"
		0x7fff,
		menuhandlerSfxVolume,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		L_OPTIONS_309, // "Music"
		0x7fff,
		menuhandlerMusicVolume,
	},
#endif
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Sound Mode\n"), // "Sound Mode"
		0,
		menuhandlerSoundMode,
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
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Ratio\n"), // "Ratio"
		0,
		menuhandlerScreenRatio,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Language\n"), // ""
		0,
		//menuhandlerLanguage,
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
		gettext_noop("Back\n"), // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AudioVideo4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Audio/Visual\n"), // "Audio/Visual"
	g_AudioVideo4MbMenuItems,
	menudialog0010559c,
	0,
	NULL,
};

struct menuitem g_MpPlayerSetup4MbMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Name\n"), // "Name"
		(uintptr_t)&mpGetCurrentPlayerName,
		(void *)&g_MpPlayerNameMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Control\n"), // "Control"
		0,
		(void *)&g_MpControlMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Player Options\n"), // "Player Options"
		0,
		(void *)&g_MpPlayerOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Statistics\n"), // "Statistics"
		0,
		(void *)&g_MpPlayerStatsMenuDialog,
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
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Load Player\n"), // "Load Player"
		0,
		(void *)&g_MpLoadPlayerMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		&mpMenuTextSavePlayerOrCopy,
		0,
		menuhandlerMpSavePlayer,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPlayerSetup4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Setup\n"), // "Player Setup"
	g_MpPlayerSetup4MbMenuItems,
	NULL,
	MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpChallengeListOrDetailsMenuDialog,
};

struct menudialogdef g_MpDropOut4MbMenuDialog;

struct menuitem g_MpQuickGo4MbMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Start Game\n"), // "Start Game"
		0,
		(void *)&g_MpReadyMenuDialog,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Load Player\n"), // "Load Player"
		0,
		(void *)&g_MpLoadPlayerMenuDialog,
	},
#endif
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Player Settings\n"), // "Player Settings"
		0,
		(void *)&g_MpPlayerSetup4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Drop Out\n"), // "Drop Out"
		0,
		(void *)&g_MpDropOut4MbMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickGo4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Quick Go\n"), // "Quick Go"
	g_MpQuickGo4MbMenuItems,
	menudialogMpQuickGo,
	0,
	NULL,
};

struct menuitem g_MpConfirmChallenge4MbMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_MPCONFIG,
		0,
		"", // previous: 0x0000007c,
		(VERSION == VERSION_PAL_FINAL ? 65 : 55),
		NULL,
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
		0,
		gettext_noop("Accept\n"), // "Accept"
		0,
		fmbHandleAcceptChallenge,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpConfirmChallenge4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&func0f17e318,
	g_MpConfirmChallenge4MbMenuItems,
	menudialog0017e3fc,
	0,
	NULL,
};

struct menuitem g_MpChallenges4MbMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		1,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"",// previous: 0x00000078,
		0x0000004d,
		mpChallengesListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpChallenges4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Combat Challenges\n"), // "Combat Challenges"
	g_MpChallenges4MbMenuItems,
	mpCombatChallengesMenuDialog,
	0,
	NULL,
};

struct menuitem g_MainMenu4MbMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Challenges\n"), // "Challenges"
		0,
		(void *)&g_MpChallenges4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Load/Preset Games\n"), // "Load/Preset Games"
		0x00000001,
		(void *)&g_MpLoadPresetMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Quick Start\n"), // "Quick Start"
		0x00000002,
		(void *)&g_MpQuickTeamMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Advanced Setup\n"), // "Advanced Setup"
		0x00000003,
		fmbHandleAdvancedSetup,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Audio/Video\n"), // "Audio/Video"
		0,
		(void *)&g_AudioVideo4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Change Agent\n"), // "Change Agent"
		0,
		(void *)&g_ChangeAgentMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Exit\n"), // "Exit Game"
		0x00000007,
		(void *)&g_ExitGameMenuDialog,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MainMenu4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Small but Perfect Menu\n"), // "Small but Perfect Menu"
	g_MainMenu4MbMenuItems,
	fmbHandleMainMenu,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

struct menuitem g_MpDropOut4MbMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Are you sure you\nwant to drop out?\n"), // "Are you sure you want to drop out?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Drop Out\n"), // "Drop Out"
		0,
		fmbHandleDropOut,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpDropOut4MbMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Drop Out\n"), // "Drop Out"
	g_MpDropOut4MbMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_UnusedAbortMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Are you sure you want\nto abort the game?\n"), // "Are you sure you want to abort the game?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Abort\n"), // "Abort"
		0,
		fmdHandleAbortGame,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_UnusedAbortMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Abort\n"), // "Abort"
	g_UnusedAbortMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpEditSimulant4MbMenuItems[] = {
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Difficulty:\n"), // "Difficulty:"
		0,
		mpBotDifficultyMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Change Type...\n"), // "Change Type..."
		0,
		menuhandlerMpChangeSimulantType,
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
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Delete Simulant\n"), // "Delete Simulant"
		0,
		menuhandlerMpDeleteSimulant,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Back\n"), // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEditSimulant4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTitleEditSimulant,
	g_MpEditSimulant4MbMenuItems,
	menudialogMpSimulant,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_AdvancedSetup4MbMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Scenario\n"), // "Scenario"
		(uintptr_t)&mpMenuTextScenarioShortName,
		(void *)&g_MpScenarioMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Options\n"), // "Options"
		0,
		menuhandlerMpOpenOptions,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Arena\n"), // "Arena"
		(uintptr_t)&mpMenuTextArenaName,
		(void *)&g_MpArenaMenuDialog,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Lock\n"), // "Lock"
		0,
		menuhandlerMpLock,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Weapons\n"), // "Weapons"
		0,
		(void *)&g_MpWeaponsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Limits\n"), // "Limits"
		0,
		(void *)&g_MpLimitsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Player Handicaps\n"), // "Player Handicaps"
		0,
		(void *)&g_MpHandicapsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Simulants\n"), // "Simulants"
		0,
		(void *)&g_MpSimulantsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Teams\n"), // "Teams"
		0,
		(void *)&g_MpTeamsMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Start Game\n"), // "Start Game"
		0,
		(void *)&g_MpReadyMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Load Settings\n"), // "Load Settings"
		0,
		(void *)&g_MpLoadSettingsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Save Settings\n"), // "Save Settings"
		0,
		menuhandlerMpSaveSettings,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AdvancedSetup4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Setup\n"), // "Game Setup"
	g_AdvancedSetup4MbMenuItems,
	menudialogMpGameSetup,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpPlayerSetup4MbMenuDialog,
};
