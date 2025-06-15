#include <ultra64.h>
#include "constants.h"
#include "lib/sched.h"
#include "lib/str.h"
#include "game/camdraw.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/lang.h"
#include "game/pak.h"
#include "bss.h"
#include "data.h"
#include "string.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

u32 g_CheatsActiveBank0;
u32 g_CheatsActiveBank1;
u32 g_CheatsEnabledBank0;
u32 g_CheatsEnabledBank1;

struct menuitem g_CheatsBuddiesMenuItems[];
struct menudialogdef g_CheatsBuddiesMenuDialog;

#define TIME(mins, secs) (mins * 60 + secs)
#define m
#define s

struct cheat g_Cheats[] = {
	{ gettext_noop("Hurricane Fists\n"), TIME(2 m,  3 s),   SOLOSTAGEINDEX_EXTRACTION,     DIFF_A,  CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK      }, // Hurricane Fists
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("Cloaking Device\n"), TIME(1 m, 40 s),   SOLOSTAGEINDEX_G5BUILDING,     DIFF_A,  CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK      }, // Cloaking Device
#else
	{ L_MPWEAPONS_076, TIME(0 m, 59 s),   SOLOSTAGEINDEX_G5BUILDING,     DIFF_A,  CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK      }, // Cloaking Device
#endif
	{ gettext_noop("Invincible\n"), TIME(3 m, 50 s),   SOLOSTAGEINDEX_ESCAPE,         DIFF_A,  CHEATFLAG_TIMED                              }, // Invincible
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("All Guns in Solo\n"), TIME(5 m, 31 s),   SOLOSTAGEINDEX_SKEDARRUINS,    DIFF_PA, CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK      }, // All Guns in Solo
	{ gettext_noop("Unlimited Ammo\n"), TIME(7 m,  7 s),   SOLOSTAGEINDEX_PELAGIC,        DIFF_SA, CHEATFLAG_TIMED                              }, // Unlimited Ammo
	{ gettext_noop("Unlimited Ammo, No Reloads\n"), TIME(3 m, 11 s),   SOLOSTAGEINDEX_AIRBASE,        DIFF_SA, CHEATFLAG_TIMED                              }, // Unlimited Ammo, No Reloads
#else
	{ L_MPWEAPONS_078, TIME(4 m,  7 s),   SOLOSTAGEINDEX_SKEDARRUINS,    DIFF_PA, CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK      }, // All Guns in Solo
	{ L_MPWEAPONS_079, TIME(5 m, 50 s),   SOLOSTAGEINDEX_PELAGIC,        DIFF_SA, CHEATFLAG_TIMED                              }, // Unlimited Ammo
	{ L_MPWEAPONS_080, TIME(2 m, 59 s),   SOLOSTAGEINDEX_AIRBASE,        DIFF_SA, CHEATFLAG_TIMED                              }, // Unlimited Ammo, No Reloads
#endif
	{ gettext_noop("Slo-mo Single Player\n"), 0,                 SOLOSTAGEINDEX_INVESTIGATION,  DIFF_A,  CHEATFLAG_COMPLETION                         }, // Slo-mo Single Player
	{ gettext_noop("DK Mode\n"), 0,                 SOLOSTAGEINDEX_CHICAGO,        DIFF_A,  CHEATFLAG_COMPLETION                         }, // DK Mode
	{ gettext_noop("Trent's Magnum\n"), TIME(2 m, 50 s),   SOLOSTAGEINDEX_CRASHSITE,      DIFF_A,  CHEATFLAG_TIMED                              }, // Trent's Magnum
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("FarSight\n"), TIME(7 m, 27 s),   SOLOSTAGEINDEX_DEEPSEA,        DIFF_PA, CHEATFLAG_TIMED                              }, // FarSight
#else
	{ L_MPWEAPONS_084, TIME(5 m, 13 s),   SOLOSTAGEINDEX_DEEPSEA,        DIFF_PA, CHEATFLAG_TIMED                              }, // FarSight
#endif
	{ gettext_noop("Small Jo\n"), 0,                 SOLOSTAGEINDEX_G5BUILDING,     DIFF_A,  CHEATFLAG_COMPLETION                         }, // Small Jo
	{ gettext_noop("Small Characters\n"), 0,                 SOLOSTAGEINDEX_INFILTRATION,   DIFF_A,  CHEATFLAG_COMPLETION                         }, // Small Characters
	{ gettext_noop("Enemy Shields\n"), 0,                 SOLOSTAGEINDEX_DEFENSE,        DIFF_A,  CHEATFLAG_COMPLETION                         }, // Enemy Shields
	{ gettext_noop("Jo Shield\n"), 0,                 SOLOSTAGEINDEX_DEEPSEA,        DIFF_A,  CHEATFLAG_COMPLETION                         }, // Jo Shield
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("Super Shield\n"), TIME(1 m, 45 s),   SOLOSTAGEINDEX_DEFENSE,        DIFF_A,  CHEATFLAG_TIMED                              }, // Super Shield
#else
	{ L_MPWEAPONS_089, TIME(1 m, 12 s),   SOLOSTAGEINDEX_DEFENSE,        DIFF_A,  CHEATFLAG_TIMED                              }, // Super Shield
