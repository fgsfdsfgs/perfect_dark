#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/ceil.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/inv.h"
#include "game/game_1531a0.h"
#include "game/file.h"
#include "game/texdecompress.h"
#include "game/trainingmenus.h"
#include "game/training.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "video.h"
#endif

#define NUM_BIO_LOCATIONS 14

struct menudialogdef g_BioProfileMenuDialog;
struct menudialogdef g_BioTextMenuDialog;
struct menudialogdef g_HangarLocationDetailsMenuDialog;
struct menudialogdef g_HangarVehicleDetailsMenuDialog;

MenuItemHandlerResult frDetailsOkMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 i;

	switch (operation) {
	case MENUOP_CHECKPREFOCUSED:
		return true;
	case MENUOP_SET:
		if (frIsInTraining() == false) {
			s32 weapon = frGetWeaponBySlot(frGetSlot());

			if (g_FrWeaponNum != WEAPON_UNARMED) {
				invRemoveItemByNum(g_FrWeaponNum);
			}

			if (weapon != WEAPON_UNARMED) {
				invGiveSingleWeapon(weapon);
			}

			invSetCurrentIndex(1);

			if (bgunGetWeaponNum(HAND_RIGHT) != weapon) {
				bgunEquipWeapon2(HAND_RIGHT, weapon);
			}

			g_FrWeaponNum = weapon;

			for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->hands[0].gunroundsspent); i++) {
				g_Vars.currentplayer->hands[0].gunroundsspent[i] = 0;
				g_Vars.currentplayer->hands[1].gunroundsspent[i] = 0;
			}

			frBeginSession(weapon);
		}

		func0f0f8120();
		break;
	}

	return 0;
}

MenuItemHandlerResult frAbortMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (frIsInTraining()) {
			frEndSession(true);
		}
	}

	return 0;
}

struct menudialogdef g_FrDifficultyMenuDialog;

MenuItemHandlerResult frWeaponListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 weaponnum;
	s32 score;
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	u32 colour;
	s32 weaponnum2;
	s32 score2;
	s32 i;
	s32 x;
	s32 y;

	switch (operation) {
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = LINEHEIGHT;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 0;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return 0;
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = 0;
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = frGetNumWeaponsAvailable();
		break;
	case MENUOP_GETOPTIONTEXT:
		return 0;
	case MENUOP_SET:
		weaponnum = frGetWeaponBySlot(data->list.value);
		score = ciGetFiringRangeScore(frGetWeaponIndexByWeapon(weaponnum));

#if VERSION < VERSION_NTSC_1_0
		if (g_Menus[g_MpPlayerNum].layers[g_Menus[g_MpPlayerNum].depth - 1].numsiblings > 1) {
			menuCloseDialog();
			menuPushDialog(&g_FrWeaponListMenuDialog);
		}
#endif

		frLoadData();
		frSetSlot(data->list.value);

		if (score) {
			frSetDifficulty(ciGetFiringRangeScore(frGetWeaponIndexByWeapon(weaponnum)));
			menuPushDialog(&g_FrDifficultyMenuDialog);
		} else {
			frSetDifficulty(FRDIFFICULTY_BRONZE);
			menuPushDialog(&g_FrTrainingInfoPreGameMenuDialog);
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = frGetSlot();
		break;
	case MENUOP_RENDER:
		gdl = data->type19.gdl;
		renderdata = data->type19.renderdata2;
		weaponnum2 = frGetWeaponBySlot(data->type19.unk04);
		score2 = ciGetFiringRangeScore(frGetWeaponIndexByWeapon(weaponnum2));

		// Render weapon name
		x = renderdata->x + 10;
		y = renderdata->y;

#if VERSION == VERSION_JPN_FINAL
		y++;
#endif

		gdl = text0f153628(gdl);
		gdl = textRenderProjected(gdl, &x, &y, bgunGetName(weaponnum2), g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		// Prepare the star texture for the difficulties
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

		// Iterate and render the 3 difficulty stars
		for (i = 0; i < 3; i++) {
			if (score2 > i) {
				switch (i) {
				case FRDIFFICULTY_BRONZE:
					colour = 0x884400ff;
					break;
				case FRDIFFICULTY_SILVER:
					colour = 0x99999fff;
					break;
				case FRDIFFICULTY_GOLD:
					colour = 0xbb9922ff;
					break;
				}
			} else {
				colour = 0x99999944;
			}

#if VERSION >= VERSION_NTSC_1_0
			colour = (colour & 0xffffff00) | (((colour & 0xff) * (renderdata->colour & 0xff)) >> 8);
#endif

			gDPSetEnvColorViaWord(gdl++, colour);

			gDPSetCombineLERP(gdl++,
					TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
					TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);

			gSPTextureRectangle(gdl++,
					(((renderdata->x + i * 13) + 125) << 2) * g_ScaleX, (renderdata->y) << 2,
					(((renderdata->x + i * 13) + 136) << 2) * g_ScaleX, (renderdata->y + 11) << 2,
					G_TX_RENDERTILE, 0, 0x0160, 1024 / g_ScaleX, -1024);
		}

		return (uintptr_t)gdl;
	}

	return 0;
}

MenuDialogHandlerResult frTrainingInfoMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	u32 stack;
	s32 weaponnum;

	switch (operation) {
	case MENUOP_OPEN:
		weaponnum = frGetWeaponBySlot(frGetSlot());
		g_Menus[g_MpPlayerNum].training.weaponnum = weaponnum;
		func0f105948(weaponnum);

		if (!frIsInTraining()) {
			frInitAmmo(weaponnum);
		}
		break;
	case MENUOP_TICK:
		if (g_Menus[g_MpPlayerNum].curdialog && g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 -= g_Vars.diffframe60;
			g_Menus[g_MpPlayerNum].menumodel.curroty = g_Menus[g_MpPlayerNum].menumodel.newroty = 18.849555969238f * g_20SecIntervalFrac;
			g_Menus[g_MpPlayerNum].menumodel.currotz = g_Menus[g_MpPlayerNum].menumodel.newrotz = 0;
		}
		break;
	case MENUOP_CLOSE:
		if (!frIsInTraining()) {
			frEndSession(true);
		}
		break;
	}

	return 0;
}

MenuDialogHandlerResult frTrainingStatsMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_CLOSE) {
		if (frIsInTraining() == false) {
			frEndSession(true);
		}
	}

	return 0;
}

/**
 * This is an unused menu handler which implements the difficulty selection
 * using a dropdown.
 */
MenuItemHandlerResult frDifficultyDropdownMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u16 names[] = {
		L_MPMENU_439, // "Bronze"
		L_MPMENU_440, // "Silver"
		L_MPMENU_441, // "Gold"
	};

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->dropdown.value = ciGetFiringRangeScore(frGetSlot()) + 1;

		if (data->dropdown.value > 3) {
			data->dropdown.value = 3;
		}
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) langGet(names[data->dropdown.value]);
	case MENUOP_SET:
		frSetDifficulty(data->dropdown.value);
		menuPushDialog(&g_FrTrainingInfoPreGameMenuDialog);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->dropdown.value = frGetDifficulty();
		break;
	}

	return 0;
}

MenuItemHandlerResult frDifficultyMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		if (ciGetFiringRangeScore(frGetWeaponIndexByWeapon(frGetWeaponBySlot(frGetSlot()))) < item->param) {
			return true;
		}
		break;
	case MENUOP_SET:
		frSetDifficulty(item->param);
		menuPushDialog(&g_FrTrainingInfoPreGameMenuDialog);
		break;
	case MENUOP_CHECKPREFOCUSED:
		if (ciGetFiringRangeScore(frGetWeaponIndexByWeapon(frGetWeaponBySlot(frGetSlot()))) >= item->param) {
			return true;
		}
		break;
	}

	return 0;
}

char *frPrimaryFunctionMenuText(struct menuitem *item)
{
	struct weaponfunc *func = weaponGetFunctionById(frGetWeaponBySlot(frGetSlot()), FUNC_PRIMARY);

	if (func) {
		return langGet(func->name);
	}

	return "\n";
}

