#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/player.h"
#include "game/hudmsg.h"
#include "game/playermgr.h"
#include "game/mplayer/setup.h"
#include "game/botcmd.h"
#include "game/botinv.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#endif

u32 var80070590 = 0x00000000;

void mpstatsIncrementPlayerShotCount(struct gset *gset, s32 region)
{
	if (!weaponHasFlag(gset->weaponnum, WEAPONFLAG_DONTCOUNTSHOTS)) {
		g_Vars.currentplayerstats->shotcount[region]++;
	}
}

void mpstatsIncrementPlayerShotCount2(struct gset *gset, s32 region)
{
	if (region == 0) {
		if (!weaponHasFlag(gset->weaponnum, WEAPONFLAG_DONTCOUNTSHOTS)) {
			var80070590 = 1;
			g_Vars.currentplayerstats->shotcount[region]++;
		}
	} else {
		if (var80070590) {
			if (!weaponHasFlag(gset->weaponnum, WEAPONFLAG_DONTCOUNTSHOTS)) {
				g_Vars.currentplayerstats->shotcount[region]++;
			}

			var80070590 = 0;
		}
	}
}

void mpstats0f0b0520(void)
{
	var80070590 = 0;
}

s32 mpstatsGetPlayerShotCountByRegion(u32 type)
{
	return g_Vars.currentplayerstats->shotcount[type];
}

void mpstatsIncrementTotalKillCount(void)
{
	g_Vars.killcount++;
}

void mpstatsIncrementTotalKnockoutCount(void)
{
	g_Vars.knockoutcount++;
}

void mpstatsDecrementTotalKnockoutCount(void)
{
	g_Vars.knockoutcount--;
}

u8 mpstatsGetTotalKnockoutCount(void)
{
	return g_Vars.knockoutcount;
}

u32 mpstatsGetTotalKillCount(void)
{
	return g_Vars.killcount;
}

void mpstatsRecordPlayerKill(void)
{
	char text[256];
	s32 simulkills;
	s32 duration;
	s32 time;

	g_Vars.currentplayerstats->killcount++;
	g_Vars.currentplayer->killsthislife++;

	if (g_Vars.normmplayerisrunning) {
		time = playerGetMissionTime();

		// Show HUD message
		// "Kill count: %d"
		sprintf(text, "%s: %d\n", _("Kill count"), g_Vars.currentplayerstats->killcount);
		hudmsgCreate(text, HUDMSGTYPE_DEFAULT);

		// Update slowest/fastest two kills
		if (g_Vars.currentplayerstats->killcount > 1) {
			duration = time - g_Vars.currentplayer->lastkilltime60;

			if (duration > g_Vars.currentplayerstats->slowest2kills) {
				g_Vars.currentplayerstats->slowest2kills = duration;
			}

			if (duration < g_Vars.currentplayerstats->fastest2kills) {
				g_Vars.currentplayerstats->fastest2kills = duration;
			}
		}

		// Update max simultaneous kills
		simulkills = 1;

		g_Vars.currentplayer->lastkilltime60_4 = g_Vars.currentplayer->lastkilltime60_3;
		g_Vars.currentplayer->lastkilltime60_3 = g_Vars.currentplayer->lastkilltime60_2;
		g_Vars.currentplayer->lastkilltime60_2 = g_Vars.currentplayer->lastkilltime60;
		g_Vars.currentplayer->lastkilltime60 = time;

		if (g_Vars.currentplayer->lastkilltime60_2 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_2 < 120) {
			simulkills++;

			if (g_Vars.currentplayer->lastkilltime60_3 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_3 < 120) {
				simulkills++;

				if (g_Vars.currentplayer->lastkilltime60_4 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_4 < 120) {
					simulkills++;
				}
			}
		}

		if (simulkills > g_Vars.currentplayerstats->maxsimulkills) {
			g_Vars.currentplayerstats->maxsimulkills = simulkills;
		}
	}
}

s32 mpstatsGetPlayerKillCount(void)
{
	return g_Vars.currentplayerstats->killcount;
}

void mpstatsIncrementPlayerGgKillCount(void)
{
	g_Vars.currentplayerstats->ggkillcount++;
}

void mpstatsRecordPlayerDeath(void)
{
	char buffer[256];

	g_Vars.currentplayer->deathcount++;

	if (g_Vars.normmplayerisrunning) {
		if (g_Vars.currentplayer->deathcount == 1) {
			sprintf(buffer, _("Died once\n")); // "Died once"
		} else {
			sprintf(buffer, "%s %d %s\n",
					_("Died"), // "Died"
					g_Vars.currentplayer->deathcount,
					_("times")); // "times"
		}

		hudmsgCreate(buffer, HUDMSGTYPE_DEFAULT);
	}
}

