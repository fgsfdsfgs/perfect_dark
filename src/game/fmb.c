#include <ultra64.h>
#include "constants.h"
#include "game/menu.h"
#include "game/filemgr.h"
#include "game/fmb.h"
#include "game/mainmenu.h"
#include "game/challenge.h"
#include "game/playermgr.h"
#include "game/mplayer/mplayer.h"
#include "game/mplayer/scenarios.h"
#include "game/mplayer/setup.h"
#include "bss.h"
#include "data.h"
#include "types.h"

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

		playermgrDisableTeamPlayers();

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
		L_OPTIONS_100, // "Copy:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_103, // "Single Player Agent File"
		0,
		filemgrOpenCopyFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		L_OPTIONS_104, // "Combat Simulator Settings File"
		0,
		filemgrOpenCopyFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		L_OPTIONS_105, // "Combat Simulator Player File"
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
		L_OPTIONS_101, // "Delete:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_OPTIONS_103, // "Single Player Agent File"
		0,
		filemgrOpenDeleteFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		L_OPTIONS_104, // "Combat Simulator Settings File"
		0,
		filemgrOpenDeleteFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		L_OPTIONS_105, // "Combat Simulator Player File"
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
		L_OPTIONS_102, // "Delete Game Notes..."
		0,
		(void *)&g_PakChoosePakMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_GameFiles4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_099, // "Game Files"
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
		L_OPTIONS_096, // "Choose Your Reality"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x000000f5,
		0,
		filemgrChooseAgentListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileSelect4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_095, // "Perfect Dark"
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
		L_OPTIONS_308, // "Sound"
		L_MPMENU_000, // ""
		menuhandlerSfxVolume,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_SLIDER_FAST | MENUITEMFLAG_SLIDER_HIDEVALUE,
		L_OPTIONS_309, // "Music"
		L_MPMENU_000, // ""
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
		L_OPTIONS_310, // "Sound Mode"
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
		L_OPTIONS_311, // "Ratio"
		0,
		menuhandlerScreenRatio,
	},
#if PAL
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		L_MPWEAPONS_269, // ""
		0,
		menuhandlerLanguage,
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
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_312, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AudioVideo4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_307, // "Audio/Visual"
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
		L_MPMENU_030, // "Name"
		(uintptr_t)&mpGetCurrentPlayerName,
		(void *)&g_MpPlayerNameMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_033, // "Control"
		0,
		(void *)&g_MpControlMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_034, // "Player Options"
		0,
		(void *)&g_MpPlayerOptionsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_035, // "Statistics"
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
		L_MPMENU_029, // "Load Player"
		0,
		(void *)&g_MpLoadPlayerMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		(uintptr_t)&mpMenuTextSavePlayerOrCopy,
		0,
		menuhandlerMpSavePlayer,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPlayerSetup4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_028, // "Player Setup"
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
		L_MISC_456, // "Start Game"
		0,
		(void *)&g_MpReadyMenuDialog,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_029, // "Load Player"
		0,
		(void *)&g_MpLoadPlayerMenuDialog,
	},
#endif
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MISC_458, // "Player Settings"
		0,
		(void *)&g_MpPlayerSetup4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MISC_457, // "Drop Out"
		0,
		(void *)&g_MpDropOut4MbMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickGo4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MISC_460, // "Quick Go"
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
		0x0000007c,
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
		L_MPMENU_057, // "Accept"
		0,
		fmbHandleAcceptChallenge,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_058, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpConfirmChallenge4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&func0f17e318,
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
		0x00000078,
		0x0000004d,
		mpChallengesListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpChallenges4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_050, // "Combat Challenges"
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
		L_MISC_441, // "Challenges"
		0,
		(void *)&g_MpChallenges4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_MISC_442, // "Load/Preset Games"
		0x00000001,
		(void *)&g_MpLoadPresetMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_MISC_443, // "Quick Start"
		0x00000002,
		(void *)&g_MpQuickTeamMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_BIGFONT,
		L_MISC_444, // "Advanced Setup"
		0x00000003,
		fmbHandleAdvancedSetup,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_305, // "Audio/Video"
		0,
		(void *)&g_AudioVideo4MbMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_306, // "Change Agent"
		0,
		(void *)&g_ChangeAgentMenuDialog,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		L_OPTIONS_110, // "Exit Game"
		0x00000007,
		(void *)&g_ExitGameMenuDialog,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MainMenu4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_OPTIONS_302, // "Small but Perfect Menu"
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
		L_MPMENU_196, // "Are you sure you want to drop out?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPMENU_197, // "Drop Out"
		0,
		fmbHandleDropOut,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_198, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpDropOut4MbMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_195, // "Drop Out"
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
		L_MPMENU_053, // "Are you sure you want to abort the game?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPMENU_054, // "Abort"
		0,
		fmdHandleAbortGame,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_055, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_UnusedAbortMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_052, // "Abort"
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
		L_MPMENU_095, // "Difficulty:"
		0,
		mpBotDifficultyMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPMENU_096, // "Change Type..."
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
		L_MPMENU_098, // "Delete Simulant"
		0,
		menuhandlerMpDeleteSimulant,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_099, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpEditSimulant4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&mpMenuTitleEditSimulant,
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
		L_MPMENU_019, // "Scenario"
		(uintptr_t)&mpMenuTextScenarioShortName,
		(void *)&g_MpScenarioMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPMENU_021, // "Options"
		0,
		menuhandlerMpOpenOptions,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_020, // "Arena"
		(uintptr_t)&mpMenuTextArenaName,
		(void *)&g_MpArenaMenuDialog,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_044, // "Lock"
		0,
		menuhandlerMpLock,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_023, // "Weapons"
		0,
		(void *)&g_MpWeaponsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_024, // "Limits"
		0,
		(void *)&g_MpLimitsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPWEAPONS_184, // "Player Handicaps"
		0,
		(void *)&g_MpHandicapsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_025, // "Simulants"
		0,
		(void *)&g_MpSimulantsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_022, // "Teams"
		0,
		(void *)&g_MpTeamsMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		L_MPMENU_036, // "Start Game"
		0,
		(void *)&g_MpReadyMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		L_MPMENU_018, // "Load Settings"
		0,
		(void *)&g_MpLoadSettingsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		L_MPMENU_026, // "Save Settings"
		0,
		menuhandlerMpSaveSettings,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_AdvancedSetup4MbMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_017, // "Game Setup"
	g_AdvancedSetup4MbMenuItems,
	menudialogMpGameSetup,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpPlayerSetup4MbMenuDialog,
};
