#include <ultra64.h>
#include "constants.h"
#include "game/activemenu.h"
#include "game/chraction.h"
#include "game/debug.h"
#include "game/chr.h"
#include "game/ceil.h"
#include "game/game_0b0fd0.h"
#include "game/playermgr.h"
#include "game/mplayer/setup.h"
#include "game/bot.h"
#include "game/botcmd.h"
#include "game/botact.h"
#include "game/botinv.h"
#include "game/mplayer/mplayer.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

/**
 * This table defines the ideal distance ranges for attacking.
 *
 * The first value is the minimum attack distance.
 * The second value is the maximum attack distance.
 * The third value doesn't appear to have any purpose.
 */
f32 g_BotDistConfigs[][3] = {
	{ 0,    120,  10000 }, // BOTDISTCFG_CLOSE
	{ 300,  450,  4500  }, // BOTDISTCFG_PISTOL
	{ 300,  600,  4500  }, // BOTDISTCFG_DEFAULT
	{ 600,  1200, 4500  }, // BOTDISTCFG_SHOOTEXPLOSIVE
	{ 150,  250,  4500  }, // BOTDISTCFG_KAZE
	{ 1000, 2000, 3000  }, // BOTDISTCFG_FARSIGHT
	{ 0,    250,  10000 }, // BOTDISTCFG_FOLLOW
	{ 450,  700,  4500  }, // BOTDISTCFG_THROWEXPLOSIVE
};

