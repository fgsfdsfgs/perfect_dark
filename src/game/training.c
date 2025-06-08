#include <ultra64.h>
#include "constants.h"
#include "game/atan2f.h"
#include "game/bg.h"
#include "game/bondgun.h"
#include "game/chraction.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/explosions.h"
#include "game/filemgr.h"
#include "game/game_0b0fd0.h"
#include "game/game_1531a0.h"
#include "game/gamefile.h"
#include "game/hudmsg.h"
#include "game/inv.h"
#include "game/lang.h"
#include "game/menu.h"
#include "game/objectives.h"
#include "game/pad.h"
#include "game/padhalllv.h"
#include "game/player.h"
#include "game/prop.h"
#include "game/propobj.h"
#include "game/propsnd.h"
#include "game/shards.h"
#include "game/training.h"
#include "game/trainingmenus.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

#define FRSCRIPTINDEX_WEAPONS 0x00
#define FRSCRIPTINDEX_TARGETS 0x22
#define FRSCRIPTINDEX_HELP    0x71

extern u8 *_firingrangeSegmentRomStart;
extern u8 *_firingrangeSegmentRomEnd;

struct frdata g_FrData;
struct trainingdata g_DtData;
struct trainingdata g_HtData;

u16 *g_FrScriptOffsets = NULL;
u8 g_FrIsValidWeapon = false;
u8 g_FrDataLoaded = false;
u8 g_FrNumSounds = 0;
u8 *g_FrRomData = NULL;

u16 g_FrPads[] = {
	0x00d6, 0x00d7, 0x00d9, 0x00d8, 0x00da, 0x00db, 0x00dc, 0x00dd,
	0x00de, 0x00df, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5,
	0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00f4, 0x00f3,
	0x00f2, 0x00f1, 0x00f0, 0x00ef, 0x00ee, 0x00ed, 0x00ec,
};

bool ciIsTourDone(void)
{
	return gamefileHasFlag(GAMEFILEFLAG_CI_TOUR_DONE);
}

u8 ciGetFiringRangeScore(s32 weaponindex)
{
	// Data at firingrangescores is a u8 array where each score uses 2 bits

#if (VERSION == VERSION_JPN_FINAL) && defined(PLATFORM_N64)
	if (weaponindex == frGetWeaponIndexByWeapon(WEAPON_COMBATKNIFE)) {
		// The knife doesn't exist in the JPN version.
		// Treat it as completed so unlockables still work.
		return 3;
	}
#endif

	return (g_GameFile.firingrangescores[weaponindex >> 2] >> (weaponindex % 4) * 2) & 3;
}

void frSaveScoreIfBest(s32 weaponindex, s32 difficulty)
{
	if (ciGetFiringRangeScore(weaponindex) < difficulty) {
		u32 byteindex = weaponindex >> 2;
		u32 shiftamount = (weaponindex % 4) * 2;
		u32 value = g_GameFile.firingrangescores[byteindex];
		u32 mask = (1 << shiftamount) + (1 << (shiftamount + 1));

		value &= 255 - mask;
		value += (difficulty << shiftamount) & mask;

		g_GameFile.firingrangescores[byteindex] = value;
	}
}

s32 func0f19ca78(u32 weaponnum)
{
	s32 slot = -1;
	s32 i;

	for (i = 0; i <= WEAPON_HORIZONSCANNER; i++) {
		switch (i) {
		case WEAPON_FALCON2:
		case WEAPON_FALCON2_SCOPE:
		case WEAPON_FALCON2_SILENCER:
		case WEAPON_MAGSEC4:
		case WEAPON_MAULER:
		case WEAPON_PHOENIX:
		case WEAPON_DY357MAGNUM:
		case WEAPON_DY357LX:
		case WEAPON_CMP150:
		case WEAPON_CYCLONE:
		case WEAPON_CALLISTO:
		case WEAPON_RCP120:
		case WEAPON_LAPTOPGUN:
		case WEAPON_DRAGON:
		case WEAPON_K7AVENGER:
		case WEAPON_AR34:
		case WEAPON_SUPERDRAGON:
		case WEAPON_SHOTGUN:
		case WEAPON_SNIPERRIFLE:
		case WEAPON_FARSIGHT:
		case WEAPON_CROSSBOW:
		case WEAPON_TRANQUILIZER:
		case WEAPON_REAPER:
		case WEAPON_DEVASTATOR:
		case WEAPON_ROCKETLAUNCHER:
		case WEAPON_SLAYER:
		case WEAPON_COMBATKNIFE:
		case WEAPON_LASER:
		case WEAPON_GRENADE:
		case WEAPON_TIMEDMINE:
		case WEAPON_PROXIMITYMINE:
		case WEAPON_REMOTEMINE:
			slot++;
		}

		if (i == weaponnum) {
			return slot;
		}
	}

	return -1;
}

u8 frIsWeaponFound(s32 weaponnum)
{
	u32 byteindex;

	if (weaponnum <= WEAPON_UNARMED) {
		return true;
	}

#if VERSION >= VERSION_NTSC_1_0
	if (weaponnum < (s32)sizeof(g_GameFile.weaponsfound) * 8) {
		byteindex = weaponnum >> 3;
		return g_GameFile.weaponsfound[byteindex] & (1 << (weaponnum % 8));
	}

	return false;
#else
	byteindex = weaponnum >> 3;
	return g_GameFile.weaponsfound[byteindex] & (1 << (weaponnum % 8));
#endif
}

void frSetWeaponFound(s32 weaponnum)
{
	if (weaponnum < (s32)sizeof(g_GameFile.weaponsfound) * 8) {
		u32 byteindex = weaponnum >> 3;
		u32 value = g_GameFile.weaponsfound[byteindex];

		value |= (1 << (weaponnum % 8));

		g_GameFile.weaponsfound[byteindex] = value;
	}
}

s32 ciIsStageComplete(s32 stageindex)
{
	return g_GameFile.besttimes[stageindex][0]
		|| g_GameFile.besttimes[stageindex][1]
		|| g_GameFile.besttimes[stageindex][2];
}

bool func0f19cbcc(s32 weapon)
{
	if (weapon <= 0 || weapon == WEAPON_PSYCHOSISGUN) {
		return false;
	}

	if (weapon == WEAPON_XRAYSCANNER && ciIsStageComplete(SOLOSTAGEINDEX_INFILTRATION)) {
		return true;
	}

	if (weapon == WEAPON_CLOAKINGDEVICE && ciIsStageComplete(SOLOSTAGEINDEX_CHICAGO)) {
		return true;
	}

	return frIsWeaponFound(weapon);
}

bool frIsWeaponAvailable(s32 weapon)
{
#if (VERSION == VERSION_JPN_FINAL) && defined(PLATFORM_N64)
	if (weapon == WEAPON_COMBATKNIFE) {
		return false;
	}
#endif

	if (weapon < WEAPON_FALCON2 || weapon > WEAPON_REMOTEMINE
			|| weapon == WEAPON_PSYCHOSISGUN
			|| weapon == WEAPON_COMBATBOOST
			|| weapon == WEAPON_NBOMB) {
		return false;
	}

	if (weapon == WEAPON_FALCON2 || weapon == WEAPON_CMP150) {
		return true;
	}

#if VERSION < VERSION_NTSC_1_0
#ifdef DEBUG
	if (debugIsAllTrainingEnabled() && weapon <= WEAPON_XRAYSCANNER) {
		return true;
	}
#endif
#endif

	return frIsWeaponFound(weapon);
}

u32 frGetWeaponIndexByWeapon(u32 weaponnum)
{
	switch (weaponnum) {
	case WEAPON_FALCON2:          return 0;
	case WEAPON_FALCON2_SCOPE:    return 1;
	case WEAPON_FALCON2_SILENCER: return 2;
	case WEAPON_MAGSEC4:          return 3;
	case WEAPON_MAULER:           return 4;
	case WEAPON_PHOENIX:          return 5;
	case WEAPON_DY357MAGNUM:      return 6;
	case WEAPON_DY357LX:          return 7;
	case WEAPON_CMP150:           return 8;
	case WEAPON_CYCLONE:          return 9;
	case WEAPON_CALLISTO:         return 10;
	case WEAPON_RCP120:           return 11;
	case WEAPON_LAPTOPGUN:        return 12;
	case WEAPON_DRAGON:           return 13;
	case WEAPON_K7AVENGER:        return 14;
	case WEAPON_AR34:             return 15;
	case WEAPON_SUPERDRAGON:      return 16;
	case WEAPON_SHOTGUN:          return 17;
	case WEAPON_SNIPERRIFLE:      return 18;
	case WEAPON_FARSIGHT:         return 19;
	case WEAPON_CROSSBOW:         return 20;
	case WEAPON_TRANQUILIZER:     return 21;
	case WEAPON_REAPER:           return 22;
	case WEAPON_DEVASTATOR:       return 23;
	case WEAPON_ROCKETLAUNCHER:   return 24;
	case WEAPON_SLAYER:           return 25;
	case WEAPON_COMBATKNIFE:      return 26;
	case WEAPON_LASER:            return 27;
	case WEAPON_GRENADE:          return 28;
	case WEAPON_TIMEDMINE:        return 29;
	case WEAPON_PROXIMITYMINE:    return 30;
	case WEAPON_REMOTEMINE:       return 31;
	}

	return 0;
}

u32 frGetWeaponScriptIndex(u32 weaponnum)
{
	switch (weaponnum) {
	case WEAPON_FALCON2:          return 1;
	case WEAPON_FALCON2_SCOPE:    return 2;
	case WEAPON_FALCON2_SILENCER: return 3;
	case WEAPON_MAGSEC4:          return 4;
	case WEAPON_MAULER:           return 5;
	case WEAPON_PHOENIX:          return 6;
	case WEAPON_DY357MAGNUM:      return 7;
	case WEAPON_DY357LX:          return 8;
	case WEAPON_CMP150:           return 9;
	case WEAPON_CYCLONE:          return 10;
	case WEAPON_CALLISTO:         return 11;
	case WEAPON_RCP120:           return 12;
	case WEAPON_LAPTOPGUN:        return 13;
	case WEAPON_DRAGON:           return 14;
	case WEAPON_K7AVENGER:        return 15;
	case WEAPON_AR34:             return 16;
	case WEAPON_SUPERDRAGON:      return 17;
	case WEAPON_SHOTGUN:          return 18;
	case WEAPON_SNIPERRIFLE:      return 19;
	case WEAPON_FARSIGHT:         return 20;
	case WEAPON_CROSSBOW:         return 21;
	case WEAPON_TRANQUILIZER:     return 22;
	case WEAPON_REAPER:           return 23;
	case WEAPON_DEVASTATOR:       return 24;
	case WEAPON_ROCKETLAUNCHER:   return 25;
	case WEAPON_SLAYER:           return 26;
	case WEAPON_COMBATKNIFE:      return 27;
	case WEAPON_LASER:            return 28;
	case WEAPON_GRENADE:          return 29;
	case WEAPON_TIMEDMINE:        return 31;
	case WEAPON_PROXIMITYMINE:    return 32;
	case WEAPON_REMOTEMINE:       return 33;
	}

	return 0;
}

s32 frIsClassicWeaponUnlocked(u32 weapon)
{
	switch (weapon) {
	case WEAPON_PP9I:
		return ciGetFiringRangeScore(0) == 3
			&& ciGetFiringRangeScore(1) == 3
			&& ciGetFiringRangeScore(2) == 3;
	case WEAPON_CC13:
		return ciGetFiringRangeScore(3) == 3
			&& ciGetFiringRangeScore(4) == 3
			&& ciGetFiringRangeScore(5) == 3
			&& ciGetFiringRangeScore(6) == 3
			&& ciGetFiringRangeScore(7) == 3;
	case WEAPON_KL01313:
		return ciGetFiringRangeScore(8) == 3
			&& ciGetFiringRangeScore(9) == 3
			&& ciGetFiringRangeScore(10) == 3
			&& ciGetFiringRangeScore(11) == 3;
	case WEAPON_KF7SPECIAL:
		return ciGetFiringRangeScore(12) == 3
			&& ciGetFiringRangeScore(13) == 3
			&& ciGetFiringRangeScore(14) == 3
			&& ciGetFiringRangeScore(15) == 3
			&& ciGetFiringRangeScore(16) == 3;
	case WEAPON_ZZT:
		return ciGetFiringRangeScore(17) == 3
			&& ciGetFiringRangeScore(18) == 3
			&& ciGetFiringRangeScore(24) == 3
			&& ciGetFiringRangeScore(25) == 3;
	case WEAPON_DMC:
#if VERSION >= VERSION_NTSC_1_0
		return ciGetFiringRangeScore(29) == 3
			&& ciGetFiringRangeScore(30) == 3
			&& ciGetFiringRangeScore(31) == 3;
#else
		return ciGetFiringRangeScore(29) == 3
			&& ciGetFiringRangeScore(30) == 3
			&& ciGetFiringRangeScore(32) == 3
			&& ciGetFiringRangeScore(33) == 3
			&& ciGetFiringRangeScore(34) == 3;
#endif
	case WEAPON_AR53:
		return ciGetFiringRangeScore(19) == 3
			&& ciGetFiringRangeScore(20) == 3
			&& ciGetFiringRangeScore(26) == 3
			&& ciGetFiringRangeScore(28) == 3;
	case WEAPON_RCP45:
		return ciGetFiringRangeScore(21) == 3
			&& ciGetFiringRangeScore(22) == 3
			&& ciGetFiringRangeScore(23) == 3;
	}

	return false;
}

s32 frGetSlot(void)
{
	return g_FrData.slot;
}

void frSetSlot(s32 slot)
{
	g_FrData.slot = slot;
}

u32 frGetWeaponBySlot(s32 slot)
{
	s32 index = -1;
	s32 weapon;

	for (weapon = WEAPON_NONE; weapon <= WEAPON_HORIZONSCANNER; weapon++) {
		if (frIsWeaponAvailable(weapon)) {
			index++;
		}

		if (slot == index) {
			return weapon;
		}
	}

	return WEAPON_UNARMED;
}

s32 frGetNumWeaponsAvailable(void)
{
	s32 count = 0;
	s32 i;

	for (i = WEAPON_UNARMED; i <= WEAPON_HORIZONSCANNER; i++) {
		if (frIsWeaponAvailable(i)) {
			count++;
		}
	}

	return count;
}

void frInitLighting(void)
{
	if (g_FrData.donelighting == false) {
		s32 roomnum;

		for (roomnum = ROOM_DISH_0007; roomnum <= ROOM_DISH_0009; roomnum++) {
			roomSetLightOp(roomnum, LIGHTOP_TRANSITION, 50, 100, TICKS(32));
		}

		roomSetLightOp(ROOM_DISH_FIRINGRANGE, LIGHTOP_TRANSITION, 25, 100, TICKS(32));

		g_FrData.donelighting = true;

		sndStart(var80095200, SFX_FR_LIGHTSON, NULL, -1, -1, -1, -1, -1);
	}

	chrSetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
}

void frRestoreLighting(void)
{
	if (g_FrData.donelighting == true) {
		s32 roomnum;

		for (roomnum = ROOM_DISH_0007; roomnum <= ROOM_DISH_0009; roomnum++) {
			roomSetLightOp(roomnum, LIGHTOP_TRANSITION, 100, 50, TICKS(8));
		}

		roomSetLightOp(ROOM_DISH_FIRINGRANGE, LIGHTOP_TRANSITION, 100, 25, TICKS(8));

		g_FrData.donelighting = false;

		sndStart(var80095200, SFX_FR_LIGHTSOFF, NULL, -1, -1, -1, -1, -1);
	}
}

void frReset(void)
{
	s32 i;

#if VERSION >= VERSION_NTSC_1_0
	g_FrScriptOffsets = NULL;
#endif

	g_FrDataLoaded = false;
	g_FrIsValidWeapon = false;
	g_FrRomData = NULL;

	g_FrData.helpscriptindex = 0;
	g_FrData.helpscriptoffset = 0;
	g_FrData.helpscriptenabled = false;
	g_FrData.helpscriptsleep = 0;

#if VERSION >= VERSION_NTSC_1_0
	g_FrData.menucountdown = 0;
	g_FrData.maxactivetargets = 0;

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		g_FrData.targets[i].prop = NULL;
		g_FrData.targets[i].inuse = false;
	}

	g_FrNumSounds = 0;
#endif
}

void *frLoadRomData(u32 len)
{
	g_FrRomData = mempAlloc(ALIGN16(len), MEMPOOL_STAGE);

	if (g_FrRomData) {
		return dmaExecWithAutoAlign(g_FrRomData, (romptr_t) REF_SEG _firingrangeSegmentRomStart, len);
	}

	return NULL;
}

void frSetDifficulty(s32 difficulty)
{
	if (difficulty < FRDIFFICULTY_BRONZE) {
		difficulty = FRDIFFICULTY_BRONZE;
	}

	if (difficulty > FRDIFFICULTY_GOLD) {
		difficulty = FRDIFFICULTY_GOLD;
	}

	g_FrData.difficulty = difficulty;
}

u32 frGetDifficulty(void)
{
	return g_FrData.difficulty;
}

void frInitDefaults(void)
{
	s32 i;
	struct pad pad;

	g_FrNumSounds = 0;

	padUnpack(g_FrPads[0], PADFIELD_POS, &pad);

	g_FrData.maxactivetargets = 0;
	g_FrData.goalscore = 0;
	g_FrData.timelimit = 200;
	g_FrData.ammolimit = 255;
	g_FrData.sdgrenadelimit = 255;
	g_FrData.goalaccuracy = 0;
	g_FrData.goaltargets = 255;
	g_FrData.speed = 1;

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		g_FrData.targets[i].dstpos.x = pad.pos.x;
		g_FrData.targets[i].dstpos.y = pad.pos.y;
		g_FrData.targets[i].dstpos.z = pad.pos.z;

#if VERSION >= VERSION_NTSC_1_0
		g_FrData.targets[i].dstpos.z += 6.0f * i;
#endif

		g_FrData.targets[i].inuse = false;
		g_FrData.targets[i].rotateoncloak = false;
		g_FrData.targets[i].destroyed = false;
		g_FrData.targets[i].damage = 0;
		g_FrData.targets[i].scriptoffset = 0;

		g_FrData.targets[i].travelspeed = 0;
		g_FrData.targets[i].scriptsleep = SECSTOTIME60(255);
		g_FrData.targets[i].timeuntilrotate = 0;

		g_FrData.targets[i].rotating = false;
		g_FrData.targets[i].rotatespeed = 0;
		g_FrData.targets[i].angle = 0;
		g_FrData.targets[i].rotatetoangle = 0;
		g_FrData.targets[i].silent = 0;
		g_FrData.targets[i].donestopsound = false;
		g_FrData.targets[i].travelling = false;
		g_FrData.targets[i].invincibletimer = 0;
		g_FrData.targets[i].frpadnum = -1;

		wallhitsFreeByProp(g_FrData.targets[i].prop, 0);
		wallhitsFreeByProp(g_FrData.targets[i].prop, 1);
	}

	g_FrData.timetaken = TICKS(-240);
	g_FrData.score = 0;
	g_FrData.numtargets = 0;
	g_FrData.targetsdestroyed = 0;
	g_FrData.menucountdown = 0;
	g_FrData.padindexoffset = 0;
	g_FrData.feedbackzone = 0;
	g_FrData.feedbackttl = 0;
	g_FrData.failreason = 0;
	g_FrData.numshots = 0;
	g_FrData.numhitsring3 = 0;
	g_FrData.numhitsring2 = 0;
	g_FrData.numhitsring1 = 0;
	g_FrData.numhitsbullseye = 0;
	g_FrData.helpscriptindex = 0;
	g_FrData.helpscriptoffset = 0;
	g_FrData.helpscriptenabled = false;
	g_FrData.helpscriptsleep = 0;
	g_FrData.proxyendtimer = 0;
	g_FrData.donealarm = false;
	g_FrData.ammohasgrace = true;
	g_FrData.ammoextra = -1;
}

struct frdata *frGetData(void)
{
	return &g_FrData;
}

u32 frResolveFrPad(u32 arg0)
{
	switch (arg0) {
	case 31: return rngRandom() % 9 + 4;  // 4 - 12
	case 32: return rngRandom() % 9 + 13; // 13 - 21
	case 33: return rngRandom() % 9 + 22; // 22 - 30
	case 34: return rngRandom() % 27 + 4; // 4 - 30
	}

	return g_FrData.padindexoffset + arg0;
}