#endif
	{ gettext_noop("Classic Sight\n"), 0,                 SOLOSTAGEINDEX_DEFECTION,      DIFF_A,  CHEATFLAG_COMPLETION                         }, // Classic Sight
	{ gettext_noop("Team Heads Only\n"), 0,                 SOLOSTAGEINDEX_AIRBASE,        DIFF_A,  CHEATFLAG_COMPLETION                         }, // Team Heads Only
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("Play as Elvis\n"), TIME(7 m, 59 s),   SOLOSTAGEINDEX_RESCUE,         DIFF_PA, CHEATFLAG_TIMED                              }, // Play as Elvis
#else
	{ L_MPWEAPONS_092, TIME(7 m,  0 s),   SOLOSTAGEINDEX_RESCUE,         DIFF_PA, CHEATFLAG_TIMED                              }, // Play as Elvis
#endif
	{ gettext_noop("Enemy Rockets\n"), 0,                 SOLOSTAGEINDEX_PELAGIC,        DIFF_A,  CHEATFLAG_COMPLETION                         }, // Enemy Rockets
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("Unlimited Ammo - Laptop Sentry Gun\n"), TIME(3 m, 55 s),   SOLOSTAGEINDEX_AIRFORCEONE,    DIFF_PA, CHEATFLAG_TIMED                              }, // Unlimited Ammo - Laptop Sentry Gun
#else
	{ L_MPWEAPONS_094, TIME(2 m, 59 s),   SOLOSTAGEINDEX_AIRFORCEONE,    DIFF_PA, CHEATFLAG_TIMED                              }, // Unlimited Ammo - Laptop Sentry Gun
#endif
	{ gettext_noop("Marquis of Queensbury Rules\n"), TIME(1 m, 30 s),   SOLOSTAGEINDEX_DEFECTION,      DIFF_SA, CHEATFLAG_TIMED                              }, // Marquis of Queensbury Rules
	{ gettext_noop("Perfect Darkness\n"), 0,                 SOLOSTAGEINDEX_CRASHSITE,      DIFF_A,  CHEATFLAG_COMPLETION                         }, // Perfect Darkness
	{ gettext_noop("Pugilist\n"), TIME(6 m, 30 s),   SOLOSTAGEINDEX_INVESTIGATION,  DIFF_PA, CHEATFLAG_TIMED                              }, // Pugilist
	{ gettext_noop("Hotshot\n"), TIME(5 m,  0 s),   SOLOSTAGEINDEX_INFILTRATION,   DIFF_SA, CHEATFLAG_TIMED                              }, // Hotshot
	{ gettext_noop("Hit and Run\n"), TIME(2 m, 30 s),   SOLOSTAGEINDEX_VILLA,          DIFF_SA, CHEATFLAG_TIMED                              }, // Hit and Run
	{ gettext_noop("Alien\n"), TIME(5 m, 17 s),   SOLOSTAGEINDEX_ATTACKSHIP,     DIFF_SA, CHEATFLAG_TIMED                              }, // Alien
	{ gettext_noop("R-Tracker/Weapon Cache Locations\n"), 0,                 SOLOSTAGEINDEX_SKEDARRUINS,    DIFF_A,  CHEATFLAG_COMPLETION | CHEATFLAG_TRANSFERPAK }, // R-Tracker/Weapon Cache Locations
	{ gettext_noop("Rocket Launcher\n"), 0,                 SOLOSTAGEINDEX_EXTRACTION,     DIFF_A,  CHEATFLAG_COMPLETION                         }, // Rocket Launcher
	{ gettext_noop("Sniper Rifle\n"), 0,                 SOLOSTAGEINDEX_VILLA,          DIFF_A,  CHEATFLAG_COMPLETION                         }, // Sniper Rifle
	{ gettext_noop("X-Ray Scanner\n"), 0,                 SOLOSTAGEINDEX_RESCUE,         DIFF_A,  CHEATFLAG_COMPLETION                         }, // X-Ray Scanner
	{ gettext_noop("SuperDragon\n"), 0,                 SOLOSTAGEINDEX_ESCAPE,         DIFF_A,  CHEATFLAG_COMPLETION                         }, // SuperDragon
	{ gettext_noop("Laptop Gun\n"), 0,                 SOLOSTAGEINDEX_AIRFORCEONE,    DIFF_A,  CHEATFLAG_COMPLETION                         }, // Laptop Gun
	{ gettext_noop("Phoenix\n"), 0,                 SOLOSTAGEINDEX_ATTACKSHIP,     DIFF_A,  CHEATFLAG_COMPLETION                         }, // Phoenix
