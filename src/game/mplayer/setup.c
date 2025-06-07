#include <stdint.h>
#include <ultra64.h>
#include "constants.h"
#include "game/camdraw.h"
#include "game/tex.h"
#include "game/savebuffer.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/filemgr.h"
#include "game/game_1531a0.h"
#include "game/music.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/setup.h"
#include "game/mplayer/scenarios.h"
#include "game/challenge.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/vi.h"
#include "lib/rng.h"
#include "lib/str.h"
#include "lib/joy.h"
#include "data.h"
#include "gbiex.h"
#include "types.h"
#include "system.h"
#include "input.h"
#include "mpsetups.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

struct menuitem g_MpCharacterMenuItems[];
struct menudialogdef g_MpAddSimulantMenuDialog;
struct menudialogdef g_MpChangeSimulantMenuDialog;
struct menudialogdef g_MpChangeTeamNameMenuDialog;
struct menudialogdef g_MpEditSimulantMenuDialog;
struct menudialogdef g_MpSaveSetupNameMenuDialog;

extern struct menudialogdef g_ManageSettingsDialog;
extern struct menudialogdef g_FilemgrFileSavedMenuDialog;
extern struct menudialogdef g_FilemgrErrorMenuDialog;

#ifndef PLATFORM_N64
extern s32 g_MpWeaponSetNum;
#endif

MenuItemHandlerResult menuhandlerMpDropOut(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPopDialog();
	}

	return 0;
}

char *mpGetCurrentPlayerName(struct menuitem *item)
{
	return g_PlayerConfigsArray[g_MpPlayerNum].base.name;
}

MenuItemHandlerResult menuhandlerMpTeamsLabel(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) == 0) {
			return true;
		}
	}

	return 0;
}

struct menuitem g_MpDropOutMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop(""), // "Are you sure you want to drop out?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Drop Out\n"), // "Drop Out"
		0,
		menuhandlerMpDropOut,
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

struct menudialogdef g_MpDropOutMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Drop Out\n"), // "Drop Out"
	g_MpDropOutMenuItems,
	NULL,
	0,
	NULL,
};

struct mparena g_MpArenas[] = {
	// Stage, unlock, name
	{ STAGE_MP_SKEDAR,     0,                          gettext_noop("Skedar") },
	{ STAGE_MP_PIPES,      0,                          gettext_noop("Pipes") },
	{ STAGE_MP_RAVINE,     MPFEATURE_STAGE_RAVINE,     gettext_noop("Ravine") },
	{ STAGE_MP_G5BUILDING, MPFEATURE_STAGE_G5BUILDING, gettext_noop("G5 Building") },
	{ STAGE_MP_SEWERS,     MPFEATURE_STAGE_SEWERS,     gettext_noop("Sewers") },
	{ STAGE_MP_WAREHOUSE,  MPFEATURE_STAGE_WAREHOUSE,  gettext_noop("Warehouse") },
	{ STAGE_MP_GRID,       MPFEATURE_STAGE_GRID,       gettext_noop("Grid") },
	{ STAGE_MP_RUINS,      MPFEATURE_STAGE_RUINS,      gettext_noop("Ruins") },
	{ STAGE_MP_AREA52,     0,                          gettext_noop("Area 52") },
	{ STAGE_MP_BASE,       MPFEATURE_STAGE_BASE,       gettext_noop("Base") },
	{ STAGE_MP_FORTRESS,   MPFEATURE_STAGE_FORTRESS,   gettext_noop("Fortress") },
	{ STAGE_MP_VILLA,      MPFEATURE_STAGE_VILLA,      gettext_noop("Villa") },
	{ STAGE_MP_CARPARK,    MPFEATURE_STAGE_CARPARK,    gettext_noop("Car Park") },
	{ STAGE_MP_TEMPLE,     MPFEATURE_STAGE_TEMPLE,     gettext_noop("Temple") },
	{ STAGE_MP_COMPLEX,    MPFEATURE_STAGE_COMPLEX,    gettext_noop("Complex") },
	{ STAGE_MP_FELICITY,   MPFEATURE_STAGE_FELICITY,   gettext_noop("Felicity") },
	{ 1,                   0,                          gettext_noop("Random") }, // "Random"
};

s32 mpGetNumStages(void)
{
	return 17;
}

s16 mpChooseRandomStage(void)
{
	s32 i;
	s32 numchallengescomplete = 0;
	s32 index;

	for (i = 0; i < 16; i++) {
		if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
			numchallengescomplete++;
		}
	}

	index = rngRandom() % numchallengescomplete;

	for (i = 0; i < 16; i++) {
		if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
			if (index == 0) {
				return g_MpArenas[i].stagenum;
			}

			index--;
		}
	}

	return STAGE_MP_SKEDAR;
}

MenuItemHandlerResult mpArenaMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct optiongroup groups[] = {
		{ 0,  gettext_noop("Dark\n") }, // "Dark"
		{ 13, gettext_noop("Classic\n") }, // "Classic"
		{ 16, gettext_noop("Random\n") }, // "Random"
	};

	s32 i;
	s32 count = 0;
	s32 groupindex;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		for (i = 0; i < ARRAYCOUNT(g_MpArenas); i++) {
			if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
				count++;
			}
		}

		data->list.value = count;
		break;
	case MENUOP_GETOPTIONTEXT:
		for (i = 0; i < ARRAYCOUNT(g_MpArenas); i++) {
			if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
				if (count == data->list.value) {
					return (uintptr_t)_(g_MpArenas[i].name);
				}

				count++;
			}
		}
		break;
	case MENUOP_SET:
		for (i = 0; i < ARRAYCOUNT(g_MpArenas); i++) {
			if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
				if (count == data->list.value) {
					break;
				}

				count++;
			}
		}

		g_MpSetup.stagenum = g_MpArenas[i].stagenum;
		break;
	case MENUOP_GETSELECTEDINDEX:
		for (i = 0; i < ARRAYCOUNT(g_MpArenas); i++) {
			if (g_MpSetup.stagenum == g_MpArenas[i].stagenum) {
				data->list.value = count;
			}

			if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
				count++;
			}
		}
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 3;

		if (!challengeIsFeatureUnlocked(MPFEATURE_STAGE_COMPLEX)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_TEMPLE)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_FELICITY)) {
			data->list.value--;
		}
		break;
	case MENUOP_GETOPTGROUPTEXT:
		count = data->list.value;

		if (!challengeIsFeatureUnlocked(MPFEATURE_STAGE_COMPLEX)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_TEMPLE)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_FELICITY)
				&& count > 0) {
			count++;
		}
		return (uintptr_t)_(groups[count].name);
	case MENUOP_GETGROUPSTARTINDEX:
		groupindex = data->list.value;

		if (!challengeIsFeatureUnlocked(MPFEATURE_STAGE_COMPLEX)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_TEMPLE)
				&& !challengeIsFeatureUnlocked(MPFEATURE_STAGE_FELICITY)
				&& groupindex == 1) {
			groupindex++;
		}

		for (i = 0; i < groups[groupindex].offset; i++) {
			if (challengeIsFeatureUnlocked(g_MpArenas[i].requirefeature)) {
				count++;
			}
		}
		data->list.groupstartindex = count;
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpControlStyle(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *labels[] = {
		_("1.1"), // "1.1"
		_("1.2"), // "1.2"
		_("1.3"), // "1.3"
		_("1.4"), // "1.4"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 5;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (intptr_t) ((data->dropdown.value == 4) ? _("Ext") : labels[data->dropdown.value]);
	case MENUOP_SET:
		optionsSetControlMode(g_MpPlayerNum, (data->dropdown.value == 4 ? CONTROLMODE_PC : data->dropdown.value));
#ifndef PLATFORM_N64
		g_PlayerExtCfg[g_MpPlayerNum & 3].extcontrols = (data->dropdown.value == 4);
#endif
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetControlMode(g_MpPlayerNum);
		if (data->dropdown.value == CONTROLMODE_PC) {
			data->dropdown.value = 4;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpWeaponSlot(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = mpGetNumWeaponOptions();
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) mpGetWeaponLabel(data->dropdown.value);
	case MENUOP_SET:
		mpSetWeaponSlot(item->param3, data->dropdown.value);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = mpGetWeaponSlot(item->param3);
	}

	return 0;
}

char *mpMenuTextWeaponNameForSlot(struct menuitem *item)
{
	return mpGetWeaponLabel(mpGetWeaponSlot(item->param));
}

MenuItemHandlerResult menuhandlerMpWeaponSetDropdown(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = func0f189058(item->param);
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) mpGetWeaponSetName(data->dropdown.value);
	case MENUOP_SET:
		mpSetWeaponSet(data->dropdown.value);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = mpGetWeaponSet();
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpControlCheckbox(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 val;

	switch (operation) {
	case MENUOP_GET:
		if (item->param3 == OPTION_FORWARDPITCH) {
			if ((g_PlayerConfigsArray[g_MpPlayerNum].options & item->param3) == 0) {
				return true;
			}
			return false;
		}
		if ((g_PlayerConfigsArray[g_MpPlayerNum].options & item->param3) == 0) {
			return false;
		}
		return true;
	case MENUOP_SET:
		val = OPTION_FORWARDPITCH;

		if (item->param3 == val) {
			if (data->checkbox.value == 0) {
				data->checkbox.value = val;
			} else {
				data->checkbox.value = 0;
			}
		}

		g_PlayerConfigsArray[g_MpPlayerNum].options &= ~item->param3;

		if (data->checkbox.value) {
			g_PlayerConfigsArray[g_MpPlayerNum].options |= item->param3;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpAimControl(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *labels[] = {
#if VERSION >= VERSION_PAL_FINAL
		gettext_noop("Hold\n"), // "Hold"
		gettext_noop("Toggle\n"), // "Toggle"
#else
		_("Hold\n"), // "Hold"
		_("Toggle\n"), // "Toggle"
#endif
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) labels[data->dropdown.value];
	case MENUOP_SET:
		optionsSetAimControl(g_MpPlayerNum, data->dropdown.value);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = optionsGetAimControl(g_MpPlayerNum);
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpCheckboxOption(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		if ((g_MpSetup.options & item->param3) == 0) {
			return false;
		}
		return true;
	case MENUOP_SET:
		g_MpSetup.options = g_MpSetup.options & ~item->param3;
		if (data->checkbox.value) {
			g_MpSetup.options = g_MpSetup.options | item->param3;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpTeamsEnabled(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		if (g_MpSetup.scenario == MPSCENARIO_CAPTURETHECASE ||
				g_MpSetup.scenario == MPSCENARIO_KINGOFTHEHILL) {
			return true;
		}

		return false;
	}

	return menuhandlerMpCheckboxOption(operation, item, data);
}

MenuItemHandlerResult menuhandlerMpDisplayOptionCheckbox(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		if ((g_PlayerConfigsArray[g_MpPlayerNum].base.displayoptions & item->param3) == 0) {
			return false;
		}
		return true;
	case MENUOP_SET:
		g_PlayerConfigsArray[g_MpPlayerNum].base.displayoptions &= ~(u8)item->param3;

		if (data->checkbox.value) {
			g_PlayerConfigsArray[g_MpPlayerNum].base.displayoptions |= (u8)item->param3;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpConfirmSaveChr(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		filemgrPushSelectLocationDialog(6, FILETYPE_MPPLAYER);
	}

	return 0;
}

extern struct menudialogdef g_StatusErrorDialog;
MenuItemHandlerResult menuhandlerMpSetupName(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;
	s32 err;

	switch (operation) {
	case MENUOP_GETTEXT:
		strcpy(name, g_MpSetup.name);
		break;
	case MENUOP_SETTEXT:
		strcpy(g_MpSetup.name, name);
		break;
	case MENUOP_SET:
		err = mpsetupSaveSetup(g_MpSetupFile.numsetups, true);
		if (!err) {
			menuPushDialog(&g_FilemgrFileSavedMenuDialog);
			g_MpCurrentSetup = g_MpSetupFile.numsetups - 1;
		}
		else {
			menuPushDialog(&g_StatusErrorDialog);
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSaveSetupOverwrite(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		mpsetupSaveSetup(g_MpCurrentSetup, true);
		menuPushDialog(&g_FilemgrFileSavedMenuDialog);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSaveSetupCopy(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPushDialog(&g_MpSaveSetupNameMenuDialog);
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
char *mpMenuTextSetupName(struct menuitem *item)
{
	return g_MpSetup.name;
}
#endif

MenuItemHandlerResult func0f179b68(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = g_PlayerConfigsArray[g_MpPlayerNum].base.unk18;
		break;
	case MENUOP_SET:
		g_PlayerConfigsArray[g_MpPlayerNum].base.unk18 = (u8) data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		sprintf(data->slider.label, "%d%%\n", data->slider.value + 20);
		break;
	}

	return 0;
}

MenuItemHandlerResult func0f179c14(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = g_PlayerConfigsArray[g_MpPlayerNum].base.unk1a;
		break;
	case MENUOP_SET:
		g_PlayerConfigsArray[g_MpPlayerNum].base.unk1a = (u8) data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		sprintf(data->slider.label, "%d%%\n", data->slider.value + 20);
		break;
	}

	return 0;
}

MenuItemHandlerResult func0f179cc0(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = g_PlayerConfigsArray[g_MpPlayerNum].base.unk1c;
		break;
	case MENUOP_SET:
		g_PlayerConfigsArray[g_MpPlayerNum].base.unk1c = data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		sprintf(data->slider.label, "%d%%\n", data->slider.value + 25);
		break;
	}

	return 0;
}

MenuItemHandlerResult func0f179d6c(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f187fbc(g_MpPlayerNum);
	}

	return 0;
}

/**
 * This function is used by both player body selection and bot body selection.
 */
MenuItemHandlerResult mpCharacterBodyMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data, s32 mpbodynum, s32 mpheadnum, bool isplayer)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->carousel.value = mpGetNumBodies();
		break;
	case MENUOP_11:
		g_Menus[g_MpPlayerNum].menumodel.newanimnum = ANIM_01FC;
		g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_MP_HEADBODY(mpheadnum, mpbodynum);
		g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 += g_Vars.diffframe60;

		if (g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 > TICKS(480)) {
			g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 -= TICKS(480);
		}

		if (g_Menus[g_MpPlayerNum].menumodel.rottimer60 > 0) {
			g_Menus[g_MpPlayerNum].menumodel.rottimer60 -= g_Vars.diffframe60;
		} else {
#if VERSION >= VERSION_PAL_BETA
			f32 value = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60freal;
#else
			f32 value = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60f;
#endif
			g_Menus[g_MpPlayerNum].menumodel.newroty = value;
			g_Menus[g_MpPlayerNum].menumodel.curroty = value;
		}

		g_Menus[g_MpPlayerNum].menumodel.partvisibility = NULL;
		g_Menus[g_MpPlayerNum].menumodel.zoom = 30;
		break;
	case MENUOP_21:
		if (!challengeIsFeatureUnlocked(mpGetBodyRequiredFeature(data->carousel.value))) {
			return 1;
		}
		break;
#if VERSION >= VERSION_NTSC_1_0
	case MENUOP_FOCUS:
		g_Menus[g_MpPlayerNum].menumodel.loaddelay = 3;
		break;
#endif
	case MENUOP_GETSELECTEDINDEX:
		data->carousel.value = mpbodynum;
		break;
	case MENUOP_SET:
	case MENUOP_CHECKPREFOCUSED:
		g_Menus[g_MpPlayerNum].menumodel.removingpiece = false;

		menuConfigureModel(&g_Menus[g_MpPlayerNum].menumodel, 0, 0, 0, 0, 0, 0, 1, MENUMODELFLAG_HASSCALE);

		g_Menus[g_MpPlayerNum].menumodel.curposx = 8.2f;
		g_Menus[g_MpPlayerNum].menumodel.newposx = 8.2f;

		g_Menus[g_MpPlayerNum].menumodel.curposy = -4.1f;
		g_Menus[g_MpPlayerNum].menumodel.newposy = -4.1f;

		g_Menus[g_MpPlayerNum].menumodel.curscale = 0.002f;

		g_Menus[g_MpPlayerNum].menumodel.curroty = -0.2f;
		g_Menus[g_MpPlayerNum].menumodel.newroty = -0.2f;

		g_Menus[g_MpPlayerNum].menumodel.rottimer60 = TICKS(60);
		g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = TICKS(120);
		g_Menus[g_MpPlayerNum].menumodel.loaddelay = 8;

#if VERSION >= VERSION_NTSC_1_0
		if (operation == MENUOP_CHECKPREFOCUSED) {
			g_Menus[g_MpPlayerNum].menumodel.loaddelay = 16;
		}
#endif

		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpCharacterBody(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		if (g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum < mpGetNumHeads()) {
#if VERSION >= VERSION_NTSC_1_0
			if (!data->carousel.unk04)
#endif
			{
				g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum = mpGetMpheadnumByMpbodynum(data->carousel.value);
			}
		}
		g_PlayerConfigsArray[g_MpPlayerNum].base.mpbodynum = data->carousel.value;
		func0f17b8f0();
		break;
	case MENUOP_CHECKPREFOCUSED:
#if VERSION >= VERSION_NTSC_1_0
		mpCharacterBodyMenuHandler(operation, item, data,
				g_PlayerConfigsArray[g_MpPlayerNum].base.mpbodynum,
				g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum, true);
#endif
		return true;
	}

	return mpCharacterBodyMenuHandler(operation, item, data,
			g_PlayerConfigsArray[g_MpPlayerNum].base.mpbodynum,
			g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum, true);
}

MenuDialogHandlerResult menudialog0017a174(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		break;
	case MENUOP_CLOSE:
		break;
	case MENUOP_TICK:
		if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem != &dialogdef->items[1]
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem != &dialogdef->items[2]) {
			union handlerdata data;
			menuhandlerMpCharacterBody(MENUOP_11, &dialogdef->items[2], &data);
		}
	}

	return 0;
}

MenuItemHandlerResult mpChallengesListHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	s32 challengeindex;
	s32 x;
	s32 y;
	s32 loopx;
	s32 maxplayers;
	s32 i;
	char *name;
	s32 size = 11;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = challengeGetAutoFocusedIndex(g_MpPlayerNum);
		break;
	case MENUOP_RENDER:
		maxplayers = 4;
		gdl = data->type19.gdl;
		renderdata = data->type19.renderdata2;
		challengeindex = data->list.unk04;

		if (IS4MB()) {
			maxplayers = 2;
		}

		x = renderdata->x + 10;
		y = renderdata->y + 1;

		gdl = text0f153628(gdl);

		name = challengeGetName2(g_MpPlayerNum, challengeindex);

		gdl = textRenderProjected(gdl, &x, &y, name,
				g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour,
				viGetWidth(), viGetHeight(), 0, 0);

		gdl = text0f153780(gdl);

		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);

		texSelect(&gdl, &g_TexGeneralConfigs[35], 2, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetTextureFilter(gdl++, G_TF_POINT);

		for (i = 0, loopx = 10; i < maxplayers; i++) {
#if VERSION >= VERSION_NTSC_1_0
			if (challengeIsCompletedByPlayerWithNumPlayers2(g_MpPlayerNum, challengeindex, i + 1)) {
				gDPSetEnvColorViaWord(gdl++, 0xb2efff00 | (renderdata->colour & 0xff) * 255 / 256);
			} else {
				gDPSetEnvColorViaWord(gdl++, 0x30407000 | (renderdata->colour & 0xff) * 255 / 256);
			}
#else
			if (challengeIsCompletedByPlayerWithNumPlayers2(g_MpPlayerNum, challengeindex, i + 1)) {
				gDPSetEnvColorViaWord(gdl++, 0xb2efffff);
			} else {
				gDPSetEnvColorViaWord(gdl++, 0x304070ff);
			}
#endif

			gDPSetCombineLERP(gdl++,
					TEXEL0, 0, ENVIRONMENT, 0,
					TEXEL0, 0, ENVIRONMENT, 0,
					TEXEL0, 0, ENVIRONMENT, 0,
					TEXEL0, 0, ENVIRONMENT, 0);

			gSPTextureRectangle(gdl++,
					((renderdata->x + loopx) << 2) * g_ScaleX,
					(renderdata->y + size) << 2,
					((renderdata->x + size + loopx) << 2) * g_ScaleX,
					(renderdata->y + size * 2) << 2,
					G_TX_RENDERTILE,
					0, 0x0160, 0x0400 / g_ScaleX, 0xfc00);

			loopx += 13;
		}

		return (uintptr_t) gdl;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = 26;
		break;
	}

	return 0;
}

const char var7f1b7ea8[] = "Menu99 -> Calling Camera Module Start\n";
const char var7f1b7ed0[] = "Menu99 -> Calling Camera Module Finish\n";

char *mpMenuTextKills(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].kills);
	return g_StringPointer;
}