bool frIsDifficulty(u32 flags)
{
	if (g_FrData.difficulty == FRDIFFICULTY_BRONZE) {
		if ((flags & FRTARGETFLAG_BRONZE) == 0) {
			return false;
		}
	} else if (g_FrData.difficulty == FRDIFFICULTY_SILVER) {
		if ((flags & FRTARGETFLAG_SILVER) == 0) {
			return false;
		}
	} else if (g_FrData.difficulty == FRDIFFICULTY_GOLD) {
		if ((flags & FRTARGETFLAG_GOLD) == 0) {
			return false;
		}
	}

	return true;
}

void frExecuteWeaponScript(s32 scriptindex)
{
	s32 offset = 0;

	if (scriptindex >= FRSCRIPTINDEX_WEAPONS && scriptindex < FRSCRIPTINDEX_TARGETS) {
		u8 *script = &g_FrRomData[g_FrScriptOffsets[scriptindex]];
		u8 mult = 1;
		u32 stack[5];
		s32 start;
		s32 capacity;
		s32 index;
		u8 *subscript;
		s32 i;
		s32 val;

		while (script[offset] != FRCMD_END) {
			switch (script[offset]) {
			case FRCMD_SETPADINDEXOFFSET:
				g_FrData.padindexoffset = script[offset + 1];
				offset += 2;
				break;
			case FRCMD_ADDTARGET:
				if (!frIsDifficulty(script[offset + 4])) {
					offset += 5;
					break;
				}
				if (g_FrData.numtargets < ARRAYCOUNT(g_FrData.targets)) {
					g_FrData.targets[g_FrData.numtargets].frpadindex = frResolveFrPad(script[offset + 1]);
					g_FrData.targets[g_FrData.numtargets].scriptindex = script[offset + 2];
					g_FrData.targets[g_FrData.numtargets].maxdamage = script[offset + 3];
					g_FrData.targets[g_FrData.numtargets].inuse = true;
					g_FrData.targets[g_FrData.numtargets].flags = script[offset + 4];

					if (g_FrData.targets[g_FrData.numtargets].flags & FRTARGETFLAG_ROTATEONCLOAK) {
						g_FrData.targets[g_FrData.numtargets].rotateoncloak = true;
					}

					if (g_FrData.targets[g_FrData.numtargets].flags & FRTARGETFLAG_ONEHITEXPLODE) {
						g_FrData.targets[g_FrData.numtargets].maxdamage = 1;
					}

					g_FrData.numtargets++;
				}
				offset += 5;
				break;
			case FRCMD_SETMAXACTIVETARGETS:
				g_FrData.maxactivetargets = script[g_FrData.difficulty + offset + 1];
				offset += 4;
				break;
			case FRCMD_SETSCOREMULTIPLIER:
				if (script[g_FrData.difficulty + offset + 1] > 0) {
					mult = script[g_FrData.difficulty + offset + 1];
				} else {
					mult = 1;
				}
				offset += 4;
				break;
			case FRCMD_SETGOALSCORE:
				g_FrData.goalscore = script[g_FrData.difficulty + offset + 1] * mult;
				offset += 4;
				break;
			case FRCMD_SETTIMELIMIT:
				g_FrData.timelimit = script[g_FrData.difficulty + offset + 1];
				if (g_FrData.timelimit == 255) {
					g_FrData.timelimit = 120;
				}
				offset += 4;
				break;
			case FRCMD_SETAMMOLIMIT:
				capacity = bgunGetCapacityByAmmotype(bgunGetAmmoTypeForWeapon(frGetWeaponBySlot(g_FrData.slot), 0));
				g_FrData.ammolimit = script[g_FrData.difficulty + offset + 1];

				if (g_FrData.ammolimit != 255) {
					if (g_FrData.ammolimit > capacity) {
						g_FrData.ammoextra = g_FrData.ammolimit - capacity;
					} else {
						g_FrData.ammoextra = 0;
					}
				}

				offset += 4;
				break;
			case FRCMD_SETGRENADELIMIT:
				capacity = bgunGetCapacityByAmmotype(AMMOTYPE_DEVASTATOR);
				g_FrData.sdgrenadelimit = script[g_FrData.difficulty + offset + 1];

				if (g_FrData.sdgrenadelimit != 255) {
					if (g_FrData.sdgrenadelimit > capacity) {
						g_FrData.sdgrenadeextra = g_FrData.sdgrenadelimit - capacity;
					} else {
						g_FrData.sdgrenadeextra = 0;
					}
				}

				offset += 4;
				break;
			case FRCMD_SETEXTRASPEED:
				g_FrData.speed = script[g_FrData.difficulty + offset + 1] * 0.1f + 1.0f;
				offset += 4;
				break;
			case FRCMD_SETGOALACCURACY:
				g_FrData.goalaccuracy = script[g_FrData.difficulty + offset + 1];
				offset += 4;
				break;
			case FRCMD_SETGOALTARGETS:
				g_FrData.goaltargets = script[g_FrData.difficulty + offset + 1];
				offset += 4;
				break;
			case FRCMD_SETHELPSCRIPT:
				g_FrData.helpscriptindex = script[offset + 1];
				g_FrData.helpscriptenabled = true;
				index = FRSCRIPTINDEX_HELP + g_FrData.helpscriptindex;
				if (&g_FrRomData[g_FrScriptOffsets[index]]);
				subscript = &g_FrRomData[g_FrScriptOffsets[index]];
				offset += 2;

				if (g_FrData.difficulty == FRDIFFICULTY_BRONZE) {
					start = FRCMD_IFBRONZE;
				} else if (g_FrData.difficulty == FRDIFFICULTY_SILVER) {
					start = FRCMD_IFSILVER;
				} else if (g_FrData.difficulty == FRDIFFICULTY_GOLD) {
					start = FRCMD_IFGOLD;
				}

				g_FrData.helpscriptoffset = 0;
				i = 0;

				while (1) {
					g_FrData.helpscriptoffset++;

					if (subscript[i] == start) {
						i++;

						val = subscript[i];

						if (val >= FRCMD_IFBRONZE) {
							g_FrData.helpscriptoffset++;
							val = subscript[i + 1];
						}

						if (val >= FRCMD_IFBRONZE) {
							g_FrData.helpscriptoffset++;
						}
						break;
					}

					i++;
				}
				break;
			}
		}
	}
}

void frSetTargetProps(void)
{
	s32 i;
	u32 targets[] = {
		0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x11,
		0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	};

	for (i = 0; i < ARRAYCOUNT(targets); i++) {
		struct defaultobj *obj = objFindByTagId(targets[i]);

		if (obj) {
			g_FrData.targets[i].prop = obj->prop;
			obj->flags2 |= OBJFLAG2_INVISIBLE;
		}
	}
}

s32 g_FrWeaponNum = WEAPON_UNARMED;

bool frTargetIsAtScriptStart(s32 targetnum)
{
	return g_FrData.targets[targetnum].scriptoffset == 0;
}

/**
 * 0 => "FIRING\n Press Z Button to fire gun.\n"
 * 1 => "AUTO RELOAD\n Release Z Button when out of ammo.\n"
 * 2 => "MANUAL RELOAD\n Press B Button to reload early if magazine not full.\n"
 * 3 => "Aiming: Hold down R Button to enter Aim mode.\n"
 * 4 => "Use Control Stick to move aiming sight.\n"
 * 5 => "AUTO FIRE\n Hold Z Button to repeatedly fire automatically.\n"
 * 6 => "ALTER AIM\n Press Up C Button or Down C Button to move sight up/down.\n"
 * 7 => "ZOOM\n Hold R Button to enter Zoom mode.\n"
 * 8 => "FAST FIRE\n Press Z Button quickly to fire faster.\n"
 */
char *frGetInstructionalText(u32 index)
{
	u16 textid = (u16)(g_FrRomData[index * 2] << 8) | g_FrRomData[index * 2 + 1];

	return langGet(textid);
}

void frExecuteHelpScript(void)
{
	if (!g_FrData.helpscriptenabled || g_Vars.lvupdate240 == 0) {
		return;
	}

	if (g_FrData.helpscriptsleep == 0) {
		s32 index = FRSCRIPTINDEX_HELP + g_FrData.helpscriptindex;
		u8 *script = &g_FrRomData[g_FrScriptOffsets[index]];
		u32 offset = g_FrData.helpscriptoffset;

		switch (script[offset]) {
		case FRCMD_END:
		case FRCMD_IFBRONZE:
		case FRCMD_IFSILVER:
		case FRCMD_IFGOLD:
			g_FrData.helpscriptenabled = false;
			break;
		case FRCMD_HUDMSG:
			hudmsgCreate(frGetInstructionalText(script[offset + 1]), HUDMSGTYPE_TRAINING);
			g_FrData.helpscriptoffset += 2;
			break;
		case FRCMD_HELPWAITSECONDS:
#if PAL
			g_FrData.helpscriptsleep = script[offset + 1] * 50;
#else
			g_FrData.helpscriptsleep = SECSTOTIME60(script[offset + 1]);
#endif
			g_FrData.helpscriptoffset += 2;
			break;
		case FRCMD_WAITUNTILSHOOT:
			if (g_FrData.numshots) {
				g_FrData.helpscriptoffset++;
			}
			break;
		}
	} else {
		g_FrData.helpscriptsleep -= g_Vars.lvupdate60;

		if (g_FrData.helpscriptsleep <= 0) {
			g_FrData.helpscriptsleep = 0;
		}
	}
}

bool frExecuteTargetScript(s32 targetnum)
{
	if (g_FrData.targets[targetnum].inuse) {
		s32 index = FRSCRIPTINDEX_TARGETS + g_FrData.targets[targetnum].scriptindex;
		u8 *script = &g_FrRomData[g_FrScriptOffsets[index]];
		s32 offset = g_FrData.targets[targetnum].scriptoffset;
		struct pad pad;
		s32 frpadnum;

		switch (script[offset]) {
		case FRCMD_END:
			g_FrData.targets[targetnum].scriptenabled = true;
			g_FrData.targets[targetnum].scriptsleep = 255 * 60;
			return true;
		case FRCMD_GOTOPAD:
			frpadnum = frResolveFrPad(script[offset + 1]);

			if (frpadnum == g_FrData.targets[targetnum].frpadnum) {
				g_FrData.targets[targetnum].scriptoffset += 4;
				return false;
			}

			g_FrData.targets[targetnum].frpadnum = frpadnum;

			padUnpack(g_FrPads[frpadnum], PADFIELD_POS, &pad);

			g_FrData.targets[targetnum].dstpos.x = pad.pos.x;
			g_FrData.targets[targetnum].dstpos.y = pad.pos.y;
			g_FrData.targets[targetnum].dstpos.z = pad.pos.z;

#if VERSION >= VERSION_NTSC_1_0
			g_FrData.targets[targetnum].dstpos.z += 6.0f * targetnum;
#endif

			if (script[offset + 2] == 0xff) {
				g_FrData.targets[targetnum].travelspeed = -1;
				g_FrData.targets[targetnum].travelling = true;
			} else {
				if (g_FrNumSounds < 3) {
					g_FrNumSounds++;
					psCreate(NULL, g_FrData.targets[targetnum].prop, SFX_FR_CONVEYER, -1,
							-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
				}

				g_FrData.targets[targetnum].travelspeed = script[offset + 2] / 60.0f * g_FrData.speed;
				g_FrData.targets[targetnum].travelling = true;
			}

			g_FrData.targets[targetnum].scriptsleep = script[offset + 3] * TICKS(60);
			g_FrData.targets[targetnum].donestopsound = false;
			g_FrData.targets[targetnum].scriptoffset += 4;
			return true;
		case FRCMD_RESTART:
			g_FrData.targets[targetnum].scriptoffset = 0;
			return true;
		case FRCMD_WAITSECONDS:
			g_FrData.targets[targetnum].scriptenabled = true;
			g_FrData.targets[targetnum].scriptsleep = script[offset + 1] * TICKS(60);
			g_FrData.targets[targetnum].scriptoffset += 2;
			return true;
		case FRCMD_ROTATE:
			if (g_FrData.targets[targetnum].rotateoncloak == false) {
				f32 angles[4];
				angles[0] = DEG2RAD(-90);
				angles[1] = DEG2RAD(-180);
				angles[2] = DEG2RAD(90);
				angles[3] = DEG2RAD(180);

				g_FrData.targets[targetnum].rotatetoangle = g_FrData.targets[targetnum].angle + angles[script[offset + 1]];
				g_FrData.targets[targetnum].rotatespeed = angles[script[offset + 1]] / (script[offset + 2] * 15);
				g_FrData.targets[targetnum].rotating = true;
				g_FrData.targets[targetnum].scriptenabled = false;
			}

			if (1);
			g_FrData.targets[targetnum].scriptoffset += 3;
			return true;
		}
	}

	return true;
}

void frHideAllTargets(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		struct prop *prop = g_FrData.targets[i].prop;
		struct defaultobj *target = prop->obj;

		target->flags2 |= OBJFLAG2_INVISIBLE;

		psStopSound(prop, PSTYPE_GENERAL, 0xffff);
	}
}

void frInitTargets(void)
{
	s32 count = 0;
	s32 i;
	struct prop *prop;
	struct defaultobj *obj;
	struct pad pad;
	struct coord pos;
	Mtxf sp144;
	f32 sp108[3][3];

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		prop = g_FrData.targets[i].prop;

#if VERSION >= VERSION_NTSC_1_0
		if (prop)
#endif
		{
			obj = prop->obj;

			objFree(obj, false, true);

			obj->damage = 0;
			prop->timetoregen = 0;

			if (g_FrData.targets[i].inuse) {
				g_FrData.targets[i].scriptenabled = false;
				g_FrData.targets[i].destroyed = false;

				if (count < g_FrData.maxactivetargets) {
					obj->flags2 &= ~OBJFLAG2_INVISIBLE;
					g_FrData.targets[i].active = true;
				} else {
					obj->flags2 |= OBJFLAG2_INVISIBLE;
					g_FrData.targets[i].active = false;
				}

				padUnpack(g_FrPads[g_FrData.targets[i].frpadindex], PADFIELD_POS, &pad);

				pos.f[0] = pad.pos.f[0];
				pos.f[1] = pad.pos.f[1];
#if VERSION >= VERSION_NTSC_1_0
				pos.f[2] = pad.pos.f[2] + 6.0f * i;
#else
				pos.f[2] = pad.pos.f[2];
#endif

				frExecuteTargetScript(i);

				if (g_FrData.targets[i].travelspeed == -1) {
					pos.x = g_FrData.targets[i].dstpos.x;
					pos.y = g_FrData.targets[i].dstpos.y;
					pos.z = g_FrData.targets[i].dstpos.z;
				}

				count++;
			} else {
				obj->flags2 |= OBJFLAG2_INVISIBLE;
			}

			if (obj->flags2 & OBJFLAG2_INVISIBLE) {
#if VERSION < VERSION_NTSC_1_0
				padUnpack(g_FrPads[g_FrData.targets[i].frpadindex], PADFIELD_POS, &pad);

				pos.x = 0.0f;
				pos.y = 5000.0f;
				pos.z = 0.0f;
#else
				pos.x = 0.0f;
				pos.y = 5000.0f;
				pos.z = 6.0f * i;
#endif
			}

			if (g_FrData.targets[i].flags & FRTARGETFLAG_SPAWNFACINGAWAY) {
				mtx4LoadYRotation(0.0f, &sp144);
				g_FrData.targets[i].angle = M_PI;
			} else {
				mtx4LoadYRotation(M_PI, &sp144);
			}

			mtx00015f04(obj->model->scale, &sp144);
			mtx4ToMtx3(&sp144, sp108);
			mtx3Copy(sp108, obj->realrot);

			prop->pos.x = pos.x;
			prop->pos.y = pos.y;
			prop->pos.z = pos.z;

			func0f069c70(obj, true, false);
		}
	}
}

void frCloseAndLockDoor(void)
{
	struct defaultobj *obj = objFindByTagId(0x91);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *)obj;
		door->keyflags |= 0x40;
		doorsRequestMode(door, DOORMODE_CLOSING);
	}
}

void frUnlockDoor(void)
{
	struct defaultobj *obj = objFindByTagId(0x91);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *)obj;
		door->keyflags &= ~0x40;
	}
}

void frLoadData(void)
{
	if (!g_FrDataLoaded) {
		s32 len = (s32) REF_SEG _firingrangeSegmentRomEnd - (s32) REF_SEG _firingrangeSegmentRomStart;
		s32 index = 0;
		u32 i;
		u32 numscripts = 1;
		s32 size;

		if (index);

		g_FrDataLoaded = true;

		frLoadRomData(len);

		for (i = 0x12; i < len; i++) {
			if (g_FrRomData[i] == FRCMD_START) {
				numscripts++;
			}
		}

		size = numscripts * sizeof(*g_FrScriptOffsets);
		g_FrScriptOffsets = mempAlloc(ALIGN16(size), MEMPOOL_STAGE);

		if (numscripts < 0);

		if (g_FrScriptOffsets) {
			for (i = 0x12; i < len; i++) {
				if (g_FrRomData[i] == FRCMD_START) {
					g_FrScriptOffsets[index] = i + 1;
					index++;
				}
			}
		}

		frSetTargetProps();

		g_FrData.slot = 0;
		g_FrData.difficulty = FRDIFFICULTY_BRONZE;
		g_FrData.donelighting = false;
	}
}

u32 frInitAmmo(s32 weaponnum)
{
	u32 scriptindex;
	u32 ammotype = bgunGetAmmoTypeForWeapon(weaponnum, 0);
	u32 capacity = bgunGetCapacityByAmmotype(ammotype);

	frInitDefaults();
	scriptindex = frGetWeaponScriptIndex(weaponnum);
	frExecuteWeaponScript(scriptindex);

	if (g_FrData.ammolimit == 255) {
		bgunSetAmmoQuantity(ammotype, capacity);
	} else {
		bgunSetAmmoQuantity(ammotype, g_FrData.ammolimit);
	}

	if (weaponnum == WEAPON_SUPERDRAGON) {
		if (g_FrData.sdgrenadelimit == 255) {
			bgunSetAmmoQuantity(AMMOTYPE_DEVASTATOR, capacity);
		} else {
			bgunSetAmmoQuantity(AMMOTYPE_DEVASTATOR, g_FrData.sdgrenadelimit);
		}
	}

	return scriptindex;
}

void frBeginSession(s32 weapon)
{
	s32 i;
	struct defaultobj *obj = objFindByTagId(0x7f); // computer

	if (obj) {
		obj->flags |= OBJFLAG_CANNOT_ACTIVATE;
	}

	frCloseAndLockDoor();

	for (i = 0; i < 2; i++) {
		if (g_Vars.currentplayer->gunctrl.ammotypes[i] >= 0) {
			g_Vars.currentplayer->hands[0].loadedammo[i] = 0;
			g_Vars.currentplayer->hands[1].loadedammo[i] = 0;
		}
	}

	g_FrIsValidWeapon = frInitAmmo(weapon) == 0 ? false : true;
	frInitTargets();
	bgunSetPassiveMode(false);
}