#if VERSION >= VERSION_NTSC_1_0
	{ gettext_noop("Psychosis Gun\n"), TIME(2 m,  0 s),   SOLOSTAGEINDEX_CHICAGO,        DIFF_PA, CHEATFLAG_TIMED                              }, // Psychosis Gun
#else
	{ L_MPWEAPONS_108, TIME(1 m, 44 s),   SOLOSTAGEINDEX_CHICAGO,        DIFF_PA, CHEATFLAG_TIMED                              }, // Psychosis Gun
#endif
	{ gettext_noop("PP9i\n"), WEAPON_PP9I,       0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // PP9i
	{ gettext_noop("CC13\n"), WEAPON_CC13,       0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // CC13
	{ gettext_noop("KL01313\n"), WEAPON_KL01313,    0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // KL01313
	{ gettext_noop("KF7 Special\n"), WEAPON_KF7SPECIAL, 0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // KF7 Special
	{ gettext_noop("ZZT (9mm)\n"), WEAPON_ZZT,        0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // ZZT (9mm)
	{ gettext_noop("DMC\n"), WEAPON_DMC,        0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // DMC
	{ gettext_noop("AR53\n"), WEAPON_AR53,       0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // AR53
	{ gettext_noop("RC-P45\n"), WEAPON_RCP45,      0,                             0,       CHEATFLAG_FIRINGRANGE                        }, // RC-P45
#ifndef PLATFORM_N64
	{ gettext_noop("(Two-Handed)"), 0,                 SOLOSTAGEINDEX_EXTRACTION,     DIFF_A,  CHEATFLAG_COMPLETION                         }, // Dual wield all guns
#endif
};

u32 cheatIsUnlocked(s32 cheat_id)
{
	struct cheat *cheat = &g_Cheats[cheat_id];
	u32 unlocked = 0;

	if (cheat->flags & CHEATFLAG_FIRINGRANGE) {
		if (frIsClassicWeaponUnlocked(cheat->time)) {
			unlocked++;
		}
	} else if (cheat->flags & CHEATFLAG_COMPLETION) {
		if (g_GameFile.besttimes[cheat->stage_index][0]) {
			unlocked++;
		}
		if (g_GameFile.besttimes[cheat->stage_index][1]) {
			unlocked++;
		}
		if (g_GameFile.besttimes[cheat->stage_index][2]) {
			unlocked++;
		}
	} else {
		if (g_GameFile.besttimes[cheat->stage_index][cheat->difficulty] &&
				g_GameFile.besttimes[cheat->stage_index][cheat->difficulty] <= cheat->time) {
			unlocked++;
		}
	}

	if ((cheat->flags & CHEATFLAG_TRANSFERPAK) && gamefileHasFlag(GAMEFILEFLAG_USED_TRANSFERPAK)) {
		unlocked++;
	}

	return unlocked;
}

bool cheatIsActive(s32 cheat_id)
{
	if (cheat_id < 32) {
		return g_CheatsActiveBank0 & (1 << cheat_id);
	}

	return g_CheatsActiveBank1 & (1 << (cheat_id - 32));
}

void cheatActivate(s32 cheat_id)
{
	u32 prevplayernum;
	s32 playernum;

	switch (cheat_id) {
	case CHEAT_INVINCIBLE:
		// Make all players invincible
		prevplayernum = g_Vars.currentplayernum;

		for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
			setCurrentPlayerNum(playernum);
			g_Vars.currentplayer->invincible = 1;
		}

		setCurrentPlayerNum(prevplayernum);
		break;
	case CHEAT_ALLGUNS:
		// Give all guns if only one player playing
		if (PLAYERCOUNT() == 1 && g_Vars.normmplayerisrunning == false) {
			prevplayernum = g_Vars.currentplayernum;

			for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
				setCurrentPlayerNum(playernum);
				invSetAllGuns(true);
			}

			setCurrentPlayerNum(prevplayernum);
		}
		break;
	}

	if (cheat_id < 32) {
		g_CheatsActiveBank0 = g_CheatsActiveBank0 | (1 << cheat_id);
	} else {
		g_CheatsActiveBank1 = g_CheatsActiveBank1 | (1 << (cheat_id - 32));
	}
}

void cheatDeactivate(s32 cheat_id)
{
	u32 prevplayernum;
	s32 playernum;

	switch (cheat_id) {
	case CHEAT_INVINCIBLE:
		prevplayernum = g_Vars.currentplayernum;

		for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
			setCurrentPlayerNum(playernum);
			g_Vars.currentplayer->invincible = 1; // @bug?
		}

		setCurrentPlayerNum(prevplayernum);
		break;
	case CHEAT_ALLGUNS:
		if (PLAYERCOUNT() == 1 && g_Vars.normmplayerisrunning == false) {
			prevplayernum = g_Vars.currentplayernum;

			for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
				setCurrentPlayerNum(playernum);
				invSetAllGuns(false);
			}

			setCurrentPlayerNum(prevplayernum);
		}
		break;
	}

	if (cheat_id < 32) {
		g_CheatsActiveBank0 = g_CheatsActiveBank0 & ~(1 << cheat_id);
	} else {
		g_CheatsActiveBank1 = g_CheatsActiveBank1 & ~(1 << (cheat_id - 32));
	}
}

void cheatsInit(void)
{
	g_CheatsActiveBank0 = 0;
	g_CheatsActiveBank1 = 0;
	g_CheatsEnabledBank0 = 0;
	g_CheatsEnabledBank1 = 0;
}

/**
 * Apply cheats at level startup.
 */
void cheatsReset(void)
{
	s32 cheat_id;

	// Copy enabled cheats to active cheats, unless in CI training
	// or weapon cheats not in solo
	if (g_Vars.stagenum != STAGE_CITRAINING) {
		g_CheatsActiveBank0 = g_CheatsEnabledBank0;
		g_CheatsActiveBank1 = g_CheatsEnabledBank1;

		if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0 || g_Vars.normmplayerisrunning) {
			// Co-op/counter-op/multi - deactivate "Weapons for Jo in Solo" cheats
			g_CheatsActiveBank0 &= ~(
				(1 << CHEAT_TRENTSMAGNUM) |
				(1 << CHEAT_FARSIGHT) |
				(1 << CHEAT_ROCKETLAUNCHER) |
				(1 << CHEAT_SNIPERRIFLE) |
				(1 << CHEAT_XRAYSCANNER) |
				(1 << CHEAT_SUPERDRAGON) |
				(1 << CHEAT_LAPTOPGUN)
			);
			g_CheatsActiveBank1 &= ~(
				(1 << (CHEAT_PHOENIX - 32)) |
				(1 << (CHEAT_PSYCHOSISGUN - 32)) |
				(1 << (CHEAT_PP9I - 32)) |
				(1 << (CHEAT_CC13 - 32)) |
				(1 << (CHEAT_KL01313 - 32)) |
				(1 << (CHEAT_KF7SPECIAL - 32)) |
				(1 << (CHEAT_ZZT - 32)) |
				(1 << (CHEAT_DMC - 32)) |
				(1 << (CHEAT_AR53 - 32)) |
				(1 << (CHEAT_RCP45 - 32))
			);
		}
	} else {
		g_CheatsActiveBank0 = 0;
		g_CheatsActiveBank1 = 0;
	}

	// Set any "always on" cheats to active and properly activate all active cheats
	for (cheat_id = 0; cheat_id < ARRAYCOUNT(g_Cheats); cheat_id++) {
		if (g_Cheats[cheat_id].flags & CHEATFLAG_ALWAYSON) {
			if (cheatIsUnlocked(cheat_id)) {
				if (cheat_id < 32) {
					g_CheatsActiveBank0 = g_CheatsActiveBank0 | (1 << cheat_id);
				} else {
					g_CheatsActiveBank1 = g_CheatsActiveBank1 | (1 << (cheat_id - 32));
				}
			} else {
				if (cheat_id < 32) {
					g_CheatsActiveBank0 = g_CheatsActiveBank0 & ~(1 << cheat_id);
				} else {
					g_CheatsActiveBank1 = g_CheatsActiveBank1 & ~(1 << (cheat_id - 32));
				}
			}
		}

		if (cheatIsActive(cheat_id)) {
			cheatActivate(cheat_id);
		}
	}
}

MenuItemHandlerResult cheatCheckboxMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		if (item->param < 32) {
			if (g_CheatsEnabledBank0 & (1 << item->param)) {
				return true;
			}

			return false;
		}

		if (g_CheatsEnabledBank1 & (1 << item->param)) {
			return true;
		}

		return false;
	case MENUOP_SET:
		if (cheatIsUnlocked(item->param)) {
			if (item->param < 32) {
				// Bank 0
				if (g_CheatsEnabledBank0 & (1 << item->param)) {
					g_CheatsEnabledBank0 = g_CheatsEnabledBank0 & ~(1 << item->param);
				} else {
					// If enabling Marquis or enemy rockets, turn off the other
					if (item->param == CHEAT_MARQUIS) {
						g_CheatsEnabledBank0 &= ~(1 << CHEAT_ENEMYROCKETS);
					}

					if (item->param == CHEAT_ENEMYROCKETS) {
						g_CheatsEnabledBank0 &= ~(1 << CHEAT_MARQUIS);
					}

					g_CheatsEnabledBank0 = g_CheatsEnabledBank0 | 1 << item->param;
				}
			} else {
				// Bank 1
				if (g_CheatsEnabledBank1 & (1 << item->param)) {
					if (1);
					g_CheatsEnabledBank1 = g_CheatsEnabledBank1 & ~(1 << item->param);
				} else {
					g_CheatsEnabledBank1 = g_CheatsEnabledBank1 | 1 << item->param;
				}
			}
		}
		break;
	}

	return 0;
}