char *frSecondaryFunctionMenuText(struct menuitem *item)
{
	struct weaponfunc *func = weaponGetFunctionById(frGetWeaponBySlot(frGetSlot()), FUNC_SECONDARY);

	if (func) {
		return langGet(func->name);
	}

	return "\n";
}

char *frMenuTextFailReason(struct menuitem *item)
{
	u16 reasons[] = {
		L_MPMENU_456, // "Not Failed"
		L_MPMENU_457, // "Out of Ammo"
		L_MPMENU_458, // "Time Over"
		L_MPMENU_459, // "Score Unattainable"
		L_MPMENU_460, // "Too Inaccurate"
	};

	struct frdata *frdata = frGetData();

	return langGet(reasons[frdata->failreason]);
}

char *frMenuTextDifficultyName(struct menuitem *item)
{
	u16 names[] = {
		L_MPMENU_439, // "Bronze"
		L_MPMENU_440, // "Silver"
		L_MPMENU_441, // "Gold"
	};

	struct frdata *frdata = frGetData();

	return langGet(names[frdata->difficulty]);
}

char *frMenuTextTimeTakenValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();
	f32 secs = frdata->timetaken / (PAL ? 50.0f : 60.0f);

	if (secs > frdata->timelimit) {
		secs = frdata->timelimit;
	}

	if (secs >= 60.0f) {
		s32 mins = 0;

		while (secs >= 60.0f) {
			secs -= 60.0f;
			mins++;
		}

		sprintf(g_StringPointer, "%dm %2ds\n", mins, (s32)ceilf(secs));
		return g_StringPointer;
	} else {
		sprintf(g_StringPointer, "%s%s%2.2fs\n", "", "", secs);
	}

	return g_StringPointer;
}

char *frMenuTextScoreValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	sprintf(g_StringPointer, "%d\n", frdata->score);
	return g_StringPointer;
}

char *frMenuTextGoalScoreValueUnconditional(struct menuitem *item)
{
	struct frdata *frdata = frGetData();
	sprintf(g_StringPointer, "%d\n", frdata->goalscore);
	return g_StringPointer;
}

char *frMenuTextWeaponName(struct menuitem *item)
{
	return bgunGetName(frGetWeaponBySlot(frGetSlot()));
}

char *frMenuTextTargetsDestroyedValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	sprintf(g_StringPointer, "%d\n", frdata->targetsdestroyed);
	return g_StringPointer;
}

char *frMenuTextAccuracyValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();
	f32 totalhits = (frdata->numhitsring3 + frdata->numhitsbullseye + frdata->numhitsring1 + frdata->numhitsring2) * 100.0f;
	f32 accuracy = 0;

	if (frdata->numshots) {
		accuracy = totalhits / frdata->numshots;
	}

	if (accuracy > 100.0f) {
		accuracy = 100.0f;
	}

	sprintf(g_StringPointer, "%s%s%.1f%%\n", "", "", accuracy);
	return g_StringPointer;
}

char *frMenuTextGoalScoreLabel(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->goalscore > 0) {
		sprintf(g_StringPointer, "%s", langGet(L_MPMENU_475)); // "Goal Score:"
		return g_StringPointer;
	}

	return NULL;
}

char *frMenuTextGoalScoreValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->goalscore > 0) {
		sprintf(g_StringPointer2, "%d\n", frdata->goalscore);
		return g_StringPointer2;
	}

	return NULL;
}

char *frMenuTextMinAccuracyOrTargetsLabel(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->goalaccuracy > 0) {
		sprintf(g_StringPointer, "%s", langGet(L_MPMENU_473)); // "Min Accuracy:"
	} else if (frdata->goaltargets != 255) {
		sprintf(g_StringPointer, "%s", langGet(L_MPMENU_474)); // "Goal Targets:"
	} else {
		return NULL;
	}

	return g_StringPointer;
}

char *frMenuTextMinAccuracyOrTargetsValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->goalaccuracy > 0) {
		sprintf(g_StringPointer2, "%d%%\n", frdata->goalaccuracy);
	} else if (frdata->goaltargets != 255) {
		sprintf(g_StringPointer2, "%d\n", frdata->goaltargets);
	} else {
		return NULL;
	}

	return g_StringPointer2;
}

char *frMenuTextTimeLimitLabel(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->timelimit != 255) {
		sprintf(g_StringPointer, "%s", langGet(L_MPMENU_472)); // "Time Limit:"
	} else {
		return NULL;
	}

	return g_StringPointer;
}

char *frMenuTextTimeLimitValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->timelimit != 255) {
		s32 secs = frdata->timelimit;
		s32 mins = 0;

		while (secs >= 60) {
			secs -= 60;
			mins++;
		}

		if (mins > 0) {
			sprintf(g_StringPointer2, "%dm %ds\n", mins, secs);
		} else {
			sprintf(g_StringPointer2, "%ds\n", secs);
		}
	} else {
		return NULL;
	}

	return g_StringPointer2;
}

char *frMenuTextAmmoLimitLabel(struct menuitem *item)
{
	struct frdata *frdata = frGetData();

	if (frdata->ammolimit != 255) {
		sprintf(g_StringPointer, "%s", langGet(L_MPMENU_471)); // "Ammo Limit:"
	} else {
		return NULL;
	}

	return g_StringPointer;
}

char *frMenuTextAmmoLimitValue(struct menuitem *item)
{
	struct frdata *frdata = frGetData();
	char suffix[16];
	s32 weaponnum;

	if (frdata->ammolimit != 255) {
		weaponnum = frGetWeaponBySlot(frdata->slot);

		if (weaponnum == WEAPON_SUPERDRAGON && frdata->sdgrenadelimit != 255) {
			sprintf(suffix, "/%d", frdata->sdgrenadelimit);
		} else {
			suffix[0] = '\0';
		}

		sprintf(g_StringPointer2, "%d%s\n", frdata->ammolimit, suffix);
	} else {
		return NULL;
	}

	return g_StringPointer2;
}

/**
 * Render the "Scoring" section of the firing range results screen.
 *
 * The screen contains a diagram of a target with the zones marked,
 * as well as the player's score chart. There are lines leading from
 * the score chart to the diagram.
 */
MenuItemHandlerResult frScoringMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		s32 x;
		s32 y;
		s32 textheight;
		s32 textwidth;
		struct textureconfig *tconfig = &g_TexGeneralConfigs[50];
		struct frdata *frdata = frGetData();
		char text[128];
		bool failed = frdata->menutype == FRMENUTYPE_FAILED;
#if VERSION >= VERSION_JPN_FINAL
		u32 linecolourmid = failed ? 0xff644477 : 0x00ff0077; // line gradient colour in middle
		u32 linecolourfig = failed ? 0xff664400 : 0x00ff0000; // line gradient colour at figures
		u32 linecolourtex = failed ? 0xff664433 : 0x00ff0033; // line gradient colour at target texture
#else
		u32 linecolourmid = failed ? 0xff000077 : 0x00ff0077; // line gradient colour in middle
		u32 linecolourfig = failed ? 0xff000000 : 0x00ff0000; // line gradient colour at figures
		u32 linecolourtex = failed ? 0xff000033 : 0x00ff0033; // line gradient colour at target texture
#endif

#if VERSION >= VERSION_NTSC_1_0
		u32 colour;
#endif

		static u32 x1 = 0;
		static u32 x2 = 0;
		static u32 y1 = 0;
		static u32 y2 = 0;
		static u32 x3 = 0;
		static u32 x4 = 0;
		static u32 y3 = 0;
		static u32 y4 = 0;

#if VERSION >= VERSION_NTSC_1_0
		linecolourmid = (linecolourmid & 0xffffff00) | ((linecolourmid & 0xff) * (renderdata->colour & 0xff) >> 8);
		linecolourfig = (linecolourfig & 0xffffff00) | ((linecolourfig & 0xff) * (renderdata->colour & 0xff) >> 8);
		linecolourtex = (linecolourtex & 0xffffff00) | ((linecolourtex & 0xff) * (renderdata->colour & 0xff) >> 8);