char *frGetWeaponDescription(void)
{
	u32 weapon = frGetWeaponBySlot(g_FrData.slot);

	switch (weapon) {
#if VERSION >= VERSION_PAL_BETA
	case WEAPON_FALCON2:          return langGet(L_DISH_283);
	case WEAPON_FALCON2_SCOPE:    return langGet(L_DISH_284);
	case WEAPON_FALCON2_SILENCER: return langGet(L_DISH_285);
	case WEAPON_MAGSEC4:          return langGet(L_DISH_286);
	case WEAPON_MAULER:           return langGet(L_DISH_287);
	case WEAPON_PHOENIX:          return langGet(L_DISH_288);
	case WEAPON_DY357MAGNUM:      return langGet(L_DISH_289);
	case WEAPON_DY357LX:          return langGet(L_DISH_290);
	case WEAPON_CMP150:           return langGet(L_DISH_291);
	case WEAPON_CYCLONE:          return langGet(L_DISH_292);
	case WEAPON_CALLISTO:         return langGet(L_DISH_293);
	case WEAPON_RCP120:           return langGet(L_DISH_294);
	case WEAPON_LAPTOPGUN:        return langGet(L_DISH_295);
	case WEAPON_DRAGON:           return langGet(L_DISH_296);
	case WEAPON_K7AVENGER:        return langGet(L_DISH_297);
	case WEAPON_AR34:             return langGet(L_DISH_298);
	case WEAPON_SUPERDRAGON:      return langGet(L_DISH_299);
	case WEAPON_SHOTGUN:          return langGet(L_DISH_300);
	case WEAPON_SNIPERRIFLE:      return langGet(L_DISH_301);
	case WEAPON_FARSIGHT:         return langGet(L_DISH_302);
	case WEAPON_CROSSBOW:         return langGet(L_DISH_303);
	case WEAPON_TRANQUILIZER:     return langGet(L_DISH_304);
	case WEAPON_REAPER:           return langGet(L_DISH_305);
	case WEAPON_DEVASTATOR:       return langGet(L_DISH_306);
	case WEAPON_ROCKETLAUNCHER:   return langGet(L_DISH_307);
	case WEAPON_SLAYER:           return langGet(L_DISH_308);
	case WEAPON_COMBATKNIFE:      return langGet(L_DISH_309);
	case WEAPON_LASER:            return langGet(L_DISH_310);
	case WEAPON_GRENADE:          return langGet(L_DISH_311);
	case WEAPON_NBOMB:            return langGet(L_DISH_312);
	case WEAPON_TIMEDMINE:        return langGet(L_DISH_313);
	case WEAPON_PROXIMITYMINE:    return langGet(L_DISH_314);
	case WEAPON_REMOTEMINE:       return langGet(L_DISH_315);
#else
	case WEAPON_FALCON2:          return _("The Falcon 2 is a very accurate handgun, so any error in this test is your own. The secondary mode is a pistol-whip and is, therefore, useless in a firing range.\n");
	case WEAPON_FALCON2_SCOPE:    return _("To make better use of the accuracy of the Falcon 2, a scope has been attached to the pistol. As with the unmodified Falcon 2, the pistol-whip secondary mode is useless during the firing range test.\n");
	case WEAPON_FALCON2_SILENCER: return _("The silenced version of the Falcon 2 is an excellent stealth weapon, designed to give you the advantage of surprise over your opponents. Test your accuracy to ensure you never waste your chances.\n");
	case WEAPON_MAGSEC4:          return _("The MagSec 4 has excellent shot power at the cost of accuracy, especially when used in the 3-round burst secondary mode. The only serious drawback to the weapon is a limited magazine size.\n");
	case WEAPON_MAULER:           return _("This is a typical Skedar weapon, brutal and powerful. By sacrificing some of the magazine, the shot can be charged up to give a devastating blast. The large ammo capacity is a bonus.\n");
	case WEAPON_PHOENIX:          return _("This versatile pistol gives two different delivery systems for the rounds it fires: a standard shot or an explosive round. Maian engineers managed to do this without compromising the energy usage of the weapon, though the fire rate is reduced.\n");
	case WEAPON_DY357MAGNUM:      return _("The key to the DY357 is in knowing when to reload. A six-round magazine means that you must always be alert in a firefight. The weight of the handgun can be useful if you have to club someone with it.\n");
	case WEAPON_DY357LX:          return _("Trent Easton is always looking for that extra edge, and this gun is no exception. The bullets are as special as the rest of the gun; they are designed to shatter inside opponents to take them down quickly.\n");
	case WEAPON_CMP150:           return _("A reliable and effective submachine gun, it is not difficult to see why this is the best-selling dataDyne weapon of the past two years even if the secondary mode is not taken into account. The target designate and lock-on system is excellent, and you should familiarize yourself with the complete operation of the weapon.\n");
	case WEAPON_CYCLONE:          return _("The Cyclone submachine gun was specifically designed NOT to be accurate, though it is worth your time practicing. It can put a lot of bullets out in a short time, however, and is an excellent suppression weapon because of it.\n");
	case WEAPON_CALLISTO:         return _("Maian flexibility in design has produced this submachine gun with an interesting secondary mode - a high-velocity bullet that can penetrate objects easily. The fire rate, as with the Phoenix, is reduced during the secondary mode; this is to prevent damage to the firing mechanism and barrel.\n");
	case WEAPON_RCP120:           return _("The RC-P120 fires a special mineral bullet that, coupled with a high fire rate and a huge magazine, makes this a perfect tool to be used against the Skedar, should the need arise. The bullets can be used to fuel a prototype Cloaking Device - the training has been altered to reflect this particular characteristic.\n");
	case WEAPON_LAPTOPGUN:        return _("Not only can the gun fold up to resemble a laptop PC, but it can be deployed as a sentry gun in secondary mode to cover an escape route or protect a location.\n");
	case WEAPON_DRAGON:           return _("This is the model that dataDyne successfully submitted to the U.S. military, though it has yet to be widely adopted. In keeping with the tendency of dataDyne to give people nasty surprises, the basic model assault rifle carries a proximity mine below the barrel. Using the secondary mode rather obviously means you have to throw the weapon away.\n");
	case WEAPON_K7AVENGER:        return _("As far as we can ascertain, the secondary mode of the weapon seems to be a threat identifier, targetting mines, hidden explosive devices (such as the Dragon in secondary mode), and automatic guns. Though a powerful gun, the assault rifle's magazine is perhaps too small. \n");
	case WEAPON_AR34:             return _("The Institute's first attempt at a support weapon, the AR34 is a fairly basic assault rifle. The secondary mode is a permanent zoom. Testing is weighted towards accuracy training.\n");
	case WEAPON_SUPERDRAGON:      return _("The heavier variant of the Dragon, with the proximity mine removed and replaced by a small grenade launcher. Adopted by the U.S. military as a squad heavy support weapon - it is not hard to see why. Use the tests to get used to the grenade trajectory.\n");
	case WEAPON_SHOTGUN:          return _("Subtlety is not an option here. There are two modes, single or double blast. The magazine is quite large for a shotgun, but be aware of the long reload times and plan your movements accordingly.\n");
	case WEAPON_SNIPERRIFLE:      return _("A finely engineered, silenced sniper rifle with a high-powered zoom. The only part of the operation that can interfere with the accuracy of the weapon is the sniper. Make sure you use the secondary mode to crouch down and steady your aim.\n");
	case WEAPON_FARSIGHT:         return _("Even though the engineering techniques are beyond us, we can still appreciate the effects of the FarSight rifle. The shot is almost unstoppable, and the scope can penetrate walls to a great depth, locking on to targets if need be.\n");
	case WEAPON_CROSSBOW:         return _("The primary function of the crossbow is a nonlethal drugged shot, while the secondary is an instant kill. It is a stealth weapon first and foremost, so the reloading can be time-consuming and inconvenient in a firefight.\n");
	case WEAPON_TRANQUILIZER:     return _("The tranquilizer gun is a common design, found in many medical laboratories and hospitals worldwide. It can be switched from the tranquilizer pellets to a short-range lethal dose, which uses up a lot more of the reservoir of sedative.\n");
	case WEAPON_REAPER:           return _("Try to control this weapon as best you can. It was designed for a far stronger user than a human, that much is certain. Kneeling down may help steady your aim, but not refine it. If any opponents do make it past the hail of fire, then the barrels can be used as a grinder to inflict damage.\n");
	case WEAPON_DEVASTATOR:       return _("A recent dataDyne product, with interesting technology inside it. The secondary mode activates a magnetic field around the bomb, providing adhesion for a limited amount of time. When a certain amount of time has passed, the field is reversed and the bomb falls from the impact point and explodes.\n");
	case WEAPON_ROCKETLAUNCHER:   return _("A compact, reuseable missile launcher - reloads after each shot. The secondary mode uses a variant of the lock-on system from the CMP150 submachine gun, with a single designated target. The missile travels at a slower speed as it tracks the target, so plan for this and time your shots carefully.\n");
	case WEAPON_SLAYER:           return _("This Skedar handheld missile launcher can fire either an unguided rocket or a user-controlled remote rocket. Presumably this gave the Skedar some perverse satisfaction when the warhead camera closed on the target, but a Carrington Institute agent is beyond such things. The guided rocket speeds up automatically, but can be slowed down for greater maneuverability. Detonation occurs on contact or when the trigger is pressed.\n");
	case WEAPON_COMBATKNIFE:      return _("A finely tempered fighting knife, which is also balanced for throwing. Practice is essential to get accustomed to the range and trajectory of the thrown blade.\n");
	case WEAPON_LASER:            return _("The experimental weapons department has come up with a small but effective wrist-mounted laser with two beam settings. The primary mode fires a blast out to a considerable range, while the secondary mode provides a continual beam for as long as the trigger is pressed.\n");
	case WEAPON_GRENADE:          return _("The fragmentation anti-personnel grenade has four seconds from activation to detonation on the basic fuse. For your entertainment and delight, we have included a secondary mode - proximity pinball. The grenade will bounce around until the charge wears off or the proximity detector is triggered.\n");
	case WEAPON_NBOMB:            return _("This Maian grenade plays havoc with the neurons in sentient creatures. If you wander into the blast radius, your vision will blur and you will lose your grip on whatever you're holding. The grenade can be set to go off on impact or by proximity trigger. Make sure you throw it far enough away.\n");
	case WEAPON_TIMEDMINE:        return _("A degree of judgement is required for the effective use of timed mines, otherwise the intended target may overrun the explosion range and come after you. Hone your skills in the tests before you make a critical error in the field. The secondary function is a threat detector identical to that found on the K7 Avenger assault rifle.\n");
	case WEAPON_PROXIMITYMINE:    return _("The key to successful placement of proximity mines is to put them where your opponent doesn't expect to encounter them. If you find yourself on the receiving end of proximity mines, use the threat detector secondary mode to make sure of your surroundings. It may be wasting your time, but if it isn't, it could save your life.\n");
	case WEAPON_REMOTEMINE:       return _("The latest variety of remote detonated mines, essentially the same as all that have gone before. The primary mode is placing the mines, while the secondary mode is giving the detonation command. If you can't see the mine, you'll have to rely on old-fashioned guesswork and timing. \n");
#endif
	}

	return NULL;
}

void frEndSession(bool hidetargets)
{
	s32 i;
	s32 j;
	s16 propnums[256];
	s16 *propnumptr;
#ifdef AVOID_UB
	RoomNum rooms[21];
#else
	RoomNum rooms[20];
#endif
	u32 stack1;
#ifdef AVOID_UB
	RoomNum rooms2[11]; // prevent bgRoomGetNeighbours from writing out of bounds
#else
	RoomNum rooms2[10];
#endif
	u32 stack2;

	if (g_FrDataLoaded) {
		struct defaultobj *terminal = objFindByTagId(0x7f);

		if (terminal) {
			terminal->flags &= ~OBJFLAG_CANNOT_ACTIVATE;
		}

		frUnlockDoor();

		if (g_Vars.currentplayer->visionmode == VISIONMODE_SLAYERROCKET) {
			g_Vars.currentplayer->visionmode = VISIONMODE_NORMAL;
		}

		bgunSetPassiveMode(true);

		g_FrIsValidWeapon = 0;

		frRestoreLighting();

		if (hidetargets) {
			frHideAllTargets();
		}

		if (g_ThrownLaptops[0].base.prop) {
			objFreePermanently(&g_ThrownLaptops[0].base, true);
		}

		roomsCopy(g_Vars.currentplayer->prop->rooms, rooms);

		for (i = 0; g_Vars.currentplayer->prop->rooms[i] != -1; i++) {
			bgRoomGetNeighbours(g_Vars.currentplayer->prop->rooms[i], rooms2, 10);
			roomsAppend(rooms2, rooms, 20);
		}

		// Remove projectiles and throwables
		roomGetProps(rooms, propnums, 256);

		propnumptr = propnums;

		while (*propnumptr >= 0) {
			struct prop *prop = &g_Vars.props[*propnumptr];

			if (prop) {
				struct defaultobj *obj = prop->obj;

				if (prop->type == PROPTYPE_WEAPON) {
					if (obj->type == OBJTYPE_AUTOGUN) {
						objFreePermanently(obj, true);
					}

					if (obj->type == OBJTYPE_WEAPON) {
						struct weaponobj *weapon = (struct weaponobj *)obj;

						if (weapon->weaponnum == WEAPON_NBOMB
								|| weapon->weaponnum == WEAPON_BOLT
								|| weapon->weaponnum == WEAPON_COMBATKNIFE
								|| weapon->weaponnum == WEAPON_HOMINGROCKET
								|| weapon->weaponnum == WEAPON_GRENADE
								|| weapon->weaponnum == WEAPON_GRENADEROUND
								|| weapon->weaponnum == WEAPON_PROXIMITYMINE
								|| weapon->weaponnum == WEAPON_REMOTEMINE
								|| weapon->weaponnum == WEAPON_ROCKET
								|| weapon->weaponnum == WEAPON_TIMEDMINE
								|| weapon->weaponnum == WEAPON_SKROCKET
								|| (weapon->weaponnum == WEAPON_DRAGON && weapon->gunfunc == FUNC_SECONDARY)
								|| (weapon->weaponnum == WEAPON_LAPTOPGUN && weapon->gunfunc == FUNC_SECONDARY)) {
							objFreePermanently(obj, true);
						}
					}
				}
			}

			propnumptr++;
		}

		// Remove explosions
		for (i = 0; i < g_MaxExplosions; i++) {
			g_Explosions[i].age = 256;

			for (j = 0; j < ARRAYCOUNT(g_Explosions[i].parts); j++) {
				g_Explosions[i].parts[j].frame = 0;
			}
		}

		// Remove smoke
		for (i = 0; i < g_MaxSmokes; i++) {
			g_Smokes[i].age = 256;

			for (j = 0; j < ARRAYCOUNT(g_Smokes[i].parts); j++) {
				g_Smokes[i].parts[j].size = 0;
			}
		}
	}

	playerDisplayHealth();

	g_Vars.currentplayer->bondhealth = 1;
}

bool frWasTooInaccurate(void)
{
	f32 sum = (g_FrData.numhitsring3 +
		+ g_FrData.numhitsbullseye
		+ g_FrData.numhitsring1
		+ g_FrData.numhitsring2) * 100.0f;

	if (g_FrData.numshots) {
		f32 accuracy = sum / g_FrData.numshots;

		if (accuracy < g_FrData.goalaccuracy) {
			return true;
		}
	}

	return false;
}

void frSetFailReason(s32 failreason)
{
	frEndSession(false);

	g_FrData.failreason = frWasTooInaccurate() ? FRFAILREASON_INACCURATE : failreason;
	g_FrData.menutype = FRMENUTYPE_FAILED;
	g_FrData.menucountdown = TICKS(60);
}

void frSetCompleted(void)
{
	frEndSession(false);

	if (frWasTooInaccurate()) {
		g_FrData.failreason = FRFAILREASON_INACCURATE;
		g_FrData.menutype = FRMENUTYPE_FAILED;
	} else {
		u32 frweaponindex = frGetWeaponIndexByWeapon(frGetWeaponBySlot(g_FrData.slot));
		frSaveScoreIfBest(frweaponindex, g_FrData.difficulty + 1);
		g_FrData.menutype = FRMENUTYPE_COMPLETED;
	}

	g_FrData.menucountdown = TICKS(60);
}

bool frIsTargetOneHitExplodable(struct prop *prop)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		if (g_FrData.targets[i].inuse
				&& g_FrData.targets[i].destroyed == false
				&& g_FrData.targets[i].active
				&& prop == g_FrData.targets[i].prop) {
			if (g_FrData.targets[i].flags & FRTARGETFLAG_ONEHITEXPLODE) {
				return true;
			}

			return false;
		}
	}

	return false;
}

f32 frGetTargetAngleToPos(struct coord *targetpos, f32 targetangle, struct coord *pos)
{
	f32 xdiff = targetpos->x - pos->x;
	f32 zdiff = targetpos->z - pos->z;
	f32 directangle = atan2f(xdiff, zdiff);
	f32 relativeangle = directangle - targetangle;

	if (directangle < targetangle) {
		relativeangle += M_BADTAU;
	}

	return relativeangle;
}

bool frIsTargetFacingPos(struct prop *prop, struct coord *pos)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		if (prop == g_FrData.targets[i].prop) {
			f32 angle;

			if (g_FrData.targets[i].destroyed) {
				return false;
			}

			angle = frGetTargetAngleToPos(&prop->pos, g_FrData.targets[i].angle, pos);

			//if (angle > DEG2RAD(90) && angle < DEG2RAD(270)) {
			if (angle > 1.5707963705063f && angle < 4.7116389274597f) {
				return false;
			}

			return true;
		}
	}

	return true;
}

struct prop *frChooseAutogunTarget(struct coord *autogunpos)
{
	f32 closestdist = 0x20000000;
	s32 facingtargets[ARRAYCOUNT(g_FrData.targets)];
	s32 len = 0;
	struct prop *closesttarget = NULL;
	s32 i;

	// Make list of targets which are facing the laptop gun
	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		if (g_FrData.targets[i].inuse
				&& g_FrData.targets[i].destroyed == false
				&& g_FrData.targets[i].active) {
			f32 angle = frGetTargetAngleToPos(&g_FrData.targets[i].prop->pos, g_FrData.targets[i].angle, autogunpos);

			if (!(angle > 1.5707963705063f && angle < 4.7116389274597f)) {
				facingtargets[len++] = i;
			}
		}
	}

	// Determine which of the facing targets is closest
	for (i = 0; i < len; i++) {
		struct prop *prop = g_FrData.targets[facingtargets[i]].prop;
		f32 xdiff = prop->pos.f[0] - autogunpos->f[0];
		f32 ydiff = prop->pos.f[1] - autogunpos->f[1];
		f32 zdiff = prop->pos.f[2] - autogunpos->f[2];
		f32 dist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

		if (dist < closestdist) {
			closestdist = dist;
			closesttarget = prop;
		}
	}

	if (facingtargets);

	return closesttarget;
}

bool frIsAmmoWasted(void)
{
	s32 weaponnum = frGetWeaponBySlot(g_FrData.slot);
	s32 i;
	s32 priammotype = bgunGetAmmoTypeForWeapon(weaponnum, 0);
	s32 secammotype = bgunGetAmmoTypeForWeapon(weaponnum, 1);
	struct hand *hand0 = &g_Vars.currentplayer->hands[0];
	struct hand *hand1 = &g_Vars.currentplayer->hands[1];
	s32 ammoloaded[2];
	s32 ammototal[2];
	s16 *propnumptr;
	s16 propnums[258];
	RoomNum rooms20[22];
	RoomNum rooms10[12];
	u32 stack[4];
	s32 ammotype;
	struct hand *hand;
	struct prop *prop;
	struct prop *child;

	// Laser has unlimited ammo
	if (weaponnum == WEAPON_LASER) {
		return false;
	}

	// Check if player has ammo
	ammoloaded[0] = hand0->loadedammo[0] + hand1->loadedammo[0];
	ammoloaded[1] = hand0->loadedammo[1] + hand1->loadedammo[1];
	ammototal[0] = bgunGetReservedAmmoCount(priammotype) + ammoloaded[0];
	ammototal[1] = bgunGetReservedAmmoCount(secammotype) + ammoloaded[1];

	if (ammototal[0] <= 0 && ammototal[1] <= 0) {
		// Don't do any further checks if this is the first frame where we've
		// gotten this far. I'm guessing this fixes a frame perfect bug, however
		// testing with this check removed doesn't cause any unusual behaviour.
		if (g_FrData.ammohasgrace) {
			g_FrData.ammohasgrace = false;
			return false;
		}

		// Check if there are any explosions
		for (i = 0; i != MAX_EXPLOSIONS; i++) {
			if (g_Explosions[i].prop) {
				return false;
			}
		}

		// Check projectiles
		if (weaponnum == WEAPON_ROCKETLAUNCHER
				|| weaponnum == WEAPON_SLAYER
				|| weaponnum == WEAPON_DEVASTATOR
				|| weaponnum == WEAPON_SUPERDRAGON
				|| weaponnum == WEAPON_COMBATKNIFE
				|| weaponnum == WEAPON_CROSSBOW
				|| weaponnum == WEAPON_GRENADE
				|| weaponnum == WEAPON_NBOMB
				|| weaponnum == WEAPON_TIMEDMINE
				|| weaponnum == WEAPON_PROXIMITYMINE
				|| weaponnum == WEAPON_REMOTEMINE) {
			roomsCopy(g_Vars.currentplayer->prop->rooms, rooms20);

			for (i = 0; g_Vars.currentplayer->prop->rooms[i] != -1; i++) {
				bgRoomGetNeighbours(g_Vars.currentplayer->prop->rooms[i], rooms10, 10);
				roomsAppend(rooms10, rooms20, 20);
			}

			roomGetProps(rooms20, propnums, 256);
			propnumptr = propnums;

			while (*propnumptr >= 0) {
				prop = &g_Vars.props[*propnumptr];
				child = prop->child;

				if ((child && child->type == PROPTYPE_WEAPON && child->weapon->weaponnum == WEAPON_TIMEDMINE)
						|| (child && child->type == PROPTYPE_WEAPON && child->weapon->weaponnum == WEAPON_REMOTEMINE)
						|| (child && child->type == PROPTYPE_WEAPON && child->weapon->weaponnum == WEAPON_PROXIMITYMINE)
						|| (child && child->type == PROPTYPE_WEAPON && child->weapon->weaponnum == WEAPON_GRENADEROUND)) {
					return false;
				}

				if (prop->type == PROPTYPE_WEAPON) {
					if (prop->weapon->weaponnum == WEAPON_ROCKET
							|| prop->weapon->weaponnum == WEAPON_HOMINGROCKET
							|| prop->weapon->weaponnum == WEAPON_GRENADE
							|| prop->weapon->weaponnum == WEAPON_GRENADEROUND) {
						return false;
					}

					if (prop->weapon->weaponnum == WEAPON_BOLT
							|| prop->weapon->weaponnum == WEAPON_COMBATKNIFE) {
						if (prop->obj->hidden & OBJHFLAG_PROJECTILE) {
							return false;
						}
					} else if (prop->weapon->weaponnum == WEAPON_TIMEDMINE
							|| prop->weapon->weaponnum == WEAPON_REMOTEMINE) {
						return false;
					} else if (prop->weapon->weaponnum == WEAPON_PROXIMITYMINE) {
						if (g_FrData.proxyendtimer == -255) {
							return false;
						}

						if (g_FrData.proxyendtimer == 0) {
							// Initial state - set the timer to 5 seconds if player is now out of mines
							ammotype = bgunGetAmmoTypeForWeapon(weaponnum, 0);
							hand = &g_Vars.currentplayer->hands[HAND_RIGHT];

							if (bgunGetReservedAmmoCount(ammotype) + hand->loadedammo[0] == 0) {
								g_FrData.proxyendtimer = TICKS(300);
							}

							return false;
						}

						g_FrData.proxyendtimer -= g_Vars.lvupdate60;

						if (g_FrData.proxyendtimer <= 0) {
							// Timer has just hit zero - remove all proxy items
							for (i = 0; i < ARRAYCOUNT(g_Proxies); i++) {
								if (g_Proxies[i]) {
									g_Proxies[i]->timer240 = 0;
								}
							}

							g_FrData.proxyendtimer = -255;
							return true;
						}

						return false;
					}
				}

				propnumptr++;
			}
		}

		return true;
	}

	return false;
}