MenuItemHandlerResult cheatMenuHandleBuddyCheckbox(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET:
		if (item->param == 0) {
			if (g_CheatsEnabledBank0 & (1 << CHEAT_PUGILIST | 1 << CHEAT_HOTSHOT | 1 << CHEAT_HITANDRUN | 1 << CHEAT_ALIEN)) {
				return false;
			}

			return true;
		}

		if (g_CheatsEnabledBank0 & (1 << item->param)) {
			return true;
		}

		return false;
	case MENUOP_SET:
		if (item->param == 0) {
			// Velvet
			g_CheatsEnabledBank0 &= ~(
				(1 << CHEAT_PUGILIST) |
				(1 << CHEAT_HOTSHOT) |
				(1 << CHEAT_HITANDRUN) |
				(1 << CHEAT_ALIEN)
			);
		} else if (cheatIsUnlocked(item->param)) {
			// Not Velvet
			g_CheatsEnabledBank0 = g_CheatsEnabledBank0 & ~(
				(1 << CHEAT_PUGILIST) |
				(1 << CHEAT_HOTSHOT) |
				(1 << CHEAT_HITANDRUN) |
				(1 << CHEAT_ALIEN)
			);
			g_CheatsEnabledBank0 = g_CheatsEnabledBank0 | (1 << item->param);
		}
	}

	return 0;
}