#endif

		mainOverrideVariable("x1", &x1);
		mainOverrideVariable("x2", &x2);
		mainOverrideVariable("y1", &y1);
		mainOverrideVariable("y2", &y2);
		mainOverrideVariable("x3", &x3);
		mainOverrideVariable("x4", &x4);
		mainOverrideVariable("y3", &y3);
		mainOverrideVariable("y4", &y4);

		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);
		gDPSetTextureFilter(gdl++, G_TF_POINT);

		texSelect(&gdl, tconfig, 2, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
		gDPSetTextureFilter(gdl++, G_TF_POINT);

#if VERSION >= VERSION_NTSC_1_0
		colour = ((failed ? 0xff777799 : 0x55ff5588) & 0xffffff00) | (((failed ? 0xff777799 : 0x55ff5588) & 0xff) * (renderdata->colour & 0xff) >> 8);
		gDPSetEnvColorViaWord(gdl++, colour);
#else
		gDPSetEnvColorViaWord(gdl++, failed ? 0xff777799 : 0x55ff5588);
#endif

		gDPSetCombineLERP(gdl++,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);

		// Top left quarter of target
		gSPTextureRectangle(gdl++,
				((renderdata->x + 10) << 2) * g_ScaleX, (renderdata->y + 5) << 2,
				((renderdata->x + 42) << 2) * g_ScaleX, (renderdata->y + 37) << 2,
				G_TX_RENDERTILE, 16, 1024, 1024 / g_ScaleX, -1024);

		// Top right quarter of target
		gSPTextureRectangle(gdl++,
				((renderdata->x + 41) << 2) * g_ScaleX, (renderdata->y + 5) << 2,
				((renderdata->x + 73) << 2) * g_ScaleX, (renderdata->y + 37) << 2,
				G_TX_RENDERTILE, 16, 1024, -1024 / g_ScaleX, -1024);

		// Bottom left quarter of target
		gSPTextureRectangle(gdl++,
				((renderdata->x + 10) << 2) * g_ScaleX, (renderdata->y + 36) << 2,
				((renderdata->x + 42) << 2) * g_ScaleX, (renderdata->y + 68) << 2,
				G_TX_RENDERTILE, 16, 1024, 1024 / g_ScaleX, 1024);

		// Bottom right quarter of target
		gSPTextureRectangle(gdl++,
				((renderdata->x + 41) << 2) * g_ScaleX, (renderdata->y + 36) << 2,
				((renderdata->x + 73) << 2) * g_ScaleX, (renderdata->y + 68) << 2,
				G_TX_RENDERTILE, 16, 1024, -1024 / g_ScaleX, 1024);

#if VERSION >= VERSION_NTSC_1_0
		gdl = textSetPrimColour(gdl, ((failed ? 0xff000055 : 0x00ff0055) & 0xffffff00) | (((failed ? 0xff000055 : 0x00ff0055) & 0xff) * (renderdata->colour & 0xff) >> 8));
		colour = ((failed ? 0xff6969aa : renderdata->colour) & 0xffffff00) | ((((failed ? 0xff6969aa : renderdata->colour) & 0xff) * (renderdata->colour & 0xff)) >> 8);
#else
		gdl = textSetPrimColour(gdl, failed ? 0xff000055 : 0x00ff0055);
#endif

		// NTSC beta uses a static alpha channel, while newer versions take the
		// alpha from the menu item's renderdata. Additionally, NTSC beta
		// repeats its colour calculation throughout this function while newer
		// versions store it in a variable. To prevent having version checks
		// everywhere, this has been implemented here using macros.
#if VERSION >= VERSION_NTSC_1_0
#define COLOUR() (colour)
#define COLOURWHITE() (0xffffff00 | (((renderdata->colour & 0xff) * 0xff) >> 8))
#else
#define COLOUR() (failed ? 0xff6969aa : renderdata->colour)
#define COLOURWHITE() 0xffffffff
#endif

		// Bull's-eye count
		sprintf(text, "%d\n", frdata->numhitsbullseye);
		x = renderdata->x + 93;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 15 : 14);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Bull's-eye"
		sprintf(text, langGet(L_MPMENU_461));
		x = renderdata->x + 122;
		y = renderdata->y + 14;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Bull's-eye score
		sprintf(text, "%d\n", frdata->numhitsbullseye * 10);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = renderdata->x - textheight + (VERSION == VERSION_JPN_FINAL ? 192 : 182);
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 15 : 14);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 1 count
		sprintf(text, "%d\n", frdata->numhitsring1);
		x = renderdata->x + 93;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 27 : 25);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Zone 1"
		sprintf(text, langGet(L_MPMENU_462));
		x = renderdata->x + 122;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 26 : 25);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 1 score
		sprintf(text, "%d\n", frdata->numhitsring1 * 5);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = renderdata->x - textheight + (VERSION == VERSION_JPN_FINAL ? 192 : 182);
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 27 : 25);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 2 count
		sprintf(text, "%d\n", frdata->numhitsring2);
		x = renderdata->x + 93;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 39 : 36);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Zone 2"
		sprintf(text, langGet(L_MPMENU_463));
		x = renderdata->x + 122;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 38 : 36);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 2 score
		sprintf(text, "%d\n", frdata->numhitsring2 * 2);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = renderdata->x - textheight + (VERSION == VERSION_JPN_FINAL ? 192 : 182);
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 39 : 36);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 3 count
		sprintf(text, "%d\n", frdata->numhitsring3);
		x = renderdata->x + 93;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 51 : 47);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Zone 3"
		// Note: developers forgot to remove last argument when copy/pasting
		sprintf(text, langGet(L_MPMENU_464), frdata->numhitsring3);
		x = renderdata->x + 122;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 50 : 47);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Zone 3 score
		sprintf(text, "%d\n", frdata->numhitsring3);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = renderdata->x - textheight + (VERSION == VERSION_JPN_FINAL ? 192 : 182);
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 51 : 47);
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Hit total"
		sprintf(text, langGet(L_MPMENU_465));
		x = renderdata->x + 133;
		y = renderdata->y + 63;

#if VERSION >= VERSION_JPN_FINAL
		x -= 44;
		y += 3;
#endif

		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// Hit total count
		sprintf(text, "%d\n", frdata->numhitsring3 + frdata->numhitsbullseye + frdata->numhitsring1 + frdata->numhitsring2);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		x = renderdata->x - textheight + 188;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 64 : 63);

#if VERSION >= VERSION_JPN_FINAL
		x -= 27;
		y += 3;
#endif

		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "Scoring"
		sprintf(text, langGet(L_MPMENU_466));
		x = renderdata->x + 83;
		y = renderdata->y + 1;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, COLOUR(), viGetWidth(), viGetHeight(), 0, 0);

		// "10"
		sprintf(text, langGet(L_MPMENU_467));
		x = renderdata->x + 38;
		y = renderdata->y + 35;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, COLOURWHITE(), viGetWidth(), viGetHeight(), 0, 0);

		// "5"
		sprintf(text, langGet(L_MPMENU_468));
		x = renderdata->x + 32;
		y = renderdata->y + 26;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, COLOURWHITE(), viGetWidth(), viGetHeight(), 0, 0);

		// "2"
		sprintf(text, langGet(L_MPMENU_469));
		x = renderdata->x + 24;
		y = renderdata->y + 16;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, COLOURWHITE(), viGetWidth(), viGetHeight(), 0, 0);

		// "1"
		sprintf(text, langGet(L_MPMENU_470));
		x = renderdata->x + 14;
		y = renderdata->y + 4;
		gdl = text0f153858(gdl, &x, &y, &textheight, &textwidth);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, COLOURWHITE(), viGetWidth(), viGetHeight(), 0, 0);

		gdl = text0f153838(gdl);

		// Render lines between the score table and the target texture

		// Horizontal lines - bottom left
		gdl = menugfxDrawFilledRect(gdl, renderdata->x + 45, renderdata->y + 36, renderdata->x + 81, renderdata->y + 37, linecolourtex, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 52, renderdata->y + 44, renderdata->x + 84, renderdata->y + 45, linecolourtex, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 62, renderdata->y + 50, renderdata->x + 87, renderdata->y + 51, linecolourtex, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 70, renderdata->y + 60, renderdata->x + 90, renderdata->y + 61, linecolourtex, linecolourmid);