void frTick(void)
{
	s32 ammotype;
	s32 capacity;
	s32 weaponnum;
	struct coord diff;
	struct coord newpos;
	u8 weaponnum2;
	struct prop *prop;
	struct defaultobj *obj;
	struct defaultobj *obj2;
	s32 invincible;
	s32 i;
	s32 j;
	f32 dist;
	u32 stack;
	struct inventory_ammo *ammo;
	u8 exploding;
	bool oldside;
	struct modelrodata_bbox *bbox;
	s32 tmp;
#if VERSION >= VERSION_NTSC_1_0
	f32 mult;
#endif
	bool newside;
	struct chrdata *chr;
	bool cloaked;
	f32 toangle;
	f32 speed;
	Mtxf spbc;
	f32 sp98[3][3];

	if (g_FrIsValidWeapon
			&& g_Vars.currentplayer->gunctrl.throwing == false
			&& invHasSingleWeaponIncAllGuns(frGetWeaponBySlot(g_FrData.slot))) {
		bgunEquipWeapon(frGetWeaponBySlot(g_FrData.slot));
	}

	// NTSC beta does the room code then menu code,
	// while everything else does the menu code then room code
#if VERSION < VERSION_NTSC_1_0
	// End the session if the player slipped through the door before it closed
	if (g_Vars.currentplayer->prop->rooms[0] != ROOM_DISH_FIRINGRANGE) {
		if (g_FrIsValidWeapon) {
			for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
				if (g_FrData.targets[i].inuse
						&& g_FrData.targets[i].destroyed == false
						&& g_FrData.targets[i].silent == false
						&& g_FrData.targets[i].travelling) {
					g_FrData.targets[i].silent = true;
					psStopSound(g_FrData.targets[i].prop, PSTYPE_GENERAL, 0xffff);
				}
			}

			g_Vars.currentplayer->training = false;
			frEndSession(true);
			g_FrData.menucountdown = 0; // This assignment is in NTSC beta only
			chrUnsetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
		}
		return;
	}
#endif

	// Handle the menu countdown
	if (g_FrData.menucountdown != 0) {
		g_FrData.menucountdown -= g_Vars.lvupdate60;

		// Prevent showing the menu until gun is put away
		if (g_FrData.menucountdown <= 0) {
			if ((g_FrData.menutype == FRMENUTYPE_FAILED || g_FrData.menutype == FRMENUTYPE_COMPLETED)
					&& g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum != WEAPON_UNARMED) {
				g_FrData.menucountdown = 1;
			}
		}

		if (g_FrData.menucountdown <= 0) {
			g_FrData.menucountdown = 0;

			for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
				if (g_FrData.targets[i].prop) {
					psStopSound(g_FrData.targets[i].prop, PSTYPE_GENERAL, 0xffff);
				}
			}

			switch (g_FrData.menutype) {
			case FRMENUTYPE_WEAPONLIST:
				func0f0f85e0(ciGetFrWeaponListMenuDialog(), MENUROOT_TRAINING);
				break;
			case FRMENUTYPE_DETAILS:
				func0f0f85e0(&g_FrTrainingInfoPreGameMenuDialog, MENUROOT_TRAINING);
				break;
			case FRMENUTYPE_FAILED:
				sndStart(var80095200, SFX_TRAINING_FAIL, NULL, -1, -1, -1, -1, -1);
				func0f0f85e0(&g_FrFailedMenuDialog, MENUROOT_TRAINING);
				break;
			case FRMENUTYPE_COMPLETED:
				sndStart(var80095200, SFX_TRAINING_COMPLETE, NULL, -1, -1, -1, -1, -1);
				func0f0f85e0(&g_FrCompletedMenuDialog, MENUROOT_TRAINING);
				filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
				break;
			}
		}
		return;
	}

#if VERSION >= VERSION_NTSC_1_0
	// End the session if the player slipped through the door before it closed
	if (g_Vars.currentplayer->prop->rooms[0] != ROOM_DISH_FIRINGRANGE) {
		if (g_FrIsValidWeapon) {
			for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
				if (g_FrData.targets[i].inuse
						&& g_FrData.targets[i].destroyed == false
						&& g_FrData.targets[i].silent == false
						&& g_FrData.targets[i].travelling) {
					g_FrData.targets[i].silent = true;
					psStopSound(g_FrData.targets[i].prop, PSTYPE_GENERAL, 0xffff);
				}
			}

			g_Vars.currentplayer->training = false;
			frEndSession(true);
			chrUnsetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
		}
		return;
	}
#endif

	if (!g_FrIsValidWeapon) {
		return;
	}

	if (g_Vars.currentplayer->isdead) {
		frEndSession(false);
	}

	// If paused, stop any target sounds
	if (g_Vars.lvupdate240 == 0) {
		for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
			if (g_FrData.targets[i].inuse
					&& g_FrData.targets[i].destroyed == false
					&& g_FrData.targets[i].silent == false
					&& g_FrData.targets[i].travelling) {
				g_FrData.targets[i].silent = true;
				psStopSound(g_FrData.targets[i].prop, PSTYPE_GENERAL, 0xffff);
			}
		}
		return;
	}

	g_Vars.currentplayer->training = true;
	frExecuteHelpScript();

	// Top up the player's ammo if the config defined more ammo than the
	// weapon allows, or if it defined unlimited ammo
	if (g_FrData.numshotssincetopup != 0) {
		weaponnum = frGetWeaponBySlot(g_FrData.slot);
		ammotype = bgunGetAmmoTypeForWeapon(weaponnum, 0);
		capacity = bgunGetCapacityByAmmotype(ammotype);
		ammo = weaponGetAmmoByFunction(weaponnum, 0);
		capacity -= (ammo ? ammo->clipsize : 0);

		if (g_FrData.ammoextra > 0) {
			tmp = bgunGetReservedAmmoCount(ammotype);
			g_FrData.ammoextra -= g_FrData.numshotssincetopup;

			if (g_FrData.ammoextra < 0) {
				g_FrData.ammoextra = 0;
			}

			capacity = tmp + g_FrData.numshotssincetopup;
			bgunSetAmmoQuantity(ammotype, capacity);
		} else if (g_FrData.ammoextra == -1) {
			bgunSetAmmoQuantity(ammotype, capacity);
		}

		if (weaponnum == WEAPON_SUPERDRAGON) {
			capacity = bgunGetCapacityByAmmotype(AMMOTYPE_DEVASTATOR);

			if (g_FrData.sdgrenadeextra > 0) {
				tmp = bgunGetReservedAmmoCount(AMMOTYPE_DEVASTATOR);
				g_FrData.sdgrenadeextra -= g_FrData.numshotssincetopup;

				if (g_FrData.sdgrenadeextra < 0) {
					g_FrData.sdgrenadeextra = 0;
				}

				capacity = tmp + g_FrData.numshotssincetopup;
				bgunSetAmmoQuantity(AMMOTYPE_DEVASTATOR, capacity);
			} else if (g_FrData.sdgrenadeextra == -1) {
				bgunSetAmmoQuantity(AMMOTYPE_DEVASTATOR, capacity);
			}
		}

		g_FrData.numshotssincetopup = 0;
	}

	g_FrData.timetaken += g_Vars.lvupdate60;

	// Handle prestart
	if (g_FrData.timetaken < 0) {
		if (g_FrData.numshots == 0) {
			if (g_FrData.donealarm == false && g_FrData.timetaken > TICKS(-180)) {
				g_FrData.donealarm = true;
				sndStart(var80095200, SFX_FR_ALARM, NULL, -1, -1, -1, -1, -1);
			}

			if (!g_FrData.donelighting && g_FrData.timetaken > TICKS(-225)) {
				frInitLighting();
			}

			return;
		}

		// Fired a shot during prestart
		if (!g_FrData.donelighting) {
			frInitLighting();
		}

		g_FrData.timetaken = 0;
		g_FrData.donealarm = true;
	}

	// Iterate each target and handle their health active/inactive state
	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		if (g_FrData.targets[i].inuse && g_FrData.targets[i].destroyed == false && g_FrData.targets[i].active) {
			invincible = false;
			exploding = false;
			weaponnum2 = frGetWeaponBySlot(g_FrData.slot);
			prop = g_FrData.targets[i].prop;
			obj = prop->obj;

			switch (weaponnum2) {
			case WEAPON_GRENADE:
			case WEAPON_PROXIMITYMINE:
				coordTriggerProxies(&prop->pos, true);
				break;
			}

			if (g_FrData.targets[i].travelling && g_FrData.targets[i].silent && g_FrData.targets[i].travelspeed != -1) {
				g_FrData.targets[i].silent = false;
				psCreate(NULL, g_FrData.targets[i].prop, SFX_FR_CONVEYER, -1,
						-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
			}

			if (g_FrData.targets[i].angle > 2.2915925979614f && g_FrData.targets[i].angle < 3.9915928840637f) {
				obj->damage = 0;
			}

			if ((g_FrData.targets[i].flags & FRTARGETFLAG_TMPINVINCIBLE)
					&& g_FrData.targets[i].invincibletimer < TICKS(300)) {
				invincible = true;
				g_FrData.targets[i].invincibletimer += g_Vars.lvupdate60;
			}

			if (obj->damage > 0) {
				if (invincible || g_FrData.targets[i].angle == M_PI) {
					obj->damage = 0;
				} else if (g_FrData.targets[i].flags & FRTARGETFLAG_ONEHITEXPLODE
						|| obj->damage >= obj->maxdamage
						|| frGetWeaponBySlot(g_FrData.slot) == WEAPON_PHOENIX) {
					g_FrData.numhitsbullseye++;
					g_FrData.score += 10;
					exploding = true;
					g_FrData.feedbackttl = TICKS(60);
					g_FrData.feedbackzone = FRZONE_EXPLODE;
				}
			}

			// Handle target being destroyed
			if (exploding || (g_FrData.targets[i].maxdamage != 255
						&& g_FrData.targets[i].damage >= g_FrData.targets[i].maxdamage)) {
				bbox = objFindBboxRodata(obj);

				if (g_FrNumSounds && g_FrData.targets[i].travelling) {
					g_FrNumSounds--;
					psStopSound(prop, PSTYPE_GENERAL, 0xffff);
				}

				if (g_FrNumSounds);

				shardsCreate(&prop->pos, &obj->realrot[0][0], &obj->realrot[1][0], &obj->realrot[2][0],
						bbox->xmin, bbox->xmax, bbox->ymin, bbox->ymax, 2, prop);

				g_FrData.targetsdestroyed++;

				if (g_FrData.targets[i].flags & FRTARGETFLAG_ONEHITEXPLODE) {
					explosionCreateSimple(g_FrData.targets[i].prop, &g_FrData.targets[i].prop->pos,
							g_FrData.targets[i].prop->rooms, EXPLOSIONTYPE_FRTARGET, 1);
				}

				g_FrData.targets[i].travelling = false;
				g_FrData.targets[i].active = false;
				g_FrData.targets[i].destroyed = true;

				obj->flags2 |= OBJFLAG2_INVISIBLE;

				prop->pos.x = 0;
				prop->pos.y = -5000;
				prop->pos.z = 0;

				func0f069c70(obj, true, false);

				// Activate another target
				for (j = 0; j < ARRAYCOUNT(g_FrData.targets); j++) {
					if (g_FrData.targets[j].destroyed == false
							&& g_FrData.targets[j].inuse
							&& g_FrData.targets[j].active == false) {
						obj2 = g_FrData.targets[j].prop->obj;
						g_FrData.targets[j].active = true;
						obj2->flags2 &= ~OBJFLAG2_INVISIBLE;
						break;
					}
				}
			}
		}
	}

	// Check if the session should end
	if (g_FrData.goaltargets == 255) {
		if (g_FrData.goalscore && g_FrData.score >= g_FrData.goalscore) {
			frSetCompleted();
			return;
		}
	} else {
		if (g_FrData.targetsdestroyed >= g_FrData.goaltargets
				&& (g_FrData.goalscore == 0 || g_FrData.score >= g_FrData.goalscore)) {
			frSetCompleted();
			return;
		}
	}

	if (g_FrData.targetsdestroyed >= g_FrData.numtargets) {
		frSetFailReason(FRFAILREASON_SCOREUNATTAINABLE);
		return;
	}

	if (frIsAmmoWasted()) {
		frSetFailReason(FRFAILREASON_OUTOFAMMO);
		return;
	}

	if (g_FrData.timelimit != 255 && g_FrData.timetaken >= g_FrData.timelimit * TICKS(60)) {
		frSetFailReason(FRFAILREASON_TIMEOVER);
		return;
	}

	// Tick each target
	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		if (g_FrData.targets[i].inuse
				&& g_FrData.targets[i].destroyed == false
				&& g_FrData.targets[i].active) {
			prop = g_FrData.targets[i].prop;
			obj = prop->obj;

			if (g_FrData.targets[i].travelling) {
				if (g_FrData.targets[i].travelspeed == -1) {
					g_FrData.targets[i].donestopsound = true;
					g_FrData.targets[i].travelling = false;
#if VERSION >= VERSION_NTSC_1_0
					mult = 1;
#endif
					dist = -2;
				} else {
					diff.x = g_FrData.targets[i].dstpos.x - prop->pos.x;
					diff.y = g_FrData.targets[i].dstpos.y - prop->pos.y;
					diff.z = g_FrData.targets[i].dstpos.z - prop->pos.z;

					dist = sqrtf(diff.f[0] * diff.f[0] + diff.f[1] * diff.f[1] + diff.f[2] * diff.f[2]);
#if VERSION >= VERSION_NTSC_1_0
					mult = 1;
#endif

					if (dist != 0) {
#if VERSION >= VERSION_NTSC_1_0

#if VERSION >= VERSION_PAL_BETA
						mult = g_FrData.targets[i].travelspeed * g_Vars.lvupdate60freal;
#else
						mult = (g_FrData.targets[i].travelspeed * g_Vars.lvupdate240) * 0.25f;
#endif
						diff.x *= 1.0f / dist;
						diff.y *= 1.0f / dist;
						diff.z *= 1.0f / dist;
						newpos.x = diff.x * mult + prop->pos.x;
						newpos.y = diff.y * mult + prop->pos.y;
						newpos.z = diff.z * mult + prop->pos.z;
#else
						diff.x *= 1.0f / dist;
						diff.y *= 1.0f / dist;
						diff.z *= 1.0f / dist;
						newpos.x = diff.x * g_FrData.targets[i].travelspeed * g_Vars.lvupdate240 * 0.25f + prop->pos.x;
						newpos.y = diff.y * g_FrData.targets[i].travelspeed * g_Vars.lvupdate240 * 0.25f + prop->pos.y;
						newpos.z = diff.z * g_FrData.targets[i].travelspeed * g_Vars.lvupdate240 * 0.25f + prop->pos.z;
#endif
					} else {
						dist = -2;
					}
				}

#if VERSION >= VERSION_NTSC_1_0
				if (mult >= dist)
#else
				if (dist < g_FrData.targets[i].travelspeed)
#endif
				{
					// Target is stopping
					newpos.x = g_FrData.targets[i].dstpos.x;
					newpos.y = g_FrData.targets[i].dstpos.y;
					newpos.z = g_FrData.targets[i].dstpos.z;

					g_FrData.targets[i].scriptenabled = true;
					g_FrData.targets[i].travelling = false;

					if (g_FrData.targets[i].donestopsound == false) {
						g_FrData.targets[i].donestopsound = true;

						if (g_FrNumSounds) {
							g_FrNumSounds--;
						}

						psStopSound(prop, PSTYPE_GENERAL, 0xffff);
						psCreate(NULL, prop, SFX_FR_CONVEYER_STOP, -1,
								-1, PSFLAG_0400, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);

						if (g_FrNumSounds);
					}
				}

				prop->pos.x = newpos.x;
				prop->pos.y = newpos.y;
				prop->pos.z = newpos.z;

				func0f069c70(obj, true, false);
			}

			if (g_FrData.targets[i].rotateoncloak && g_FrData.targets[i].rotating == false) {
				if (g_FrData.targets[i].timeuntilrotate == 0) {
					chr = g_Vars.currentplayer->prop->chr;
					cloaked = chr->hidden & CHRHFLAG_CLOAKED;

					if (cloaked) {
#ifdef PLATFORM_N64
						if (g_FrData.targets[i].angle == M_PI) {
#else
						if (g_FrData.targets[i].angle != 0 && !g_FrData.targets[i].rotating) {
#endif
							g_FrData.targets[i].timeuntilrotate = TICKS(60);
							g_FrData.targets[i].rotatetoangle = 0;
							g_FrData.targets[i].rotatespeed = -M_PI / 90;
						}
					} else {
						if (g_FrData.targets[i].angle == 0) {
							g_FrData.targets[i].timeuntilrotate = TICKS(60);
							g_FrData.targets[i].rotatetoangle = M_PI;
							g_FrData.targets[i].rotatespeed = M_PI / 90;
						}
					}
				} else {
					g_FrData.targets[i].timeuntilrotate -= g_Vars.lvupdate60;

					if (g_FrData.targets[i].timeuntilrotate <= 0) {
						g_FrData.targets[i].timeuntilrotate = 0;
						g_FrData.targets[i].rotating = true;
					}
				}
			} else if (g_FrData.targets[i].rotating) {
				toangle = g_FrData.targets[i].rotatetoangle;
				speed = g_FrData.targets[i].rotatespeed;

				if (toangle);

				oldside = 0;

				if (g_FrData.targets[i].angle < toangle) {
					oldside = 1;
				}

				oldside = (u8)oldside;

#if VERSION >= VERSION_PAL_BETA
				g_FrData.targets[i].angle += speed * g_Vars.lvupdate60freal;
#else
				g_FrData.targets[i].angle += speed * g_Vars.lvupdate240 * 0.25f;
#endif

				newside = 0;

				toangle = g_FrData.targets[i].rotatetoangle;

				if (g_FrData.targets[i].angle < toangle) {
					newside = 1;
				}

				newside = (u8)newside;

				if (newside != oldside || g_FrData.targets[i].angle == toangle) {
					// Reached desired angle
					g_FrData.targets[i].angle = g_FrData.targets[i].rotatetoangle;
					g_FrData.targets[i].rotating = false;
					g_FrData.targets[i].scriptenabled = true;
					g_FrData.targets[i].scriptsleep = 0;

					while (g_FrData.targets[i].angle > M_BADTAU) {
						g_FrData.targets[i].angle -= M_BADTAU;
					}

					while (g_FrData.targets[i].angle < 0) {
						g_FrData.targets[i].angle += M_BADTAU;
					}
				}

				mtx4LoadYRotation(g_FrData.targets[i].angle + M_PI, &spbc);
				mtx00015f04(obj->model->scale, &spbc);
				mtx4ToMtx3(&spbc, sp98);
				mtx3Copy(sp98, obj->realrot);
			}

			if (g_FrData.targets[i].scriptenabled && g_FrData.targets[i].scriptsleep != SECSTOTIME60(255)) {
				g_FrData.targets[i].scriptsleep -= g_Vars.lvupdate60;

				if (g_FrData.targets[i].scriptsleep <= 0) {
					g_FrData.targets[i].scriptenabled = false;

					while (!frExecuteTargetScript(i));

					if (frTargetIsAtScriptStart(i)) {
						while (!frExecuteTargetScript(i));
					}
				}
			}
		}
	}
}