void mpstatsRecordPlayerSuicide(void)
{
	char text[256];
	s32 simulkills;
	s32 duration;
	s32 time;
	s32 mpindex;
	struct mpchrconfig *mpchr;

	if (g_Vars.normmplayerisrunning) {
		time = playerGetMissionTime();
		mpindex = g_Vars.currentplayerstats->mpindex;

		mpchr = MPCHR(mpindex);

		// Show HUD message
		// "Suicide count: %d"
		sprintf(text, "%s: %d\n", _("Suicide count"), mpchr->killcounts[mpindex]);
		hudmsgCreate(text, HUDMSGTYPE_DEFAULT);

		// Update slowest/fastest two kills
		if (g_Vars.currentplayerstats->killcount > 1) {
			duration = time - g_Vars.currentplayer->lastkilltime60;

			if (duration > g_Vars.currentplayerstats->slowest2kills) {
				g_Vars.currentplayerstats->slowest2kills = duration;
			}

			if (duration < g_Vars.currentplayerstats->fastest2kills) {
				g_Vars.currentplayerstats->fastest2kills = duration;
			}
		}

		// Update max simultaneous kills
		simulkills = 1;

		g_Vars.currentplayer->lastkilltime60_4 = g_Vars.currentplayer->lastkilltime60_3;
		g_Vars.currentplayer->lastkilltime60_3 = g_Vars.currentplayer->lastkilltime60_2;
		g_Vars.currentplayer->lastkilltime60_2 = g_Vars.currentplayer->lastkilltime60;
		g_Vars.currentplayer->lastkilltime60 = time;

		if (g_Vars.currentplayer->lastkilltime60_2 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_2 < 120) {
			simulkills++;

			if (g_Vars.currentplayer->lastkilltime60_3 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_3 < 120) {
				simulkills++;

				if (g_Vars.currentplayer->lastkilltime60_4 != -1 && g_Vars.currentplayer->lastkilltime60 - g_Vars.currentplayer->lastkilltime60_4 < 120) {
					simulkills++;
				}
			}
		}

		if (simulkills > g_Vars.currentplayerstats->maxsimulkills) {
			g_Vars.currentplayerstats->maxsimulkills = simulkills;
		}
	}
}

void mpstatsRecordDeath(s32 aplayernum, s32 vplayernum)
{
	s32 vmpindex = -1;
	struct mpchrconfig *vmpchr = NULL;
	s32 ampindex;
	struct mpchrconfig *ampchr = NULL;
	s32 prevplayernum;
	char text[256];

	if (g_Vars.normmplayerisrunning && g_MpSetup.scenario == MPSCENARIO_POPACAP) {
		pacHandleDeath(aplayernum, vplayernum);
	}

	// Find attacker and victim mpchrs
	if (aplayernum >= 0) {
		ampindex = func0f18d074(aplayernum);

		if (ampindex >= 0) {
			ampchr = MPCHR(ampindex);
		}
	}

	if (vplayernum >= 0) {
		vmpindex = func0f18d074(vplayernum);

		if (vmpindex >= 0) {
			vmpchr = MPCHR(vmpindex);
		}
	}

	if (vplayernum >= 0 && aplayernum == vplayernum) {
		// Player suicide
		if (vmpchr && vmpindex >= 0) {
			vmpchr->numdeaths++;
			vmpchr->killcounts[vmpindex]++;
		}

		if (vplayernum < PLAYERCOUNT()) {
			prevplayernum = g_Vars.currentplayernum;
			setCurrentPlayerNum(vplayernum);
			mpstatsRecordPlayerSuicide();
			setCurrentPlayerNum(prevplayernum);
		}
	} else {
		// Normal kill
		if (vplayernum >= 0) {
			if (vmpchr) {
				vmpchr->numdeaths++;
			}

			if (vplayernum < PLAYERCOUNT()) {
				// Victim was a player
				prevplayernum = g_Vars.currentplayernum;
				setCurrentPlayerNum(vplayernum);

				if (g_Vars.normmplayerisrunning && aplayernum >= 0) {
					// "Killed by %s"
					sprintf(text, "%s %s", _("Killed by"), g_MpAllChrConfigPtrs[aplayernum]->name);
					hudmsgCreate(text, HUDMSGTYPE_DEFAULT);
				}

				mpstatsRecordPlayerDeath();
				setCurrentPlayerNum(prevplayernum);
			}
		}

		if (ampchr && vmpindex >= 0) {
			ampchr->killcounts[vmpindex]++;
		}

		if (aplayernum >= 0 && aplayernum < PLAYERCOUNT()) {
			// Attacker was a player
			prevplayernum = g_Vars.currentplayernum;
			setCurrentPlayerNum(aplayernum);

			if (g_Vars.normmplayerisrunning && vplayernum >= 0) {
				// "Killed %s"
				sprintf(text, "%s %s", _("Killed"), g_MpAllChrConfigPtrs[vplayernum]->name);
				hudmsgCreate(text, HUDMSGTYPE_DEFAULT);
			}

			mpstatsRecordPlayerKill();
			setCurrentPlayerNum(prevplayernum);
		}

		// If someone killed an aibot
		if (g_Vars.normmplayerisrunning
				&& aplayernum >= 0
				&& vplayernum >= PLAYERCOUNT()
				&& aplayernum != vplayernum) {
			g_MpAllChrPtrs[vplayernum]->aibot->lastkilledbyplayernum = aplayernum;
		}
	}

	if (g_Vars.normmplayerisrunning && aplayernum >= 0 && g_MpAllChrPtrs[aplayernum]->aibot) {
		s32 index = mpGetWeaponSlotByWeaponNum(g_MpAllChrPtrs[aplayernum]->aibot->weaponnum);

		if (index >= 0) {
			if (aplayernum == vplayernum) {
				g_MpAllChrPtrs[aplayernum]->aibot->suicidesbygunfunc[index][g_MpAllChrPtrs[aplayernum]->aibot->gunfunc]++;
			} else {
				g_MpAllChrPtrs[aplayernum]->aibot->killsbygunfunc[index][g_MpAllChrPtrs[aplayernum]->aibot->gunfunc]++;
			}
		}
	}

	g_Vars.totalkills++;
}