#if VERSION >= VERSION_JPN_FINAL
		// Vertical lines
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 81, renderdata->y + 17, renderdata->x + 82, renderdata->y + 37, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 84, renderdata->y + 29, renderdata->x + 85, renderdata->y + 45, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 87, renderdata->y + 41, renderdata->x + 88, renderdata->y + 51, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 90, renderdata->y + 53, renderdata->x + 91, renderdata->y + 61, linecolourmid, linecolourmid);

		// Horizontal lines - top right
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 81, renderdata->y + 17, renderdata->x + 96, renderdata->y + 18, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 84, renderdata->y + 29, renderdata->x + 96, renderdata->y + 30, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 87, renderdata->y + 41, renderdata->x + 96, renderdata->y + 42, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 90, renderdata->y + 53, renderdata->x + 96, renderdata->y + 54, linecolourmid, linecolourfig);
#else
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 81, renderdata->y + 17, renderdata->x + 82, renderdata->y + 37, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 84, renderdata->y + 28, renderdata->x + 85, renderdata->y + 45, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 87, renderdata->y + 39, renderdata->x + 88, renderdata->y + 51, linecolourmid, linecolourmid);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 90, renderdata->y + 50, renderdata->x + 91, renderdata->y + 61, linecolourmid, linecolourmid);

		// Horizontal lines - top right
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 81, renderdata->y + 17, renderdata->x + 96, renderdata->y + 18, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 84, renderdata->y + 28, renderdata->x + 96, renderdata->y + 29, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 87, renderdata->y + 39, renderdata->x + 96, renderdata->y + 40, linecolourmid, linecolourfig);
		gdl = menugfxDrawDialogBorderLine(gdl, renderdata->x + 90, renderdata->y + 50, renderdata->x + 96, renderdata->y + 51, linecolourmid, linecolourfig);
#endif

		return (uintptr_t)gdl;
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult menuhandlerFrFailedContinue(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (g_Vars.currentplayer->prop->rooms[0] == 0xa) {
			func0f0f3704(&g_FrWeaponListMenuDialog);
		} else {
			menuPopDialog();
		}
	}

	return 0;
}
#endif

struct menuitem g_FrDifficultyMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING,
		L_MPMENU_444, // "Select Difficulty:"
		L_MPMENU_445, // ""
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_439, // "Bronze"
		L_OPTIONS_003, // ""
		frDifficultyMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_440, // "Silver"
		L_OPTIONS_003, // ""
		frDifficultyMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_441, // "Gold"
		L_OPTIONS_003, // ""
		frDifficultyMenuHandler,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_429, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrDifficultyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_442, // "Difficulty"
	g_FrDifficultyMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_FrWeaponListMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x000000aa,
		0,
		frWeaponListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrWeaponListMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_446, // "Weapon"
	g_FrWeaponListMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_FrTrainingInfoInGameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_443, // "Difficulty:"
		(uintptr_t)&frMenuTextDifficultyName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextGoalScoreLabel,
		(uintptr_t)&frMenuTextGoalScoreValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextMinAccuracyOrTargetsLabel,
		(uintptr_t)&frMenuTextMinAccuracyOrTargetsValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextTimeLimitLabel,
		(uintptr_t)&frMenuTextTimeLimitValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextAmmoLimitLabel,
		(uintptr_t)&frMenuTextAmmoLimitValue,
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
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_FRWEAPON,
		MENUITEMFLAG_DARKERBG,
		0x0000010e,
		PAL ? 0x69 : 0x5f,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_003, // ""
		L_MPMENU_428, // "Resume"
		frDetailsOkMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_003, // ""
		L_MPMENU_430, // "Abort"
		frAbortMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrTrainingInfoInGameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_447, // "Training Info"
	g_FrTrainingInfoInGameMenuItems,
	frTrainingInfoMenuDialog,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE | MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_FrTrainingInfoPreGameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_443, // "Difficulty:"
		(uintptr_t)&frMenuTextDifficultyName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextGoalScoreLabel,
		(uintptr_t)&frMenuTextGoalScoreValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextMinAccuracyOrTargetsLabel,
		(uintptr_t)&frMenuTextMinAccuracyOrTargetsValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextTimeLimitLabel,
		(uintptr_t)&frMenuTextTimeLimitValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		(uintptr_t)&frMenuTextAmmoLimitLabel,
		(uintptr_t)&frMenuTextAmmoLimitValue,
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
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_FRWEAPON,
		MENUITEMFLAG_DARKERBG,
		0x0000010e,
		PAL ? 0x69 : 0x5f,
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_003, // ""
		L_MPMENU_427, // "Ok"
		frDetailsOkMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_003, // ""
		L_MPMENU_429, // "Cancel"
		frAbortMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrTrainingInfoPreGameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_447, // "Training Info"
	g_FrTrainingInfoPreGameMenuItems,
	frTrainingInfoMenuDialog,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE | MENUDIALOGFLAG_0400,
	NULL,
};

struct menuitem g_FrCompletedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_449, // "Completed!"
		0,
		NULL,
	},
#if VERSION < VERSION_NTSC_1_0
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
		MENUITEMFLAG_00000002,
		L_MPMENU_450, // "Score:"
		(uintptr_t)&frMenuTextScoreValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_451, // "Targets Destroyed:"
		(uintptr_t)&frMenuTextTargetsDestroyedValue,
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
		MENUITEMFLAG_00000002,
		L_MPMENU_452, // "Difficulty:"
		(uintptr_t)&frMenuTextDifficultyName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_453, // "Time Taken:"
		(uintptr_t)&frMenuTextTimeTakenValue,
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
		MENUITEMFLAG_00000002,
		L_MPMENU_454, // "Weapon:"
		(uintptr_t)&frMenuTextWeaponName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_455, // "Accuracy:"
		(uintptr_t)&frMenuTextAccuracyValue,
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
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x000000d2,
		0x00000050,
		frScoringMenuHandler,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_252, // "Continue"
		0,
		menuhandlerFrFailedContinue,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPMENU_448, // "Training Stats"
	g_FrCompletedMenuItems,
	frTrainingStatsMenuDialog,
#if VERSION >= VERSION_NTSC_1_0
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
#else
	0,
	&g_FrWeaponListMenuDialog,
#endif
};

struct menuitem g_FrFailedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		(uintptr_t)&frMenuTextFailReason,
		0,
		NULL,
	},
#if VERSION < VERSION_NTSC_1_0
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
		MENUITEMFLAG_00000002,
		L_MPMENU_450, // "Score:"
		(uintptr_t)&frMenuTextScoreValue,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_451, // "Targets Destroyed:"
		(uintptr_t)&frMenuTextTargetsDestroyedValue,
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
		MENUITEMFLAG_00000002,
		L_MPMENU_452, // "Difficulty:"
		(uintptr_t)&frMenuTextDifficultyName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_453, // "Time Taken:"
		(uintptr_t)&frMenuTextTimeTakenValue,
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
		MENUITEMFLAG_00000002,
		L_MPMENU_454, // "Weapon:"
		(uintptr_t)&frMenuTextWeaponName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_455, // "Accuracy:"
		(uintptr_t)&frMenuTextAccuracyValue,
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
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x000000d2,
		0x00000050,
		frScoringMenuHandler,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPWEAPONS_252, // "Continue"
		0,
		menuhandlerFrFailedContinue,
	},
#endif
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FrFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_448, // "Training Stats"
	g_FrFailedMenuItems,
	frTrainingStatsMenuDialog,
#if VERSION >= VERSION_NTSC_1_0
	MENUDIALOGFLAG_STARTSELECTS,
	NULL,
#else
	0,
	&g_FrWeaponListMenuDialog,
#endif
};