void func0f1a0924(struct prop *prop)
{
	struct defaultobj *obj = prop->obj;
	s32 i;

	if (obj->modelnum == MODEL_TARGET) {
		f32 sp68;
		f32 sp64;
		f32 sp60;
		f32 sp56;

		sp64 = -1;
		sp68 = -1;
		sp56 = -2;
		sp60 = -2;

		modelGetScreenCoords(obj->model, &sp56, &sp64, &sp60, &sp68);

		for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
			if (g_Vars.currentplayer->trackedprops[i].prop == prop) {
				return;
			}

			if (g_Vars.currentplayer->trackedprops[i].prop == NULL) {
				g_Vars.currentplayer->trackedprops[i].prop = prop;

				g_Vars.currentplayer->trackedprops[i].x1 = sp64 - 2;
				g_Vars.currentplayer->trackedprops[i].x2 = sp56 + 2;
				g_Vars.currentplayer->trackedprops[i].y1 = sp68 - 2;
				g_Vars.currentplayer->trackedprops[i].y2 = sp60 + 2;
				g_Vars.currentplayer->targetset[i] = 0;
				return;
			}
		}
	}
}

bool frChooseFarsightTarget(void)
{
	struct prop *bestprop = NULL;
	f32 bestvalue = 1;
	f32 bestdist = -1;
	bool found = false;
	s32 i;

	if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT) {
		for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
			if (g_FrData.targets[i].inuse
					&& g_FrData.targets[i].destroyed == false
					&& g_FrData.targets[i].active
					&& g_FrData.targets[i].flags & FRTARGETFLAG_FARSIGHTAUTOTARGETABLE) {
				struct prop *prop = g_FrData.targets[i].prop;
				f32 xdiff = g_Vars.currentplayer->bond2.unk10.x - prop->pos.x;
				f32 ydiff = g_Vars.currentplayer->bond2.unk10.y - prop->pos.y;
				f32 zdiff = g_Vars.currentplayer->bond2.unk10.z - prop->pos.z;
				f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

				if (dist > 0) {
					f32 value = (xdiff * g_Vars.currentplayer->bond2.unk1c.f[0]
							+ ydiff * g_Vars.currentplayer->bond2.unk1c.f[1]
							+ zdiff * g_Vars.currentplayer->bond2.unk1c.f[2]) / dist;

					if (value);

					if (value < 0 && value < bestvalue) {
						bestvalue = value;
						bestprop = prop;
						bestdist = dist;
						found = true;
					}
				}
			}
		}
	}

	g_Vars.currentplayer->autoeraserdist = bestdist;
	g_Vars.currentplayer->autoerasertarget = bestprop;

	return found;
}

s32 frIsInTraining(void)
{
	if (g_FrData.menucountdown > 0 &&
			(g_FrData.menutype == FRMENUTYPE_FAILED || g_FrData.menutype == FRMENUTYPE_COMPLETED)) {
		return true;
	}

	return g_Vars.currentplayer->prop->rooms[0] == ROOM_DISH_FIRINGRANGE
		&& g_FrIsValidWeapon
		&& mainGetStageNum() == STAGE_CITRAINING;
}

void frCalculateHit(struct defaultobj *obj, struct coord *hitpos, f32 maulercharge)
{
	s32 i;

	if (g_FrIsValidWeapon == false) {
		return;
	}

	for (i = 0; i < ARRAYCOUNT(g_FrData.targets); i++) {
		struct prop *prop = g_FrData.targets[i].prop;

		if (obj == prop->obj) {
			f32 xdiff = hitpos->x - prop->pos.x;
			f32 ydiff = hitpos->y - prop->pos.y;
			f32 zdiff = hitpos->z - prop->pos.z;

			f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

			if (g_FrData.targets[i].flags & FRTARGETFLAG_ONEHITEXPLODE) {
				g_FrData.targets[i].damage = g_FrData.targets[i].maxdamage;
			} else if (frGetWeaponBySlot(g_FrData.slot) == WEAPON_MAULER) {
				g_FrData.targets[i].damage += (f32)((s32)(maulercharge * 0.1f) + 1);
			} else if ((g_FrData.targets[i].flags & FRTARGETFLAG_TMPINVINCIBLE) == 0
					|| g_FrData.targets[i].invincibletimer >= TICKS(300)) {
				g_FrData.targets[i].damage++;
			}

			if (dist < 18) {
				g_FrData.feedbackzone = FRZONE_BULLSEYE;
				g_FrData.numhitsbullseye++;
			} else if (dist < 37) {
				g_FrData.feedbackzone = FRZONE_RING1;
				g_FrData.numhitsring1++;
			} else if (dist < 56) {
				g_FrData.feedbackzone = FRZONE_RING2;
				g_FrData.numhitsring2++;
			} else {
				g_FrData.feedbackzone = FRZONE_RING3;
				g_FrData.numhitsring3++;
			}

			g_FrData.feedbackttl = TICKS(60);
			g_FrData.score += g_FrData.feedbackzone;
		}
	}
}

void frIncrementNumShots(void)
{
	g_FrData.numshots++;
	g_FrData.numshotssincetopup++;
}

bool ciIsChrBioUnlocked(u32 bodynum)
{
	switch (bodynum) {
	case BODY_DARK_COMBAT:
	case BODY_CARRINGTON:
		return true;
	case BODY_CASSANDRA:
		return ciIsStageComplete(SOLOSTAGEINDEX_DEFECTION);
	case BODY_DRCAROLL:
		return ciIsStageComplete(SOLOSTAGEINDEX_INVESTIGATION);
	case BODY_MRBLONDE:
		return ciIsStageComplete(SOLOSTAGEINDEX_EXTRACTION);
	case BODY_TRENT:
		return ciIsStageComplete(SOLOSTAGEINDEX_G5BUILDING);
	case BODY_JONATHAN:
		return ciIsStageComplete(SOLOSTAGEINDEX_INFILTRATION);
	case BODY_THEKING:
		return ciIsStageComplete(SOLOSTAGEINDEX_RESCUE);
	case BODY_PRESIDENT:
		return ciIsStageComplete(SOLOSTAGEINDEX_AIRFORCEONE);
	}

	return false;
}

u8 g_ChrBioSlot = 0;

struct chrbio *ciGetChrBioByBodynum(u32 bodynum)
{
#ifdef AVOID_UB
	static
#endif
	struct chrbio bios[] = {
		// name, race, age, profile
#if VERSION >= VERSION_PAL_BETA
		/*0*/ { L_DISH_125, L_DISH_126, L_DISH_127, L_DISH_128 }, // Joanna Dark
		/*1*/ { L_DISH_129, L_DISH_130, L_DISH_131, L_DISH_132 }, // Jonathan
		/*2*/ { L_DISH_133, L_DISH_134, L_DISH_135, L_DISH_136 }, // Daniel Carrington
		/*3*/ { L_DISH_137, L_DISH_138, L_DISH_139, L_DISH_140 }, // Cassandra De Vries
		/*4*/ { L_DISH_141, L_DISH_142, L_DISH_143, L_DISH_144 }, // Trent Easton
		/*5*/ { L_DISH_145, L_DISH_146, L_DISH_147, L_DISH_148 }, // Dr. Caroll
		/*6*/ { L_DISH_149, L_DISH_150, L_DISH_151, L_DISH_152 }, // Elvis
		/*7*/ { L_DISH_153, L_DISH_154, L_DISH_155, L_DISH_156 }, // Mr. Blonde
		/*8*/ { L_DISH_157, L_DISH_158, L_DISH_159, L_DISH_160 }, // Mr. Blonde (repeat)
		/*9*/ { L_DISH_161, L_DISH_162, L_DISH_163, L_DISH_164 }, // The U.S. President
#else
		/*0*/ { gettext_noop("Joanna Dark\n"), gettext_noop("Human (Female)\n"), gettext_noop("23 years 2 months\n"), gettext_noop("|CI File #027 -\n\nTraining Status: Complete\nTraining Grade: A++\nActive Status: Assigned\n\n|Profile -\n\nHighly trained but inexperienced. Reactions superb. Proficient with a variety of weapons. Very competent all-round agent. Highest recorded training scores resulted in the creation of a new class of training grade. The embodiment of the Carrington Institute's ideal agent, hence the call sign 'Perfect Dark.'\n") }, // Joanna Dark
		/*1*/ { gettext_noop("Jonathan\n"), gettext_noop("Human (Male)\n"), gettext_noop("28 years 5 months\n"), gettext_noop("|CI File #009 -\n\nTraining Status: Complete\nTraining Grade: A+\nActive Status: Undercover\n\n|Profile -\n\nOur most experienced undercover agent. Highly accurate with his chosen weapon (a Magnum Revolver). Perfectly suited to undercover missions. Less suited to out-and-out combat. Before Joanna Dark, he held the honor of having the highest recorded training scores.\n") }, // Jonathan
		/*2*/ { gettext_noop("Daniel Carrington\n"), gettext_noop("Human (Male)\n"), gettext_noop("62 years 8 months\n"), gettext_noop("|CI File #000 -\n\nTraining Status: N/A\nTraining Grade: N/A\nActive Status: N/A\n\n|Profile -\n\nIntelligent patriarchal scientist/entrepreneur, and founder of the Carrington Institute. Plans all missions carried out by his agents and runs each operation direct from a link in his office. Strange taste in clothes.\n") }, // Daniel Carrington
		/*3*/ { gettext_noop("Cassandra De Vries\n"), gettext_noop("Human (Female)\n"), gettext_noop("39 years ? months\n"), gettext_noop("|Analyst note -\n\nThe head of dataDyne Corp. Addicted to power; dislikes being anybody's underling. Hates it when she loses the initiative. Is prepared to do extremely unscrupulous things in order to get ahead of her competition, to whit Daniel Carrington, whom she loathes.\n") }, // Cassandra De Vries
		/*4*/ { gettext_noop("Trent Easton\n"), gettext_noop("Human (Male)\n"), gettext_noop("46 years ? months\n"), gettext_noop("|Analyst note -\n\nHead of the National Security Agency. Has a friendship of sorts with Cassandra De Vries, although it operates more like a partnership of interest. He will tend to do what Cassandra says, possibly because although he has a dominant personality, it is not as dominant as hers. Figurehead for some of the rogue elements in the NSA.\n") }, // Trent Easton
		/*5*/ { gettext_noop("Dr. Caroll\n"), gettext_noop("The Caroll Sapient (AI)\n"), gettext_noop("6 months\n"), gettext_noop("|Profile -\n\nAn artificial intelligence created by the dataDyne Corp. with an emphasis on language skills and code breaking. Fortunately, he has morals, and due to his formidable level of intelligence, has guessed some of dataDyne's future plans. The voice is highly precise and educated and simulates the character of an academic.\n") }, // Dr. Caroll
		/*6*/ { gettext_noop("Elvis\n"), gettext_noop("Maian (Male)\n"), gettext_noop("320 years\n"), gettext_noop("|Profile -\n\nAn alien from the Maian race. He is a 'Protector' (bodyguard) for the Maian ambassador who travels to Earth at Daniel Carrington's request. Protectors are trained to excel in the use of an assortment of weaponry. Elvis is a terraphile, finding Earth and everything about it fascinating.\n") }, // Elvis
		/*7*/ { gettext_noop("Mr. Blonde\n"), gettext_noop("Human (Male)\n"), gettext_noop("Late 20's\n"), gettext_noop("|Profile -\n\nA striking blonde human male. Very tall, wears white clothing, usually a raincoat. Appears to be masterminding the conspiracy in which Cassandra and Trent are involved. Little else is known.\n") }, // Mr. Blonde
		/*8*/ { gettext_noop("Mr. Blonde\n"), gettext_noop("Skedar (disguised)\n"), gettext_noop("unknown\n"), gettext_noop("|Updated Profile -\n\nThis is a Skedar warrior lurking within a holographic projection of a striking blonde young human male in his late 20's.  The oral modulation unit gives the Skedar a precise, persuasive, and intelligent voice. It is a propaganda and manipulation tool for the Skedar, and an unusually subtle one.\n") }, // Mr. Blonde (repeat)
		/*9*/ { gettext_noop("The U.S. President\n"), gettext_noop("Human (Male)\n"), gettext_noop("50 years\n"), gettext_noop("|Profile -\n\nA highly educated, shrewd African-American who is trying to do what is right but is surrounded by people like Trent Easton. He believes he has Trent under control after refusing the request for the loan of the Pelagic II to the dataDyne Corp. Perceived as being easily led by the majority of political commentators, which is perhaps unfair.\n") }, // The U.S. President
#endif
	};

	switch (bodynum) {
	case BODY_DARK_COMBAT:
		return &bios[0];
	case BODY_JONATHAN:
		return &bios[1];
	case BODY_CARRINGTON:
		return &bios[2];
	case BODY_CASSANDRA:
		return &bios[3];
	case BODY_TRENT:
		return &bios[4];
	case BODY_DRCAROLL:
		return &bios[5];
	case BODY_THEKING:
		return &bios[6];
	case BODY_MRBLONDE:
		if (ciIsStageComplete(SOLOSTAGEINDEX_CRASHSITE)) {
			return &bios[8];
		}
		return &bios[7];
	case BODY_PRESIDENT:
		return &bios[9];
	}

	return NULL;
}

char *ciGetChrBioDescription(void)
{
	struct chrbio *bio = ciGetChrBioByBodynum(ciGetChrBioBodynumBySlot(g_ChrBioSlot));
	return _(bio->description);
}

s32 ciGetNumUnlockedChrBios(void)
{
	s32 count = 0;
	s32 bodynum;

	for (bodynum = 0; bodynum < ARRAYCOUNT(g_HeadsAndBodies) - 1; bodynum++) {
		if (ciIsChrBioUnlocked(bodynum)) {
			count++;
		}
	}

	return count;
}

s32 ciGetChrBioBodynumBySlot(s32 slot)
{
	s32 index = -1;
	s32 bodynum;

	for (bodynum = 0; bodynum < ARRAYCOUNT(g_HeadsAndBodies) - 1; bodynum++) {
		if (ciIsChrBioUnlocked(bodynum)) {
			index++;
		}

		if (index == slot) {
			return bodynum;
		}
	}

	return 0;
}

struct miscbio *ciGetMiscBio(s32 index)
{
#ifdef AVOID_UB
	static
#endif
	struct miscbio bios[] = {
		// name, description
#if VERSION >= VERSION_PAL_BETA
		{ L_DISH_165, L_DISH_166 },
		{ L_DISH_167, L_DISH_168 },
		{ L_DISH_169, L_DISH_170 },
		{ L_DISH_171, L_DISH_172 },
#else
		{ gettext_noop("Maians\n"), gettext_noop("The Maians are the race of aliens that have come to be known on Earth as 'Greys.' They have been monitoring Earth for a long time - several centuries - and are benevolent towards mankind, sensing great potential in the human race. Their contact on Earth is Daniel Carrington. A formal political contact has yet to be made.\n") },
		{ gettext_noop("Skedar Warrior\n"), gettext_noop("The Skedar are a warlike alien race who have fought the Maians for centuries and have only recently agreed to a ceasefire. They tend to use mechanized armatures to walk about and fight in, since they are in actual fact smaller, snakelike creatures. Very aggressive -  they have made war a religion - and are extremely devout.\n") },
		{ gettext_noop("Background\n"), gettext_noop("Millions of years ago, a huge alien ship of immense power was scuttled in the Pacific Ocean. It was equipped with an untried and vastly dangerous weapon: the weak nuclear force de-coupler, which could in theory cause the fundamental bonds between molecules to fail. So that watching eyes would believe in the destruction of the ship, a message pod was despatched to the home galaxy, and vital components of the ship's drives were destroyed in the star system it had chosen as a hiding place. For this ship was a Cetan, a massive, sentient alien creature.\n\nAnother extraterrestrial race, the Maians, encountered life on Earth in 2000 BC. They saw great potential, but decided to let the primitive human race develop without their interference.\n\nThe Maians encountered the Skedar near Delta Eridanus in 1650 AD (Earth date), and the resultant skirmish soon blossomed into all-out war. Only after hundreds of years of fighting did an uneasy peace develop, and even then Skedar fanatics persistently tested the boundaries of this peace with terrorist activities. Fortunately, the Maians refused to be drawn.\n\nMankind, too, continued to be a cause of concern to the Maians, and they watched with disappointment as Earth suffered more and more crises brought on by the rise of technology and 'outside context situations.' They feared that announcing their presence would only precipitate further wars on the planet, and quietly decided to continue their observation until the race became more mature.\n\nDaniel Carrington jumped the gun in 1985 AD by contacting a Maian ship in orbit above Earth. He put forward a plan that would help both parties, resulting in accelerated contact between Humans and Maians. Having agreed that his plan was sound, and finding him to be a person of integrity, the Maians allowed Carrington to release their technology into the public domain through his considerable Research and Development holdings.\n\nThen, in 2006 AD, Skedar fanatics stumbled across the Cetan message pod. They immediately began to scout Earth in a particularly heavy-handed way, abducting people and mutilating animals in an attempt to determine the location of the lost battle cruiser. In time, their tests inevitably led them to its resting place on the bed of the Pacific Ocean; however, in order to actually reach it, they would need the help of the natives.\n\nPerforming their own study of Earth, the Skedar came up with a shortlist of companies possessing the resources to help them recover the sunken ship. At the top of the list was the dataDyne Corporation. They contacted the company head, Cassandra De Vries, and presented her with a deal that sounded too good to be true - which, of course, it was. In return for helping to raise 'their' ship from the ocean floor, the Skedar promised to supply dataDyne with enough alien technology to become the biggest corporation on the face of the planet. All that dataDyne had to do was build an AI unit with language and code-breaking abilities, which was tricky, but certainly within their means. Work commenced on the project immediately.\n\nCassandra De Vries approached NSA head Trent Easton with the details of the plan, but his attempts to get Presidential approval for the loan of a deep-sea research vessel were repeatedly turned down. There was no way that the Skedar or dataDyne could steal the vessel without triggering the wrath of the U.S. Government, so over time they settled instead on a far more sinister plan of replacing the President with a clone that they could control. Of course, this also promised extra rewards for Trent and Cassandra when the Skedar operation was consigned to history.\n\nHowever, they would not be given much time to gloat. The Skedar fanatics' ultimate plan was to test the weak nuclear force de-coupler on Earth before wiping the Maians out of existence, and both Cassandra and Trent remained blind to the possibility of being double-crossed, so enamored were they with the visions of power and luxury that lay ahead.\n\nBut dataDyne had unwittingly created something that could see all too clearly the danger within this multilayered conspiracy: the sapient AI built to crack the core access codes of the Cetan ship. When it expressed concern over the mission for which it had been designed, Cassandra's response was to order its personality removed. Clearly, it was thinking too much. In desperation, and in a move which would make or break a great deal more than dataDyne's 'deal' with the Skedar, the sapient adopted the pseudonym Dr. Caroll and contacted the Carrington Institute with a plea for help...\n") },
		{ gettext_noop("The Story\n"), gettext_noop("Untried agent Joanna Dark is assigned a mission involving the extraction of a scientist from the high security research area beneath the dataDyne skyscraper. Upon rendezvous, she is surprised to discover that this 'Dr. Caroll' is an AI created by dataDyne itself - but she continues with her task of escorting him/it to a place of safety. When the alarm is raised, Joanna has to fight her way up the tower to reach the helipad and the dropship which presents her only means of escape.\n\nIn response to this incursion, dataDyne make Daniel Carrington a personal target and take him hostage two days later at his private villa. They demand the return of the AI - their 'property' - in exchange for his life. Unknown to them, the AI is already at the villa, where it was being questioned by Carrington on dataDyne's future plans. Joanna eventually rescues the head of the Institute but is unable to prevent dataDyne from taking back Dr. Caroll. However, Carrington has heard enough to take the drastic step of summoning a team of Maian specialists to Earth.\n\nJoanna is dispatched to Chicago to spy on a conspirators' meeting at the G5 Building, a front for the dataDyne Corp. It is here that she learns of the involvement of Trent Easton, head of the NSA, and of the strange Nordic men that appear to be in control of the whole deadly scheme. Once Trent discloses the plan to usurp the Presidential position, Jo radios the news back to base - only to find that another, more urgent task requires her attention before further action can be taken against Trent.\n\nThe Maian specialists have been intercepted and brought down by the conspirators, with survivors and wreckage alike being transported to Area 51 in Nevada. Briefed to rescue any survivors and retrieve their equipment, Joanna is dropped in to link up with another Carrington Institute agent. Upon breaking into the medlab, Joanna finally discovers the Institute's secret - their allies are Maians, the alien race commonly known as Greys. The particular Maian she manages to rescue goes by the name of Elvis. He was a bodyguard for the Ambassador who had been flying in at the head of the specialist team.\n\nHaving aided the remnants of the Maian delegation, Joanna can return to the matter of the President and dataDyne's designs against him. She poses as a member of the President's entourage to gain access to the air base where Air Force One is stationed, and successfully conceals herself aboard the plane. As soon as it's airborne, Trent and the cloaked Skedar make their move - but Joanna is there to stop them. She rushes the President to a safety capsule while a team of Trent's men scour the aircraft, intent on dragging the hapless politician aboard the Skedar UFO now docked to AF1 via an umbilical. Jo weakens the umbilical but is unable to break it, and ultimately Elvis makes a last-ditch attempt to sever the cord by crashing into it at high speed. All three craft plummet towards the Alaskan wilderness, and the AF1 escape pod is launched.\n\nComing around, Jo tries to report in but finds her communications jammed by a transmission from the Skedar craft. She sets off through the snow to find the President and Elvis, encountering teams of cloaked Skedar out searching for her, the President, Elvis, and the President's belongings. She tracks one group back to the downed Skedar ship, where she discovers a clone of the President: quickly she destroys it, shuts down the jamming device and calls in the cavalry. The Skedar are thwarted once more, the President is safe, and Trent's incompetence has earned him a nasty fate at the hands of his 'allies.'\n\nBut it's not over. Throwing caution to the wind, dataDyne and the Skedar steal the Government's deep-sea research vessel 'Pelagic II' and head out to the Pacific crash site. Joanna and Elvis leave to disrupt activity on board the ship and find out what lies at the heart of the grand Skedar plan. After crippling the vessel's diving operations and recalling the submersible, they head down to the ocean floor, where they get their first sight of the downed Cetan battle cruiser. A portal made by the Skedar offers them access to the ship, where they stumble across a dead Skedar warrior... Clearly the assault team didn't have it all their own way. Fighting off the remaining vengeful Skedar and avoiding the Cetans themselves, Joanna and Elvis make their way down to the core of the ship, where they find the AI that was once Dr. Caroll. They manage to restore his personality, whereupon he urges Joanna and Elvis to leave the ship so that he may destroy it for good.\n\nLater, back at the Carrington Institute, Joanna is about to leave for a Presidential reception at the White House when all hell breaks loose. The Skedar assault team survivors are venting their anger on those they deem responsible for their failure to recover the Cetan ship. Joanna dashes around the Institute, helping CI employees to reach the safety of the hangars, as the Skedar launch attacks on various parts of the building: the majority of the employees make good their escape while Joanna holds the fort, but eventually she is knocked unconscious and taken prisoner.\n\nShe comes to in a holding cell on board the Skedar assault ship, with only Cassandra De Vries for company. In a shocking move, the dataDyne CEO willingly gives her life to create the distraction necessary for Joanna to break out. Battling her way through the ship, Jo eventually manages to locate and disable the docking bay shields, allowing Elvis to bring in a few friends for the long-awaited shooting party.\n\nThe captured assault ship enters orbit above the Skedar Battle Shrine, which shocks Elvis as the Maians never managed to find this Skedar 'holiest of holies' throughout the long years of interracial war. If the Shrine were destroyed, Skedar morale would be dealt a fatal blow and true peace would finally replace the uneasy ceasefire. So Jo sets her sights on the Skedar leader, the high priest of the Battle Shrine, while Elvis returns to the assault ship in order to summon the Maian fleet.\n\nIn the wake of the climactic battle, the temple lies devastated with Elvis calling for a lull in the bombardment so that he can find Joanna. She is alive, held under a pile of rubble by the last tenacious Skedar that caught up during the last few seconds of her escape bid. Elvis quickly offers her a gun to convince the Skedar to let go, and with the job finally done, the Skedar Shrine and morale both in ruins and the conspiracy dealt a mortal blow to the heart, the two of them depart for orbit.\n") },
#endif
	};