char *mpMenuTextDeaths(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].deaths);
	return g_StringPointer;
}

char *mpMenuTextGamesPlayed(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].gamesplayed);
	return g_StringPointer;
}

char *mpMenuTextGamesWon(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].gameswon);
	return g_StringPointer;
}

char *mpMenuTextGamesLost(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].gameslost);
	return g_StringPointer;
}

char *mpMenuTextHeadShots(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].headshots);
	return g_StringPointer;
}

char *mpMenuTextMedalAccuracy(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].accuracymedals);
	return g_StringPointer;
}

char *mpMenuTextMedalHeadShot(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].headshotmedals);
	return g_StringPointer;
}

char *mpMenuTextMedalKillMaster(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].killmastermedals);
	return g_StringPointer;
}

char *mpMenuTextMedalSurvivor(struct menuitem *item)
{ \
	sprintf(g_StringPointer, "%d\n", g_PlayerConfigsArray[g_MpPlayerNum].survivormedals);
	return g_StringPointer;
}

char *mpMenuTextAmmoUsed(struct menuitem *item)
{
	s32 value = g_PlayerConfigsArray[g_MpPlayerNum].ammoused;

	if (value > 100000) {
		value = value / 1000;

		if (value > 100000) {
			value = value / 1000;
			sprintf(g_StringPointer, "%dM\n", value);
		} else {
			sprintf(g_StringPointer, "%dK\n", value);
		}
	} else {
		sprintf(g_StringPointer, "%d\n", value);
	}

	return g_StringPointer;
}

char *mpMenuTextDistance(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s%s%.1fkm\n", "", "", g_PlayerConfigsArray[g_MpPlayerNum].distance / 10.0f);
	return g_StringPointer;
}

char *mpMenuTextTime(struct menuitem *item)
{
	u32 raw = g_PlayerConfigsArray[g_MpPlayerNum].time;
	s32 secs = raw % 60;
	s32 hours;
	s32 days;

	if (raw == 0) {
		return "--:--\n";
	}

	if (raw >= 0x7fffffff) {
		return "==:==\n";
	}

	raw = raw / 60;
	hours = raw / 60;
	days = hours / 24;

	if (days == 0) {
		sprintf(g_StringPointer, "%d:%02d.%02d", hours % 24, raw % 60, secs);
	} else {
		sprintf(g_StringPointer, "%d:%02d:%02d", days, hours % 24, raw % 60);
	}

	return g_StringPointer;
}

char *mpMenuTextAccuracy(struct menuitem *item)
{
#if VERSION < VERSION_NTSC_1_0
	if (g_PlayerConfigsArray[g_MpPlayerNum].gamesplayed < 8) {
		return "-\n";
	}
#endif

	sprintf(g_StringPointer, "%s%s%.1f%%", "", "", g_PlayerConfigsArray[g_MpPlayerNum].accuracy / 10.0f);
	return g_StringPointer;
}

void mpFormatDamageValue(char *dst, f32 damage)
{
#if VERSION >= VERSION_NTSC_1_0
	if (damage < 1000) {
		sprintf(dst, "%s%s%.1f", "", "", damage);
	} else if (damage < 10000) {
		sprintf(dst, "%s%s%.0f", "", "", damage);
	} else if (damage < 100000) {
		damage = damage / 1000;
		sprintf(dst, "%s%s%.1fK", "", "", damage);
	} else if (damage < 1000000) {
		damage = damage / 1000;
		sprintf(dst, "%s%s%.0fK", "", "", damage);
	} else if (damage < 10000000) {
		damage = damage / 1000;
		damage = damage / 1000;
		sprintf(dst, "%s%s%.1fM", "", "", damage);
	} else {
		damage = damage / 1000;
		damage = damage / 1000;
		sprintf(dst, "%s%s%.0fM", "", "", damage);
	}
#else
	if (damage > 100000) {
		damage = damage / 1000;
		sprintf(dst, "%s%s%.1fKL", "", "", damage);
	} else {
		sprintf(dst, "%s%s%.1fL", "", "", damage);
	}
#endif
}

char *mpMenuTextPainReceived(struct menuitem *item)
{
	mpFormatDamageValue(g_StringPointer, g_PlayerConfigsArray[g_MpPlayerNum].painreceived / 10.0f);
	return g_StringPointer;
}

char *mpMenuTextDamageDealt(struct menuitem *item)
{
	mpFormatDamageValue(g_StringPointer, g_PlayerConfigsArray[g_MpPlayerNum].damagedealt / 10.0f);
	return g_StringPointer;
}

MenuItemHandlerResult mpMedalMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		u32 colour;

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

		switch (item->param) {
		case 0: // KillMaster - red
			colour = 0xff7f7fff;
			break;
		case 1: // Headshot - yellow
			colour = 0xbfbf00ff;
			break;
		case 2: // Accuracy - green
			colour = 0x00ff00ff;
			break;
		case 3: // Survivor - blue
			colour = 0x00bfbfff;
			break;
		}

#if VERSION >= VERSION_NTSC_1_0
		colour = (colour & 0xffffff00) | (colour & 0xff) * (renderdata->colour & 0xff) >> 8;
#endif

		gDPSetEnvColorViaWord(gdl++, colour);

		gDPSetCombineLERP(gdl++,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0);

		gSPTextureRectangle(gdl++,
				((renderdata->x + 9) << 2) * g_ScaleX, renderdata->y << 2,
				((renderdata->x + 20) << 2) * g_ScaleX, (renderdata->y + 11) << 2,
				G_TX_RENDERTILE, 0, 0x0160, 1024 / g_ScaleX, -1024);

		return (uintptr_t) gdl;
	}

	return 0;
}

char *mpMenuTitleStatsForPlayerName(struct menudialogdef *dialogdef)
{
	// "Stats for %s"
	sprintf(g_StringPointer, _("Stats for %s\n"), g_PlayerConfigsArray[g_MpPlayerNum].base.name);
	return g_StringPointer;
}

MenuItemHandlerResult menuhandlerMpUsernamePassword(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_PlayerConfigsArray[g_MpPlayerNum].title != MPPLAYERTITLE_PERFECT) {
			return true;
		}
	}

	return 0;
}

struct menuitem g_MpSavePlayerMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop(""), // "Your player file is always saved automatically."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Save a copy now?\n"), // "Save a copy now?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("No\n"), // "No"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Yes\n"), // "Yes"
		0,
		menuhandlerMpConfirmSaveChr,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSavePlayerMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Confirm\n"), // "Confirm"
	g_MpSavePlayerMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpSaveSetupNameMenuItems[] = {
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Enter the setup name:\n"),
		0,
		NULL,
	},
#endif
	{
		MENUITEMTYPE_KEYBOARD,
		MPSETUP_MAXNAME,
		0,
		0,
		1,
		menuhandlerMpSetupName,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSaveSetupNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game File Name\n"), // "Game File Name"
	g_MpSaveSetupNameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpSaveSetupExistsMenuItems[] = {
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		gettext_noop("Name:\n"), // "Name:"
		(uintptr_t)&mpMenuTextSetupName,
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
#endif
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Save over your\noriginal setup?\n"),
		0,
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
		gettext_noop("Save Over Original\n"), // "Save Over Original"
		0,
		menuhandlerMpSaveSetupOverwrite,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Save Copy\n"), // "Save Copy"
		0,
		menuhandlerMpSaveSetupCopy,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Do Not Save\n"), // "Do Not Save"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSaveSetupExistsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Save Game Setup\n"), // "Save Game Setup"
	g_MpSaveSetupExistsMenuItems,
	NULL,
	0,
	NULL,
};