MenuItemHandlerResult ciOfficeInformationMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct optiongroup groups[2] = {
		{ 0, L_MPMENU_421 }, // "Character Profiles"
		{ 0, L_MPMENU_422 }, // "Other Information"
	};

	s32 numunlockedchrbios = ciGetNumUnlockedChrBios();
	s32 numunlockedmiscbios = ciGetNumUnlockedMiscBios();
	struct chrbio *chrbio;
	struct miscbio *miscbio;

	groups[1].offset = numunlockedchrbios;

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = numunlockedchrbios + numunlockedmiscbios;
		break;
	case MENUOP_GETOPTIONTEXT:
		if (data->list.value < numunlockedchrbios) {
			chrbio = ciGetChrBioByBodynum(ciGetChrBioBodynumBySlot(data->list.value));
			return (uintptr_t) langGet(chrbio->name);
		} else {
			miscbio = ciGetMiscBio(ciGetMiscBioIndexBySlot(data->list.value - numunlockedchrbios));
			return (uintptr_t) langGet(miscbio->name);
		}
		break;
	case MENUOP_SET:
		g_ChrBioSlot = data->list.value;
		if (g_ChrBioSlot < numunlockedchrbios) {
			menuPushDialog(&g_BioProfileMenuDialog);
		} else {
			menuPushDialog(&g_BioTextMenuDialog);
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = g_ChrBioSlot;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 2;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return (uintptr_t) langGet(groups[data->list.value].name);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = data->list.value == 0 ? 0 : numunlockedchrbios;
		break;
	}

	return 0;
}

struct menuitem g_BioListMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		0x000000c8,
		0,
		ciOfficeInformationMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_BioListMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_418, // "Information"
	g_BioListMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_NowSafeMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_437, // "It is now safe to turn off your computer."
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
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_438, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_NowSafeMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_436, // "Cheats"
	g_NowSafeMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

MenuDialogHandlerResult ciCharacterProfileMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	u32 bodynum = ciGetChrBioBodynumBySlot(g_ChrBioSlot);
	u32 mpbodynum = mpGetMpbodynumByBodynum(bodynum);
	u32 mpheadnum = mpGetMpheadnumByMpbodynum(mpbodynum);
	f32 x;
	f32 y;
	f32 scale;

#ifndef PLATFORM_N64
#if VERSION == VERSION_PAL_FINAL
		if (g_ViRes != VIRES_HI) {
			x = -117;

			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				x = -87;
			}
		} else {
			x = -177;

			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				x = -127;
			}
		}
#elif VERSION == VERSION_PAL_BETA
		x = -117;

		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			x = -87;
		}
#else
		x = -130;

		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			x = -100;
		}
#endif
#endif

	switch (operation) {
	case MENUOP_OPEN:
		if (bodynum == BODY_DRCAROLL) {
			scale = 0.7f;
			g_Menus[g_MpPlayerNum].menumodel.zoom = -1;
		} else {
			g_Menus[g_MpPlayerNum].menumodel.zoom = 30;
			scale = 1.0f;
		}

		g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = TICKS(120);
		g_Menus[g_MpPlayerNum].menumodel.removingpiece = false;

#ifdef PLATFORM_N64
#if VERSION == VERSION_PAL_FINAL
		if (g_ViRes != VIRES_HI) {
			x = -117;

			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				x = -87;
			}
		} else {
			x = -177;

			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				x = -127;
			}
		}
#elif VERSION == VERSION_PAL_BETA
		x = -117;

		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			x = -87;
		}
#else
		x = -130;

		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			x = -100;
		}
#endif
#endif

		y = -15;

		if (bodynum == BODY_MRBLONDE) {
			y -= 12;
		}

		if (bodynum == BODY_THEKING) {
			scale = 0.8f;
		}

#ifndef PLATFORM_N64
		x = (float)x * ((f32)SCREEN_WIDTH_LO / (f32)SCREEN_HEIGHT_LO) / videoGetAspect();
#endif

		menuConfigureModel(&g_Menus[g_MpPlayerNum].menumodel, x, y, 0, 0, 0, 0, scale,
				MENUMODELFLAG_HASSCALE | MENUMODELFLAG_HASPOSITION | MENUMODELFLAG_HASROTATION);

		g_Menus[g_MpPlayerNum].menumodel.curposx = 8.2f;
		g_Menus[g_MpPlayerNum].menumodel.curposy = -4.1f;
		g_Menus[g_MpPlayerNum].menumodel.curscale = 0.00393f;
		g_Menus[g_MpPlayerNum].menumodel.curroty = 0;
		g_Menus[g_MpPlayerNum].menumodel.newroty = 0;
		g_Menus[g_MpPlayerNum].menumodel.rottimer60 = TICKS(60);
		break;
	case MENUOP_CLOSE:
		break;
	case MENUOP_TICK:
#ifndef PLATFORM_N64
		x = (float)x * ((f32)SCREEN_WIDTH_LO / (f32)SCREEN_HEIGHT_LO) / videoGetAspect();
		g_Menus[g_MpPlayerNum].menumodel.newposx = x;
#endif

		if (bodynum == BODY_DRCAROLL) {
			static struct modelpartvisibility vis[] = {
				{ MODELPART_DRCAROLL_0001, false },
				{ MODELPART_DRCAROLL_0002, false },
				{ MODELPART_DRCAROLL_0003, false },
				{ MODELPART_DRCAROLL_0004, false },
				{ MODELPART_DRCAROLL_0005, false },
				{ MODELPART_DRCAROLL_0007, false },
				{ MODELPART_DRCAROLL_0008, false },
				{ MODELPART_DRCAROLL_0009, false },
				{ MODELPART_DRCAROLL_000A, false },
				{ MODELPART_DRCAROLL_000B, false },
				{ 255 },
			};

			g_Menus[g_MpPlayerNum].menumodel.partvisibility = vis;
			g_Menus[g_MpPlayerNum].menumodel.newanimnum = ANIM_013E;
		} else {
			g_Menus[g_MpPlayerNum].menumodel.newanimnum = ANIM_006A;
		}

		if (bodynum == BODY_THEKING) {
			static struct modelpartvisibility vis[] = {
				{ MODELPART_CHR_RIGHTHAND, false },
				{ 255 },
			};

			g_Menus[g_MpPlayerNum].menumodel.partvisibility = vis;
		}

		g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_MP_HEADBODY(mpheadnum, mpbodynum);

		if (g_Menus[g_MpPlayerNum].menumodel.rottimer60 > 0) {
			g_Menus[g_MpPlayerNum].menumodel.rottimer60 -= g_Vars.diffframe60;
		} else {
#if VERSION >= VERSION_PAL_BETA
			f32 roty = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60freal;
#else
			f32 roty = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60f;
#endif
			g_Menus[g_MpPlayerNum].menumodel.newroty = roty;
			g_Menus[g_MpPlayerNum].menumodel.curroty = roty;
		}

		break;
	}

	return 0;
}

char *ciMenuTextChrBioName(struct menuitem *item)
{
	struct chrbio *bio = ciGetChrBioByBodynum(ciGetChrBioBodynumBySlot(g_ChrBioSlot));
	sprintf(g_StringPointer, "%s\n", langGet(bio->name));

	return g_StringPointer;
}

char *ciMenuTextChrBioAge(struct menuitem *item)
{
	struct chrbio *bio = ciGetChrBioByBodynum(ciGetChrBioBodynumBySlot(g_ChrBioSlot));
	sprintf(g_StringPointer, "%s\n", langGet(bio->age));

	return g_StringPointer;
}

char *ciMenuTextChrBioRace(struct menuitem *item)
{
	struct chrbio *bio = ciGetChrBioByBodynum(ciGetChrBioBodynumBySlot(g_ChrBioSlot));
	sprintf(g_StringPointer, "%s\n", langGet(bio->race));

	return g_StringPointer;
}

char *ciMenuTextMiscBioName(struct menuitem *item)
{
	struct miscbio *bio = ciGetMiscBio(ciGetMiscBioIndexBySlot(g_ChrBioSlot - ciGetNumUnlockedChrBios()));

	sprintf(g_StringPointer, "%s\n", langGet(bio->name));

	return g_StringPointer;
}