char *cheatGetNameIfUnlocked(struct menuitem *item)
{
	if (cheatIsUnlocked(item->param)) {
		return _(g_Cheats[item->param].nametextid);
	}

	return _("----------\n"); // "----------"
}

MenuDialogHandlerResult cheatMenuHandleDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		func0f14a52c();

		if (gbpakIsAnyPerfectDark()) {
			gamefileSetFlag(GAMEFILEFLAG_USED_TRANSFERPAK);
		}

#if PIRACYCHECKS
		{
			u32 *ptr = (u32 *)&__scHandleTasks;
			u32 *end = (u32 *)&__scHandleRSP;
			u32 checksum = 0;

			while (ptr < end) {
				checksum ^= ~*ptr;
				ptr++;
			}

			if (checksum != CHECKSUM_PLACEHOLDER) {
				ptr = (u32 *)&__scHandleTasks + 20;
				if (1);
				end = &ptr[4];

				while (ptr < end) {
					*ptr = 0x00000012;
					ptr++;
				}
			}
		}
#endif
	}

	if (operation == MENUOP_CLOSE) {
		if (gbpakIsAnyPerfectDark()) {
			gamefileSetFlag(GAMEFILEFLAG_USED_TRANSFERPAK);
		}

		func0f14a560();
	}

	return 0;
}

struct menuitem g_CheatsWarningMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		gettext_noop("If you activate any cheats, then you\nwill be unable to progress further in the game\nwhile those cheats are active.\n"), // "If you activate any cheats, then you will be unable to progress further in the game while those cheats are active."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("OK\n"), // "OK"
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
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsWarningMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	gettext_noop("Warning\n"), // "Warning"
	g_CheatsWarningMenuItems,
	NULL,
	0,
	NULL,
};

/**
 * NTSC Beta uses g_StringPointer while newer versions use g_CheatMarqueeString.
 *
 * PAL Final introduces a string length check which ultimately doesn't do
 * anything.
 *
 * JPN final removes the colon characters from the format strings.
 */