#ifndef PLATFORM_N64
MenuItemHandlerResult mpSelectRandomWeaponListHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	static const char *labels[] = {
		gettext_noop("Select Dark"),
		gettext_noop("Select Classic"),
		gettext_noop("Select All"),
		gettext_noop("Select None"),
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = mpGetNumWeaponOptions() + 4;
		break;
	case MENUOP_GETOPTIONTEXT:
		{
			s32 numweapons = mpGetNumWeaponOptions();

			if (data->list.value < numweapons) {
				return (uintptr_t) mpGetWeaponLabel(data->list.value);
			} else {
				return (intptr_t)_(labels[data->list.value - numweapons]);
			}
		}
	case MENUOP_SET:
		{
			s32 numweapons = mpGetNumWeaponOptions();
			s32 mpweaponnum = data->list.value;
			s32 optionindex = mpweaponnum;
			s32 i;

			if (data->list.value < numweapons) {
				if (data->list.unk04 == 0) {
					for (i = 0; i <= mpweaponnum; i++) {
						if (challengeIsFeatureUnlocked(g_MpWeapons[i].unlockfeature) == 0) {
							mpweaponnum++;
						}

						optionindex = mpweaponnum;
					}

					g_MpWeaponSetRandomFilters[optionindex] = 1 - g_MpWeaponSetRandomFilters[optionindex];
				}
			} else {
				s32 index = data->list.value - numweapons;

				switch (index) {
				case 0:
					// Select Dark
					for (i = 0; i < ARRAYCOUNT(g_MpWeapons); i++) {
						if ((i >= MPWEAPON_NONE && i <= MPWEAPON_XRAYSCANNER)
								|| i == MPWEAPON_CLOAKINGDEVICE
								|| i == MPWEAPON_COMBATBOOST
								|| i >= MPWEAPON_SHIELD) {
							g_MpWeaponSetRandomFilters[i] = 1;
						} else {
							g_MpWeaponSetRandomFilters[i] = 0;
						}
					}
					break;
				case 1:
					// Select Classic
					for (i = 0; i < ARRAYCOUNT(g_MpWeapons); i++) {
						if (i >= MPWEAPON_PP9I && i <= MPWEAPON_RCP45) {
							g_MpWeaponSetRandomFilters[i] = 1;
						} else {
							g_MpWeaponSetRandomFilters[i] = 0;
						}
					}
					break;
				case 2:
					// Select All
					for (i = 0; i < ARRAYCOUNT(g_MpWeapons); i++) {
						g_MpWeaponSetRandomFilters[i] = 1;
					}
					break;
				case 3:
					// Select None
					for (i = 0; i < ARRAYCOUNT(g_MpWeapons); i++) {
						g_MpWeaponSetRandomFilters[i] = 0;
					}
					break;
				}
			}
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0x000fffff;
		break;
	case MENUOP_GETLISTITEMCHECKBOX:
		{
			s32 numweapons = mpGetNumWeaponOptions();
			s32 mpweaponnum = data->list.value;
			s32 optionindex = mpweaponnum;
			s32 i;

			if (data->list.value < numweapons) {

				for (i = 0; i <= mpweaponnum; i++) {
					if (challengeIsFeatureUnlocked(g_MpWeapons[i].unlockfeature) == 0) {
						mpweaponnum++;
					}

					optionindex = mpweaponnum;
				}

				data->list.unk04 = g_MpWeaponSetRandomFilters[optionindex];
			}
		}
		break;
	}

	return 0;
}

struct menuitem g_MpSelectRandomWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		"", // previous: 0x00000078
		0x0000004d,
		mpSelectRandomWeaponListHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSelectRandomWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Select Weapons"),
	g_MpSelectRandomWeaponsMenuItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	NULL,
};

MenuItemHandlerResult menuhandlerMpSelectRandomWeapons(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKDISABLED:
	case MENUOP_CHECKHIDDEN:
		if (g_MpWeaponSetNum == WEAPONSET_RANDOM
				|| g_MpWeaponSetNum == WEAPONSET_RANDOMFIVE) {
			return false;
		}
		return true;
	case MENUOP_SET:
		menuPushDialog(&g_MpSelectRandomWeaponsMenuDialog);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpAutoRandomWeapon(s32 operation, struct menuitem *item, union handlerdata *data)
{
	static const char *labels[] = {
		gettext_noop("Off"),
		gettext_noop("Start"),
		gettext_noop("End"),
	};

	switch (operation) {
	case MENUOP_CHECKDISABLED:
	case MENUOP_CHECKHIDDEN:
		if (g_MpWeaponSetNum == WEAPONSET_RANDOM
				|| g_MpWeaponSetNum == WEAPONSET_RANDOMFIVE) {
			return false;
		}
		return true;
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = ARRAYCOUNT(labels);
		break;
	case MENUOP_GETOPTIONTEXT:
		return (intptr_t)_(labels[data->dropdown.value]);
	case MENUOP_SET:
		g_MpSetup.options &= ~(MPOPTION_AUTORANDOMWEAPON_START | MPOPTION_AUTORANDOMWEAPON_END);

		if (data->dropdown.value == AUTORANDOMWEAPON_START) {
			g_MpSetup.options |= MPOPTION_AUTORANDOMWEAPON_START;
		} else if (data->dropdown.value == AUTORANDOMWEAPON_END) {
			g_MpSetup.options |= MPOPTION_AUTORANDOMWEAPON_END;
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		if (g_MpSetup.options & MPOPTION_AUTORANDOMWEAPON_END) {
			data->dropdown.value = AUTORANDOMWEAPON_END;
		} else if (g_MpSetup.options & MPOPTION_AUTORANDOMWEAPON_START) {
			data->dropdown.value = AUTORANDOMWEAPON_START;
		} else {
			data->dropdown.value = AUTORANDOMWEAPON_OFF;
		}
		break;
	}

	return 0;
}
#endif

struct menuitem g_MpWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_DROPDOWN,
		1,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Set:\n"), // "Set:"
		0,
		menuhandlerMpWeaponSetDropdown,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Select Weapons\n"),
		0,
		menuhandlerMpSelectRandomWeapons,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Auto Random\n"),
		0,
		menuhandlerMpAutoRandomWeapon,
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
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		gettext_noop("Current Weapon Setup:\n"), // "Current Weapon Setup:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("1:\n"), // "1:"
		0,
		menuhandlerMpWeaponSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("2:\n"), // "2:"
		1,
		menuhandlerMpWeaponSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("3:\n"), // "3:"
		2,
		menuhandlerMpWeaponSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("4:\n"), // "4:"
		3,
		menuhandlerMpWeaponSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("5:\n"), // "5:"
		4,
		menuhandlerMpWeaponSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_MPWEAPONSLOT,
		gettext_noop("6:\n"), // "6:"
		5,
		menuhandlerMpWeaponSlot,
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

struct menudialogdef g_MpWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Weapons\n"), // "Weapons"
	g_MpWeaponsMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpQuickTeamWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DROPDOWN_BELOW | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Set:\n"), // "Set:"
		0,
		menuhandlerMpWeaponSetDropdown,
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
		MENUITEMFLAG_00000002,
		gettext_noop("1:\n"), // "1:"
		(uintptr_t)&mpMenuTextWeaponNameForSlot,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		1,
		MENUITEMFLAG_00000002,
		gettext_noop("2:\n"), // "2:"
		(uintptr_t)&mpMenuTextWeaponNameForSlot,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		2,
		MENUITEMFLAG_00000002,
		gettext_noop("3:\n"), // "3:"
		(uintptr_t)&mpMenuTextWeaponNameForSlot,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		3,
		MENUITEMFLAG_00000002,
		gettext_noop("4:\n"), // "4:"
		(uintptr_t)&mpMenuTextWeaponNameForSlot,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		4,
		MENUITEMFLAG_00000002,
		gettext_noop("5:\n"), // "5:"
		(uintptr_t)&mpMenuTextWeaponNameForSlot,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Back\n"), // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickTeamWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Weapons\n"), // "Weapons"
	g_MpQuickTeamWeaponsMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpPlayerOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Highlight Pickups\n"), // "Highlight Pickups"
		MPDISPLAYOPTION_HIGHLIGHTPICKUPS,
		menuhandlerMpDisplayOptionCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Highlight Players\n"), // "Highlight Players"
		MPDISPLAYOPTION_HIGHLIGHTPLAYERS,
		menuhandlerMpDisplayOptionCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Highlight Teams\n"), // "Highlight Teams"
		MPDISPLAYOPTION_HIGHLIGHTTEAMS,
		menuhandlerMpDisplayOptionCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Radar\n"), // "Radar"
		MPDISPLAYOPTION_RADAR,
		menuhandlerMpDisplayOptionCheckbox,
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

struct menudialogdef g_MpPlayerOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Options\n"), // "Options"
	g_MpPlayerOptionsMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpControlMenuItems[] = {
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Control Style\n"), // "Control Style"
		0,
		menuhandlerMpControlStyle,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Reverse Pitch\n"), // "Reverse Pitch"
		OPTION_FORWARDPITCH,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Look Ahead\n"), // "Look Ahead"
		OPTION_LOOKAHEAD,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Head Roll\n"), // "Head Roll"
		OPTION_HEADROLL,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Auto-Aim\n"), // "Auto-Aim"
		OPTION_AUTOAIM,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Aim Control\n"), // "Aim Control"
		0,
		menuhandlerMpAimControl,
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
		0,
		gettext_noop("Sight on Screen\n"), // "Sight on Screen"
		OPTION_SIGHTONSCREEN,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Show Target\n"), // "Show Target"
		OPTION_ALWAYSSHOWTARGET,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Zoom Range\n"), // "Zoom Range"
		OPTION_SHOWZOOMRANGE,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Ammo on Screen\n"), // "Ammo on Screen"
		OPTION_AMMOONSCREEN,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Gun Function\n"), // "Gun Function"
		OPTION_SHOWGUNFUNCTION,
		menuhandlerMpControlCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Paintball\n"), // "Paintball"
		OPTION_PAINTBALL,
		menuhandlerMpControlCheckbox,
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

struct menudialogdef g_MpControlMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Control\n"), // "Control"
	g_MpControlMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpCompletedChallengesMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"", // previous: 0x00000078
		0x0000004d,
		mpChallengesListHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpCompletedChallengesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Completed Challenges\n"), // "Completed Challenges"
	g_MpCompletedChallengesMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	NULL,
};

#if VERSION >= VERSION_NTSC_1_0
char *mpMenuTextUsernamePassword(struct menuitem *item)
{
	// Phrases included here to assist people searching the code for them:
	// EnTROpIcDeCAy
	// ZeRo-Tau

	u8 username[] = {
		'E' + 9 * 1,
		'n' + 9 * 2,
		'T' + 9 * 3,
		'R' + 9 * 4,
		'O' + 9 * 5,
		'p' + 9 * 6,
		'I' + 9 * 7,
		'c' + 9 * 8,
		'D' + 9 * 9,
		'e' + 9 * 10,
		'C' + 9 * 11,
		'A' + 9 * 12,
		'y' + 9 * 13,
		'\n' + 9 * 14,
		'\0' + 9 * 15,
	};

	u8 password[] = {
		'Z' + 4 * 1,
		'e' + 4 * 2,
		'R' + 4 * 3,
		'o' + 4 * 4,
		'-' + 4 * 5,
		'T' + 4 * 6,
		'a' + 4 * 7,
		'u' + 4 * 8,
		'\n' + 4 * 9,
		'\0' + 4 * 10,
	};

	u32 stack;
	s32 i;

	if (item->param == 0) {
		for (i = 0; i < ARRAYCOUNT(username); i++) {
			g_StringPointer[i] = username[i] - i * 9 - 9;
		}
	} else {
		for (i = 0; i < ARRAYCOUNT(password); i++) {
			g_StringPointer[i] = password[i] - i * 4 - 4;
		}
	}

	return g_StringPointer;
}
#endif

struct menuitem g_MpPlayerStatsMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Kills:\n"), // "Kills:"
		(uintptr_t)&mpMenuTextKills,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Deaths:\n"), // "Deaths:"
		(uintptr_t)&mpMenuTextDeaths,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Accuracy:\n"), // "Accuracy:"
		(uintptr_t)&mpMenuTextAccuracy,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Head Shots:\n"), // "Head Shots:"
		(uintptr_t)&mpMenuTextHeadShots,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Ammo Used:\n"), // "Ammo Used:"
		(uintptr_t)&mpMenuTextAmmoUsed,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Damage Dealt:\n"), // "Damage Dealt:"
		(uintptr_t)&mpMenuTextDamageDealt,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Pain Received:\n"), // "Pain Received:"
		(uintptr_t)&mpMenuTextPainReceived,
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
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Games Played:\n"), // "Games Played:"
		(uintptr_t)&mpMenuTextGamesPlayed,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Games Won:\n"), // "Games Won:"
		(uintptr_t)&mpMenuTextGamesWon,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Games Lost:\n"), // "Games Lost:"
		(uintptr_t)&mpMenuTextGamesLost,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Time:\n"), // "Time:"
		(uintptr_t)&mpMenuTextTime,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Distance:\n"), // "Distance:"
		(uintptr_t)&mpMenuTextDistance,
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
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		gettext_noop("Medals Won:\n"), // "Medals Won:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		2,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		gettext_noop("   Accuracy:\n"), // "Accuracy:"
		(uintptr_t)&mpMenuTextMedalAccuracy,
		mpMedalMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		1,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		gettext_noop("   Head Shot:\n"), // "Head Shot:"
		(uintptr_t)&mpMenuTextMedalHeadShot,
		mpMedalMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		gettext_noop("   KillMaster:\n"), // "KillMaster:"
		(uintptr_t)&mpMenuTextMedalKillMaster,
		mpMedalMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		3,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		gettext_noop("   Survivor:\n"), // "Survivor:"
		(uintptr_t)&mpMenuTextMedalSurvivor,
		mpMedalMenuHandler,
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
		0,
		gettext_noop("Your Title:\n"), // "Your Title:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		&mpMenuTextPlayerTitle,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		gettext_noop("USERNAME:\n"), // "USERNAME:"
		0,
		menuhandlerMpUsernamePassword,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT,
#if VERSION >= VERSION_NTSC_1_0
		&mpMenuTextUsernamePassword,
#else
		0x51f0,
#endif
		0,
		menuhandlerMpUsernamePassword,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		gettext_noop("PASSWORD:\n"), // "PASSWORD:"
		0,
		menuhandlerMpUsernamePassword,
	},
	{
		MENUITEMTYPE_LABEL,
		(VERSION >= VERSION_NTSC_1_0 ? 1 : 0),
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT,
#if VERSION >= VERSION_NTSC_1_0
		&mpMenuTextUsernamePassword,
#else
		0x51f1,
#endif
		0,
		menuhandlerMpUsernamePassword,
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

struct menudialogdef g_MpPlayerStatsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTitleStatsForPlayerName,
	g_MpPlayerStatsMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEITEMSCROLL | MENUDIALOGFLAG_SMOOTHSCROLLABLE,
	&g_MpCompletedChallengesMenuDialog,
};