	switch (index) {
	case MISCBIO_MAIANS:     return &bios[0];
	case MISCBIO_SKEDAR:     return &bios[1];
	case MISCBIO_BACKGROUND: return &bios[2];
	case MISCBIO_STORY:      return &bios[3];
	}

	return NULL;
}

bool ciIsMiscBioUnlocked(s32 index)
{
	switch (index) {
	case MISCBIO_MAIANS:
		return ciIsStageComplete(SOLOSTAGEINDEX_RESCUE);
	case MISCBIO_SKEDAR:
		return ciIsStageComplete(SOLOSTAGEINDEX_ATTACKSHIP);
	case MISCBIO_BACKGROUND:
	case MISCBIO_STORY:
		return ciIsStageComplete(SOLOSTAGEINDEX_MBR);
	}

	return false;
}

s32 ciGetNumUnlockedMiscBios(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < 4; i++) {
		if (ciIsMiscBioUnlocked(i)) {
			count++;
		}
	}

	return count;
}

s32 ciGetMiscBioIndexBySlot(s32 slot)
{
	s32 index = -1;
	s32 i;

	for (i = 0; i < 4; i++) {
		if (ciIsMiscBioUnlocked(i)) {
			index++;
		}

		if (index == slot) {
			return i;
		}
	}

	return 0;
}

char *ciGetMiscBioDescription(void)
{
	s32 index = ciGetMiscBioIndexBySlot(g_ChrBioSlot - ciGetNumUnlockedChrBios());
	struct miscbio *bio = ciGetMiscBio(index);

	return _(bio->description);
}

bool ciIsHangarBioAVehicle(s32 index)
{
	return index >= HANGARBIO_JUMPSHIP;
}

u8 g_HangarBioSlot = 0;

struct hangarbio *ciGetHangarBio(s32 index)
{
#ifdef AVOID_UB
	static
#endif
	struct hangarbio bios[] = {
		// name, description
#if VERSION >= VERSION_PAL_BETA
		{ L_DISH_196, L_DISH_219 }, // Carrington Institute
		{ L_DISH_197, L_DISH_220 }, // Lucerne Tower
		{ L_DISH_198, L_DISH_221 }, // Laboratory Basement
		{ L_DISH_199, L_DISH_222 }, // Carrington Villa
		{ L_DISH_200, L_DISH_223 }, // Chicago
		{ L_DISH_201, L_DISH_224 }, // G5 Building
		{ L_DISH_202, L_DISH_225 }, // Area 51
		{ L_DISH_203, L_DISH_226 }, // Alaskan Air Base
		{ L_DISH_204, L_DISH_227 }, // Air Force One
		{ L_DISH_205, L_DISH_228 }, // Crash Site
		{ L_DISH_206, L_DISH_229 }, // Pelagic II
		{ L_DISH_207, L_DISH_230 }, // Cetan Ship
		{ L_DISH_208, L_DISH_231 }, // Skedar Assault Ship
		{ L_DISH_209, L_DISH_232 }, // Skedar Homeworld
		{ L_DISH_210, L_DISH_233 }, // Jumpship
		{ L_DISH_211, L_DISH_234 }, // HoverCrate
		{ L_DISH_212, L_DISH_235 }, // HoverBike
		{ L_DISH_213, L_DISH_236 }, // Cleaning Hovbot
		{ L_DISH_214, L_DISH_237 }, // Hovercopter
		{ L_DISH_215, L_DISH_238 }, // G5 Robot
		{ L_DISH_216, L_DISH_239 }, // A51 Interceptor
		{ L_DISH_217, L_DISH_240 }, // Maian Vessel
		{ L_DISH_218, L_DISH_241 }, // Skedar Shuttle
#else
		{ gettext_noop("Carrington Institute|Base of operations\n"), gettext_noop("The Institute building comprises many different areas: offices and laboratories, workshops and hangars. It is quite isolated from the outside world, which helps keep the operations covert.\n") }, // Carrington Institute
		{ gettext_noop("Lucerne Tower|Global headquarters\n"), gettext_noop("In the midst of the business district, the austere skyscraper of the dataDyne Corporation stands out from the surroundings.\n") }, // Lucerne Tower
		{ gettext_noop("Laboratory Basement|Underground research labs\n"), gettext_noop("These heavily guarded, well-hidden labs hold the key to dataDyne's future. Within them, techs work on top-secret projects aimed at putting dataDyne on the top of the heap.\n") }, // Laboratory Basement
		{ gettext_noop("Carrington Villa|Private coastal retreat\n"), gettext_noop("Owned by the Institute, this secluded residence is used by Daniel Carrington as a retreat from the pressures of the Institute. As well as an observatory, it has a power generator and an extensive wine cellar.\n") }, // Carrington Villa
		{ gettext_noop("Chicago|Backstreets of the city\n"), gettext_noop("A seedy, grimy part of the city of Chicago, now closed to ground traffic. It is here that the G5 Corporation has its headquarters.\n") }, // Chicago
		{ gettext_noop("G5 Building|dataDyne front corporation\n"), gettext_noop("Inside the G5 Building is a meeting room protected by anti-recording safeguards. This is the safest place for dataDyne to formulate confidential plans.\n") }, // G5 Building
		{ gettext_noop("Area 51|Near Groom Dry Lake, Nevada\n"), gettext_noop("This section of the extensive facility known as Area 51 is based in the foothills around Groom Dry Lake. Exterior helipads and communication towers hint at the size of the complex beneath ground level.\n") }, // Area 51
		{ gettext_noop("Alaskan Air Base|Brooks Range, Alaska\n"), gettext_noop("One of a series of reinforced air bases available as staging posts for Air Force One. Typically, the air base is remote, in an inhospitable region, far away from prying eyes.\n") }, // Alaskan Air Base
		{ gettext_noop("Air Force One|The President's airplane\n"), gettext_noop("This particular Air Force One is optimized for cold climates, useful when it is based in the north of Alaska. The flight destination is Oslo, Norway.\n") }, // Air Force One
		{ gettext_noop("Crash Site|Victoria Island 71N 118W\n"), gettext_noop("In the rocky snow-covered landscape of the Arctic Circle, the wreckage of the stricken plane has come to rest.\n") }, // Crash Site
		{ gettext_noop("Pelagic II|Specialized deep-sea research ship\n"), gettext_noop("Owned by the U.S. government, the Pelagic II is capable of staging deep-sea diving operations in all weather conditions. It is the only fully integrated ocean floor research vessel in the world.\n") }, // Pelagic II
		{ gettext_noop("Cetan Ship|The most alien environment on Earth\n"), gettext_noop("A huge ship of alien construction that has lain on the ocean floor for millions of years.\n") }, // Cetan Ship
		{ gettext_noop("Skedar Assault Ship|Troop carrying spacecraft\n"), gettext_noop("An interplanetary assault ship that carries Skedar warriors to their war zones. Part of the much-reduced Skedar battle fleet.\n") }, // Skedar Assault Ship
		{ gettext_noop("Skedar Homeworld|The planet of the Battle Shrine\n"), gettext_noop("A highly arid planet, racked by earthquakes, sandstorms, and hurricanes. It is part of a complex solar system that includes three suns.\n") }, // Skedar Homeworld
		{ gettext_noop("Jumpship|Agile troop craft\n"), gettext_noop("A small, fast, highly maneuverable agent-deployment craft designed for use in urban areas. Can be either computer controlled or remote piloted. It has enough room inside for three to four agents, plus equipment.\n") }, // Jumpship
		{ gettext_noop("HoverCrate|Gravity-negation device\n"), gettext_noop("An antigrav device designed to aid warehouse workers. It is attached to the side of a crate. When activated, the AG field lifts the crate and removes some of the inertia.\n") }, // HoverCrate
		{ gettext_noop("HoverBike|Low altitude vehicle\n"), gettext_noop("A low-altitude patrol bike. Uses a small AG unit to hover, then a small but powerful turbine with vectored thrust to move and provide directional control.\n") }, // HoverBike
		{ gettext_noop("Cleaning Hovbot|Your helpful buddy\n"), gettext_noop("Keeps the place clean and tidy. Tends to have access to all areas of a building - agents are advised to leave such robots intact where possible, as they can unwittingly provide a means of ingress to sensitive zones.\n") }, // Cleaning Hovbot
		{ gettext_noop("Hovercopter|Urban AG gunship\n"), gettext_noop("An urban patrol and suppression vehicle. Two-man crew, armed with a vulcan cannon on the nose pod, and two wingtip-mounted dumbfire missile pods. Can be taken out with sustained gunfire or, preferably, one well-aimed rocket.\n") }, // Hovercopter
		{ gettext_noop("G5 Robot|Urban combat droid\n"), gettext_noop("A combat robot designed for urban warfare. Uses an antigrav unit to hover; heavily armed and shielded. Often used to keep out unwanted visitors, due to the 'shoot first and don't ask questions later' programming.\n") }, // G5 Robot
		{ gettext_noop("A51 Interceptor|Robotic air interceptor\n"), gettext_noop("A robotic variant of the HoverBike, with more powerful AG and turbine units; it can reach Mach 2 with ease and yet can cruise for hours at walking pace. Although quite well armed, it relies on maneuverability rather than shielding.\n") }, // A51 Interceptor
		{ gettext_noop("Maian Vessel|Scout and patrol vessel\n"), gettext_noop("Designed to carry a single Maian pilot.  A passenger would find themselves cramped - a human passenger even more so.\n") }, // Maian Vessel
		{ gettext_noop("Skedar Shuttle|Alien troop dropship\n"), gettext_noop("Capable of carrying ten fully armed and armored Skedar warriors to battle. Undetectable by conventional radar. It can broadcast powerful jamming waves over a considerable area - these disrupt communications as well as detection equipment.\n") }, // Skedar Shuttle
#endif
	};

	switch (index) {
	case HANGARBIO_INSTITUTE:      return &bios[0];
	case HANGARBIO_DDTOWER:        return &bios[1];
	case HANGARBIO_LABBASEMENT:    return &bios[2];
	case HANGARBIO_VILLA:          return &bios[3];
	case HANGARBIO_CHICAGO:        return &bios[4];
	case HANGARBIO_G5:             return &bios[5];
	case HANGARBIO_AREA51:         return &bios[6];
	case HANGARBIO_AIRBASE:        return &bios[7];
	case HANGARBIO_AIRFORCEONE:    return &bios[8];
	case HANGARBIO_CRASHSITE:      return &bios[9];
	case HANGARBIO_PELAGIC:        return &bios[10];
	case HANGARBIO_DEEPSEA:        return &bios[11];
	case HANGARBIO_ATTACKSHIP:     return &bios[12];
	case HANGARBIO_SKEDARRUINS:    return &bios[13];
	case HANGARBIO_JUMPSHIP:       return &bios[14];
	case HANGARBIO_HOVERCRATE:     return &bios[15];
	case HANGARBIO_HOVERBIKE:      return &bios[16];
	case HANGARBIO_HOVERBOT:       return &bios[17];
	case HANGARBIO_HOVERCOPTER:    return &bios[18];
	case HANGARBIO_G5ROBOT:        return &bios[19];
	case HANGARBIO_A51INTERCEPTOR: return &bios[20];
	case HANGARBIO_MAIANVESSEL:    return &bios[21];
	case HANGARBIO_SKEDARSHUTTLE:  return &bios[22];
	}

	return NULL;
}

u8 g_DtSlot = 0;
u8 var80088adc = 0;

bool ciIsHangarBioUnlocked(u32 bioindex)
{
	u32 stage;

	switch (bioindex) {
	case HANGARBIO_INSTITUTE:
	case HANGARBIO_HOVERCRATE:
		return true;
	case HANGARBIO_DDTOWER:
		stage = SOLOSTAGEINDEX_DEFECTION;
		break;
	case HANGARBIO_LABBASEMENT:
	case HANGARBIO_HOVERBOT:
		stage = SOLOSTAGEINDEX_INVESTIGATION;
		break;
	case HANGARBIO_HOVERCOPTER:
		stage = SOLOSTAGEINDEX_EXTRACTION;
		break;
	case HANGARBIO_VILLA:
	case HANGARBIO_JUMPSHIP:
		stage = SOLOSTAGEINDEX_VILLA;
		break;
	case HANGARBIO_CHICAGO:
		stage = SOLOSTAGEINDEX_CHICAGO;
		break;
	case HANGARBIO_G5:
	case HANGARBIO_G5ROBOT:
		stage = SOLOSTAGEINDEX_G5BUILDING;
		break;
	case HANGARBIO_AREA51:
	case HANGARBIO_HOVERBIKE:
	case HANGARBIO_A51INTERCEPTOR:
		stage = SOLOSTAGEINDEX_INFILTRATION;
		break;
	case HANGARBIO_AIRBASE:
		stage = SOLOSTAGEINDEX_AIRBASE;
		break;
	case HANGARBIO_AIRFORCEONE:
		stage = SOLOSTAGEINDEX_AIRFORCEONE;
		break;
	case HANGARBIO_CRASHSITE:
	case HANGARBIO_MAIANVESSEL:
		stage = SOLOSTAGEINDEX_CRASHSITE;
		break;
	case HANGARBIO_PELAGIC:
		stage = SOLOSTAGEINDEX_PELAGIC;
		break;
	case HANGARBIO_DEEPSEA:
		stage = SOLOSTAGEINDEX_DEEPSEA;
		break;
	case HANGARBIO_ATTACKSHIP:
	case HANGARBIO_SKEDARSHUTTLE:
		stage = SOLOSTAGEINDEX_DEFENSE;
		break;
	case HANGARBIO_SKEDARRUINS:
		stage = SOLOSTAGEINDEX_ATTACKSHIP;
		break;
	default:
		return false;
	}

	return ciIsStageComplete(stage);
}

s32 ciGetNumUnlockedLocationBios(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < 23; i++) {
		if (ciIsHangarBioAVehicle(i)) {
			return count;
		}

		if (ciIsHangarBioUnlocked(i)) {
			count++;
		}
	}

	return count;
}

s32 ciGetNumUnlockedHangarBios(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < 23; i++) {
		if (ciIsHangarBioUnlocked(i)) {
			count++;
		}
	}

	return count;
}

s32 ciGetHangarBioIndexBySlot(s32 slot)
{
	s32 index = -1;
	s32 i;

	for (i = 0; i < 23; i++) {
		if (ciIsHangarBioUnlocked(i)) {
			index++;
		}

		if (index == slot) {
			return i;
		}
	}

	return 0;
}

char *ciGetHangarBioDescription(void)
{
	struct hangarbio *bio = ciGetHangarBio(ciGetHangarBioIndexBySlot(g_HangarBioSlot));
	return _(bio->description);
}

struct trainingdata *dtGetData(void)
{
	return &g_DtData;
}

void dtRestorePlayer(void)
{
	bgunSetPassiveMode(true);

	if (g_DtData.obj) {
		objFreePermanently(g_DtData.obj, true);
	}

	g_DtData.obj = NULL;

	if (dtGetWeaponByDeviceIndex(dtGetIndexBySlot(g_DtSlot)) == WEAPON_ECMMINE) {
		bgunSetAmmoQuantity(AMMOTYPE_ECM_MINE, 0);
	}

	if (g_Vars.currentplayer->eyespy) {
		struct chrdata *chr = g_Vars.currentplayer->eyespy->prop->chr;
		g_Vars.currentplayer->eyespy->deployed = false;
		g_Vars.currentplayer->eyespy->held = true;
		g_Vars.currentplayer->eyespy->active = false;

		chr->chrflags |= CHRCFLAG_HIDDEN;

		psStopSound(g_Vars.currentplayer->eyespy->prop, PSTYPE_GENERAL, 0xffff);

		g_Vars.currentplayer->devicesactive &= ~DEVICE_EYESPY;
	}
}

void dtPushEndscreen(void)
{
	if (g_DtData.completed) {
		func0f0f85e0(&g_DtCompletedMenuDialog, MENUROOT_TRAINING);
	} else if (g_DtData.failed) {
		func0f0f85e0(&g_DtFailedMenuDialog, MENUROOT_TRAINING);
	}

	g_DtData.timeleft = 0;
	g_DtData.completed = false;
	g_DtData.failed = false;
	g_DtData.finished = false;
	g_DtData.holographedpc = false;
}