char *cheatGetMarquee(struct menuitem *arg0)
{
	u32 cheat_id;
	char *ptr;
	char difficultyname[256];
	char cheatname[256];

#if VERSION >= VERSION_JPN_FINAL
	s32 len;
	static s32 var80074020pf = 0;
	static s32 var80074024pf = 0;

	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->type == MENUITEMTYPE_CHECKBOX) {
		cheat_id = g_Menus[g_MpPlayerNum].curdialog->focuseditem->param;

		if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem == &g_CheatsBuddiesMenuItems[0]) {
			// Velvet
			sprintf(g_CheatMarqueeString, "%s %s", langGet(L_MPWEAPONS_143), langGet(L_MPWEAPONS_117)); // "Buddy Available", "Velvet Dark"
		} else if (cheatIsUnlocked(cheat_id)) {
			// Show cheat name
			sprintf(g_CheatMarqueeString, "%s %s\n",
					g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog ? langGet(L_MPWEAPONS_143) : langGet(L_MPWEAPONS_136), // "Buddy Available", "Cheat available"
					langGet(g_Cheats[cheat_id].nametextid)
			);
		} else {
			// Locked
			strcpy(cheatname, langGet(g_Cheats[cheat_id].nametextid));
			ptr = cheatname;

			while (*ptr != '\n') {
				ptr++;
			}

			*ptr = '\0';

			if (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) {
				sprintf(g_CheatMarqueeString, "%s %s %s %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			} else {
				// Timed
				strcpy(difficultyname, langGet(L_OPTIONS_251 + g_Cheats[cheat_id].difficulty));
				ptr = difficultyname;

				while (*ptr != '\n') {
					ptr++;
				}

				*ptr = '\0';

				sprintf(g_CheatMarqueeString, "%s %s %s %s %s %s %d:%02d %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_139), // "on"
						&difficultyname,
						langGet(L_MPWEAPONS_140), // "in under"
						g_Cheats[cheat_id].time / 60,
						g_Cheats[cheat_id].time % 60,
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			}

			if (g_Cheats[cheat_id].flags & CHEATFLAG_TRANSFERPAK) {
				strcat(g_CheatMarqueeString, langGet(L_MPWEAPONS_141)); // " or insert Game Boy ..."
			}

			strcat(g_CheatMarqueeString, "\n");
		}

		len = strlen(g_CheatMarqueeString);

		if (var80074024pf != len) {
			var80074024pf = len;

			if (len > var80074020pf) {
				var80074020pf = len;
			}
		}

		return g_CheatMarqueeString;
	}
#elif VERSION >= VERSION_PAL_FINAL
	s32 len;
	static s32 var80074020pf = 0;
	static s32 var80074024pf = 0;

	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->type == MENUITEMTYPE_CHECKBOX) {
		cheat_id = g_Menus[g_MpPlayerNum].curdialog->focuseditem->param;

		if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem == &g_CheatsBuddiesMenuItems[0]) {
			// Velvet
			sprintf(g_CheatMarqueeString, "%s: %s", langGet(L_MPWEAPONS_143), langGet(L_MPWEAPONS_117)); // "Buddy Available", "Velvet Dark"
		} else if (cheatIsUnlocked(cheat_id)) {
			// Show cheat name
			sprintf(g_CheatMarqueeString, "%s: %s\n",
					g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog ? langGet(L_MPWEAPONS_143) : langGet(L_MPWEAPONS_136), // "Buddy Available", "Cheat available"
					langGet(g_Cheats[cheat_id].nametextid)
			);
		} else {
			// Locked
			strcpy(cheatname, langGet(g_Cheats[cheat_id].nametextid));
			ptr = cheatname;

			while (*ptr != '\n') {
				ptr++;
			}

			*ptr = '\0';

			if (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) {
				sprintf(g_CheatMarqueeString, "%s %s: %s %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			} else {
				// Timed
				strcpy(difficultyname, langGet(L_OPTIONS_251 + g_Cheats[cheat_id].difficulty));
				ptr = difficultyname;

				while (*ptr != '\n') {
					ptr++;
				}

				*ptr = '\0';

				sprintf(g_CheatMarqueeString, "%s %s: %s %s %s %s %d:%02d %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_139), // "on"
						&difficultyname,
						langGet(L_MPWEAPONS_140), // "in under"
						g_Cheats[cheat_id].time / 60,
						g_Cheats[cheat_id].time % 60,
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			}

			if (g_Cheats[cheat_id].flags & CHEATFLAG_TRANSFERPAK) {
				strcat(g_CheatMarqueeString, langGet(L_MPWEAPONS_141)); // " or insert Game Boy ..."
			}

			strcat(g_CheatMarqueeString, "\n");
		}

		len = strlen(g_CheatMarqueeString);

		if (var80074024pf != len) {
			var80074024pf = len;

			if (len > var80074020pf) {
				var80074020pf = len;
			}
		}

		return g_CheatMarqueeString;
	}
#elif VERSION >= VERSION_NTSC_1_0
	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->type == MENUITEMTYPE_CHECKBOX) {
		cheat_id = g_Menus[g_MpPlayerNum].curdialog->focuseditem->param;

		if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem == &g_CheatsBuddiesMenuItems[0]) {
			// Velvet
			sprintf(g_CheatMarqueeString, "%s: %s", _("Buddy Available"), _("Velvet Dark\n")); // "Buddy Available", "Velvet Dark"
		} else if (cheatIsUnlocked(cheat_id)) {
			// Show cheat name
			sprintf(g_CheatMarqueeString, "%s: %s\n",
					g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog ? _("Buddy Available") : _("Cheat available"), // "Buddy Available", "Cheat available"
					_(g_Cheats[cheat_id].nametextid)
			);
		} else {
			// Locked
			strcpy(cheatname, _(g_Cheats[cheat_id].nametextid));
			ptr = cheatname;

			while (*ptr != '\n') {
				ptr++;
			}

			*ptr = '\0';

			if (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) {
				sprintf(g_CheatMarqueeString, "%s %s: %s %s %s",
						_("Complete"), // "Complete"
						_(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						_(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						_("for cheat:"), // "for cheat:"
						&cheatname
				);
			} else {
				// Timed
				strcpy(difficultyname, _("Agent\n" + g_Cheats[cheat_id].difficulty));
				ptr = difficultyname;

				while (*ptr != '\n') {
					ptr++;
				}

				*ptr = '\0';

				sprintf(g_CheatMarqueeString, "%s %s: %s %s %s %s %d:%02d %s %s",
						_("Complete"), // "Complete"
						_(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						_(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						_("on"), // "on"
						&difficultyname,
						_("in under"), // "in under"
						g_Cheats[cheat_id].time / 60,
						g_Cheats[cheat_id].time % 60,
						_("for cheat:"), // "for cheat:"
						&cheatname
				);
			}

			if (g_Cheats[cheat_id].flags & CHEATFLAG_TRANSFERPAK) {
				strcat(g_CheatMarqueeString, _(" or insert Game Boy (r) Perfect Dark into Transfer Pak (tm), connect Transfer Pak to any controller, then exit and enter menu")); // " or insert Game Boy ..."
			}

			strcat(g_CheatMarqueeString, "\n");
		}

		return g_CheatMarqueeString;
	}
#else
	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->type == MENUITEMTYPE_CHECKBOX) {
		cheat_id = g_Menus[g_MpPlayerNum].curdialog->focuseditem->param;

		if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog
				&& g_Menus[g_MpPlayerNum].curdialog->focuseditem == &g_CheatsBuddiesMenuItems[0]) {
			// Velvet
			sprintf(g_StringPointer, "%s: %s", langGet(L_MPWEAPONS_143), langGet(L_MPWEAPONS_117)); // "Buddy Available", "Velvet Dark"
		} else if (cheatIsUnlocked(cheat_id)) {
			// Show cheat name
			sprintf(g_StringPointer, "%s: %s\n",
					g_Menus[g_MpPlayerNum].curdialog->definition == &g_CheatsBuddiesMenuDialog ? langGet(L_MPWEAPONS_143) : langGet(L_MPWEAPONS_136), // "Buddy Available", "Cheat available"
					langGet(g_Cheats[cheat_id].nametextid)
			);
		} else {
			// Locked
			strcpy(cheatname, langGet(g_Cheats[cheat_id].nametextid));
			ptr = cheatname;

			while (*ptr != '\n') {
				ptr++;
			}

			*ptr = '\0';

			if (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) {
				sprintf(g_StringPointer, "%s %s: %s %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			} else {
				// Timed
				strcpy(difficultyname, langGet(L_OPTIONS_251 + g_Cheats[cheat_id].difficulty));
				ptr = difficultyname;

				while (*ptr != '\n') {
					ptr++;
				}

				*ptr = '\0';

				sprintf(g_StringPointer, "%s %s: %s %s %s %s %d:%02d %s %s",
						langGet(L_MPWEAPONS_137), // "Complete"
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_SoloStages[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_139), // "on"
						&difficultyname,
						langGet(L_MPWEAPONS_140), // "in under"
						g_Cheats[cheat_id].time / 60,
						g_Cheats[cheat_id].time % 60,
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			}

			if (g_Cheats[cheat_id].flags & CHEATFLAG_TRANSFERPAK) {
				strcat(g_StringPointer, langGet(L_MPWEAPONS_141)); // " or insert Game Boy ..."
			}

			strcat(g_StringPointer, "\n");
		}

		return g_StringPointer;
	}
#endif

	// No cheat selected
	return _("Select cheat for information\n"); // "Select cheat for information"
}

MenuItemHandlerResult cheatMenuHandleTurnOffAllCheats(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_CheatsEnabledBank0 = 0;
		g_CheatsEnabledBank1 = 0;
	}

	return false;
}

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByTimedStageIndex(s32 stage_index, s32 difficulty)
{
	s32 cheat_id;

	for (cheat_id = 0; cheat_id < ARRAYCOUNT(g_Cheats); cheat_id++) {
		if (g_Cheats[cheat_id].stage_index == stage_index &&
				g_Cheats[cheat_id].difficulty == difficulty &&
				(g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) == 0 &&
				(g_Cheats[cheat_id].flags & CHEATFLAG_FIRINGRANGE) == 0) {
			return cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByCompletedStageIndex(s32 stage_index)
{
	s32 cheat_id;

	for (cheat_id = 0; cheat_id < ARRAYCOUNT(g_Cheats); cheat_id++) {
		if (g_Cheats[cheat_id].stage_index == stage_index && (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION)) {
			return cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetTime(s32 cheat_id)
{
	return g_Cheats[cheat_id].time;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *cheatGetName(s32 cheat_id)
{
	return _(g_Cheats[cheat_id].nametextid);
}
#endif

#ifndef PLATFORM_N64

static MenuItemHandlerResult menuhandlerUnlockEverything(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		gamefileUnlockEverything();
	}
	return 0;
}

struct menuitem g_CheatsConfirmUnlockMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Are you sure?\n\nThis will overwrite any progress\nsaved to the current profile.\n"),
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES | MENUITEMFLAG_LITERAL_TEXT,
		gettext_noop("Unlocks all cheats, weapons, missions, challenges and combat simulator items.\n"),
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000082,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("No\n"), // "No"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Yes\n"), // "Yes"
		0,
		menuhandlerUnlockEverything,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsConfirmUnlockMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Warning\n"), // "Warning"
	g_CheatsConfirmUnlockMenuItems,
	NULL,
	0,
	NULL,
};

#endif

struct menuitem g_CheatsFunMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_DKMODE,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SMALLJO,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SMALLCHARACTERS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_TEAMHEADSONLY,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PLAYASELVIS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SLOMO,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		MENUITEMFLAG_HANDLER_TEXT,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES | MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsFunMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Fun\n"), // "Fun"
	g_CheatsFunMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsGameplayMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_INVINCIBLE,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_CLOAKINGDEVICE,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_MARQUIS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_JOSHIELD,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SUPERSHIELD,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ENEMYSHIELDS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ENEMYROCKETS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PERFECTDARKNESS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_DUALWIELDALLGUNS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES | MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsGameplayMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Gameplay\n"), // "Gameplay"
	g_CheatsGameplayMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsSoloWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ROCKETLAUNCHER,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SNIPERRIFLE,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_SUPERDRAGON,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_LAPTOPGUN,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PHOENIX,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PSYCHOSISGUN,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_TRENTSMAGNUM,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_FARSIGHT,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsSoloWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Weapons for Jo in Solo\n"), // "Weapons for Jo in Solo"
	g_CheatsSoloWeaponsMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsClassicWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PP9I,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_CC13,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_KL01313,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_KF7SPECIAL,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ZZT,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_DMC,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_AR53,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_RCP45,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x000000c8,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		gettext_noop("Win Golds on the firing range to enable classic guns.\n"), // "Win Golds on the firing range to enable classic guns."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previosu: 0x000000c8,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsClassicWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Classic Weapons for Jo in Solo\n"), // "Classic Weapons for Jo in Solo"
	g_CheatsClassicWeaponsMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsWeaponsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_CLASSICSIGHT,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_UNLIMITEDAMMOLAPTOP,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_HURRICANEFISTS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_UNLIMITEDAMMO,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_UNLIMITEDAMMONORELOADS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_XRAYSCANNER,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_RTRACKER,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ALLGUNS,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsWeaponsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Weapons\n"), // "Weapons"
	g_CheatsWeaponsMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsBuddiesMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		0,
		gettext_noop("Velvet Dark\n"), // "Velvet Dark"
		0,
		cheatMenuHandleBuddyCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_PUGILIST,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatMenuHandleBuddyCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_HOTSHOT,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatMenuHandleBuddyCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_HITANDRUN,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatMenuHandleBuddyCheckbox,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		CHEAT_ALIEN,
		MENUITEMFLAG_HANDLER_TEXT,
		&cheatGetNameIfUnlocked,
		0,
		cheatMenuHandleBuddyCheckbox,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsBuddiesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Buddies\n"), // "Buddies"
	g_CheatsBuddiesMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

struct menuitem g_CheatsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Fun\n"), // "Fun"
		0,
		(void *)&g_CheatsFunMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Gameplay\n"), // "Gameplay"
		0,
		(void *)&g_CheatsGameplayMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Weapons for Jo in Solo\n"), // "Weapons for Jo in Solo"
		0,
		(void *)&g_CheatsSoloWeaponsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Classic Weapons for Jo in Solo\n"), // "Classic Weapons for Jo in Solo"
		0,
		(void *)&g_CheatsClassicWeaponsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Weapons\n"), // "Weapons"
		0,
		(void *)&g_CheatsWeaponsMenuDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Buddies\n"), // "Buddies"
		0,
		(void *)&g_CheatsBuddiesMenuDialog,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Turn off all Cheats\n"), // "Turn off all Cheats"
		0,
		cheatMenuHandleTurnOffAllCheats,
	},
#ifndef PLATFORM_N64
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Unlock Everything\n"),
		0,
		(void *)&g_CheatsConfirmUnlockMenuDialog,
	},
#endif
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"",// previous: 0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Done\n"), // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Cheats\n"), // "Cheats"
	g_CheatsMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};