MenuItemHandlerResult mpCharacterHeadMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data, s32 mpheadnum, bool arg4)
{
	f32 diffframe;
	s32 headnum;

	static struct modelpartvisibility visibility[] = {
		{ MODELPART_HEAD_SUNGLASSES, false },
		{ MODELPART_HEAD_EYESCLOSED, false },
		{ MODELPART_HEAD_HUDPIECE,   false },
		{ 255, false },
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->carousel.value = mpGetNumHeads2();
		break;
	case MENUOP_11:
#if VERSION >= VERSION_PAL_BETA
		diffframe = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60freal;
#else
		diffframe = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60f;
#endif

		g_Menus[g_MpPlayerNum].menumodel.newroty = diffframe;
		g_Menus[g_MpPlayerNum].menumodel.curroty = diffframe;

		if (mpheadnum < mpGetNumHeads2()) {
			headnum = mpGetHeadId(mpheadnum);

			g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(g_HeadsAndBodies[headnum].filenum);
			g_Menus[g_MpPlayerNum].menumodel.isperfecthead = false;
		} else {
			headnum = mpGetBeauHeadId(func0f14a9f8(mpheadnum - mpGetNumHeads2()));

			g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(g_HeadsAndBodies[headnum].filenum);
			g_Menus[g_MpPlayerNum].menumodel.isperfecthead = true;
			g_Menus[g_MpPlayerNum].menumodel.perfectheadnum = mpheadnum - mpGetNumHeads2();
		}

		g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = 0;
		g_Menus[g_MpPlayerNum].menumodel.partvisibility = visibility;
		g_Menus[g_MpPlayerNum].menumodel.zoom = 30;
		break;
	case MENUOP_21:
		if (!challengeIsFeatureUnlocked(mpGetHeadRequiredFeature(data->carousel.value))) {
			return 1;
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->carousel.value = mpheadnum;
		break;
	case MENUOP_SET:
	case MENUOP_FOCUS:
#if VERSION >= VERSION_NTSC_1_0
		g_Menus[g_MpPlayerNum].menumodel.loaddelay = 3;
#endif

		mpGetNumHeads2();

		menuConfigureModel(&g_Menus[g_MpPlayerNum].menumodel, 0, 0, 0, 0, 0, 0, 1, MENUMODELFLAG_HASSCALE);

		g_Menus[g_MpPlayerNum].menumodel.curposx = 0;
		g_Menus[g_MpPlayerNum].menumodel.curposy = 0;

		g_Menus[g_MpPlayerNum].menumodel.newposx = 0;
		g_Menus[g_MpPlayerNum].menumodel.newposy = -3;

		g_Menus[g_MpPlayerNum].menumodel.curscale = 0.01f;

		g_Menus[g_MpPlayerNum].menumodel.curroty = -0.3f;
		g_Menus[g_MpPlayerNum].menumodel.newroty = -0.3f;

		g_Menus[g_MpPlayerNum].menumodel.newscale = 1;
		g_Menus[g_MpPlayerNum].menumodel.zoom = 30;
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpCharacterHead(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum = data->carousel.value;
	}

	return mpCharacterHeadMenuHandler(operation, item, data, g_PlayerConfigsArray[g_MpPlayerNum].base.mpheadnum, 1);
}

char *mpMenuTextBodyName(struct menuitem *item)
{
	return mpGetBodyName(g_PlayerConfigsArray[g_MpPlayerNum].base.mpbodynum);
}

void func0f17b8f0(void)
{
	func0f0f139c(g_MpCharacterMenuItems, -0.4f);
}

MenuItemHandlerResult mpPlayerNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;
	s32 i;

	switch (operation) {
	case MENUOP_GETTEXT:
		i = 0;

		while (g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\n'
				&& g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\0'
				&& i < 11) {
			name[i] = g_PlayerConfigsArray[g_MpPlayerNum].base.name[i];
			i++;
		}

		while (i < 11) {
			name[i] = '\0';
			i++;
		}
		break;
	case MENUOP_SETTEXT:
		i = 0;

		while (i < 11 && name[i] != '\0') {
			g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = name[i];
			i++;
		}

		g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = '\n';
		i++;

		while (i < 11) {
			g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] = '\0';
			i++;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult mpLoadSettingsMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u8 presets = g_Menus[g_MpPlayerNum].mpsetup.showpresets;
	s32 numpresets = mpGetNumUnlockedPresets()*presets;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = numpresets + g_MpSetupFile.numsetups;
		break;
	case MENUOP_GETOPTIONTEXT:
		if (presets && data->list.value < mpGetNumUnlockedPresets()) {
			return (uintptr_t)mpGetPresetNameBySlot(data->list.value);
		}
		if (g_MpSetupFile.numsetups > 0) {
			struct setupblock *block = &g_MpSetupFile.setups[data->list.value - numpresets];
			func0f0d564c_ext(block->bytes, g_StringPointer, false, MPSETUP_MAXNAME+1);
			return (uintptr_t)g_StringPointer;
		}
		break;
	case MENUOP_SET:
		mpCloseDialogsForNewSetup();

		if (presets && data->list.value < mpGetNumUnlockedPresets()) {
			mp0f18dec4(data->list.value);
			g_MpCurrentSetup = -1;
		} else {
			mpsetupLoadSetup(data->list.value - numpresets);
		}

		if (item->param == 1) {
			if (IS4MB()) {
				func0f0f820c(&g_MpQuickGo4MbMenuDialog, MENUROOT_4MBMAINMENU);
			} else {
				func0f0f820c(&g_MpQuickGoMenuDialog, MENUROOT_MPSETUP);
			}
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = presets ? 2 : 1;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		if (presets && data->list.value == 0) {
			return (uintptr_t)_("Presets\n"); // "Presets"
		}
		return (uintptr_t)_("Custom");
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = data->list.value == 0 ? 0 : numpresets;
		break;
	case MENUOP_LISTITEMFOCUS:
		if (presets && data->list.value < mpGetNumUnlockedPresets()) {
			g_Menus[g_MpPlayerNum].mpsetup.slotindex = 0xffff;
		} else {
			g_Menus[g_MpPlayerNum].mpsetup.slotindex = data->list.value - numpresets;
		}
		break;
	}

	return 0;
}

char *mpMenuTextMpconfigMarquee(struct menuitem *item)
{
	char filename[MPSETUP_MAXNAME+1];
	u16 numsims;
	u16 stagenum;
	u16 scenarionum;
	s32 arenanum;
	s32 i;

	if (g_Menus[g_MpPlayerNum].mpsetup.slotindex < 0xffff && g_MpSetupFile.numsetups > 0) {
#if VERSION >= VERSION_NTSC_1_0
		arenanum = -1;
#else
		arenanum = 0;
#endif

		mpsetupfileGetOverview(g_MpSetupFile.setups[g_Menus[g_MpPlayerNum].mpsetup.slotindex].bytes,
				filename, &numsims, &stagenum, &scenarionum);

		for (i = 0; i < ARRAYCOUNT(g_MpArenas); i++) {
			if (g_MpArenas[i].stagenum == stagenum) {
				arenanum = i;
			}
		}

#if VERSION >= VERSION_NTSC_1_0
		if (scenarionum <= 5 && arenanum != -1 && numsims >= 0 && filename[0] != '\0' && numsims <= MAX_BOTS) {
			// "%s:  Scenario: %s   Arena: %s    Simulants: %d"
			sprintf(g_StringPointer, _("%s:  Scenario: %s   Arena: %s    Simulants: %d"),
					filename,
					_(g_MpScenarioOverviews[scenarionum].name),
					_(g_MpArenas[arenanum].name),
					numsims);
		} else {
			return "";
		}
#else
		// "%s:  Scenario: %s   Arena: %s    Simulants: %d"
		sprintf(g_StringPointer, langGet(gettext_noop("%s:  Scenario: %s   Arena: %s    Simulants: %d")),
				filename,
				langGet(g_MpScenarioOverviews[scenarionum].name),
				langGet(g_MpArenas[arenanum].name),
				numsims);
#endif

		return g_StringPointer;
	}

	return "";
}

MenuItemHandlerResult mpLoadPlayerMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 i;
	struct fileguid guid;
	struct filelistfile *file;
	bool available;

	if (g_FileLists[0] == NULL) {
		return 0;
	}

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = g_FileLists[0]->numfiles;
		break;
	case MENUOP_GETOPTIONTEXT:
		filemgrGetSelectName(g_StringPointer, &g_FileLists[0]->files[data->list.value], FILETYPE_MPPLAYER);
		return (uintptr_t)g_StringPointer;
	case MENUOP_SET:
		file = &g_FileLists[0]->files[data->list.value];
		available = true;

		for (i = 0; i < MAX_PLAYERS; i++) {
			if (file->fileid == g_PlayerConfigsArray[i].fileguid.fileid
					&& file->deviceserial == g_PlayerConfigsArray[i].fileguid.deviceserial) {
				if ((g_MpSetup.chrslots & (1 << i)) == 0) {
					mpPlayerSetDefaults(i, true);
				} else {
					available = false;
				}
			}
		}

		if (available) {
			guid.fileid = file->fileid;
			guid.deviceserial = file->deviceserial;

			menuPopDialog();

			filemgrSaveOrLoad(&guid, FILEOP_LOAD_MPPLAYER, g_MpPlayerNum);
		} else {
			filemgrPushErrorDialog(FILEERROR_ALREADYLOADED);
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = g_FileLists[0]->numdevices;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return filemgrGetDeviceNameOrStartIndex(0, operation, data->list.value);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = filemgrGetDeviceNameOrStartIndex(0, operation, data->list.value);
		return 0;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpTimeLimitSlider(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = g_MpSetup.timelimit;
		break;
	case MENUOP_SET:
		g_MpSetup.timelimit = data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		if (data->slider.value == 60) {
			sprintf(data->slider.label, _("No Limit\n")); // "No Limit"
		} else {
			sprintf(data->slider.label, _("%d Min\n"), data->slider.value + 1); // "%d Min"
		}
	}
	return 0;
}

MenuItemHandlerResult menuhandlerMpScoreLimitSlider(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = g_MpSetup.scorelimit;
		break;
	case MENUOP_SET:
		g_MpSetup.scorelimit = data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		if (data->slider.value == 100) {
			sprintf(data->slider.label, _("No Limit\n")); // "No Limit"
		} else {
			sprintf(data->slider.label, _("%d\n"), data->slider.value + 1); // "%d"
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpTeamScoreLimitSlider(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETSLIDER:
		data->slider.value = mpCalculateTeamScoreLimit();
		break;
	case MENUOP_SET:
		g_MpSetup.teamscorelimit = data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		if (data->slider.value == 400) {
			sprintf(data->slider.label, _("No Limit\n")); // "No Limit"
		} else {
			sprintf(data->slider.label, _("%d\n"), data->slider.value + 1); // "%d"
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpRestoreScoreDefaults(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f187fec();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpHandicapPlayer(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		if ((g_MpSetup.chrslots & (1 << item->param)) == 0) {
			return 1;
		}
		break;
	case MENUOP_GETSLIDER:
		data->slider.value = g_PlayerConfigsArray[item->param].handicap;
		break;
	case MENUOP_SET:
		g_PlayerConfigsArray[item->param].handicap = (u16)data->slider.value;
		break;
	case MENUOP_GETSLIDERLABEL:
		sprintf(data->slider.label, "%s%s%.00f%%\n", "", "", mpHandicapToDamageScale(g_PlayerConfigsArray[item->param].handicap) * 100);
		break;
	}

	return 0;
}

char *mpMenuTextHandicapPlayerName(struct menuitem *item)
{
	if (g_MpSetup.chrslots & (1 << item->param)) {
		return g_PlayerConfigsArray[item->param].base.name;
	}

	return "";
}

MenuItemHandlerResult menuhandlerMpRestoreHandicapDefaults(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;

		for (i = 0; i < MAX_PLAYERS; i++) {
			g_PlayerConfigsArray[i].handicap = 0x80;
		}
	}

	return 0;
}

MenuDialogHandlerResult menudialogMpReady(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		if (g_PlayerConfigsArray[g_MpPlayerNum].fileguid.fileid && g_PlayerConfigsArray[g_MpPlayerNum].fileguid.deviceserial) {
			filemgrSaveOrLoad(&g_PlayerConfigsArray[g_MpPlayerNum].fileguid, FILEOP_SAVE_MPPLAYER, g_MpPlayerNum);
		}
	}

	return false;
}

MenuDialogHandlerResult menudialogMpSimulant(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if ((u8)g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.name[0] == '\0') {
			menuPopDialog();
		}
	}

	return false;
}

MenuDialogHandlerResult mpLoadSettingsDialogHandler(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (menuAltAnyPressed(g_MpPlayerNum)) {
			u8 presets = g_Menus[g_MpPlayerNum].mpsetup.showpresets;
			g_Menus[g_MpPlayerNum].mpsetup.showpresets = 1 - presets;
		}
	}
}

struct menuitem g_MpCharacterMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_DARKERBG,
		&mpMenuTextBodyName,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_CAROUSEL,
		0,
		0,
		0,
		0x00000022,
		menuhandlerMpCharacterHead,
	},
	{
		MENUITEMTYPE_CAROUSEL,
		0,
		0,
		0,
		0x0000001b,
		menuhandlerMpCharacterBody,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpCharacterMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Character\n"), // "Character"
	g_MpCharacterMenuItems,
	menudialog0017a174,
	MENUDIALOGFLAG_0002,
	NULL,
};

struct menuitem g_MpPlayerNameMenuItems[] = {
	{
		MENUITEMTYPE_KEYBOARD,
		0,
		0,
		0,
		0,
		mpPlayerNameMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpPlayerNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Name\n"), // "Player Name"
	g_MpPlayerNameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpLoadSettingsMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		"", // previous: 0x00000078
		0x00000042,
		mpLoadSettingsMenuHandler,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&mpMenuTextMpconfigMarquee,
		0,
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
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SMALLFONT,
		gettext_noop("Menu Alt: Toggle Presets\n"),
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpLoadSettingsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Load Game Settings\n"), // "Load Game Settings"
	g_MpLoadSettingsMenuItems,
	mpLoadSettingsDialogHandler,
	MENUDIALOGFLAG_CLOSEONSELECT,
	NULL,
};

struct menuitem g_MpLoadPresetMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		1,
		0,
		"", // previous: 0x00000078
		0x00000042,
		mpLoadSettingsMenuHandler,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&mpMenuTextMpconfigMarquee,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpLoadPresetMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Load Game Settings\n"), // "Load Game Settings"
	g_MpLoadPresetMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpLoadPlayerMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		0,
		"", // previous: 0x0000007e,
		0x00000042,
		mpLoadPlayerMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SMALLFONT,
		gettext_noop("B Button to cancel\n"), // "B Button to cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpLoadPlayerMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Load Player\n"), // "Load Player"
	g_MpLoadPlayerMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpArenaMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		"", // previous: 0x00000078
		0x0000004d,
		mpArenaMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpArenaMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Arena\n"), // "Arena"
	g_MpArenaMenuItems,
	NULL,
	MENUDIALOGFLAG_CLOSEONSELECT | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpLimitsMenuItems[] = {
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Time\n"), // "Time"
		0x0000003c,
		menuhandlerMpTimeLimitSlider,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Score\n"), // "Score"
		0x00000064,
		menuhandlerMpScoreLimitSlider,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Team Score\n"), // "Team Score"
		0x00000190,
		menuhandlerMpTeamScoreLimitSlider,
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
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Restore Defaults\n"), // "Restore Defaults"
		0,
		menuhandlerMpRestoreScoreDefaults,
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

struct menudialogdef g_MpLimitsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Limits\n"), // "Limits"
	g_MpLimitsMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpHandicapsMenuItems[] = {
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextHandicapPlayerName,
		0x000000ff,
		menuhandlerMpHandicapPlayer,
	},
	{
		MENUITEMTYPE_SLIDER,
		1,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextHandicapPlayerName,
		0x000000ff,
		menuhandlerMpHandicapPlayer,
	},
	{
		MENUITEMTYPE_SLIDER,
		2,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextHandicapPlayerName,
		0x000000ff,
		menuhandlerMpHandicapPlayer,
	},
	{
		MENUITEMTYPE_SLIDER,
		3,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextHandicapPlayerName,
		0x000000ff,
		menuhandlerMpHandicapPlayer,
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
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Restore Defaults\n"), // "Restore Defaults"
		0,
		menuhandlerMpRestoreHandicapDefaults,
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

struct menudialogdef g_MpHandicapsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Handicaps\n"), // "Player Handicaps"
	g_MpHandicapsMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpReadyMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("...and waiting\n"), // "...and waiting"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpReadyMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	gettext_noop("Ready!\n"), // "Ready!"
	g_MpReadyMenuItems,
	menudialogMpReady,
	MENUDIALOGFLAG_CLOSEONSELECT,
	NULL,
};

MenuItemHandlerResult mpAddChangeSimulantMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 i;
	s32 count = 0;

	struct optiongroup groups[] = {
		{ 0, gettext_noop("Normal Simulants\n") }, // "Normal Simulants"
		{ 6, gettext_noop("Special Simulants\n") }, // "Special Simulants"
	};

	s32 botnum;
	bool creating;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		for (i = 0; i < ARRAYCOUNT(g_BotProfiles); i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				count++;
			}
		}

		data->list.value = count;
		break;
	case MENUOP_GETOPTIONTEXT:
		for (i = 0; i < ARRAYCOUNT(g_BotProfiles); i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				if (count == data->list.value) {
					return (uintptr_t)_(g_BotProfiles[i].name);
				}

				count++;
			}
		}
		break;
	case MENUOP_SET:
		botnum = g_Menus[g_MpPlayerNum].mpsetup.slotindex;
		creating = false;

		if (botnum < 0) {
			botnum = mpGetSlotForNewBot();
			creating = 1;
		} else if ((g_MpSetup.chrslots & (1 << (botnum + 4))) == 0) {
			creating = 1;
		}

		for (i = 0; i < ARRAYCOUNT(g_BotProfiles); i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				if (count == data->list.value) {
					break;
				}

				count++;
			}
		}

		if (creating) {
			mpCreateBotFromProfile(botnum, i);
		} else {
			g_BotConfigsArray[botnum].type = g_BotProfiles[i].type;

			if (g_BotConfigsArray[botnum].type == BOTTYPE_GENERAL) {
				mpSetBotDifficulty(botnum, g_BotProfiles[i].difficulty);
			}
		}

		mpGenerateBotNames();
		g_Menus[g_MpPlayerNum].mpsetup.slotcount = data->list.value;
		break;
	case MENUOP_LISTITEMFOCUS:
		for (i = 0; i < ARRAYCOUNT(g_BotProfiles); i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				if (count == data->list.value) {
					break;
				}

				count++;
			}
		}

		g_Menus[g_MpPlayerNum].mpsetup.unke24 = i;
		// fall-through
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = g_Menus[g_MpPlayerNum].mpsetup.slotcount;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 2;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return (uintptr_t)_(groups[data->list.value].name);
	case MENUOP_GETGROUPSTARTINDEX:
		for (i = 0; i < groups[data->list.value].offset; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				count++;
			}
		}

		data->list.groupstartindex = count;
		break;
	}

	return 0;
}