MenuItemHandlerResult dtDeviceListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = dtGetNumAvailable();
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) bgunGetName(dtGetWeaponByDeviceIndex(dtGetIndexBySlot(data->list.value)));
	case MENUOP_SET:
		g_DtSlot = data->list.value;
		menuPushDialog(&g_DtDetailsMenuDialog);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = g_DtSlot;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 0;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return 0;
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = 0;
		break;
	}

	return 0;
}

char *dtMenuTextName(struct menuitem *item)
{
	u32 weaponnum = dtGetWeaponByDeviceIndex(dtGetIndexBySlot(g_DtSlot));

	return bgunGetName(weaponnum);
}

MenuItemHandlerResult menuhandlerDtOkOrResume(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		// @bug: dtBegin() should not be called if training is already in
		// progress. Doing this resets the training timer.
		dtBegin();
		func0f0f8120();
	}

	return 0;
}

MenuItemHandlerResult menuhandler001a6514(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		dtEnd();
	}

	return 0;
}

struct menuitem g_BioProfileMenuItems[] = {
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002,
		0x00000046,
		0x00000096,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_NEWCOLUMN | MENUITEMFLAG_00000002,
		L_MPMENU_432, // "Name:"
		(uintptr_t)&ciMenuTextChrBioName,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_433, // "Age:"
		(uintptr_t)&ciMenuTextChrBioAge,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_434, // "Race:"
		(uintptr_t)&ciMenuTextChrBioRace,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_CHRBIO,
		0,
		0x000000b4,
		0x00000064,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_435, // "Press the B Button to go back."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_BioProfileMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_431, // "Character Profile"
	g_BioProfileMenuItems,
	ciCharacterProfileMenuDialog,
	MENUDIALOGFLAG_0002,
	NULL,
};

struct menuitem g_BioTextMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_MISCBIO,
		0,
		0x000000c8,
		0x00000096,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_414, // "Press the B Button to go back."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_BioTextMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&ciMenuTextMiscBioName,
	g_BioTextMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_DtListMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		0x000000a0,
		0,
		dtDeviceListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_DtListMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_417, // "Device List"
	g_DtListMenuItems,
	NULL,
	0,
	NULL,
};

MenuDialogHandlerResult dtTrainingDetailsMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		{
			s32 weaponnum = dtGetWeaponByDeviceIndex(dtGetIndexBySlot(g_DtSlot));
			u16 unused[] = {64250, 38500, 25650, 25700, 12950};
			func0f1a1ac0();
			g_Menus[g_MpPlayerNum].training.weaponnum = weaponnum;
			func0f105948(weaponnum);

#ifdef PLATFORM_N64
#if VERSION == VERSION_PAL_FINAL
			if (g_ViRes == VIRES_HI) {
				if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
					g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
					g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
				} else {
					g_Menus[g_MpPlayerNum].menumodel.newposx = 104;
					g_Menus[g_MpPlayerNum].menumodel.curposx = 104;
				}
			} else {
				if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
					g_Menus[g_MpPlayerNum].menumodel.newposx = 64;
					g_Menus[g_MpPlayerNum].menumodel.curposx = 64;
				} else {
					g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
					g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
				}
			}
#elif VERSION == VERSION_PAL_BETA
			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 64;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 64;
			} else {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
			}
#else
			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 70;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 70;
			} else {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 90;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 90;
			}
#endif
#endif

			g_Menus[g_MpPlayerNum].menumodel.newscale /= 2.5f;
		}
		break;
	case MENUOP_CLOSE:
		break;
	case MENUOP_TICK:
#ifndef PLATFORM_N64
#if VERSION == VERSION_PAL_FINAL
		if (g_ViRes == VIRES_HI) {
			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
			} else {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 104;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 104;
			}
		} else {
			if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 64;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 64;
			} else {
				g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
				g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
			}
		}
#elif VERSION == VERSION_PAL_BETA
		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			g_Menus[g_MpPlayerNum].menumodel.newposx = 64;
			g_Menus[g_MpPlayerNum].menumodel.curposx = 64;
		} else {
			g_Menus[g_MpPlayerNum].menumodel.newposx = 84;
			g_Menus[g_MpPlayerNum].menumodel.curposx = 84;
		}
#else
		if (optionsGetScreenRatio() == SCREENRATIO_16_9) {
			g_Menus[g_MpPlayerNum].menumodel.newposx = 70;
			g_Menus[g_MpPlayerNum].menumodel.curposx = 70;
		} else {
			g_Menus[g_MpPlayerNum].menumodel.newposx = 90;
			g_Menus[g_MpPlayerNum].menumodel.curposx = 90;
		}
#endif

		g_Menus[g_MpPlayerNum].menumodel.newposx *= ((f32)SCREEN_WIDTH_LO / (f32)SCREEN_HEIGHT_LO) / videoGetAspect();
		g_Menus[g_MpPlayerNum].menumodel.curposx = g_Menus[g_MpPlayerNum].menumodel.newposx;
#endif

		if (g_Menus[g_MpPlayerNum].curdialog && g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			if (dtGetWeaponByDeviceIndex(dtGetIndexBySlot(g_DtSlot)) == WEAPON_DISGUISE41) {
				g_Menus[g_MpPlayerNum].menumodel.newanimnum = ANIM_006A;
				g_Menus[g_MpPlayerNum].menumodel.rottimer60 = TICKS(60);
				g_Menus[g_MpPlayerNum].menumodel.zoomtimer60 = TICKS(120);
			}

			g_Menus[g_MpPlayerNum].menumodel.curroty = g_Menus[g_MpPlayerNum].menumodel.newroty = 18.849555969238f * g_20SecIntervalFrac;
			g_Menus[g_MpPlayerNum].menumodel.currotz = g_Menus[g_MpPlayerNum].menumodel.newrotz = 0;
		}
		break;
	}

	return 0;
}

char *dtMenuTextOkOrResume(struct menuitem *item)
{
	struct trainingdata *data = dtGetData();

	if (data->intraining) {
		return langGet(L_MPMENU_428); // "Resume"
	}

	return langGet(L_MPMENU_427); // "Ok"
}

char *dtMenuTextCancelOrAbort(struct menuitem *item)
{
	struct trainingdata *data = dtGetData();

	if (data->intraining) {
		return langGet(L_MPMENU_430); // "Abort"
	}

	return langGet(L_MPMENU_429); // "Cancel"
}

char *dtMenuTextTimeTakenValue(struct menuitem *item)
{
	struct trainingdata *data = dtGetData();
	f32 secs = data->timetaken / (PAL ? 50.0f : 60.0f);

	if (secs >= 60.0f) {
		s32 mins = 0;

		while (secs >= 60.0f) {
			secs -= 60.0f;
			mins++;
		}

		sprintf(g_StringPointer, "%dm %2ds\n", mins, (s32)ceilf(secs));
		return g_StringPointer;
	} else {
		sprintf(g_StringPointer, "%s%s%2.2fs\n", "", "", secs);
	}

	return g_StringPointer;
}

MenuDialogHandlerResult menudialogDeviceTrainingResults(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_CLOSE) {
		chrSetStageFlag(NULL, 0x08000000);
	}

	return false;
}

MenuItemHandlerResult htHoloListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = htGetNumUnlocked();
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) htGetName(htGetIndexBySlot(data->list.value));
	case MENUOP_SET:
		var80088bb4 = data->list.value;
		menuPushDialog(&g_HtDetailsMenuDialog);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = var80088bb4;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 0;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return 0;
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = 0;
		break;
	}

	return 0;
}

char *htMenuTextName(struct menuitem *item)
{
	return htGetName(htGetIndexBySlot(var80088bb4));
}

MenuItemHandlerResult menuhandler001a6a34(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		htBegin();
		func0f0f8120();
	}

	return 0;
}

MenuItemHandlerResult menuhandler001a6a70(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		htEnd();
	}

	return 0;
}

MenuDialogHandlerResult menudialog001a6aa4(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
		func0f1a2198();
		break;
	case MENUOP_CLOSE:
		break;
	}

	return false;
}

char *htMenuTextOkOrResume(struct menuitem *item)
{
	struct trainingdata *data = getHoloTrainingData();

	if (data->intraining) {
		return langGet(L_MPMENU_428); // "Resume"
	}

	return langGet(L_MPMENU_427); // "Ok"
}