void dtTick(void)
{
	if (var80088adc) {
		if (g_DtData.intraining) {
			g_DtData.timetaken += g_Vars.lvupdate60;

			if (g_Vars.currentplayer->isdead) {
				dtEnd();
			}

			if (chrHasStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_FAILURE)) {
				dtEnd();
				g_DtData.failed = true;
				g_DtData.timeleft = 1;
				g_DtData.finished = true;
			} else if (chrHasStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_SUCCESS)) {
				dtEnd();
				g_DtData.completed = true;
				g_DtData.timeleft = 1;
				g_DtData.finished = true;
#ifndef PLATFORM_N64
				filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
#endif
			}
		} else if (g_DtData.finished) {
			if (g_DtData.timeleft <= 0) {
				dtPushEndscreen();
			} else {
				g_DtData.timeleft -= g_Vars.lvupdate60;
			}
		}
	}
}

void func0f1a1ac0(void)
{
	if (var80088adc == false) {
		var80088adc = true;
		g_DtData.intraining = false;
		g_DtData.failed = false;
		g_DtData.completed = false;
		g_DtData.finished = false;
		g_DtData.timeleft = 0;
		g_DtData.holographedpc = false;
		g_DtData.timetaken = 0;
		g_DtData.obj = NULL;
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_DEVICE_ABORTING);
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_SUCCESS);
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_FAILURE);
	}
}

void dtBegin(void)
{
	g_DtData.intraining = true;
	g_DtData.timetaken = 0;
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_DEVICE_ABORTING);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_SUCCESS);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_FAILURE);
	chrSetStageFlag(NULL, ciGetStageFlagByDeviceIndex(dtGetIndexBySlot(g_DtSlot)));
	g_Vars.currentplayer->training = true;
	bgunSetPassiveMode(false);
	chrSetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
}

void dtEnd(void)
{
	g_DtData.intraining = false;
	dtRestorePlayer();
	bgunSetAmmoQuantity(AMMOTYPE_CLOAK, 0);
	chrSetStageFlag(NULL, STAGEFLAG_CI_DEVICE_ABORTING);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_DEVICE_FAILURE);
	chrUnsetStageFlag(NULL, ciGetStageFlagByDeviceIndex(dtGetIndexBySlot(g_DtSlot)));
	g_Vars.currentplayer->training = false;
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
	playerDisplayHealth();
	g_Vars.currentplayer->bondhealth = 1;
}

bool dtIsAvailable(s32 deviceindex)
{
	u8 flags[] = {
		GAMEFILEFLAG_CI_UPLINK_DONE,
		GAMEFILEFLAG_CI_ECMMINE_DONE,
		GAMEFILEFLAG_CI_CAMSPY_DONE,
		GAMEFILEFLAG_CI_NIGHTVISION_DONE,
		GAMEFILEFLAG_CI_DOORDECODER_DONE,
		GAMEFILEFLAG_CI_RTRACKER_DONE,
		GAMEFILEFLAG_CI_IR_DONE,
		GAMEFILEFLAG_CI_XRAY_DONE,
		GAMEFILEFLAG_CI_DISGUISE_DONE,
		GAMEFILEFLAG_CI_CLOAK_DONE,
	};

	deviceindex--;

	if (deviceindex >= ARRAYCOUNT(flags)) {
		return true;
	}

	if (deviceindex < 0 || gamefileHasFlag(flags[deviceindex])) {
		return true;
	}

	return false;
}

s32 dtGetNumAvailable(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < NUM_DEVICETESTS; i++) {
		if (dtIsAvailable(i)) {
			count++;
		}
	}

	return count;
}

s32 dtGetIndexBySlot(s32 wantindex)
{
	s32 index = -1;
	s32 i;

	for (i = 0; i < NUM_DEVICETESTS; i++) {
		if (dtIsAvailable(i)) {
			index++;
		}

		if (index == wantindex) {
			return i;
		}
	}

	return 0;
}

u32 dtGetWeaponByDeviceIndex(s32 deviceindex)
{
	u32 weapons[] = {
		WEAPON_DATAUPLINK,
		WEAPON_ECMMINE,
		WEAPON_EYESPY,
		WEAPON_NIGHTVISION,
		WEAPON_DOORDECODER,
		WEAPON_RTRACKER,
		WEAPON_IRSCANNER,
		WEAPON_XRAYSCANNER,
		WEAPON_DISGUISE41,
		WEAPON_CLOAKINGDEVICE,
	};

	return weapons[deviceindex];
}

u32 ciGetStageFlagByDeviceIndex(u32 deviceindex)
{
	u32 flags[] = {
		STAGEFLAG_CI_TRIGGER_UPLINK,
		STAGEFLAG_CI_TRIGGER_ECMMINE,
		STAGEFLAG_CI_TRIGGER_CAMSPY,
		STAGEFLAG_CI_TRIGGER_NIGHTVISION,
		STAGEFLAG_CI_TRIGGER_DOORDECODER,
		STAGEFLAG_CI_TRIGGER_RTRACKER,
		STAGEFLAG_CI_TRIGGER_IR,
		STAGEFLAG_CI_TRIGGER_XRAY,
		STAGEFLAG_CI_TRIGGER_DISGUISE,
		STAGEFLAG_CI_TRIGGER_CLOAK,
	};

	return flags[deviceindex];
}

char *dtGetDescription(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		/*0*/ L_DISH_186, // Data uplink
		/*1*/ L_DISH_185, // ECM mine
		/*2*/ L_DISH_177, // CamSpy
		/*3*/ L_DISH_178, // Night vision
		/*4*/ L_DISH_179, // Door decoder
		/*5*/ L_DISH_183, // R-tracker
		/*6*/ L_DISH_182, // IR scanner
		/*7*/ L_DISH_180, // X-ray scanner
		/*8*/ L_DISH_181, // Disguise
		/*9*/ L_DISH_184, // Cloak
#else
		/*0*/ _("Description\n- Provides a link from the field agent to the Institute hackers back at HQ, who can then download data or crack electronic locks remotely.\n\nTraining Instructions\n- Use the Data Uplink to hack the terminal in the corner, unlocking a secret door.\n\nOperation\n- Stand next to the terminal and press the B Button when holding Data Uplink.\n"), // Data uplink
		/*1*/ _("Description\n- Emits a constantly shifting signal designed to jam any electronic communications device. Must be placed on the object to be effective.\n\nTraining Instructions\n- Throw the ECM Mine onto the lighting hub located through the secret door.\n\nOperation\n- Press the Z Button to throw the mine. Hold down the R Button and move the Control Stick to fine-tune your aim before throwing.\n"), // ECM mine
		/*2*/ _("Description\n- A tiny remote camera for stealthy exploration. Equipped for spectroscopic holography. Opens doors by projecting a human-sized pulse of heat.\n\nTraining Instructions\n- Holograph the hacker's terminal next door in the Info room.\n\nOperation\n- Press the Z Button to take a holograph. Pressing the B Button will open any doors in the way. Hold down the R Button to look around.\n"), // CamSpy
		/*3*/ _("Description\n- Enhances any visible light to produce an image of the surrounding area. Also highlights life forms. Overloads in normal light conditons, 'whiting out' the display.\n\nTraining Instructions\n- Head into the darkness, find the light switch, and activate it to turn the lights back on.\n\nOperation\n- Select the Night Vision from your inventory to activate it. Reselect to deactivate.\n"), // Night vision
		/*4*/ _("Description\n\n- Stand-alone code-breaking device. Attaches to the control panel and sifts through the possible combinations until the lock is opened.\n\nTraining Instructions\n- Find the pad by the locked door and use the Door Decoder on it to unlock the door.\n\nOperation\n- Stand next to the door pad and press the B Button while holding the Door Decoder to use it.\n"), // Door decoder
		/*5*/ _("Description\n- Locates a particular object on a HUD radar map. Shows the relative bearing and distance.\n\nTraining Instructions\n- Activate the Tracker and follow the radar signature to retrieve the item.\n\nOperation\n- Selecting the Tracker from your inventory will activate it. Reselect to deactivate.\n"), // R-tracker
		/*6*/ _("Description\n\n- Translates thermal data into visible images. Can be used in darkness and will also reveal anomalies such as hidden doors and weak wall sections.\n\nTraining Instructions\n- Turning on the IR Scanner, find the hidden door and open it.\n\nOperation\n- Select the IR Scanner from your inventory to activate it. Reselect to deactivate.\n"), // IR scanner
		/*7*/ _("Description\n\n- Used to look through otherwise solid walls and objects. Can see things that the Night Vision and IR Scanner cannot.\n\nTraining Instructions\n- With the X-Ray Scanner on, search for the two hidden switches and activate them to turn off the laser grid.\n\nOperation\n- Select the X-Ray Scanner from your inventory to activate it. Reselect to deactivate.\n"), // X-ray scanner
		/*8*/ _("Description\n\n- Allay suspicion by the use of a disguise. But, always be alert for the possibility of being unmasked by a quick-witted enemy.\n\nTraining Instructions\n- Grimshaw has a Cloaking Device waiting to be serviced. Head next door and 'acquire' it from him.\n\nOperation\n- To wear the disguise, simply select it from your inventory.\n"), // Disguise
		/*9*/ _("Description\n- Disrupts the visible spectrum of light around the wearer, creating an almost perfect chameleonlike effect. This field is disrupted when the wearer fires.\n\nTraining Instructions\n- Activate the Cloaking Device and head to Carrington's office to surprise him!\n\nOperation\n- Selecting the Cloaking Device from your inventory will activate it. Reselect to deactivate.\n"), // Cloak
#endif
	};

	return texts[dtGetIndexBySlot(g_DtSlot)];
}

char *dtGetTip1(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		/*0*/ L_DISH_263,
		/*1*/ L_DISH_264,
		/*2*/ L_DISH_265,
		/*3*/ L_DISH_266,
		/*4*/ L_DISH_267,
		/*5*/ L_DISH_268,
		/*6*/ L_DISH_269,
		/*7*/ L_DISH_270,
		/*8*/ L_DISH_271,
		/*9*/ L_DISH_272,
#else
		/*0*/ _("Always keep the target object in view when using the Data Uplink, and stay close to prevent the connection from being severed.\n"),
		/*1*/ _("Be sure the mine will land in the correct place by getting as close as you can to the target. Adjust your aim using the C Buttons or the R Button Aiming mode.\n"),
		/*2*/ _("The CamSpy can be difficult to control, so take your time.\n"),
		/*3*/ _("Useful in combat, but can cause problems when overloading.\n"),
		/*4*/ _("If you are having trouble attaching the Decoder, try standing closer to the target and facing it.\n"),
		/*5*/ _("The Tracker only indicates direction and relative height; it doesn't display a map guiding you to the target. Pay attention to your surroundings, and be prepared to explore.\n"),
		/*6*/ _("The visor narrows your peripheral vision, which can cause problems in combat. Be sure of your situation before you use it.\n"),
		/*7*/ _("If you use this device at the wrong time, it could cost you your life, since it can prevent you from seeing anything beyond a certain distance. Use sparingly.\n"),
		/*8*/ _("A disguise is not just the clothing, it is the manner of the person wearing it. Don't behave out of character for the person you are trying to be.\n"),
		/*9*/ _("Preserve the supply of the Cloaking Device when there is no one around to observe you.\n"),
#endif
	};

	return texts[dtGetIndexBySlot(g_DtSlot)];
}

char *dtGetTip2(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		/*0*/ L_DISH_273,
		/*1*/ L_DISH_274,
		/*2*/ L_DISH_275,
		/*3*/ L_DISH_276,
		/*4*/ L_DISH_277,
		/*5*/ L_DISH_278,
		/*6*/ L_DISH_279,
		/*7*/ L_DISH_280,
		/*8*/ L_DISH_281,
		/*9*/ L_DISH_282,
#else
		/*0*/ _("Always keep the target object in view when using the Data Uplink, and stay close to prevent the connection from being severed.\n"),
		/*1*/ _("Be sure the mine will land in the correct place by getting as close as you can to the target. Adjust your aim using the C Buttons or the R Button Aiming mode.\n"),
		/*2*/ _("The CamSpy is not invisible - enemies may spot it - so be careful when entering an inhabited area.  Remember that the CamSpy will remain where you left it unless you pick it up.\n"),
		/*3*/ _("Useful in combat, but can cause problems when overloading. Try to anticipate such situations and react before the enemy.\n"),
		/*4*/ _("If you are having trouble attaching the Decoder, try standing closer to the target and facing it.\n"),
		/*5*/ _("The Tracker only indicates direction and relative height; it doesn't display a map guiding you to the target. Pay attention to your surroundings, and be prepared to explore.\n"),
		/*6*/ _("The visor narrows your peripheral vision, which can cause problems in combat. Be sure of your situation before you use it.\n"),
		/*7*/ _("If you use this device at the wrong time, it could cost you your life, since it can prevent you from seeing anything beyond a certain distance. Use sparingly.\n"),
		/*8*/ _("A disguise is not just the clothing, it is the manner of the person wearing it. Don't behave out of character for the person you are trying to be.\n"),
		/*9*/ _("Preserve the supply of the Cloaking Device when there is no one around to observe you. Avoid firing unless absolutely necessary and until you are assured of the success of your attack.\n"),
#endif
	};

	return texts[dtGetIndexBySlot(g_DtSlot)];
}

struct trainingdata *getHoloTrainingData(void)
{
	return &g_HtData;
}

void htPushEndscreen(void)
{
	if (g_HtData.completed) {
		func0f0f85e0(&g_HtCompletedMenuDialog, MENUROOT_TRAINING);
	} else if (g_HtData.failed) {
		func0f0f85e0(&g_HtFailedMenuDialog, MENUROOT_TRAINING);
	}

	g_HtData.timeleft = 0;
	g_HtData.completed = false;
	g_HtData.failed = false;
	g_HtData.finished = false;
}

u8 var80088bb4 = 0;
u8 var80088bb8 = 0;

void htTick(void)
{
	if (var80088bb8) {
		if (g_HtData.intraining) {
			g_HtData.timetaken += g_Vars.lvupdate60;

			if (g_Vars.currentplayer->isdead) {
				htEnd();
			}

			if (chrHasStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_FAILURE)) {
				htEnd();
				g_HtData.failed = true;
				g_HtData.timeleft = 1;
				g_HtData.finished = true;
			} else if (chrHasStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_SUCCESS)) {
				htEnd();
				g_HtData.completed = true;
				g_HtData.timeleft = 1;
				g_HtData.finished = true;
#ifndef PLATFORM_N64
				filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
#endif
			}
		} else if (g_HtData.finished) {
			if (g_HtData.timeleft <= 0) {
				htPushEndscreen();
			} else {
				g_HtData.timeleft -= g_Vars.lvupdate60;
			}
		}
	}
}

void func0f1a2198(void)
{
	if (var80088bb8 == false) {
		var80088bb8 = true;
		g_HtData.intraining = false;
		g_HtData.failed = false;
		g_HtData.completed = false;
		g_HtData.finished = false;
		g_HtData.timeleft = 0;
		g_HtData.timetaken = 0;
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_HOLO_ABORTING);
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_SUCCESS);
		chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_FAILURE);
	}
}

void htBegin(void)
{
	struct waypoint *waypoints = g_StageSetup.waypoints;

	g_HtData.intraining = true;
	g_HtData.timetaken = 0;
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_HOLO_ABORTING);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_SUCCESS);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_FAILURE);
	chrSetStageFlag(NULL, func0f1a25c0(htGetIndexBySlot(var80088bb4)));

	// Disable segment leading out of the door
	navDisableSegment(&waypoints[0x20], &waypoints[0x31]);

	g_Vars.currentplayer->training = true;
	bgunSetPassiveMode(false);
	chrSetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
}

void htEnd(void)
{
	struct prop *prop;
	s16 *propnum;
	s16 propnums[256];
	RoomNum rooms[5] = { 0x0016, 0x0017, 0x0018, 0x0019, -1 };
	struct waypoint *waypoints = g_StageSetup.waypoints;

	g_HtData.intraining = false;
	chrSetStageFlag(NULL, STAGEFLAG_CI_HOLO_ABORTING);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_TRIGGER_HOLO_FAILURE);
	chrUnsetStageFlag(NULL, func0f1a25c0(htGetIndexBySlot(var80088bb4)));

	// Enable segment leading out of the door
	navEnableSegment(&waypoints[0x20], &waypoints[0x31]);

	g_Vars.currentplayer->training = false;
	roomGetProps(rooms, propnums, 256);
	propnum = &propnums[0];

	// Remove dropped weapons
	while (*propnum >= 0) {
		prop = &g_Vars.props[*propnum];

		if (prop && prop->type == PROPTYPE_WEAPON) {
			struct defaultobj *obj = prop->obj;

			if (obj->type == OBJTYPE_WEAPON) {
				objFreePermanently(obj, true);
			}
		}

		propnum++;
	}

	bgunSetPassiveMode(true);
	chrUnsetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
	playerDisplayHealth();
	g_Vars.currentplayer->bondhealth = 1;
}

bool htIsUnlocked(u32 value)
{
	switch (value) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		return true;
	}

	return false;
}

s32 htGetNumUnlocked(void)
{
	s32 count = 0;
	s32 i;

	for (i = 0; i < NUM_HOLOTESTS; i++) {
		if (htIsUnlocked(i)) {
			count++;
		}
	}

	return count;
}

s32 htGetIndexBySlot(s32 slot)
{
	s32 index = -1;
	s32 i;

	for (i = 0; i < NUM_HOLOTESTS; i++) {
		if (htIsUnlocked(i)) {
			index++;
		}

		if (index == slot) {
			return i;
		}
	}

	return 0;
}

char *htGetName(s32 index)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		L_DISH_316, // "Holo 1 - Looking Around"
		L_DISH_317, // "Holo 2 - Movement 1"
		L_DISH_318, // "Holo 3 - Movement 2"
		L_DISH_319, // "Holo 4 - Unarmed Combat 1"
		L_DISH_320, // "Holo 5 - Unarmed Combat 2"
		L_DISH_321, // "Holo 6 - Live Combat 1"
		L_DISH_322, // "Holo 7 - Live Combat 2"
#else
		_("Holo 1 - Looking Around\n"), // "Holo 1 - Looking Around"
		_("Holo 2 - Movement 1\n"), // "Holo 2 - Movement 1"
		_("Holo 3 - Movement 2\n"), // "Holo 3 - Movement 2"
		_("Holo 4 - Unarmed Combat 1\n"), // "Holo 4 - Unarmed Combat 1"
		_("Holo 5 - Unarmed Combat 2\n"), // "Holo 5 - Unarmed Combat 2"
		_("Holo 6 - Live Combat 1\n"), // "Holo 6 - Live Combat 1"
		_("Holo 7 - Live Combat 2\n"), // "Holo 7 - Live Combat 2"
#endif
	};

	return texts[index];
}

u32 func0f1a25c0(s32 index)
{
	u32 flags[] = {
		STAGEFLAG_CI_IN_HOLO1,
		STAGEFLAG_CI_IN_HOLO2,
		STAGEFLAG_CI_IN_HOLO3,
		STAGEFLAG_CI_IN_HOLO4,
		STAGEFLAG_CI_IN_HOLO5,
		STAGEFLAG_CI_IN_HOLO6,
		STAGEFLAG_CI_IN_HOLO7,
		STAGEFLAG_CI_GENERAL_PURPOSE,
	};

	return flags[index];
}

char *htGetDescription(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		L_DISH_242,
		L_DISH_243,
		L_DISH_244,
		L_DISH_245,
		L_DISH_246,
		L_DISH_247,
		L_DISH_248,
#else
		_("Description\n- A good agent notices everything around him or her.\n\nTraining Instructions\n- Examine all of the objects by looking directly at them.\n\nOperation\n-Look up- Press the Down C Button\n-Look down- Press the Up C Button\n-Free Look- Hold down the R Button to enter Aim mode, and use the Control Stick to look.\n"),
		_("Description\n- Sidestepping and strafing can get you out of trouble.\n\nTraining Instructions\n- Activate all of the switches in front. Be quick, though, as each switch will reset on a time limit.\n\nOperation\n- Sidestep left- Left C Button\n- Sidestep right- Right C Button\n\n"),
		_("Description\n- Ducking and crouching can open up new areas for exploration.\n\nTraining Instructions\n- Work your way through the obstacles using the moves available to you. Activate the switches.\n\nOperation\n- Duck- Hold the R Button, then press the Down C Button.\n- Crouch- When ducking, hold the R Button, then press the Down C Button.\n- Get up- Tap the R Button.\n\n"),
		_("Description\n- Fighting multiple opponents in hand-to-hand combat can be difficult.\n\nTraining Instructions\n- Knock out all enemies without getting hit.\n\nOperation\n- Punch - Press the Z Button repeatedly when unarmed to launch a flurry of punches.\n\n"),
		_("Description\n- Disarming an enemy can get you a new weapon.\n\nTraining Instructions\n- Knock out/disarm all enemies without getting hit.\n\nOperation\n- Disarm - Hold the B Button, then press the Z Button to disarm an enemy.\n- Knockout Punch - Punch an unaware enemy from behind.\n"),
		_("Description\n- Fighting multiple opponents in hand-to-hand combat.\n\nTraining Instructions\n- Beat all the unarmed enemies without getting hit.\n\n"),
		_("Description\n- Fighting multiple opponents, both armed and unarmed.\n\nTraining Instructions\n- Beat all the armed enemies without getting hit.\n\n"),
#endif
	};

	return texts[htGetIndexBySlot(var80088bb4)];
}