char *mpMenuTextSimulantDescription(struct menuitem *item)
{
	//TODO - lang: Fix it, Simulant description
	//return langGet(gettext_noop("") + g_Menus[g_MpPlayerNum].mpsetup.unke24);
	return _("TODO");
}

MenuItemHandlerResult menuhandlerMpSimulantHead(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 start = 0;

	if (item->title == 1) {
		start = mpGetNumHeads();
	}

	/**
	 * Rare developers forgot to add a break statement to the first case,
	 * and when they noticed a problem their fix was to add an additional
	 * MENUOP_FOCUS check in the next case.
	 */
	switch (operation) {
	case MENUOP_SET:
		g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpheadnum = start + data->carousel.value;
	case MENUOP_FOCUS:
		if (operation == MENUOP_FOCUS
				&& item->title == 1
				&& g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpheadnum < start) {
			g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpheadnum = start;
		}
		break;
	}

	return mpCharacterHeadMenuHandler(operation, item, data, g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpheadnum, 0);
}

MenuItemHandlerResult menuhandlerMpSimulantBody(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpbodynum = data->carousel.value;
	}

	return mpCharacterBodyMenuHandler(operation, item, data,
			g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpbodynum,
			g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.mpheadnum,
			false);
}

MenuDialogHandlerResult menudialog0017ccfc(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_TICK:
		if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem != &dialogdef->items[0]
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem != &dialogdef->items[1]) {
			union handlerdata data;
			menuhandlerMpCharacterBody(MENUOP_11, &dialogdef->items[1], &data);
		}
	}

	return menudialogMpSimulant(operation, dialogdef, data);
}

MenuItemHandlerResult mpBotDifficultyMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 count = 0;
	s32 i;

	switch (operation) {
	case MENUOP_SET:
		mpSetBotDifficulty(g_Menus[g_MpPlayerNum].mpsetup.slotindex, data->dropdown.value);
		mpGenerateBotNames();
		break;
	case MENUOP_GETSELECTEDINDEX:
		if (g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].difficulty >= 0
				&& g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].difficulty < BOTDIFF_DISABLED) {
			data->dropdown.value = g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].difficulty;
		} else {
			data->dropdown.value = 0;
		}
		break;
	case MENUOP_GETOPTIONCOUNT:
		for (i = 0; i < BOTDIFF_DISABLED; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				count++;
			}
		}

		data->dropdown.value = count;
		break;
	case MENUOP_GETOPTIONTEXT:
		for (i = 0; i < BOTDIFF_DISABLED; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				if (count == data->dropdown.value) {
					// "Meat", "Easy", "Normal" etc 
					// return (uintptr_t) langGet(gettext_noop("Meat") + i);
					return (uintptr_t) _("TODO"); // TODO - Lang: Fix it, simulant name
				}

				count++;
			}
		}

		return (uintptr_t)"\n";
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpDeleteSimulant(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		mpRemoveSimulant(g_Menus[g_MpPlayerNum].mpsetup.slotindex);
		menuPopDialog();
	}

	return 0;
}

#ifndef PLATFORM_N64
MenuItemHandlerResult menuhandlerMpCopySimulant(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		mpCopySimulant(g_Menus[g_MpPlayerNum].mpsetup.slotindex);
		menuPopDialog();
		break;
	case MENUOP_CHECKDISABLED:
		if (mpHasUnusedBotSlots() == 0) {
			return true;
		}
	}

	return 0;
}
#endif

char *mpMenuTitleEditSimulant(struct menudialogdef *dialogdef)
{
	sprintf(g_StringPointer, "%s", &g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].base.name);
	return g_StringPointer;
}