char *htMenuTextCancelOrAbort(struct menuitem *item)
{
	struct trainingdata *data = getHoloTrainingData();

	if (data->intraining) {
		return langGet(L_MPMENU_430); // "Abort"
	}

	return langGet(L_MPMENU_429); // "Cancel"
}

char *htMenuTextTimeTakenValue(struct menuitem *item)
{
	struct trainingdata *data = getHoloTrainingData();
	f32 secs = data->timetaken / (PAL ? 50.0f : 60.0f);

	if (secs >= 60.0f) {
		s32 mins = 0;

		while (secs >= 60.0f) {
			secs -= 60.0f;
			mins++;
		}

		sprintf(g_StringPointer, "%dm %2ds\n", mins, (s32)ceilf(secs));
		return g_StringPointer;
	} else {
		sprintf(g_StringPointer, "%s%s%2.2fs\n", "", "", secs);
	}

	return g_StringPointer;
}

MenuDialogHandlerResult menudialogFiringRangeResults(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_CLOSE) {
		chrSetStageFlag(NULL, 0x08000000);
	}

	return false;
}

char *bioMenuTextName(struct menuitem *item)
{
	struct hangarbio *bio = ciGetHangarBio(ciGetHangarBioIndexBySlot(g_HangarBioSlot));

	return langGet(bio->name);
}

/**
 * The subheading is stored in the same string as the name, separated by a pipe.
 * eg. "Lucerne Tower\0|Global headquarters\n"
 */
char *ciMenuTextHangarBioSubheading(struct menuitem *item)
{
	s32 index = 0;
	struct hangarbio *bio = ciGetHangarBio(ciGetHangarBioIndexBySlot(g_HangarBioSlot));
	char *name = langGet(bio->name);

	while (name[index] != '|') {
		index++;
	}

	sprintf(g_StringPointer, "%s\n", &name[index + 1]);

	return g_StringPointer;
}

struct menuitem g_DtDetailsMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_DEVICETRAINING,
		0,
		0x000000aa,
		PAL ? 0xd6 : 0xbe,
		NULL,
	},
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_NEWCOLUMN | MENUITEMFLAG_00000002,
		0x0000008c,
		PAL ? 0xb4 : 0x9c,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t)&dtMenuTextOkOrResume,
		0,
		menuhandlerDtOkOrResume,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t)&dtMenuTextCancelOrAbort,
		0,
		menuhandler001a6514,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_DtDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&dtMenuTextName,
	g_DtDetailsMenuItems,
	dtTrainingDetailsMenuDialog,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_DtFailedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_426, // "Failed!"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_424, // "Time Taken:"
		(uintptr_t)&dtMenuTextTimeTakenValue,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_DEVICETIP1,
		0,
		0x00000082,
		PAL ? 110 : 100,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_DtFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_423, // "Training Stats"
	g_DtFailedMenuItems,
	menudialogDeviceTrainingResults,
	MENUDIALOGFLAG_DISABLERESIZE,
	&g_DtListMenuDialog,
};

struct menuitem g_DtCompletedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_425, // "Completed!"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_424, // "Time Taken:"
		(uintptr_t)&dtMenuTextTimeTakenValue,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_DEVICETIP2,
		0,
		0x00000082,
		PAL ? 110 : 100,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_DtCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPMENU_423, // "Training Stats"
	g_DtCompletedMenuItems,
	menudialogDeviceTrainingResults,
	MENUDIALOGFLAG_DISABLERESIZE,
	&g_DtListMenuDialog,
};

struct menuitem g_HtListMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		0x000000a0,
		0,
		htHoloListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HtListMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_416, // "Holotraining"
	g_HtListMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_HtDetailsMenuItems[] = {
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_HOLOTRAINING,
		0,
		0x000000aa,
		0x000000be,
		NULL,
	},
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_NEWCOLUMN | MENUITEMFLAG_00000002,
		0x0000008c,
		0x0000009c,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t)&htMenuTextOkOrResume,
		0,
		menuhandler001a6a34,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t)&htMenuTextCancelOrAbort,
		0,
		menuhandler001a6a70,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HtDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&htMenuTextName,
	g_HtDetailsMenuItems,
	menudialog001a6aa4,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_STARTSELECTS | MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_HtFailedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_426, // "Failed!"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_424, // "Time Taken:"
		(uintptr_t)&htMenuTextTimeTakenValue,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_HOLOTIP1,
		0,
		0x00000082,
#if VERSION >= VERSION_JPN_FINAL
		120,
#elif PAL
		110,
#else
		100,
#endif
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HtFailedMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	L_MPMENU_423, // "Training Stats"
	g_HtFailedMenuItems,
	menudialogFiringRangeResults,
	MENUDIALOGFLAG_DISABLERESIZE,
	&g_HtListMenuDialog,
};

struct menuitem g_HtCompletedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_425, // "Completed!"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002,
		L_MPMENU_424, // "Time Taken:"
		(uintptr_t)&htMenuTextTimeTakenValue,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_HOLOTIP2,
		0,
		0x00000082,
#if VERSION >= VERSION_JPN_FINAL
		120,
#elif PAL
		110,
#else
		100,
#endif
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HtCompletedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPMENU_423, // "Training Stats"
	g_HtCompletedMenuItems,
	menudialogFiringRangeResults,
	MENUDIALOGFLAG_DISABLERESIZE,
	&g_HtListMenuDialog,
};

MenuItemHandlerResult ciHangarInformationMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct optiongroup groups[2] = {
		{ 0, L_MPMENU_419 }, // "Locations"
		{ 0, L_MPMENU_420 }, // "Vehicles"
	};

	s32 bioindex;
	struct hangarbio *bio;

	groups[1].offset = ciGetNumUnlockedLocationBios();

	switch (operation) {
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = ciGetNumUnlockedHangarBios();
		break;
	case MENUOP_GETOPTIONTEXT:
		bio = ciGetHangarBio(ciGetHangarBioIndexBySlot(data->list.value));
		return (uintptr_t) langGet(bio->name);
	case MENUOP_SET:
		g_HangarBioSlot = data->list.value;
		bioindex = ciGetHangarBioIndexBySlot(g_HangarBioSlot);

		if (bioindex <= HANGARBIO_SKEDARRUINS) {
			menuPushDialog(&g_HangarLocationDetailsMenuDialog);
		} else {
			menuPushDialog(&g_HangarVehicleDetailsMenuDialog);
		}
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = g_HangarBioSlot;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = 2;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return (uintptr_t) langGet(groups[data->list.value].name);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = data->list.value == 0 ? 0 : groups[1].offset;
		break;
	}

	return 0;
}

MenuItemHandlerResult ciHangarTitleMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_RENDER) {
		Gfx *gdl = data->type19.gdl;
		struct menuitemrenderdata *renderdata = data->type19.renderdata2;
		s32 textwidth;
		s32 textheight;
		s32 leftmargin;
		char *text;
		s32 index = ciGetHangarBioIndexBySlot(g_HangarBioSlot);

		if (index < NUM_BIO_LOCATIONS) {
			// Location bio - render texture
			u8 texturenums[] = { 0x1b, 0x0d, 0x0e, 0x10, 0x11, 0x12, 0x13, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1c, 0x1d };
			s32 texturenum = texturenums[index];

			gDPPipeSync(gdl++);
			gDPSetTexturePersp(gdl++, G_TP_NONE);
			gDPSetAlphaCompare(gdl++, G_AC_NONE);
			gDPSetTextureLOD(gdl++, G_TL_TILE);
			gDPSetTextureConvert(gdl++, G_TC_FILT);

			texSelect(&gdl, &g_TexGeneralConfigs[texturenum], 1, 0, 2, 1, NULL);

			gDPSetCycleType(gdl++, G_CYC_1CYCLE);
			gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
			gDPSetTextureFilter(gdl++, G_TF_POINT);

#if VERSION >= VERSION_NTSC_1_0
			gDPSetEnvColorViaWord(gdl++, 0xffffff00 | ((renderdata->colour & 0xff) * 255) >> 8);
#endif

			gSPTextureRectangle(gdl++,
					((renderdata->x + 6) << 2) * g_ScaleX, (renderdata->y + 3) << 2,
					((renderdata->x + 60) << 2) * g_ScaleX, (renderdata->y + 39) << 2,
					G_TX_RENDERTILE, 0, 0x0480, 1024 / g_ScaleX, -1024);

			leftmargin = -1;
		} else {
			// Vehicle bio
			leftmargin = item->param2 / 2;
		}

		gdl = text0f153628(gdl);

		// Render title
		text = bioMenuTextName(NULL);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicMd, g_FontHandelGothicMd, 0);

		if (leftmargin == -1) {
			textwidth = renderdata->x + 64;
		} else {
			textwidth = (renderdata->x + leftmargin) - (textwidth >> 1);
		}

		textheight = renderdata->y + 8;
		gdl = textRenderProjected(gdl, &textwidth, &textheight, text, g_CharsHandelGothicMd, g_FontHandelGothicMd, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

		// Render subheading
		text = ciMenuTextHangarBioSubheading(NULL);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		if (leftmargin == -1) {
			textwidth = renderdata->x + 64;
		} else {
			textwidth = (renderdata->x + leftmargin) - (textwidth >> 1);
		}

		textheight = renderdata->y + 25;
		gdl = textRenderProjected(gdl, &textwidth, &textheight, text, g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

		gdl = text0f153780(gdl);

		return (uintptr_t)gdl;
	}

	return 0;
}