void botcmdTickDistMode(struct chrdata *chr)
{
	s32 confignum;
	f32 *limits;
	f32 distance;
	u8 newmode;
	struct aibot *aibot = chr->aibot;
	s32 prevmode = aibot->distmode;
	struct prop *targetprop = NULL;
	bool insight = false;
	f32 minattackdistance;
	f32 maxattackdistance;
	f32 limit3;
	u32 stack;

	if (aibot->config->type == BOTTYPE_KAZE) {
		confignum = BOTDISTCFG_KAZE;
	} else {
		confignum = botinvGetDistConfig(aibot->weaponnum, aibot->gunfunc);
	}

	if (chr->myaction == MA_AIBOTFOLLOW && aibot->followingplayernum >= 0) {
		limits = g_BotDistConfigs[BOTDISTCFG_FOLLOW];
		targetprop = g_MpAllChrPtrs[aibot->followingplayernum]->prop;
		insight = aibot->chrsinsight[aibot->followingplayernum];

		if (chr->target != -1 && (confignum == BOTDISTCFG_CLOSE || confignum == BOTDISTCFG_KAZE)) {
			struct prop *target = chrGetTargetProp(chr);
			f32 xdiff = targetprop->pos.x - target->pos.x;
			f32 ydiff = targetprop->pos.y - target->pos.y;
			f32 zdiff = targetprop->pos.z - target->pos.z;

			if (xdiff * xdiff + ydiff * ydiff + zdiff * zdiff < 500 * 500) {
				limits = g_BotDistConfigs[confignum];
				targetprop = target;
				insight = aibot->targetinsight;
			}
		}
	} else {
		limits = g_BotDistConfigs[confignum];

		if (chr->myaction == MA_AIBOTATTACK && aibot->attackingplayernum >= 0) {
			targetprop = g_MpAllChrPtrs[aibot->attackingplayernum]->prop;
			insight = aibot->chrsinsight[aibot->attackingplayernum];
		} else if (chr->target != -1) {
			targetprop = chrGetTargetProp(chr);
			insight = aibot->targetinsight;
		}
	}

	if (targetprop == NULL) {
		return;
	}

	if (!botHasGround(targetprop->chr)) {
		return;
	}

	distance = chrGetDistanceToCoord(chr, &targetprop->pos);
	minattackdistance = limits[0];
	maxattackdistance = limits[1];
	limit3 = limits[2];

	if (chr->aibot->config->difficulty == BOTDIFF_MEAT) {
		minattackdistance *= 0.35f;
	} else if (chr->aibot->config->difficulty == BOTDIFF_EASY) {
		minattackdistance *= 0.5f;
	}

	if (aibot->distmode == BOTDISTMODE_BACKUP) {
		minattackdistance += 25.0f;
	} else if (aibot->distmode == BOTDISTMODE_ADVANCE || aibot->distmode == BOTDISTMODE_GOTO) {
		maxattackdistance -= 25.0f;
	}

	if (distance < minattackdistance) {
		// Bot is too close to target - move back a bit
		newmode = BOTDISTMODE_BACKUP;
	} else if (distance >= minattackdistance && distance < maxattackdistance) {
		// Bot is a good distance from target for attacking
		newmode = BOTDISTMODE_OK;
	} else if (distance >= maxattackdistance && distance < limit3) {
		// Bot needs to come closer to target
		newmode = BOTDISTMODE_ADVANCE;
	} else {
		// Bot needs to come a lot closer to target
		newmode = BOTDISTMODE_GOTO;
	}

#if VERSION >= VERSION_NTSC_1_0
	if (newmode == BOTDISTMODE_BACKUP && insight && aibot->distoverrideprop == targetprop) {
		// don't unset
	} else {
		aibot->distoverrideprop = NULL;
		aibot->distoverridetimer60 = 0;
	}

	if (newmode == BOTDISTMODE_OK) {
		if (!insight) {
			newmode = BOTDISTMODE_ADVANCE;
		}
	} else if (newmode == BOTDISTMODE_BACKUP) {
		// If the bot is too close to the target, it'll try to back up. But if
		// the target goes out of sight during the back up, the bot needs to
		// advance again, regardless of the distance to the target.
		// When the target returns to sight, force the distance mode to be okay
		// for a random duration before returning to the normal distmode behaviour.
		// This likely prevents them from going into a backup/advance loop as
		// they go around a corner.
		if (!insight) {
			newmode = BOTDISTMODE_ADVANCE;
			aibot->distoverrideprop = targetprop;
			aibot->distoverridetimer60 = TICKS(20) + (rngRandom() % TICKS(120));
		} else if (aibot->distoverrideprop) {
			if (g_Vars.lvupdate60 < aibot->distoverridetimer60) {
				aibot->distoverridetimer60 -= g_Vars.lvupdate60;
				newmode = BOTDISTMODE_OK;
			} else {
				aibot->distoverrideprop = NULL;
				aibot->distoverridetimer60 = 0;
			}
		}
	}
#else
	if (newmode == BOTDISTMODE_OK && !insight) {
		newmode = BOTDISTMODE_ADVANCE;
	}
#endif

	aibot->distmode = newmode;

	if (aibot->distmodettl60 >= 0) {
		aibot->distmodettl60 -= g_Vars.lvupdate60;
	}

	if (newmode != prevmode
			|| (newmode != BOTDISTMODE_OK && (chr->actiontype == ACT_STAND || aibot->distmodettl60 <= 0))) {
		switch (newmode) {
		case BOTDISTMODE_BACKUP:
			chrRunFromPos(chr, GOPOSFLAG_RUN, 10000, &targetprop->pos);
			break;
		case BOTDISTMODE_OK:
			chrTryStop(chr);
			break;
		case BOTDISTMODE_ADVANCE:
			chrGoToProp(chr, targetprop, GOPOSFLAG_RUN);
			break;
		case BOTDISTMODE_GOTO:
			chrGoToProp(chr, targetprop, GOPOSFLAG_RUN);
			break;
		}

		aibot->distmodettl60 = TICKS(60);
	}
}

void botcmdApply(struct chrdata *chr, u32 command)
{
	f32 value;

	switch (command) {
	case AIBOTCMD_ATTACK:
		amOpenPickTarget();
		break;
	case AIBOTCMD_FOLLOW:
		botApplyFollow(chr, g_Vars.currentplayer->prop);
		break;
	case AIBOTCMD_PROTECT:
		botApplyProtect(chr, g_Vars.currentplayer->prop);
		break;
	case AIBOTCMD_DEFEND:
		value = chrGetInverseTheta(g_Vars.currentplayer->prop->chr);
		botApplyDefend(chr, &g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms, value);
		break;
	case AIBOTCMD_HOLD:
		value = chrGetInverseTheta(g_Vars.currentplayer->prop->chr);
		botApplyHold(chr, &g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms, value);
		break;
	default:
		botApplyScenarioCommand(chr, command);
		break;
	}
}