MenuItemHandlerResult menuhandlerMpChangeSimulantType(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;
		s32 count = 0;
		s32 profilenum = mpFindBotProfile(
				g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].type,
				g_BotConfigsArray[g_Menus[g_MpPlayerNum].mpsetup.slotindex].difficulty);

		for (i = 0; i < profilenum; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				count++;
			}
		}

		g_Menus[g_MpPlayerNum].mpsetup.slotcount = count;

		menuPushDialog(&g_MpChangeSimulantMenuDialog);
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpClearAllSimulants(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;
		for (i = 0; i < MAX_BOTS; i++) {
			mpRemoveSimulant(i);
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpAddSimulant(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		g_Menus[g_MpPlayerNum].mpsetup.slotindex = -1;
		menuPushDialog(&g_MpAddSimulantMenuDialog);
		break;
	case MENUOP_CHECKDISABLED:
		if (mpHasUnusedBotSlots() == 0) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSimulantSlot(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		g_Menus[g_MpPlayerNum].mpsetup.slotindex = item->param;

		if ((g_MpSetup.chrslots & (1 << (item->param + 4))) == 0) {
			menuPushDialog(&g_MpAddSimulantMenuDialog);
		} else if (IS4MB()) {
			menuPushDialog(&g_MpEditSimulant4MbMenuDialog);
		} else {
			menuPushDialog(&g_MpEditSimulantMenuDialog);
		}
		break;
	case MENUOP_CHECKHIDDEN:
		if (item->param >= 4 && !challengeIsFeatureUnlocked(MPFEATURE_8BOTS)) {
			return true;
		}
		break;
	case MENUOP_CHECKDISABLED:
		if (!mpIsSimSlotEnabled(item->param)) {
			return true;
		}
	}

	return 0;
}

char *mpMenuTextSimulantName(struct menuitem *item)
{
	s32 index = item->param;

	if (g_BotConfigsArray[index].base.name[0] == '\0' || (g_MpSetup.chrslots & 1 << (index + 4)) == 0) {
		return "";
	}

	return g_BotConfigsArray[index].base.name;
}

char *func0f17d3dc(struct menuitem *item)
{
	s32 index = item->param;

	if (g_BotConfigsArray[index].base.name[0] == '\0'
			|| ((g_MpSetup.chrslots & 1 << (index + 4)) == 0)) {
		return "";
	}

	sprintf(g_StringPointer, "%d:\n", index + 1);
	return g_StringPointer;
}

MenuDialogHandlerResult menudialogMpSimulants(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Menus[g_MpPlayerNum].mpsetup.slotcount = 0;
	}

	return false;
}

struct menuitem g_MpAddChangeSimulantMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		"", // previous: 0x00000078
		0x00000042,
		mpAddChangeSimulantMenuHandler,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&mpMenuTextSimulantDescription,
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpAddSimulantMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Add Simulant\n"), // "Add Simulant"
	g_MpAddChangeSimulantMenuItems,
	NULL,
	MENUDIALOGFLAG_CLOSEONSELECT | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menudialogdef g_MpChangeSimulantMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Change Simulant\n"), // "Change Simulant"
	g_MpAddChangeSimulantMenuItems,
	menudialogMpSimulant,
	MENUDIALOGFLAG_CLOSEONSELECT | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpSimulantCharacterMenuItems[] = {
	{
		MENUITEMTYPE_CAROUSEL,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		0,
		0x00000025,
		menuhandlerMpSimulantHead,
	},
	{
		MENUITEMTYPE_CAROUSEL,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		0,
		0x0000001b,
		menuhandlerMpSimulantBody,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSimulantCharacterMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Simulant Character\n"), // "Simulant Character"
	g_MpSimulantCharacterMenuItems,
	menudialog0017ccfc,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpEditSimulantMenuItems[] = {
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
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Character...\n"), // "Character..."
		0,
		(void *)&g_MpSimulantCharacterMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Copy Simulant\n"),
		0,
		menuhandlerMpCopySimulant,
	},
#endif
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

struct menudialogdef g_MpEditSimulantMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTitleEditSimulant,
	g_MpEditSimulantMenuItems,
	menudialogMpSimulant,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpSimulantsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Add Simulant...\n"), // "Add Simulant..."
		0,
		menuhandlerMpAddSimulant,
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
		gettext_noop("1:\n"), // "1:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		gettext_noop("2:\n"), // "2:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		gettext_noop("3:\n"), // "3:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		3,
		0,
		gettext_noop("4:\n"), // "4:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		4,
		0,
		gettext_noop("5:\n"), // "5:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		5,
		0,
		gettext_noop("6:\n"), // "6:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		6,
		0,
		gettext_noop("7:\n"), // "7:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		7,
		0,
		gettext_noop("8:\n"), // "8:"
		(uintptr_t)&mpMenuTextSimulantName,
		menuhandlerMpSimulantSlot,
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
		gettext_noop("Clear All\n"), // "Clear All"
		0,
		menuhandlerMpClearAllSimulants,
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

struct menudialogdef g_MpSimulantsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Simulants\n"), // "Simulants"
	g_MpSimulantsMenuItems,
	menudialogMpSimulants,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

MenuItemHandlerResult menuhandlerMpNTeams(s32 operation, struct menuitem *item, union handlerdata *data, s32 numteams)
{
	if (operation == MENUOP_SET) {
		s32 numchrs = mpGetNumChrs();
		s32 array[] = {0, 0, 0, 0};
		s32 somevalue = (numchrs + numteams - 1) / numteams;
		s32 teamsremaining = numteams;
		s32 chrsremaining = numchrs;
		s32 start = rngRandom() % numchrs;

		s32 i;
		s32 teamnum;

#if VERSION >= VERSION_NTSC_1_0
		if (!numchrs) {
			return 0;
		}
#endif

		i = (start + 1) % numchrs;

		do {
			struct mpchrconfig *mpchr = mpGetChrConfigBySlotNum(i);

#if VERSION >= VERSION_NTSC_1_0
			if (teamsremaining);
#else
			if (start);
#endif

			if (teamsremaining >= chrsremaining) {
				teamnum = rngRandom() % numteams;

				while (true) {
					if (array[teamnum] == 0) {
						mpchr->team = teamnum;

						array[teamnum]++;
						teamsremaining--;
						chrsremaining--;
						break;
					} else {
						teamnum = (teamnum + 1) % numteams;
					}
				}
			} else {
				teamnum = rngRandom() % numteams;

				while (true) {
					if (array[teamnum] < somevalue) {
						mpchr->team = teamnum;

						if (array[teamnum] == 0) {
							teamsremaining--;
						}

						array[teamnum]++;
						chrsremaining--;
						break;
					} else {
						teamnum = (teamnum + 1) % numteams;
					}
				}
			}

			if (i == start) {
				break;
			}

			i = (i + 1) % numchrs;
		} while (true);

		menuPopDialog();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpTwoTeams(s32 operation, struct menuitem *item, union handlerdata *data)
{
	return menuhandlerMpNTeams(operation, item, data, 2);
}

MenuItemHandlerResult menuhandlerMpThreeTeams(s32 operation, struct menuitem *item, union handlerdata *data)
{
	return menuhandlerMpNTeams(operation, item, data, 3);
}

MenuItemHandlerResult menuhandlerMpFourTeams(s32 operation, struct menuitem *item, union handlerdata *data)
{
	return menuhandlerMpNTeams(operation, item, data, 4);
}

MenuItemHandlerResult menuhandlerMpMaximumTeams(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;
		u8 team = 0;

		for (i = 0; i != MAX_MPCHRS; i++) {
			if (g_MpSetup.chrslots & (1 << i)) {
				struct mpchrconfig *mpchr = MPCHR(i);

				mpchr->team = team++;

				if (team >= scenarioGetMaxTeams()) {
					team = 0;
				}
			}
		}

		menuPopDialog();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpHumansVsSimulants(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;

		for (i = 0; i != MAX_MPCHRS; i++) {
			if (g_MpSetup.chrslots & (1 << i)) {
				struct mpchrconfig *mpchr = MPCHR(i);

				mpchr->team = i < 4 ? 0 : 1;
			}
		}

		menuPopDialog();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpHumanSimulantPairs(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		u8 team_ids[4] = {0, 1, 2, 3};
		s32 i;
		s32 playerindex = 0;
		s32 simindex = 0;

		for (i = 0; i != MAX_MPCHRS; i++) {
			if (g_MpSetup.chrslots & (1 << i)) {
				struct mpchrconfig *mpchr = MPCHR(i);

				if (i < 4) {
					mpchr->team = team_ids[playerindex++];
				} else {
					mpchr->team = team_ids[simindex++];

					if (simindex >= playerindex) {
						simindex = 0;
					}
				}
			}
		}

		menuPopDialog();
	}

	return 0;
}

char *mpMenuTextChrNameForTeamSetup(struct menuitem *item)
{
	struct mpchrconfig *mpchr = mpGetChrConfigBySlotNum(item->param);

	if (mpchr) {
		return mpchr->name;
	}

	return "";
}

MenuItemHandlerResult func0f17dac4(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = scenarioGetMaxTeams();
		break;
	case MENUOP_GETOPTIONTEXT:
		if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) == 0) {
			return (uintptr_t) "\n";
		}

		return (uintptr_t) g_BossFile.teamnames[data->list.value];
	}

	return menuhandlerMpTeamsLabel(operation, item, data);
}

MenuItemHandlerResult menuhandlerMpTeamSlot(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct mpchrconfig *mpchr;

	switch (operation) {
	case MENUOP_SET:
		mpchr = mpGetChrConfigBySlotNum(item->param);
		mpchr->team = data->dropdown.value;
		break;
	case MENUOP_GETSELECTEDINDEX:
		mpchr = mpGetChrConfigBySlotNum(item->param);

		if (!mpchr) {
			data->dropdown.value = 0xff;
		} else {
			data->dropdown.value = mpchr->team;
		}

		break;
	case MENUOP_CHECKDISABLED:
		mpchr = mpGetChrConfigBySlotNum(item->param);

		if (!mpchr) {
			return 1;
		}

		return menuhandlerMpTeamsLabel(operation, item, data);
	}

	return func0f17dac4(operation, item, data);
}

char *mpMenuTextSelectTuneOrTunes(struct menuitem *item)
{
	// TODO - Lang: Fix it, use plural feature
	if (mpGetUsingMultipleTunes()) {
		return _("Select Tunes\n"); // "Select Tune"
	}

	return _("Select Tune\n"); // "Select Tunes"
}

struct menuitem g_MpAutoTeamMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Two Teams\n"), // "Two Teams"
		0,
		menuhandlerMpTwoTeams,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Three Teams\n"), // "Three Teams"
		0,
		menuhandlerMpThreeTeams,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Four Teams\n"), // "Four Teams"
		0,
		menuhandlerMpFourTeams,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Maximum Teams\n"), // "Maximum Teams"
		0,
		menuhandlerMpMaximumTeams,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Humans vs. Simulants\n"), // "Humans vs. Simulants"
		0,
		menuhandlerMpHumansVsSimulants,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Human-Simulant Pairs\n"), // "Human-Simulant Pairs"
		0,
		menuhandlerMpHumanSimulantPairs,
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
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpAutoTeamMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Auto Team\n"), // "Auto Team"
	g_MpAutoTeamMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpTeamsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Teams Enabled\n"), // "Teams Enabled"
		0x00000002,
		menuhandlerMpTeamsEnabled,
	},
#if VERSION >= VERSION_PAL_FINAL
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x85,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Teams:\n"), // "Teams:"
		0,
		menuhandlerMpTeamsLabel,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		1,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		2,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		3,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		4,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		5,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		6,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		7,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		8,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		9,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		10,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		11,
		MENUITEMFLAG_ADJUSTWIDTH | MENUITEMFLAG_LOCKABLEMINOR,
		(uintptr_t)&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
#else
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
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Teams:\n"), // "Teams:"
		0,
		menuhandlerMpTeamsLabel,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		1,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		2,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		3,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		4,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		5,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		6,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		7,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		8,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		9,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		10,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		11,
		MENUITEMFLAG_LOCKABLEMINOR,
		&mpMenuTextChrNameForTeamSetup,
		0,
		menuhandlerMpTeamSlot,
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
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Auto Team...\n"), // "Auto Team..."
		0,
		(void *)&g_MpAutoTeamMenuDialog,
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

struct menudialogdef g_MpTeamsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Team Control\n"), // "Team Control"
	g_MpTeamsMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

char *trackList[] = {
	gettext_noop("Random\n"), // "Random"
	gettext_noop("Select All\n"), // "Select All"
	gettext_noop("Select None\n"), // "Select None"
	gettext_noop("Randomize\n"), // "Randomize"
};

/**
 * List handler for the select tune dialog.
 *
 * If multiple tracks are disabled, the listing contains the track listing plus
 * one item for Randomize.
 *
 * If multiple tracks are disabled, the listing contains the track listing plus
 * 3 items for Select All, Select None and Randomize.
 */
MenuItemHandlerResult mpSelectTuneListHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = mpGetNumUnlockedTracks();

		if (mpGetUsingMultipleTunes()) {
			data->list.value += 3;
		} else {
			data->list.value++;
		}
		break;
	case MENUOP_GETOPTIONTEXT:
		{
			s32 numtracks = mpGetNumUnlockedTracks();

			if (data->list.value < numtracks) {
				return (uintptr_t) mpGetTrackName(data->list.value);
			}

			if (mpGetUsingMultipleTunes()) {
				return (uintptr_t) _(trackList[1 + data->list.value - numtracks]);
			}

			return (uintptr_t) _(trackList[data->list.value - numtracks]);
		}
	case MENUOP_SET:
		{
			s32 numtracks = mpGetNumUnlockedTracks();

			if (data->list.value < numtracks) {
				if (data->list.unk04 == 0) {
					mpSetTrackSlotEnabled(data->list.value);
				}
				g_Vars.modifiedfiles |= MODFILE_MPSETUP;
			} else if (mpGetUsingMultipleTunes()) {
				s32 index = data->list.value - numtracks;

				switch (index) {
				case 0:
					mpEnableAllMultiTracks();
					g_Vars.modifiedfiles |= MODFILE_MPSETUP;
					break;
				case 1:
					mpDisableAllMultiTracks();
					g_Vars.modifiedfiles |= MODFILE_MPSETUP;
					break;
				case 2:
					mpRandomiseMultiTracks();
					g_Vars.modifiedfiles |= MODFILE_MPSETUP;
					break;
				}
			} else {
				mpSetTrackToRandom();
				g_Vars.modifiedfiles |= MODFILE_MPSETUP;
			}
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		if (mpGetUsingMultipleTunes()) {
			data->list.value = 0x000fffff;
		} else {
			s32 slotnum = mpGetCurrentTrackSlotNum();

			if (slotnum < 0) {
				data->list.value = mpGetNumUnlockedTracks();
			} else {
				data->list.value = slotnum;
			}
		}
		break;
	case MENUOP_LISTITEMFOCUS:
		if (data->list.value < mpGetNumUnlockedTracks()) {
			musicStartTrackAsMenu(mpGetTrackMusicNum(data->list.value));
		}
		break;
	case MENUOP_GETLISTITEMCHECKBOX:
		{
			s32 numtracks = mpGetNumUnlockedTracks();

			if (mpGetUsingMultipleTunes() && data->list.value < numtracks) {
				data->list.unk04 = mpIsMultiTrackSlotEnabled(data->list.value);
			}
		}
		break;
	}

	return 0;
}

MenuDialogHandlerResult menudialogMpSelectTune(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_MusicInterval240 = 80;
	}

	if (operation == MENUOP_CLOSE) {
		g_MusicInterval240 = 15;
	}

	return false;
}

char *mpMenuTextCurrentTrack(struct menuitem *item)
{
	s32 slotnum;

	if (mpGetUsingMultipleTunes()) {
		return _("Multiple Tunes\n"); // "Multiple Tunes"
	}

	slotnum = mpGetCurrentTrackSlotNum();

	if (slotnum >= 0) {
		return mpGetTrackName(slotnum);
	}

	return _("Random\n"); // "Random"
}

MenuItemHandlerResult menuhandlerMpMultipleTunes(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		return mpGetUsingMultipleTunes();
	case MENUOP_SET:
		mpSetUsingMultipleTunes(data->checkbox.value);
		g_Vars.modifiedfiles |= MODFILE_MPSETUP;
	}

	return 0;
}

MenuItemHandlerResult mpTeamNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;
	s32 i;

	switch (operation) {
	case MENUOP_GETTEXT:
		i = 0;

		while (g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i] != '\n'
				&& g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i] != '\0'
				&& i < 11) {
			name[i] = g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i];
			i++;
		}

		while (i < 11) {
			name[i] = '\0';
			i++;
		}
		break;
	case MENUOP_SETTEXT:
		i = 0;

		while (i < 11 && name[i] != '\0') {
			g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i] = name[i];
			i++;
		}

		g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i] = '\n';
		i++;

		while (i < 11) {
			g_BossFile.teamnames[g_Menus[g_MpPlayerNum].mpsetup.slotindex][i] = '\0';
			i++;
		}

		g_Vars.modifiedfiles |= MODFILE_MPSETUP;
		break;
	}

	return 0;
}

/**
 * item->param2 is a text ID for that team's colour. The text IDs for team
 * colours are consecutive, so the index of the team is determined by
 * subtracting the first team's colour text ID.
 */
char *mpMenuTextTeamName(struct menuitem *item)
{
	s32 index = 0;
	// TODO - Lang: Fix it, properly get team name
	/*s32 index = item->title;
	index -= gettext_noop("Red\n");*/

	return g_BossFile.teamnames[index];
}

MenuItemHandlerResult menuhandlerMpTeamNameSlot(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Menus[g_MpPlayerNum].mpsetup.slotindex = (uintptr_t)item->title - 0x5608; //TODO - Lang: Fix it, will crash :D
		menuPushDialog(&g_MpChangeTeamNameMenuDialog);
	}

	return 0;
}

char *func0f17e318(struct menudialogdef *dialogdef)
{
	sprintf(g_StringPointer, _("%s\n"), challengeGetNameBySlot(g_Menus[g_MpPlayerNum].mpsetup.slotindex));
	return g_StringPointer;
}

/**
 * An "Accept" item somewhere. Probably accepting a challenge.
 */
MenuItemHandlerResult menuhandler0017e38c(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
#if VERSION >= VERSION_NTSC_1_0
		challengeUnsetCurrent();
#endif

		menuPopDialog();
		challengeSetCurrentBySlot(g_Menus[g_MpPlayerNum].mpsetup.slotindex);
	}

	return 0;
}

MenuDialogHandlerResult menudialog0017e3fc(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		g_Menus[g_MpPlayerNum].menumodel.curparams = 0;

		g_Menus[g_MpPlayerNum].training.mpconfig = challengeLoadBySlot(
				g_Menus[g_MpPlayerNum].training.unke1c,
				g_Menus[g_MpPlayerNum].menumodel.allocstart,
				g_Menus[g_MpPlayerNum].menumodel.alloclen);
		break;
	case MENUOP_CLOSE:
		break;
	case MENUOP_TICK:
		if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
			menuPopDialog();
		}
		break;
	}

	return 0;
}

struct menuitem g_MpSelectTunesMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		"", // previous: 0x00000078
		0x0000004d,
		mpSelectTuneListHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpSelectTunesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&mpMenuTextSelectTuneOrTunes,
	g_MpSelectTunesMenuItems,
	menudialogMpSelectTune,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpSoundtrackMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("Current:\n"), // "Current:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		"", // ""
		(uintptr_t)&mpMenuTextCurrentTrack,
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
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		&mpMenuTextSelectTuneOrTunes,
		0,
		(void *)&g_MpSelectTunesMenuDialog,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Multiple Tunes\n"), // "Multiple Tunes"
		0,
		menuhandlerMpMultipleTunes,
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

struct menudialogdef g_MpSoundtrackMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Soundtrack\n"), // "Soundtrack"
	g_MpSoundtrackMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpChangeTeamNameMenuItems[] = {
	{
		MENUITEMTYPE_KEYBOARD,
		0,
		0,
		0,
		0,
		mpTeamNameMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpChangeTeamNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Change Team Name\n"), // "Change Team Name"
	g_MpChangeTeamNameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpTeamNamesMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Red\n"), // "Red"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Yellow\n"), // "Yellow"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Blue\n"), // "Blue"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Magenta\n"), // "Magenta"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Cyan\n"), // "Cyan"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Orange\n"), // "Orange"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Pink\n"), // "Pink"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Brown\n"), // "Brown"
		(uintptr_t)&mpMenuTextTeamName,
		menuhandlerMpTeamNameSlot,
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

struct menudialogdef g_MpTeamNamesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Team Names\n"), // "Team Names"
	g_MpTeamNamesMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpConfirmChallengeViaListOrDetailsMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_MPCONFIG,
		0,
		"", // previous: 0x0000007c,
		PAL ? 0x41 : 0x37,
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
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Accept\n"), // "Accept"
		0,
		menuhandler0017e38c,
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

struct menudialogdef g_MpConfirmChallengeViaListOrDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&func0f17e318,
	g_MpConfirmChallengeViaListOrDetailsMenuItems,
	menudialog0017e3fc,
	MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menuitem g_MpChallengesListOrDetailsMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"", // previous: 0x00000078
		0x0000004d,
		mpChallengesListMenuHandler,
	},