struct biovehicleitem {
	u32 fileid;
	s32 y_offset;
	u16 size;
};

struct modelpartvisibility g_BioPartVisibility[] = {
	{ MODELPART_SKSHUTTLE_GANGWAY, false },
	{ MODELPART_CHOPPER_0005,      false },
	{ MODELPART_ROBOT_0004,        false },
	{ 255 },
};

MenuDialogHandlerResult ciHangarHolographMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	s32 index = ciGetHangarBioIndexBySlot(g_HangarBioSlot);

	if (index >= NUM_BIO_LOCATIONS) {
		struct biovehicleitem items[] = {
			// File ID, Y offset, size
			{ FILE_PDROPSHIP,       7,   8    },
			{ FILE_PHOVERCRATE1,    -5,  600  },
			{ FILE_PHOVBIKE,        -10, 50   },
			{ FILE_PHOOVERBOT,      -20, 1000 },
			{ FILE_PDD_HOVERCOPTER, 35,  30   },
			{ FILE_CCHICROB,        0,   70   },
			{ FILE_PA51INTERCEPTOR, -30, 500  },
			{ FILE_PELVIS_SAUCER,   0,   15   },
			{ FILE_PSK_SHUTTLE,     0,   10   },
		};

		switch (operation) {
		case MENUOP_OPEN:
			g_Menus[g_MpPlayerNum].menumodel.currotx = g_Menus[g_MpPlayerNum].menumodel.newrotx = 0;
			g_Menus[g_MpPlayerNum].menumodel.curroty = g_Menus[g_MpPlayerNum].menumodel.newroty = 0;
			g_Menus[g_MpPlayerNum].menumodel.curposx = g_Menus[g_MpPlayerNum].menumodel.newposx = 0;
			g_Menus[g_MpPlayerNum].menumodel.curposy = g_Menus[g_MpPlayerNum].menumodel.newposy = 0;
			g_Menus[g_MpPlayerNum].menumodel.curscale = 0;
			break;
		case MENUOP_CLOSE:
			break;
		case MENUOP_TICK:
			if (g_Menus[g_MpPlayerNum].curdialog) {
#if VERSION >= VERSION_JPN_FINAL
				if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
					f32 offset;
					f32 size;

					index -= NUM_BIO_LOCATIONS;

					offset = items[index].y_offset;
					size = items[index].size * 0.001f;

					g_Menus[g_MpPlayerNum].menumodel.newscale = size;
					g_Menus[g_MpPlayerNum].menumodel.curposy = offset;
					g_Menus[g_MpPlayerNum].menumodel.newposy = offset;

					if (g_Menus[g_MpPlayerNum].menumodel.newparams != items[index].fileid) {
						g_Menus[g_MpPlayerNum].menumodel.loaddelay = 8;
					}

					g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(items[index].fileid);
					g_Menus[g_MpPlayerNum].menumodel.newroty = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60freal;
					g_Menus[g_MpPlayerNum].menumodel.curroty += 0.01f * g_Vars.diffframe60freal;
					g_Menus[g_MpPlayerNum].menumodel.partvisibility = g_BioPartVisibility;
				} else {
					g_Menus[g_MpPlayerNum].menumodel.currotx = g_Menus[g_MpPlayerNum].menumodel.newrotx = 0;
					g_Menus[g_MpPlayerNum].menumodel.curroty = g_Menus[g_MpPlayerNum].menumodel.newroty = 0;
					g_Menus[g_MpPlayerNum].menumodel.curposx = g_Menus[g_MpPlayerNum].menumodel.newposx = 0;
					g_Menus[g_MpPlayerNum].menumodel.curposy = g_Menus[g_MpPlayerNum].menumodel.newposy = 0;
					g_Menus[g_MpPlayerNum].menumodel.curscale = g_Menus[g_MpPlayerNum].menumodel.newscale = 0;
				}
#else
				if (g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
					f32 offset;
					f32 size;

					index -= NUM_BIO_LOCATIONS;

					offset = items[index].y_offset;
					size = items[index].size * 0.001f;

					g_Menus[g_MpPlayerNum].menumodel.newscale = size;
					g_Menus[g_MpPlayerNum].menumodel.curposy = offset;
					g_Menus[g_MpPlayerNum].menumodel.newposy = offset;
					g_Menus[g_MpPlayerNum].menumodel.newparams = MENUMODELPARAMS_SET_FILENUM(items[index].fileid);
#if VERSION >= VERSION_PAL_BETA
					g_Menus[g_MpPlayerNum].menumodel.newroty = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60freal;
					g_Menus[g_MpPlayerNum].menumodel.curroty += 0.01f * g_Vars.diffframe60freal;
#else
					g_Menus[g_MpPlayerNum].menumodel.newroty = g_Menus[g_MpPlayerNum].menumodel.curroty + 0.01f * g_Vars.diffframe60f;
					g_Menus[g_MpPlayerNum].menumodel.curroty += 0.01f * g_Vars.diffframe60f;
#endif
					g_Menus[g_MpPlayerNum].menumodel.partvisibility = g_BioPartVisibility;
				}
#endif
			}
			break;
		}
	}

	return 0;
}

struct menudialogdef *ciGetFrWeaponListMenuDialog(void)
{
	return &g_FrWeaponListMenuDialog;
}

struct menuitem g_HangarDetailsMenuItems[] = {
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_LIST_CUSTOMRENDER,
		0x00000104,
		0x0000002c,
		ciHangarTitleMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SCROLLABLE,
		DESCRIPTION_HANGARBIO,
		0,
		0x00000104,
		0x0000005a,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_414, // "Press the B Button to go back."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_HangarVehicleHolographMenuItems[] = {
	{
		MENUITEMTYPE_MODEL,
		0,
		MENUITEMFLAG_00000002,
		0x00000104,
		0x0000006e,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_00000002,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_00000002 | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_414, // "Press the B Button to go back."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HangarVehicleHolographMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MISC_471, // "Holograph"
	g_HangarVehicleHolographMenuItems,
	ciHangarHolographMenuDialog,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menudialogdef g_HangarVehicleDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&bioMenuTextName,
	g_HangarDetailsMenuItems,
	NULL,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE,
	&g_HangarVehicleHolographMenuDialog,
};

struct menudialogdef g_HangarLocationDetailsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)&bioMenuTextName,
	g_HangarDetailsMenuItems,
	NULL,
	MENUDIALOGFLAG_0002 | MENUDIALOGFLAG_DISABLERESIZE,
	NULL,
};

struct menuitem g_HangarListMenuItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		0x000000a0,
		0,
		ciHangarInformationMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_HangarListMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_415, // "Hangar Information"
	g_HangarListMenuItems,
	NULL,
	0,
	NULL,
};