char *htGetTip1(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		L_DISH_249, // "For greater precision..."
		L_DISH_250, // "Think about where you want to go..."
		L_DISH_251, // "Ducking enables you to..."
		L_DISH_252, // "Attacking opponents from behind..."
		L_DISH_253, // "Only stay close long enough..."
		L_DISH_254, // "Don't hang around and wait..."
		L_DISH_255, // "Go for the armed opponents..."
#else
		_("For greater precision and freedom when looking around, use the Aiming mode.\n"), // "For greater precision..."
		_("Think about where you want to go before attempting the test. Set yourself up in a place that will require the least amount of movement between targets.\n"), // "Think about where you want to go..."
		_("Ducking enables you to reach places normally inaccessible to you, but remember that you move quickest when standing upright. To move through the test as fast as possible, try ducking only when you have to.\n"), // "Ducking enables you to..."
		_("Attacking opponents from behind may be dishonorable, but doing so makes it easier to take them down. Only close to fighting distance when you have to, and be prepared to step back out of range when they attack you.\n"), // "Attacking opponents from behind..."
		_("Only stay close long enough to grab the weapon. Back out of range of any retribution and use your new weapon.\n"), // "Only stay close long enough..."
		_("Don't hang around and wait to get hit, move! Don't focus on one opponent; try to be aware of where all of them are. If you can't see all of them, move until you can.\n"), // "Don't hang around and wait..."
		_("Go for the armed opponents once you are confident of your hand-to-hand skills. Back away from the disarmed enemies and use their weapons on them before they get too close to you.\n"), // "Go for the armed opponents..."
#endif
	};

	return texts[htGetIndexBySlot(var80088bb4)];
}

char *htGetTip2(void)
{
	char *texts[] = {
#if VERSION >= VERSION_PAL_BETA
		L_DISH_256, // "For greater precision..."
		L_DISH_257, // "Sidestepping and strafing..."
		L_DISH_258, // "Ducking enables you to..."
		L_DISH_259, // "Attacking opponents from behind..."
		L_DISH_260, // "Only stay close long enough..."
		L_DISH_261, // "Don't hang around and wait..."
		L_DISH_262, // "Go for the armed opponents..."
#else
		_("For greater precision and freedom when looking around, use the Aiming mode.\n"), // "For greater precision..."
		_("Sidestepping and strafing can get you out of trouble.\n"), // "Sidestepping and strafing..."
		_("Ducking enables you to reach places normally inaccessible to you, but remember that you move quickest when standing upright. To move as fast as possible, try ducking only when you have to.\n"), // "Ducking enables you to..."
		_("Attacking opponents from behind may be dishonorable, but doing so makes it easier to take them down. Only close to fighting distance when you have to, and be prepared to step back out of range when they attack you.\n"), // "Attacking opponents from behind..."
		_("Only stay close long enough to grab the weapon. Back out of range of any retribution and use your new weapon.\n"), // "Only stay close long enough..."
		_("Don't hang around and wait to get hit; move! Don't focus on one opponent; try to be aware of where all of them are. If you can't see all of them, move until you can.\n"), // "Don't hang around and wait..."
		_("Go for the armed opponents once you are confident of your hand-to-hand skills. Back away from the disarmed enemies and use their weapons on them before they get too close to you.\n"), // "Go for the armed opponents..."
#endif
	};

	return texts[htGetIndexBySlot(var80088bb4)];
}

#if VERSION >= VERSION_JPN_FINAL
void frGetGoalTargetsText(char *buffer, char *buffer2)
{
	sprintf(buffer, "%s", langGet(gettext_noop("GOAL TARGETS:")));
	sprintf(buffer2, "%d\n", g_FrData.goaltargets);
}
#else
void frGetGoalTargetsText(char *buffer)
{
	// "GOAL TARGETS:"
	sprintf(buffer, "%s %d\n", _("GOAL TARGETS:"), g_FrData.goaltargets);
}
#endif

void frGetTargetsDestroyedValue(char *buffer)
{
	sprintf(buffer, "%02d\n", g_FrData.targetsdestroyed);
}

void frGetScoreValue(char *buffer)
{
	sprintf(buffer, "%03d\n", g_FrData.score);
}

#if VERSION >= VERSION_JPN_FINAL
void frGetGoalScoreText(char *buffer1, char *buffer2)
{
	if (g_FrData.goalscore) {
		sprintf(buffer1, "%s", _("GOAL SCORE:"));
		sprintf(buffer2, "%d\n", g_FrData.goalscore);
	} else {
		sprintf(buffer1, "");
		sprintf(buffer2, "");
	}
}
#else
void frGetGoalScoreText(char *buffer)
{
	if (g_FrData.goalscore) {
		// "GOAL SCORE:"
		sprintf(buffer, "%s %d\n", _("GOAL SCORE:"), g_FrData.goalscore);
	} else {
		sprintf(buffer, "");
	}
}
#endif

f32 frGetAccuracy(char *buffer)
{
	f32 sum = (g_FrData.numhitsring3
		+ g_FrData.numhitsbullseye
		+ g_FrData.numhitsring1
		+ g_FrData.numhitsring2) * 100.0f;
	f32 accuracy = 100.0f;

	if (g_FrData.numshots) {
		accuracy = sum / (f32)g_FrData.numshots;
	}

	if (accuracy > 100.0f) {
		accuracy = 100.0f;
	}

	sprintf(buffer, "%s%s%.2f%%\n", "", "", accuracy);

	return accuracy;
}

#if VERSION >= VERSION_JPN_FINAL
bool frGetMinAccuracy(char *buffer1, f32 accuracy, char *buffer2)
{
	sprintf(buffer1, "%s", _("MIN ACCURACY:"));
	sprintf(buffer2, "%d%%\n", g_FrData.goalaccuracy);

	return accuracy < g_FrData.goalaccuracy;
}
#else
bool frGetMinAccuracy(char *buffer, f32 accuracy)
{
	// "MIN ACCURACY:"
	sprintf(buffer, "%s %d%%\n", _("MIN ACCURACY:"), g_FrData.goalaccuracy);

	return accuracy < g_FrData.goalaccuracy;
}
#endif

/**
 * Formats either the time taken or time limit into buffer, and returns true if
 * the time should induce a failure.
 *
 * The time limit will be used if it exists and the time take exceeds it,
 * otherwise time taken will be used.
 *
 * Negative time taken (such as when the player aborts before the challenge
 * starts) is wrapped to positive and will induce a failure.
 */
bool frFormatTime(char *buffer)
{
	s32 mins = 0;
	s32 mult = 1;
	f32 secs = g_FrData.timetaken / TICKS(60.0f);
	u8 failed = false;

	if (g_FrData.timelimit != 255 && secs >= g_FrData.timelimit) {
		failed = true;
		secs = g_FrData.timelimit;
	} else if (g_FrData.timetaken < 0) {
		failed = true;
	}

	if (secs < 0) {
		mult = -1;
		secs = -secs;
	}

	if (secs >= 60) {
		while (secs >= 60) {
			secs -= 60;
			mins++;
		}
	}

	sprintf(buffer, "%02d:%02d\n", mult * mins, (s32)secs);

	return failed;
}

#if VERSION >= VERSION_JPN_FINAL
bool frGetHudMiddleSubtext(char *buffer1, char *buffer2)
{
	s32 secs;
	s32 mins;

	sprintf(buffer2, "");

	if (g_FrData.timetaken < TICKS(-180)) {
		sprintf(buffer1, "%s", _("FIRE TO START\n")); // "FIRE TO START"
		return false;
	}

	if (g_FrData.timetaken < 0) {
		sprintf(buffer1, "%s", _("GET READY!\n")); // "GET READY!"
		return true;
	}

	if (g_FrData.timelimit == 255) {
		return false;
	}

	secs = g_FrData.timelimit;
	mins = 0;

	if (secs >= 60) {
		while (secs >= 60) {
			secs -= 60;
			mins++;
		}
	}

	sprintf(buffer1, "%s", _("LIMIT:")); // "LIMIT:"
	sprintf(buffer2, "%02d:%02d\n", mins, secs);
	return true;
}
#else
bool frGetHudMiddleSubtext(char *buffer)
{
	s32 secs;
	s32 mins;

	if (g_FrData.timetaken < TICKS(-180)) {
		sprintf(buffer, "%s", _("FIRE TO START\n")); // "FIRE TO START"
		return false;
	}

	if (g_FrData.timetaken < 0) {
		sprintf(buffer, "%s", _("GET READY!\n")); // "GET READY!"
		return true;
	}

	if (g_FrData.timelimit == 255) {
		return false;
	}

	secs = g_FrData.timelimit;
	mins = 0;

	if (secs >= 60) {
		while (secs >= 60) {
			secs -= 60;
			mins++;
		}
	}

	sprintf(buffer, "%s %02d:%02d\n", _("LIMIT:"), mins, secs); // "LIMIT:"
	return true;
}
#endif

#if VERSION >= VERSION_JPN_FINAL
bool frGetFeedback(char *scorebuffer, char *zonebuffer, char *extrabuffer)
{
	u32 texts[] = {
		gettext_noop("ZONE 3\n"), // "ZONE 3"
		gettext_noop("ZONE 2\n"), // "ZONE 2"
		gettext_noop("ZONE 1\n"), // "ZONE 1"
		gettext_noop("BULL'S-EYE\n"), // "BULL'S-EYE"
		gettext_noop("EXPLODED\n"), // "EXPLODED"
	};

	sprintf(extrabuffer, "");

	if (g_FrData.feedbackzone) {
		g_FrData.feedbackttl -= g_Vars.lvupdate60;

		if (g_FrData.feedbackttl <= 0) {
			g_FrData.feedbackzone = 0;
			g_FrData.feedbackttl = 0;
			return false;
		}

		if (g_FrData.feedbackzone == FRZONE_EXPLODE) {
			sprintf(scorebuffer, "010\n");
		} else {
			sprintf(scorebuffer, "%03d\n", g_FrData.feedbackzone);
		}

		switch (g_FrData.feedbackzone) {
		case FRZONE_RING3:
			sprintf(zonebuffer, "%s", langGet(texts[0]));
			return true;
		case FRZONE_RING2:
			sprintf(zonebuffer, "%s", langGet(texts[1]));
			return true;
		case FRZONE_RING1:
			sprintf(zonebuffer, "%s", langGet(texts[2]));
			return true;
		case FRZONE_BULLSEYE:
			sprintf(zonebuffer, "%s", langGet(texts[3]));
			return true;
		case FRZONE_EXPLODE:
			sprintf(zonebuffer, "%s", langGet(texts[4]));
			return true;
		}

		sprintf(zonebuffer, "\n");
		return true;
	}

	return false;
}
#else
bool frGetFeedback(char *scorebuffer, char *zonebuffer)
{
	char *texts[] = {
		_("ZONE 3\n"), // "ZONE 3"
		_("ZONE 2\n"), // "ZONE 2"
		_("ZONE 1\n"), // "ZONE 1"
		_("BULL'S-EYE\n"), // "BULL'S-EYE"
		_("EXPLODED\n"), // "EXPLODED"
	};

	if (g_FrData.feedbackzone) {
		g_FrData.feedbackttl -= g_Vars.lvupdate60;

		if (g_FrData.feedbackttl <= 0) {
			g_FrData.feedbackzone = 0;
			g_FrData.feedbackttl = 0;
			return false;
		}

		if (g_FrData.feedbackzone == FRZONE_EXPLODE) {
			sprintf(scorebuffer, "010\n");
		} else {
			sprintf(scorebuffer, "%03d\n", g_FrData.feedbackzone);
		}

		switch (g_FrData.feedbackzone) {
		case FRZONE_RING3:
			sprintf(zonebuffer, "%s", texts[0]);
			return true;
		case FRZONE_RING2:
			sprintf(zonebuffer, "%s", texts[1]);
			return true;
		case FRZONE_RING1:
			sprintf(zonebuffer, "%s", texts[2]);
			return true;
		case FRZONE_BULLSEYE:
			sprintf(zonebuffer, "%s", texts[3]);
			return true;
		case FRZONE_EXPLODE:
			sprintf(zonebuffer, "%s", texts[4]);
			return true;
		}

		sprintf(zonebuffer, "\n");
		return true;
	}

	return false;
}
#endif

#if VERSION >= VERSION_JPN_FINAL
Gfx *frRenderHudElement(Gfx *gdl, s32 x, s32 y, char *string1, char *string2, char *string3, u32 colour, u8 alpha)
#else
Gfx *frRenderHudElement(Gfx *gdl, s32 x, s32 y, char *string1, char *string2, u32 colour, u8 alpha)
#endif
{
	s32 textheight;
	s32 textwidth;
	s32 x2;
	s32 y2;

	u32 halfalpha = alpha >> 1;
	u32 fullcolour = (colour & 0xffffff00) | alpha;

	textMeasure(&textheight, &textwidth, string1, g_CharsHandelGothicMd, g_FontHandelGothicMd, 0);

	x2 = x - (textwidth >> 1);
	y2 = y;
	gdl = text0f153858(gdl, &x2, &y2, &textwidth, &textheight);

#if VERSION >= VERSION_JPN_FINAL
	gdl = func0f1574d0jf(gdl, &x2, &y2, string1,
			g_CharsHandelGothicMd, g_FontHandelGothicMd, fullcolour, halfalpha, viGetWidth(), viGetHeight(), 0, 0);

	if (string2) {
		s32 textheight2;
		s32 textwidth2;
		s32 textheight3;
		s32 textwidth3;

		textMeasure(&textheight2, &textwidth2, string2, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
		textMeasure(&textheight3, &textwidth3, string3, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		textheight = textheight2;
		textwidth = textwidth2 + textwidth3;
		x2 = x - (textwidth >> 1);
		y2 = y;
		y2 += 17;

		gdl = text0f153858(gdl, &x2, &y2, &textwidth, &textheight);

		gdl = func0f1574d0jf(gdl, &x2, &y2, string2,
			g_CharsHandelGothicSm, g_FontHandelGothicSm, fullcolour, halfalpha, viGetWidth(), viGetHeight(), 0, 0);

		y2 = y;
		y2 += 17;
		y2++;
		x2 -= 4;

		gdl = func0f1574d0jf(gdl, &x2, &y2, string3,
			g_CharsHandelGothicSm, g_FontHandelGothicSm, fullcolour, halfalpha, viGetWidth(), viGetHeight(), 0, 0);
	}
#else
	gdl = textRender(gdl, &x2, &y2, string1,
			g_CharsHandelGothicMd, g_FontHandelGothicMd, fullcolour, halfalpha, viGetWidth(), viGetHeight(), 0, 0);

	if (string2) {
		textMeasure(&textheight, &textwidth, string2, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		x2 = x - (textwidth >> 1);
		y2 = y + 17;
		gdl = text0f153858(gdl, &x2, &y2, &textwidth, &textheight);

		gdl = textRender(gdl, &x2, &y2, string2,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, fullcolour, halfalpha, viGetWidth(), viGetHeight(), 0, 0);
	}
#endif

	return gdl;
}

#if VERSION >= VERSION_JPN_FINAL
Gfx *frRenderHud(Gfx *gdl)
{
	char string1[128];
	char string2[128];
	char string3[128];
	bool red;
	bool exists;
	s32 alpha = 0xa0;
	f32 mult;

	if (viGetViewWidth() > 400) {
		mult = 1.7f;
	} else {
		mult = 1;
	}

	if (!g_FrIsValidWeapon && g_FrData.menucountdown <= 0) {
		return gdl;
	}

	if (g_FrData.menucountdown != 0) {
		alpha = (f32)(g_FrData.menucountdown * 160) / TICKS(60.0f);
	}

	gdl = text0f153628(gdl);

	// Time
	red = frFormatTime(string1);
	exists = frGetHudMiddleSubtext(string2, string3);

	gdl = frRenderHudElement(gdl, viGetViewWidth() >> 1, viGetViewTop() + 12,
			string1,
			exists ? string2 : NULL,
			exists ? string3 : NULL,
			red ? 0xff4444ff : 0x00ff00a0,
			alpha);

	// Score
	frGetScoreValue(string1);
	frGetGoalScoreText(string2, string3);
	gdl = frRenderHudElement(gdl, viGetViewLeft() + 65.0f * mult, viGetViewTop() + 12,
			string1, string2, string3, 0x00ff00a0, alpha);

	// Feedback
	if (frGetFeedback(string1, string2, string3)) {
		gdl = frRenderHudElement(gdl,viGetViewLeft() + 65.0f * mult, viGetViewTop() + 48,
				string1, string2, string3, 0x00ff00a0, alpha);
	}

	if (g_FrData.goalaccuracy > 0) {
		red = frGetMinAccuracy(string2, frGetAccuracy(string1), string3);

		gdl = frRenderHudElement(gdl, viGetViewLeft() + viGetViewWidth() - 70.0f * mult, viGetViewTop() + 12,
				string1, string2, string3,
				red ? 0xff4444ff : 0x00ff00a0,
				alpha);
	} else if (g_FrData.goaltargets != 255) {
		frGetTargetsDestroyedValue(string1);
		frGetGoalTargetsText(string2, string3);

		if (mult == 2) {
			mult = 2.4;
		}

		gdl = frRenderHudElement(gdl, viGetViewLeft() + viGetViewWidth() - 70.0f * mult, viGetViewTop() + 12,
				string1, string2, string3, 0x00ff00a0, alpha);
	}

	return text0f153780(gdl);
}
#else
Gfx *frRenderHud(Gfx *gdl)
{
	char string1[128];
	char string2[128];
	bool red;
	bool exists;
	s32 alpha = 0xa0;
	f32 mult;

	if (viGetViewWidth() > (VERSION >= VERSION_PAL_FINAL ? 330 : 400)) {
		mult = VERSION >= VERSION_PAL_FINAL ? 1.5f : 2;
	} else {
		mult = 1;
	}

	if (!g_FrIsValidWeapon && g_FrData.menucountdown <= 0) {
		return gdl;
	}

	if (g_FrData.menucountdown != 0) {
		alpha = (f32)(g_FrData.menucountdown * 160) / TICKS(60.0f);
	}

	gdl = text0f153628(gdl);

	// Time
	red = frFormatTime(string1);
	exists = frGetHudMiddleSubtext(string2);

	gdl = frRenderHudElement(gdl, viGetViewWidth() >> 1, viGetViewTop() + 12,
			string1, exists ? string2 : NULL,
			red ? 0xff0000a0 : 0x00ff00a0,
			alpha);

	// Score
	frGetScoreValue(string1);
	frGetGoalScoreText(string2);
	gdl = frRenderHudElement(gdl, viGetViewLeft() + 65.0f * mult, viGetViewTop() + 12,
			string1, string2, 0x00ff00a0, alpha);

	// Feedback
	if (frGetFeedback(string1, string2)) {
		gdl = frRenderHudElement(gdl,viGetViewLeft() + 65.0f * mult, viGetViewTop() + 40,
				string1, string2, 0x00ff00a0, alpha);
	}

	if (g_FrData.goalaccuracy > 0) {
		red = frGetMinAccuracy(string2, frGetAccuracy(string1));

		gdl = frRenderHudElement(gdl, viGetViewLeft() + viGetViewWidth() - 70.0f * mult, viGetViewTop() + 12,
				string1, string2,
				red ? 0xff0000a0 : 0x00ff00a0,
				alpha);
	} else if (g_FrData.goaltargets != 255) {
		frGetTargetsDestroyedValue(string1);
		frGetGoalTargetsText(string2);

		if (mult == 2) {
			mult = 2.4;
		}

		gdl = frRenderHudElement(gdl, viGetViewLeft() + viGetViewWidth() - 70.0f * mult, viGetViewTop() + 12,
				string1, string2, 0x00ff00a0, alpha);
	}

	return text0f153780(gdl);
}
#endif