#if VERSION < VERSION_NTSC_1_0
	{
		MENUITEMTYPE_LABEL,
		2,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_LABEL_ALTCOLOUR,
		0x7f179198,
		0,
		(void *)0x7f1790a8,
	},
#endif
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_MPCHALLENGE,
		0,
		"", // previous: 0x0000007c,
		PAL ? 0x41 : 0x37,
		menuhandler0017e9d8,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		menuhandler0017e9d8,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Start Challenge\n"), // "Start Challenge"
		0,
		menuhandlerMpStartChallenge,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Abort Challenge\n"), // "Abort Challenge"
		0,
		menuhandlerMpAbortChallenge,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpChallengeListOrDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
#if VERSION >= VERSION_NTSC_1_0
	&mpMenuTextChallengeName,
#else
	0x5032,
#endif
	g_MpChallengesListOrDetailsMenuItems,
	mpCombatChallengesMenuDialog,
#if VERSION >= VERSION_NTSC_1_0
	0x00000808,
#else
	MENUDIALOGFLAG_DROPOUTONCLOSE,
#endif
	NULL,
};

struct menudialogdef g_MpAdvancedSetupViaAdvChallengeMenuDialog;

struct menudialogdef g_MpChallengeListOrDetailsViaAdvChallengeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
#if VERSION >= VERSION_NTSC_1_0
	&mpMenuTextChallengeName,
#else
	0x5032,
#endif
	g_MpChallengesListOrDetailsMenuItems,
	mpCombatChallengesMenuDialog,
#if VERSION >= VERSION_NTSC_1_0
	0x00000808,
	&g_MpAdvancedSetupViaAdvChallengeMenuDialog,
#else
	MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpAdvancedSetupMenuDialog,
#endif
};

struct menuitem g_MpConfirmChallengeMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_MPCONFIG,
		0,
		"", // previous: 0x0000007c,
		PAL ? 0x41 : 0x37,
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
		menuhandler0017ec64,
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

struct menudialogdef g_MpConfirmChallengeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	&func0f17e318,
	g_MpConfirmChallengeMenuItems,
	menudialog0017e3fc,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

MenuItemHandlerResult mpChallengesListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	s32 x;
	s32 y;
	s32 maxchrs;
	s32 marginleft;
	s32 i;

	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
			return 1;
		}
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = challengeGetNumAvailable();
		break;
	case MENUOP_SET:
		if (data->list.unk04 != 0) {
			data->list.unk04 = 2;
		}

		g_Menus[g_MpPlayerNum].mpsetup.slotindex = data->list.value;

		if (item->param == 0) {
			menuPushDialog(&g_MpConfirmChallengeViaListOrDetailsMenuDialog);
		} else if (IS4MB()) {
			menuPushDialog(&g_MpConfirmChallenge4MbMenuDialog);
		} else {
			menuPushDialog(&g_MpConfirmChallengeMenuDialog);
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 0;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return 0;
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = 0;
		break;
	case MENUOP_RENDER:
		gdl = data->type19.gdl;
		renderdata = data->type19.renderdata2;
		marginleft = 10;
		maxchrs = 4;

		if (IS4MB()) {
			maxchrs = 2;
		}

		x = renderdata->x + 10;
		y = renderdata->y + 1;

		gdl = text0f153628(gdl);
		gdl = textRenderProjected(gdl, &x, &y, challengeGetNameBySlot(data->type19.unk04), g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);

		texSelect(&gdl, &g_TexGeneralConfigs[35], 2, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetTextureFilter(gdl++, G_TF_POINT);

		for (i = 0; i < maxchrs; i++) {
#if VERSION >= VERSION_NTSC_1_0
			if (challengeIsCompletedByAnyChrWithNumPlayersBySlot(data->type19.unk04, i + 1)) {
				gDPSetEnvColorViaWord(gdl++, (renderdata->colour & 0xff) * 0xff >> 8 | 0xffe56500);
			} else {
				gDPSetEnvColorViaWord(gdl++, (renderdata->colour & 0xff) * 0xff >> 8 | 0x43430000);
			}
#else
			if (challengeIsCompletedByAnyChrWithNumPlayersBySlot(data->type19.unk04, i + 1)) {
				gDPSetEnvColorViaWord(gdl++, 0xffe565ff);
			} else {
				gDPSetEnvColorViaWord(gdl++, 0x434300ff);
			}
#endif

			gDPSetCombineLERP(gdl++,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0);

			gSPTextureRectangle(gdl++,
				((renderdata->x + marginleft) << 2) * g_ScaleX, (renderdata->y + 11) << 2,
				((renderdata->x + marginleft + 11) << 2) * g_ScaleX, (renderdata->y + 22) << 2,
				G_TX_RENDERTILE, 0, 0x0160, 1024 / g_ScaleX, -1024);

			marginleft += 13;
		}
		return (uintptr_t)gdl;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = 26;
		break;
	}

	return 0;
}

/**
 * This is for a separator and fixed height thing in the dialog at:
 * Combat Simulator > Advanced Setup > Challenges > pick one > Accept
 */
MenuItemHandlerResult menuhandler0017e9d8(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_BossFile.locktype != MPLOCKTYPE_CHALLENGE) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpAbortChallenge(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_BossFile.locktype != MPLOCKTYPE_CHALLENGE) {
			return true;
		}
	}

	if (operation == MENUOP_SET) {
		challengeRemovePlayerLock();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpStartChallenge(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_BossFile.locktype != MPLOCKTYPE_CHALLENGE) {
			return true;
		}
	}
	if (operation == MENUOP_SET) {
		menuPushDialog(&g_MpReadyMenuDialog);
	}

	return 0;
}

char *mpMenuTextChallengeName(struct menuitem *item)
{
#if VERSION >= VERSION_NTSC_1_0
	if (g_BossFile.locktype != MPLOCKTYPE_CHALLENGE) {
		return _("Combat Challenges\n"); // "Combat Challenges"
	}
#endif

	sprintf(g_StringPointer, "%s:\n", challengeGetName(challengeGetCurrent()));
	return g_StringPointer;
}

MenuDialogHandlerResult mpCombatChallengesMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE
				&& g_Menus[g_MpPlayerNum].curdialog
				&& g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef
				&& !challengeIsLoaded()) {
			g_Menus[g_MpPlayerNum].menumodel.curparams = 0x4fac5ace;

			challengeLoadAndStoreCurrent(
					g_Menus[g_MpPlayerNum].menumodel.allocstart,
					g_Menus[g_MpPlayerNum].menumodel.alloclen);
		}
	}

	if (operation == MENUOP_CLOSE) {
		if (g_Menus[g_MpPlayerNum].menumodel.curparams == 0x4fac5ace) {
			challengeUnsetCurrent();
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandler0017ec64(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		challengeSetCurrentBySlot(g_Menus[g_MpPlayerNum].mpsetup.slotindex);
		func0f0f820c(&g_MpQuickGoMenuDialog, 3);
	}

	return 0;
}

struct menuitem g_MpChallengesMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		1,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"", // previous: 0x00000078
		0x0000004d,
		mpChallengesListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpChallengesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Combat Challenges\n"), // "Combat Challenges"
	g_MpChallengesMenuItems,
	mpCombatChallengesMenuDialog,
	0,
	NULL,
};

MenuItemHandlerResult menuhandlerMpLock(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *labels[] = {
		_("None\n"), // "None"
		_("Last Winner\n"), // "Last Winner"
		_("Last Loser\n"), // "Last Loser"
		_("Random\n"), // "Random"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = mpGetLockType() == MPLOCKTYPE_CHALLENGE ? 1 : 5;
		break;
	case MENUOP_GETOPTIONTEXT:
		if (mpGetLockType() == MPLOCKTYPE_CHALLENGE) {
			return (uintptr_t) _("Challenge\n"); // "Challenge"
		}
		if (data->dropdown.value <= 3) {
			return (uintptr_t) labels[data->dropdown.value];
		}
		if (mpGetLockType() == MPLOCKTYPE_PLAYER) {
			return (uintptr_t) g_PlayerConfigsArray[mpGetLockPlayerNum()].base.name;
		}
		return (uintptr_t) mpGetCurrentPlayerName(item);
	case MENUOP_SET:
		if (mpGetLockType() != MPLOCKTYPE_CHALLENGE) {
			mpSetLock(data->dropdown.value, g_MpPlayerNum);
		}
		g_Vars.modifiedfiles |= MODFILE_MPSETUP;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = mpGetLockType() == MPLOCKTYPE_CHALLENGE ? 0 : mpGetLockType();
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSavePlayer(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_PlayerConfigsArray[g_MpPlayerNum].fileguid.fileid == 0) {
			filemgrPushSelectLocationDialog(6, FILETYPE_MPPLAYER);
		} else {
			menuPushDialog(&g_MpSavePlayerMenuDialog);
		}
	}

	return 0;
}

char *mpMenuTextSavePlayerOrCopy(struct menuitem *item)
{
	if (g_PlayerConfigsArray[g_MpPlayerNum].fileguid.fileid == 0) {
		return _("Save Player\n"); // "Save Player"
	}

	return _("Save Copy of Player\n"); // "Save Copy of Player"
}

MenuItemHandlerResult menuhandler0017ef30(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_Vars.stagenum == STAGE_CITRAINING) {
			if (IS4MB()) {
				func0f0f820c(&g_CiMenuViaPauseMenuDialog, 2);
			} else {
				func0f0f820c(&g_CiMenuViaPcMenuDialog, 2);
			}
		} else {
			func0f0f820c(&g_SoloMissionPauseMenuDialog, 2);
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSaveSettings(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_MpCurrentSetup < 0) {
			menuPushDialog(&g_MpSaveSetupNameMenuDialog);
		}
		else {
			menuPushDialog(&g_MpSaveSetupExistsMenuDialog);
		}
	}

	return 0;
}

char *mpMenuTextArenaName(struct menuitem *item)
{
	s32 i;

	for (i = 0; i != ARRAYCOUNT(g_MpArenas); i++) {
		if (g_MpArenas[i].stagenum == g_MpSetup.stagenum) {
			return _(g_MpArenas[i].name);
		}
	}

	return "\n";
}

char *mpMenuTextWeaponSetName(struct menuitem *item)
{
	return mpGetWeaponSetName(mpGetWeaponSet());
}

MenuDialogHandlerResult menudialogMpGameSetup(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Vars.mpsetupmenu = MPSETUPMENU_ADVSETUP;
		g_Vars.usingadvsetup = true;
	}

	return false;
}

MenuDialogHandlerResult menudialogMpQuickGo(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Vars.mpsetupmenu = MPSETUPMENU_QUICKGO;
	}

	return false;
}

void mpConfigureQuickTeamPlayers(void)
{
	s32 i;

	if (g_Vars.mpquickteam != MPQUICKTEAM_NONE) {
		for (i = 0; i < MAX_BOTS; i++) {
			mpRemoveSimulant(i);
		}

		switch (g_Vars.mpquickteam) {
		case MPQUICKTEAM_PLAYERSONLY:
			g_MpSetup.options &= ~MPOPTION_TEAMSENABLED;
			break;
		case MPQUICKTEAM_PLAYERSANDSIMS:
			g_MpSetup.options &= ~MPOPTION_TEAMSENABLED;
			break;
		case MPQUICKTEAM_PLAYERSTEAMS:
			g_MpSetup.options |= MPOPTION_TEAMSENABLED;

			for (i = 0; i < MAX_PLAYERS; i++) {
				g_PlayerConfigsArray[i].base.team = g_Vars.mpplayerteams[i];
			}

			break;
		case MPQUICKTEAM_PLAYERSVSSIMS:
			g_MpSetup.options |= MPOPTION_TEAMSENABLED;

			for (i = 0; i < MAX_PLAYERS; i++) {
				g_PlayerConfigsArray[i].base.team = 0;
			}

			break;
		case MPQUICKTEAM_PLAYERSIMTEAMS:
			g_MpSetup.options |= MPOPTION_TEAMSENABLED;

			for (i = 0; i < MAX_PLAYERS; i++) {
				g_PlayerConfigsArray[i].base.team = i;
			}

			break;
		}
	}
}

void mpConfigureQuickTeamSimulants(void)
{
	struct mpchrconfig *mpchr;
	s32 numchrs;
	s32 botnum;
	s32 i;
	s32 j;

	if (g_Vars.mpquickteam != MPQUICKTEAM_NONE) {
		switch (g_Vars.mpquickteam) {
		case MPQUICKTEAM_PLAYERSANDSIMS:
			for (i = 0; i < g_Vars.mpquickteamnumsims; i++) {
				botnum = mpGetSlotForNewBot();

				if (botnum >= 0) {
					mpCreateBotFromProfile(botnum, g_Vars.mpsimdifficulty);
				}
			}

			mpGenerateBotNames();
			break;
		case MPQUICKTEAM_PLAYERSVSSIMS:
			for (i = 0; i < g_Vars.mpquickteamnumsims; i++) {
				botnum = mpGetSlotForNewBot();

				if (botnum >= 0) {
					mpCreateBotFromProfile(botnum, g_Vars.mpsimdifficulty);
				}
			}

			mpGenerateBotNames();

			for (i = 0; i < ARRAYCOUNT(g_BotConfigsArray); i++) {
				g_BotConfigsArray[i].base.team = 1;
			}

			break;
		case MPQUICKTEAM_PLAYERSIMTEAMS:
			for (i = mpGetNumChrs() - 1; i >= 0; i--) {
				mpchr = mpGetChrConfigBySlotNum(i);

				for (j = 0; j < g_Vars.unk0004a0; j++) {
					botnum = mpGetSlotForNewBot();

					if (botnum >= 0) {
						mpCreateBotFromProfile(botnum, g_Vars.mpsimdifficulty);
						g_BotConfigsArray[botnum].base.team = mpchr->team;
					}
				}
			}

			mpGenerateBotNames();
			break;
		case MPQUICKTEAM_PLAYERSONLY:
		case MPQUICKTEAM_PLAYERSTEAMS:
			break;
		}
	}
}

void func0f17f428(void)
{
	mpConfigureQuickTeamPlayers();

	if (IS4MB()) {
		func0f0f820c(&g_MpQuickGo4MbMenuDialog, MENUROOT_4MBMAINMENU);
	} else {
		func0f0f820c(&g_MpQuickGoMenuDialog, MENUROOT_MPSETUP);
	}
}

MenuItemHandlerResult menuhandlerMpFinishedSetup(s32 operation, struct menuitem *item, union handlerdata *data)
{
#if VERSION >= VERSION_NTSC_1_0
	if (operation == MENUOP_CHECKPREFOCUSED) {
		return true;
	}
#endif

	if (operation == MENUOP_SET) {
		func0f17f428();
	}

	return 0;
}

MenuItemHandlerResult menuhandlerQuickTeamSeparator(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_Vars.mpquickteam == MPQUICKTEAM_PLAYERSONLY) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerPlayerTeam(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
#if VERSION >= VERSION_JPN_FINAL
		data->dropdown.value = scenarioGetMaxTeams();
#else
		data->dropdown.value = MAX_TEAMS;
#endif
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) &g_BossFile.teamnames[data->dropdown.value];
	case MENUOP_SET:
		g_Vars.mpplayerteams[item->param] = data->dropdown.value;
		break;
	case MENUOP_GETSELECTEDINDEX:
#if VERSION >= VERSION_JPN_FINAL
		if (g_Vars.mpplayerteams[item->param] >= scenarioGetMaxTeams()) {
			g_Vars.mpplayerteams[item->param] %= scenarioGetMaxTeams();
		}
#endif
		data->dropdown.value = g_Vars.mpplayerteams[item->param];
		break;
	case MENUOP_CHECKHIDDEN:
		if (g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSTEAMS) {
			return true;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpNumberOfSimulants(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = !challengeIsFeatureUnlocked(MPFEATURE_8BOTS) ? 4 : MAX_BOTS;
		break;
	case MENUOP_GETOPTIONTEXT:
		sprintf(g_StringPointer, "%d\n", data->dropdown.value + 1);
		return (uintptr_t) g_StringPointer;
	case MENUOP_SET:
		g_Vars.mpquickteamnumsims = data->dropdown.value + 1;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_Vars.mpquickteamnumsims - 1;
		break;
	case MENUOP_CHECKHIDDEN:
		if (g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSANDSIMS
				&& g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSVSSIMS) {
			return true;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpSimulantsPerTeam(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = 2;
		break;
	case MENUOP_GETOPTIONTEXT:
		sprintf(g_StringPointer, "%d\n", data->dropdown.value + 1);
		return (uintptr_t) g_StringPointer;
	case MENUOP_SET:
		g_Vars.unk0004a0 = data->dropdown.value + 1;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_Vars.unk0004a0 - 1;
		break;
	case MENUOP_CHECKHIDDEN:
		if (g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSIMTEAMS) {
			return true;
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult mpQuickTeamSimulantDifficultyHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 count = 0;
	s32 i;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		for (i = 0; i < NUM_BOTDIFFS; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				count++;
			}
		}

		data->dropdown.value = count;
		break;
	case MENUOP_GETOPTIONTEXT:
		for (i = 0; i < NUM_BOTDIFFS; i++) {
			if (challengeIsFeatureUnlocked(g_BotProfiles[i].requirefeature)) {
				if (count == data->dropdown.value) { //TODO - Lang: Fix it, simulat name
					return (uintptr_t) _("TODO"); //langGet(i + gettext_noop("Meat"));
				}

				count++;
			}
		}
		break;
	case MENUOP_SET:
		g_Vars.mpsimdifficulty = data->dropdown.value;
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = g_Vars.mpsimdifficulty;
		break;
	case MENUOP_CHECKHIDDEN:
		if (g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSANDSIMS
				&& g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSVSSIMS
				&& g_Vars.mpquickteam != MPQUICKTEAM_PLAYERSIMTEAMS) {
			return true;
		}
	}

	return 0;
}

MenuItemHandlerResult menuhandlerMpQuickTeamOption(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Vars.mpquickteam = item->param;

		if (mpGetWeaponSet() >= func0f189058(0)) {
			mpSetWeaponSet(0);
		}

		if (g_Vars.mpquickteam == MPQUICKTEAM_PLAYERSONLY ||
				g_Vars.mpquickteam == MPQUICKTEAM_PLAYERSANDSIMS) {
			if (g_MpSetup.scenario == MPSCENARIO_KINGOFTHEHILL ||
					g_MpSetup.scenario == MPSCENARIO_CAPTURETHECASE) {
				g_MpSetup.scenario = MPSCENARIO_COMBAT;
			}
		}

		menuPushDialog(&g_MpQuickTeamGameSetupMenuDialog);
	}

	return 0;
}

MenuDialogHandlerResult menudialogCombatSimulator(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		g_Vars.waitingtojoin[0] = false;
		g_Vars.waitingtojoin[1] = false;
		g_Vars.waitingtojoin[2] = false;
		g_Vars.waitingtojoin[3] = false;

		// load the setup file when entering the Combat Simulator
		mpsetupCopyAllFromPak();
		mpsetupLoadCurrentFile();
	}

	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->definition == &g_CombatSimulatorMenuDialog
			&& operation == MENUOP_TICK) {
		g_Vars.mpsetupmenu = MPSETUPMENU_GENERAL;
		g_Vars.mpquickteam = MPQUICKTEAM_NONE;
		g_Vars.usingadvsetup = false;
		challengeUnsetCurrent();
		challengeRemovePlayerLock();
	}

	return false;
}

MenuItemHandlerResult menuhandlerMpAdvancedSetup(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		func0f0f820c(&g_MpAdvancedSetupMenuDialog, 3);
	}

	return 0;
}

/**
 * When a player is loading a saved setup, check which dialogs the other players
 * have open and close them if they no longer apply or need to be updated.
 */
void mpCloseDialogsForNewSetup(void)
{
	s32 i;
	s32 prevplayernum = g_MpPlayerNum;
	s32 j;
	s32 k;

	// Loop through each player
	for (i = 0; i < MAX_PLAYERS; i++) {
		g_MpPlayerNum = i;

		// If they have a menu open
		if (g_Menus[g_MpPlayerNum].curdialog) {
			bool ok = false;

			// Repeat the following steps until we've stopped finding dialogs
			// that should be closed
			while (!ok) {
				ok = true;

				// Loop through each layer of menus
				for (j = 0; j < g_Menus[g_MpPlayerNum].depth; j++) {
					// Loop through the siblings (left/right) in this layer
					for (k = 0; k < g_Menus[g_MpPlayerNum].layers[j].numsiblings; k++) {
						if (g_Menus[g_MpPlayerNum].layers[j].siblings[k]) {
							struct menudialogdef *dialogdef = g_Menus[g_MpPlayerNum].layers[j].siblings[k]->definition;

							if (dialogdef == &g_MpSaveSetupNameMenuDialog) ok = false;
							if (dialogdef == &g_MpSaveSetupExistsMenuDialog) ok = false;
							if (dialogdef == &g_MpAddSimulantMenuDialog) ok = false;
							if (dialogdef == &g_MpChangeSimulantMenuDialog) ok = false;
							if (dialogdef == &g_MpEditSimulantMenuDialog) ok = false;
							if (dialogdef == &g_MpCombatOptionsMenuDialog) ok = false;
							if (dialogdef == &g_HtbOptionsMenuDialog) ok = false;
							if (dialogdef == &g_CtcOptionsMenuDialog) ok = false;
							if (dialogdef == &g_KohOptionsMenuDialog) ok = false;
							if (dialogdef == &g_HtmOptionsMenuDialog) ok = false;
							if (dialogdef == &g_PacOptionsMenuDialog) ok = false;
						}
					}
				}

				// Close the leaf layer
				if (!ok) {
					menuPopDialog();
				}
			}
		}
	}

	g_MpPlayerNum = prevplayernum;
}

struct menudialogdef g_MpAbortMenuDialog;

struct menuitem g_MpStuffMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Soundtrack\n"), // "Soundtrack"
		0,
		(void *)&g_MpSoundtrackMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Team Names\n"), // "Team Names"
		0,
		(void *)&g_MpTeamNamesMenuDialog,
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
		0,
		0,
		NULL,
	},
#ifdef PLATFORM_N64
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Ratio\n"), // "Ratio"
		0,
		menuhandlerScreenRatio,
	},
#endif
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Split\n"), // "Split"
		0,
		menuhandlerScreenSplit,
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
		gettext_noop("Start Game\n"), // "Start Game"
		0,
		(void *)&g_MpReadyMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Drop Out\n"), // "Drop Out"
		0,
		(void *)&g_MpDropOutMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Abort Game\n"), // "Abort Game"
		0,
		(void *)&g_MpAbortMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpStuffMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Stuff\n"), // "Stuff"
	g_MpStuffMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpChallengeListOrDetailsMenuDialog,
};

struct menudialogdef g_MpStuffViaAdvChallengeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Stuff\n"), // "Stuff"
	g_MpStuffMenuItems,
	NULL,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	NULL,
};

struct menuitem g_MpPlayerSetup234MenuItems[] = {
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
		gettext_noop("Character\n"), // "Character"
		0,
		(void *)&g_MpCharacterMenuDialog,
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

struct menudialogdef g_MpPlayerSetupViaAdvMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Setup\n"), // "Player Setup"
	g_MpPlayerSetup234MenuItems,
	NULL,
	MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpStuffMenuDialog,
};

struct menudialogdef g_MpPlayerSetupViaAdvChallengeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Setup\n"), // "Player Setup"
	g_MpPlayerSetup234MenuItems,
	NULL,
	MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpStuffViaAdvChallengeMenuDialog,
};

struct menudialogdef g_MpPlayerSetupViaQuickGoMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Player Setup\n"), // "Player Setup"
	g_MpPlayerSetup234MenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpAbortMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop(""), // "Are you sure you want to abort the game?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Abort\n"), // "Abort"
		0,
		menuhandler0017ef30,
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

struct menudialogdef g_MpAbortMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Abort\n"), // "Abort"
	g_MpAbortMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpAdvancedSetupMenuItems[] = {
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
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Manage Settings\n"),
		0,
		(void *)&g_ManageSettingsDialog,
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

struct menudialogdef g_MpAdvancedSetupMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Setup\n"), // "Game Setup"
	g_MpAdvancedSetupMenuItems,
	menudialogMpGameSetup,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpPlayerSetupViaAdvMenuDialog,
};

struct menudialogdef g_MpAdvancedSetupViaAdvChallengeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Setup\n"), // "Game Setup"
	g_MpAdvancedSetupMenuItems,
	menudialogMpGameSetup,
	MENUDIALOGFLAG_MPLOCKABLE | MENUDIALOGFLAG_DROPOUTONCLOSE,
	&g_MpPlayerSetupViaAdvChallengeMenuDialog,
};

struct menuitem g_MpQuickGoMenuItems[] = {
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
		(void *)&g_MpPlayerSetupViaQuickGoMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Drop Out\n"), // "Drop Out"
		0,
		(void *)&g_MpDropOutMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickGoMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Quick Go\n"), // "Quick Go"
	g_MpQuickGoMenuItems,
	menudialogMpQuickGo,
	0,
	NULL,
};

struct menuitem g_MpQuickTeamGameSetupMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_LOCKABLEMINOR,
		gettext_noop("Scenario\n"), // "Scenario"
		(uintptr_t)&mpMenuTextScenarioShortName,
		(void *)&g_MpQuickTeamScenarioMenuDialog,
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
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Weapons\n"), // "Weapons"
		(uintptr_t)&mpMenuTextWeaponSetName,
		(void *)&g_MpQuickTeamWeaponsMenuDialog,
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
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000082,
		0,
		menuhandlerQuickTeamSeparator,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Player 1 Team\n"), // "Player 1 Team"
		0,
		menuhandlerPlayerTeam,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		1,
		0,
		gettext_noop("Player 2 Team\n"), // "Player 2 Team"
		0,
		menuhandlerPlayerTeam,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		2,
		0,
		gettext_noop("Player 3 Team\n"), // "Player 3 Team"
		0,
		menuhandlerPlayerTeam,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		3,
		0,
		gettext_noop("Player 4 Team\n"), // "Player 4 Team"
		0,
		menuhandlerPlayerTeam,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Number Of Simulants\n"), // "Number Of Simulants"
		0,
		menuhandlerMpNumberOfSimulants,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Simulants Per Team\n"), // "Simulants Per Team"
		0,
		menuhandlerMpSimulantsPerTeam,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		0,
		gettext_noop("Simulant Difficulty\n"), // "Simulant Difficulty"
		0,
		mpQuickTeamSimulantDifficultyHandler,
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
		0,
		gettext_noop("Finished Setup\n"), // "Finished Setup"
		0,
		menuhandlerMpFinishedSetup,
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
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LOCKABLEMAJOR,
		gettext_noop("Save Settings\n"), // "Save Settings"
		0,
		menuhandlerMpSaveSettings,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickTeamGameSetupMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Setup\n"), // "Game Setup"
	g_MpQuickTeamGameSetupMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_MpQuickTeamMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Players Only\n"), // "Players Only"
		0,
		menuhandlerMpQuickTeamOption,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Players and Simulants\n"), // "Players and Simulants"
		0,
		menuhandlerMpQuickTeamOption,
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
		2,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Player Teams\n"), // "Player Teams"
		0,
		menuhandlerMpQuickTeamOption,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		3,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Players vs. Simulants\n"), // "Players vs. Simulants"
		0,
		menuhandlerMpQuickTeamOption,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		4,
		MENUITEMFLAG_BIGFONT,
		gettext_noop("Player-Simulant Teams\n"), // "Player-Simulant Teams"
		0,
		menuhandlerMpQuickTeamOption,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_MpQuickTeamMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Quick Team\n"), // "Quick Team"
	g_MpQuickTeamMenuItems,
	NULL,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

struct menuitem g_CombatSimulatorMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG | MENUITEMFLAG_BIGFONT,
		gettext_noop("Challenges\n"), // "Challenges"
		0,
		(void *)&g_MpChallengesMenuDialog,
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
		menuhandlerMpAdvancedSetup,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CombatSimulatorMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Combat Simulator\n"), // "Combat Simulator"
	g_CombatSimulatorMenuItems,
	menudialogCombatSimulator,
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
};

void func0f17fcb0(s32 silent)
{
	g_Menus[g_MpPlayerNum].playernum = g_MpPlayerNum;

	if (IS4MB()) {
		menuPushRootDialog(&g_AdvancedSetup4MbMenuDialog, MENUROOT_4MBMAINMENU);
		func0f0f8300();
	} else {
		if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
			menuPushRootDialog(&g_MpChallengeListOrDetailsViaAdvChallengeMenuDialog, MENUROOT_MPSETUP);
		} else {
			menuPushRootDialog(&g_MpAdvancedSetupMenuDialog, MENUROOT_MPSETUP);
		}

		func0f0f8300();
	}

	if (!silent) {
		// Explosion sound
		sndStart(var80095200, SFX_EXPLOSION_809A, NULL, -1, -1, -1, -1, -1);
	}
}
