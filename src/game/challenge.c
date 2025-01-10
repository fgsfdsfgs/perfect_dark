#include <ultra64.h>
#include "constants.h"
#include "game/atan2f.h"
#include "game/bot.h"
#include "game/challenge.h"
#include "game/chrai.h"
#include "game/chraicommands.h"
#include "game/debug.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/mplayer/scenarios.h"
#include "game/mplayer/setup.h"
#include "game/pad.h"
#include "game/playermgr.h"
#include "game/prop.h"
#include "game/training.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

u8 g_MpFeaturesForceUnlocked[40];
u8 g_MpFeaturesUnlocked[80];

u32 g_MpChallengeIndex = 0;
struct mpconfigfull *g_MpCurrentChallengeConfig = NULL;

struct challenge g_MpChallenges[] = {
	{ L_OPTIONS_406, MPCONFIG_CHALLENGE01 }, // "Challenge 1"
	{ L_OPTIONS_407, MPCONFIG_CHALLENGE02 }, // "Challenge 2"
	{ L_OPTIONS_408, MPCONFIG_CHALLENGE03 }, // "Challenge 3"
	{ L_OPTIONS_409, MPCONFIG_CHALLENGE04 }, // "Challenge 4"
	{ L_OPTIONS_410, MPCONFIG_CHALLENGE05 }, // "Challenge 5"
	{ L_OPTIONS_411, MPCONFIG_CHALLENGE06 }, // "Challenge 6"
	{ L_OPTIONS_412, MPCONFIG_CHALLENGE07 }, // "Challenge 7"
	{ L_OPTIONS_413, MPCONFIG_CHALLENGE08 }, // "Challenge 8"
	{ L_OPTIONS_414, MPCONFIG_CHALLENGE09 }, // "Challenge 9"
	{ L_OPTIONS_415, MPCONFIG_CHALLENGE10 }, // "Challenge 10"
	{ L_OPTIONS_416, MPCONFIG_CHALLENGE11 }, // "Challenge 11"
	{ L_OPTIONS_417, MPCONFIG_CHALLENGE12 }, // "Challenge 12"
	{ L_OPTIONS_418, MPCONFIG_CHALLENGE13 }, // "Challenge 13"
	{ L_OPTIONS_419, MPCONFIG_CHALLENGE14 }, // "Challenge 14"
	{ L_OPTIONS_420, MPCONFIG_CHALLENGE15 }, // "Challenge 15"
	{ L_OPTIONS_421, MPCONFIG_CHALLENGE16 }, // "Challenge 16"
	{ L_OPTIONS_422, MPCONFIG_CHALLENGE17 }, // "Challenge 17"
	{ L_OPTIONS_423, MPCONFIG_CHALLENGE18 }, // "Challenge 18"
	{ L_OPTIONS_424, MPCONFIG_CHALLENGE19 }, // "Challenge 19"
	{ L_OPTIONS_425, MPCONFIG_CHALLENGE20 }, // "Challenge 20"
	{ L_OPTIONS_426, MPCONFIG_CHALLENGE21 }, // "Challenge 21"
	{ L_OPTIONS_427, MPCONFIG_CHALLENGE22 }, // "Challenge 22"
	{ L_OPTIONS_428, MPCONFIG_CHALLENGE23 }, // "Challenge 23"
	{ L_OPTIONS_429, MPCONFIG_CHALLENGE24 }, // "Challenge 24"
	{ L_OPTIONS_430, MPCONFIG_CHALLENGE25 }, // "Challenge 25"
	{ L_OPTIONS_431, MPCONFIG_CHALLENGE26 }, // "Challenge 26"
	{ L_OPTIONS_432, MPCONFIG_CHALLENGE27 }, // "Challenge 27"
	{ L_OPTIONS_433, MPCONFIG_CHALLENGE28 }, // "Challenge 28"
	{ L_OPTIONS_434, MPCONFIG_CHALLENGE29 }, // "Challenge 29"
	{ L_OPTIONS_435, MPCONFIG_CHALLENGE30 }, // "Challenge 30"
};

bool challengeIsAvailable(s32 challengeindex)
{
	return (g_MpChallenges[challengeindex].availability & 1) != 0;
}

bool challengeIsAvailableToPlayer(s32 chrnum, s32 challengeindex)
{
	if ((g_MpSetup.chrslots & (1 << chrnum)) == 0) {
		return 0;
	}

	return ((g_MpChallenges[challengeindex].availability & (2 << chrnum)) != 0);
}

bool challengeIsAvailableToAnyPlayer(s32 challengeindex)
{
	return (g_MpChallenges[challengeindex].availability & (((g_MpSetup.chrslots & 0xf) << 1) | 1)) != 0;
}

void challengeDetermineUnlockedFeatures(void)
{
	s32 challengeindex;
	s32 numgifted; // number of unlocked but not completed challenges
	u8 flag;
	s32 prev;
	s32 i;
	s32 j;
	s32 k;

	// Clear all challenge availability
	for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		g_MpChallenges[challengeindex].availability = 0;
	}

	numgifted = 0;

	// Mark challenges completed by any player
	for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		flag = 0;

		if (challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex, 1)
				|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex, 2)
				|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex, 3)
				|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex, 4)) {
			// Completed challenge
			flag = 1;
		} else if (challengeindex < 4) {
			// Not yet completed, but challenges 1-4 are always available
			flag = 1;
			numgifted++;
		} else if (challengeindex > 0
				&& (challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex - 1, 1)
					|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex - 1, 2)
					|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex - 1, 3)
					|| challengeIsCompletedByAnyPlayerWithNumPlayers(challengeindex - 1, 4))) {
			// Challenges are available if their previous one is complete
			flag = 1;
			numgifted++;
		}
#ifdef DEBUG
		else if (debugIsAllChallengesEnabled()) {
			flag = 1;
		}
#endif

		g_MpChallenges[challengeindex].availability |= flag;
	}

	// Gift up to 4 challenges
	for (challengeindex = 0; numgifted < 4 && challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		if ((g_MpChallenges[challengeindex].availability & 1) == 0) {
			g_MpChallenges[challengeindex].availability |= 1;
			numgifted++;
		}
	}

	// Now same as above, but per player
	for (j = 0; j < MAX_PLAYERS; j++) {
		numgifted = 0;

		for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
			flag = 0;

			if (challengeIsCompletedByPlayerWithNumPlayers(j, challengeindex, 1)
					|| challengeIsCompletedByPlayerWithNumPlayers(j, challengeindex, 2)
					|| challengeIsCompletedByPlayerWithNumPlayers(j, challengeindex, 3)
					|| challengeIsCompletedByPlayerWithNumPlayers(j, challengeindex, 4)) {
				// Completed challenge
				flag = 2 << j;
			} else if (challengeindex < 4) {
				// Not yet completed, but challenges 1-4 are always available
				flag = 2 << j;
				numgifted++;
			} else if (challengeindex > 0) {
				// Challenges are available if their previous one is complete
				prev = challengeindex - 1;

				if (challengeIsCompletedByPlayerWithNumPlayers(j, prev, 1)
						|| challengeIsCompletedByPlayerWithNumPlayers(j, prev, 2)
						|| challengeIsCompletedByPlayerWithNumPlayers(j, prev, 3)
						|| challengeIsCompletedByPlayerWithNumPlayers(j, prev, 4)) {
					flag = 2 << j;
					numgifted++;
				}
			}

			g_MpChallenges[challengeindex].availability |= flag;
		}

		// Gift up to 4 challenges
		for (challengeindex = 0; numgifted < 4 && challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
			if ((g_MpChallenges[challengeindex].availability & (2 << j)) == 0) {
				g_MpChallenges[challengeindex].availability |= 2 << j;
				numgifted++;
			}
		}
	}

	for (j = 0; j < ARRAYCOUNT(g_MpFeaturesUnlocked); j++) {
		flag = 0;

		for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
			if (challengeIsAvailableToAnyPlayer(challengeindex)) {
				for (i = 0; i < ARRAYCOUNT(g_MpChallenges[challengeindex].unlockfeatures); i++) {
					if (g_MpChallenges[challengeindex].unlockfeatures[i] == j) {
						flag |= 1;
					}
				}
			}
		}

		for (i = 0; i < ARRAYCOUNT(g_MpFeaturesForceUnlocked); i++) {
			if (g_MpFeaturesForceUnlocked[i] == j) {
				flag |= 1;
			}
		}

		for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
			for (prev = 0; prev < MAX_PLAYERS; prev++) {
				if (challengeIsAvailableToPlayer(prev, challengeindex)) {
					for (i = 0; i < ARRAYCOUNT(g_MpChallenges[challengeindex].unlockfeatures); i++) {
						if (g_MpChallenges[challengeindex].unlockfeatures[i] == j) {
							flag |= 2 << prev;
						}
					}
				}
			}
		}

		g_MpFeaturesUnlocked[j] = flag;
	}

	for (j = 0; j < func0f188bcc(); j++) {
		struct mpweapon *weapon = &g_MpWeapons[j];

		if (weapon->unlockfeature > 0 && func0f19cbcc(weapon->weaponnum)) {
			g_MpFeaturesUnlocked[weapon->unlockfeature] |= 1;
		}
	}

	func0f1895e8();

	// If the ability to have 8 simulants hasn't been unlocked, limit them to 4
	if (!challengeIsFeatureUnlocked(MPFEATURE_8BOTS)) {
		for (k = 4; k < MAX_BOTS; k++) {
			if (g_MpSetup.chrslots & (1 << (MAX_PLAYERS + k))) {
				mpRemoveSimulant(k);
			}
		}

		if (g_Vars.mpquickteamnumsims > 4) {
			g_Vars.mpquickteamnumsims = 4;
		}
	}
}

void challengePerformSanityChecks(void)
{
	if (g_BossFile.locktype == MPLOCKTYPE_CHALLENGE) {
		s32 numplayers = 0;
		s32 i;

		// Reset player handicaps
		for (i = 0; i < MAX_PLAYERS; i++) {
			if (g_MpSetup.chrslots & (1 << i)) {
				*g_PlayerConfigsArray[i].handicap = 0x80;
				numplayers++;
			}
		}

		// Turn off all simulants and turn them on if enabled
		// for this number of players
		g_MpSetup.chrslots &= 0x000f;

		for (i = 0; i < MAX_BOTS; i++) {
			g_BotConfigsArray[i].difficulty = g_MpSimulantDifficultiesPerNumPlayers[i][numplayers - 1];

			if (g_BotConfigsArray[i].difficulty != BOTDIFF_DISABLED) {
				g_MpSetup.chrslots |= 1 << (i + MAX_PLAYERS);
			}
		}

		if (g_MpSetup.scenario == MPSCENARIO_KINGOFTHEHILL) {
			g_Vars.mphilltime = 10;
		}
	} else if (!challengeIsFeatureUnlocked(MPFEATURE_8BOTS)) {
		// Limit to 4 players and 4 simulants
		g_MpSetup.chrslots &= 0x00ff;
	}
}

s32 challengeGetNumAvailable(void)
{
	s32 challengeindex;
	s32 count = 0;

	for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		if (challengeIsAvailableToAnyPlayer(challengeindex)) {
			count++;
		}
	}

	return count;
}

char *challengeGetName(s32 challengeindex)
{
	return langGet(g_MpChallenges[challengeindex].name);
}

char *challengeGetNameBySlot(s32 slot)
{
	s32 index = 0;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpChallenges); i++) {
		if (challengeIsAvailableToAnyPlayer(i)) {
			if (index == slot) {
				return challengeGetName(i);
			}

			index++;
		}
	}

	return "";
}

void challengeSetCurrentBySlot(s32 slotnum)
{
	s32 challengeindex;
	g_MpChallengeIndex = 0;

	for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		if (challengeIsAvailableToAnyPlayer(challengeindex)) {
			if (slotnum == 0) {
				g_MpChallengeIndex = challengeindex;
				break;
			}

			slotnum--;
		}
	}

	challengeApply();
}

s32 challengeGetCurrent(void)
{
	return g_MpChallengeIndex;
}

bool challengeIsCompletedByAnyChrWithNumPlayersBySlot(s32 slot, s32 numplayers)
{
	s32 availableindex = 0;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpChallenges); i++) {
		if (challengeIsAvailableToAnyPlayer(i)) {
			if (availableindex == slot) {
				return challengeIsCompletedByAnyPlayerWithNumPlayers(i, numplayers);
			}

			availableindex++;
		}
	}

	return false;
}

bool challengeIsCompletedByChrWithNumPlayersBySlot(s32 mpchrnum, s32 slot, s32 numplayers)
{
	s32 availableindex = 0;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpChallenges); i++) {
		if (challengeIsAvailableToAnyPlayer(i)) {
			if (availableindex == slot) {
				return challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, i, numplayers);
			}

			availableindex++;
		}
	}

	return false;
}

#ifdef PLATFORM_N64
#define BTYPE s32
#else
#define BTYPE uintptr_t
#endif

struct mpconfigfull *challengeLoadConfig(s32 confignum, u8 *buffer, s32 len)
{
	struct mpconfigfull *mpconfig;
	u8 buffer2[sizeof(struct mpstrings) + 40];
	struct mpstrings *loadedstrings;
	BTYPE bank;
	u32 language_id = langGetFileNumOffset();
#ifdef PLATFORM_N64
	extern struct mpconfig _mpconfigsSegmentRomStart[];
#else
	extern u8 EXT_SEG _mpconfigsSegmentRomStart;
#endif
	extern struct mpstrings EXT_SEG _mpstringsESegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsJSegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsPSegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsGSegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsFSegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsSSegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsISegmentRomStart;
	extern struct mpstrings EXT_SEG _mpstringsESegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsJSegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsPSegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsGSegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsFSegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsSSegmentRomEnd;
	extern struct mpstrings EXT_SEG _mpstringsISegmentRomEnd;

	BTYPE banks[][2] = {
		{ (BTYPE)REF_SEG _mpstringsESegmentRomStart, (BTYPE)REF_SEG _mpstringsESegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsJSegmentRomStart, (BTYPE)REF_SEG _mpstringsJSegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsPSegmentRomStart, (BTYPE)REF_SEG _mpstringsPSegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsGSegmentRomStart, (BTYPE)REF_SEG _mpstringsGSegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsFSegmentRomStart, (BTYPE)REF_SEG _mpstringsFSegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsSSegmentRomStart, (BTYPE)REF_SEG _mpstringsSSegmentRomEnd },
		{ (BTYPE)REF_SEG _mpstringsISegmentRomStart, (BTYPE)REF_SEG _mpstringsISegmentRomEnd },
	};

	// Load mpconfigs
#ifdef PLATFORM_N64
	mpconfig = dmaExecWithAutoAlign(buffer, (BTYPE)&_mpconfigsSegmentRomStart[confignum], sizeof(struct mpconfig));
#else
	mpconfig = dmaExecWithAutoAlign(buffer, (BTYPE)REF_SEG _mpconfigsSegmentRomStart + confignum * sizeof(struct mpconfig), sizeof(struct mpconfig));
#endif

	// Load mpstrings
	bank = banks[language_id][0];
	loadedstrings = dmaExecWithAutoAlign(buffer2, bank + confignum * sizeof(struct mpstrings), sizeof(struct mpstrings));

	mpconfig->strings = *loadedstrings;

	return mpconfig;
}

struct mpconfigfull *challengeLoad(s32 challengeindex, u8 *buffer, s32 len)
{
	return challengeLoadConfig(g_MpChallenges[challengeindex].confignum, buffer, len);
}

struct mpconfigfull *challengeLoadBySlot(s32 n, u8 *buffer, s32 len)
{
	s32 numavailable = 0;
	s32 challengeindex;

	for (challengeindex = 0; challengeindex < ARRAYCOUNT(g_MpChallenges); challengeindex++) {
		if (challengeIsAvailableToAnyPlayer(challengeindex)) {
			if (numavailable == n) {
				return challengeLoad(challengeindex, buffer, len);
			}

			numavailable++;
		}
	}

	return 0;
}

struct mpconfigfull *challengeLoadCurrent(u8 *buffer, s32 len)
{
	return challengeLoad(g_MpChallengeIndex, buffer, len);
}

/**
 * This is adding featurenum to the array, provided it's unique.
 */
s32 challengeForceUnlockFeature(s32 featurenum, u8 *array, s32 tail, s32 len)
{
	s32 i;

	for (i = 0; i < tail; i++) {
		if (array[i] == featurenum) {
			break;
		}
	}

	if (i >= tail && tail < len) {
		array[tail] = featurenum;
		tail++;
	}

	return tail;
}

s32 challengeForceUnlockSetupFeatures(struct mpsetup *setup, u8 *array, s32 len)
{
	s32 index = 0;
	s32 i;

	// Force unlock the weapons (if never held before)
	for (i = 0; i < ARRAYCOUNT(setup->weapons); i++) {
		s32 featurenum = g_MpWeapons[setup->weapons[i]].unlockfeature;

		if (featurenum) {
			index = challengeForceUnlockFeature(featurenum, array, index, len);
		}
	}

	// Force unlock the stage
	for (i = 0; i < mpGetNumStages(); i++) {
		if (g_MpArenas[i].stagenum == setup->stagenum) {
			s32 featurenum = g_MpArenas[i].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, array, index, len);
			}
		}
	}

	// Force unlock the scenario
	if (setup->scenario <= MPSCENARIO_CAPTURETHECASE) {
		s32 featurenum = g_MpScenarioOverviews[setup->scenario].requirefeature;

		if (featurenum) {
			index = challengeForceUnlockFeature(featurenum, array, index, len);
		}
	}

	// Force unlock the scenario options
	if (setup->options & MPOPTION_ONEHITKILLS) {
		index = challengeForceUnlockFeature(MPFEATURE_ONEHITKILLS, array, index, len);
	}

	if (setup->options & (MPOPTION_SLOWMOTION_ON | MPOPTION_SLOWMOTION_SMART)) {
		index = challengeForceUnlockFeature(MPFEATURE_SLOWMOTION, array, index, len);
	}

	return index;
}

void challengeForceUnlockConfigFeatures(struct mpconfig *config, u8 *array, s32 len, s32 challengeindex)
{
	s32 index = challengeForceUnlockSetupFeatures(&config->setup, array, len);
	s32 featurenum;
	s32 numplayers;
	s32 i;

	for (i = 0; i < MAX_BOTS; i++) {
		s32 simtype = mpFindBotProfile(config->simulants[i].type, BOTDIFF_NORMAL);

		if (simtype >= 0) {
			featurenum = g_BotProfiles[simtype].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, array, index, len);
			}
		}

		for (numplayers = 0; numplayers < MAX_PLAYERS; numplayers++) {
			simtype = mpFindBotProfile(0, config->simulants[i].difficulties[numplayers]);

			if (simtype >= 0) {
				featurenum = g_BotProfiles[simtype].requirefeature;

				if (featurenum) {
					index = challengeForceUnlockFeature(featurenum, array, index, len);
				}
			}
		}

		if (config->simulants[i].mpbodynum < ARRAYCOUNT(g_MpBodies)) {
			featurenum = g_MpBodies[config->simulants[i].mpbodynum].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, array, index, len);
			}
		}

		if (config->simulants[i].mpheadnum < ARRAYCOUNT(g_MpHeads)) {
			featurenum = g_MpHeads[config->simulants[i].mpheadnum].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, array, index, len);
			}
		}
	}

#if VERSION >= VERSION_NTSC_1_0
	if (challengeindex >= 25) {
		index = challengeForceUnlockFeature(MPFEATURE_BOTDIFF_DARK, array, index, len);
	} else if (challengeindex >= 20) {
		index = challengeForceUnlockFeature(MPFEATURE_STAGE_CARPARK, array, index, len);
	} else if (challengeindex >= 15) {
		index = challengeForceUnlockFeature(MPFEATURE_SCENARIO_PAC, array, index, len);
	}

	if (challengeindex >= 10) {
		index = challengeForceUnlockFeature(MPFEATURE_8BOTS, array, index, len);
	}
#else
	if (challengeindex >= 10) {
		index = challengeForceUnlockFeature(MPFEATURE_8BOTS, array, index, len);
	}

	if (challengeindex >= 15) {
		index = challengeForceUnlockFeature(MPFEATURE_SCENARIO_PAC, array, index, len);
	}

	if (challengeindex >= 20) {
		index = challengeForceUnlockFeature(MPFEATURE_STAGE_CARPARK, array, index, len);
	}
#endif

	// Clear the remainder of the array
	for (i = index; i < len; i++) {
		array[i] = 0;
	}
}

void challengeForceUnlockBotFeatures(void)
{
	s32 numsims = 0;
	s32 index = challengeForceUnlockSetupFeatures(&g_MpSetup, g_MpFeaturesForceUnlocked, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_BotConfigsArray); i++) {
		// Force unlock the simulant type
		s32 simtypeindex = mpFindBotProfile(g_BotConfigsArray[i].type, BOTDIFF_NORMAL);

		if (simtypeindex >= 0) {
			s32 featurenum = g_BotProfiles[simtypeindex].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, g_MpFeaturesForceUnlocked, index, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
			}
		}

		// Force unlock the simulant difficulty
		simtypeindex = mpFindBotProfile(BOTTYPE_GENERAL, g_BotConfigsArray[i].difficulty);

		if (simtypeindex >= 0) {
			s32 featurenum = g_BotProfiles[simtypeindex].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, g_MpFeaturesForceUnlocked, index, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
			}
		}

		if (simtypeindex >= 0) {
			numsims++;
		}

		// Force unlock the simulant's body
		if (g_BotConfigsArray[i].base.mpbodynum < ARRAYCOUNT(g_MpBodies)) {
			s32 featurenum = g_MpBodies[g_BotConfigsArray[i].base.mpbodynum].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, g_MpFeaturesForceUnlocked, index, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
			}
		}

		// Force unlock the simulant's head
		if (g_BotConfigsArray[i].base.mpheadnum < ARRAYCOUNT(g_MpHeads)) {
			s32 featurenum = g_MpHeads[g_BotConfigsArray[i].base.mpheadnum].requirefeature;

			if (featurenum) {
				index = challengeForceUnlockFeature(featurenum, g_MpFeaturesForceUnlocked, index, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
			}
		}
	}

	// Force unlock 8 simulants
	if (numsims > 4) {
		index = challengeForceUnlockFeature(MPFEATURE_8BOTS, g_MpFeaturesForceUnlocked, index, ARRAYCOUNT(g_MpFeaturesForceUnlocked));
	}

	// Clear the remainder of the array
	for (i = index; i < ARRAYCOUNT(g_MpFeaturesForceUnlocked); i++) {
		g_MpFeaturesForceUnlocked[i] = 0;
	}

	challengeDetermineUnlockedFeatures();
}

void challengeRemoveForceUnlocks(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpFeaturesForceUnlocked); i++) {
		g_MpFeaturesForceUnlocked[i] = 0;
	}

	challengeDetermineUnlockedFeatures();
}

void challengeApply(void)
{
	s32 i;
	u8 buffer[0x1ca];

	mpApplyConfig(challengeLoadCurrent(buffer, 0x1ca));
	mpSetLock(MPLOCKTYPE_CHALLENGE, 5);

	for (i = 0; i < MAX_PLAYERS; i++) {
		g_PlayerConfigsArray[i].base.team = 0;
	}
}

s32 challengeRemovePlayerLock(void)
{
	return mpSetLock(MPLOCKTYPE_NONE, 0);
}

void challengeLoadAndStoreCurrent(u8 *buffer, s32 len)
{
	g_MpCurrentChallengeConfig = challengeLoadCurrent(buffer, len);
}

void challengeUnsetCurrent(void)
{
	g_MpCurrentChallengeConfig = NULL;
}

bool challengeIsLoaded(void)
{
	return g_MpCurrentChallengeConfig != NULL;
}

char *challengeGetCurrentDescription(void)
{
	if (g_MpCurrentChallengeConfig) {
		return g_MpCurrentChallengeConfig->strings.description;
	}

	return "";
}

char *challengeGetConfigDescription(struct mpconfigfull *mpconfig)
{
	if (mpconfig) {
		return mpconfig->strings.description;
	}

	return "";
}

/**
 * Return the index of the first incomplete challenge, but if it's in the first
 * 4 then return index 4 because the first 4 are always shown.
 */
s32 challengeGetAutoFocusedIndex(s32 mpchrnum)
{
	s32 challengeindex;
	s32 index = 0;

	for (challengeindex = ARRAYCOUNT(g_MpChallenges) - 1; challengeindex >= 0; challengeindex--) {
		if (challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, challengeindex, 1) ||
				challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, challengeindex, 2) ||
				challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, challengeindex, 3) ||
				challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, challengeindex, 4)) {
			index = challengeindex + 1;
			break;
		}
	}

	if (index < 4) {
		index = 4;
	}

	return index;
}

char *challengeGetName2(s32 playernum, s32 challengeindex)
{
	return langGet(g_MpChallenges[challengeindex].name);
}

bool challengeIsCompletedByPlayerWithNumPlayers2(s32 mpchrnum, s32 index, s32 numplayers)
{
	return challengeIsCompletedByPlayerWithNumPlayers(mpchrnum, index, numplayers);
}

bool challengeIsCompletedByAnyPlayerWithNumPlayers(s32 index, s32 numplayers)
{
	return (g_MpChallenges[index].completions[numplayers - 1] & 1) != 0;
}

void challengeSetCompletedByAnyPlayerWithNumPlayers(s32 index, s32 numplayers, bool completed)
{
	if (completed) {
		g_MpChallenges[index].completions[numplayers - 1] |= 1;
		return;
	}

	g_MpChallenges[index].completions[numplayers - 1] &= ~1;
}

bool challengeIsCompletedByPlayerWithNumPlayers(s32 mpchrnum, s32 index, s32 numplayers)
{
	return (g_MpChallenges[index].completions[numplayers - 1] & (2 << mpchrnum)) != 0;
}

void challengeSetCompletedByPlayerWithNumPlayers(u32 mpchrnum, s32 index, s32 numplayers, bool completed)
{
	if (completed) {
		g_MpChallenges[index].completions[numplayers - 1] |= 2 << mpchrnum;
		return;
	}

	g_MpChallenges[index].completions[numplayers - 1] &= ~(2 << mpchrnum);
}

bool challengeIsCompleteForEndscreen(void)
{
	s32 prevplayernum = g_Vars.currentplayernum;
	s32 result = false;
	s32 aborted = false;
	s32 i;
	u32 stack;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);

		if (g_Vars.currentplayer->aborted) {
			aborted = true;
		}
	}

	setCurrentPlayerNum(prevplayernum);

	if (!aborted) {
		struct ranking rankings[MAX_MPCHRS];
		mpGetTeamRankings(rankings);

		if (rankings[0].teamnum == 0) {
			result = true;
		}
	}

	return result;
}

void challengeConsiderMarkingComplete(void)
{
	bool result = challengeIsCompleteForEndscreen();

#if VERSION >= VERSION_NTSC_1_0 && defined(DEBUG)
	if ((g_CheatsActiveBank0 == 0 && g_CheatsActiveBank1 == 0) && (result || debugIsSetCompleteEnabled()))
#elif VERSION >= VERSION_NTSC_1_0
	if (g_CheatsActiveBank0 == 0 && g_CheatsActiveBank1 == 0 && result)
#else
	if (result && g_CheatsActiveBank0 == 0 && g_CheatsActiveBank1 == 0)
#endif
	{
		u32 prevplayernum;
		s32 i;

		challengeSetCompletedByAnyPlayerWithNumPlayers(g_MpChallengeIndex, PLAYERCOUNT(), 1);
		prevplayernum = g_Vars.currentplayernum;

		for (i = 0; i < PLAYERCOUNT(); i++) {
			setCurrentPlayerNum(i);
			challengeSetCompletedByPlayerWithNumPlayers(g_Vars.currentplayerstats->mpindex, g_MpChallengeIndex, PLAYERCOUNT(), true);
		}

		setCurrentPlayerNum(prevplayernum);
		challengeDetermineUnlockedFeatures();
	}
}

bool challengeIsFeatureUnlocked(s32 featurenum)
{
	if (featurenum == 0) {
		return true;
	}

	return (g_MpFeaturesUnlocked[featurenum] & 1) != 0;
}

bool challengeIsFeatureUnlockedByPlayer(u32 numplayers, s32 featurenum)
{
	if (featurenum == 0) {
		return true;
	}

	return (g_MpFeaturesUnlocked[featurenum] & (2 << numplayers)) != 0;
}

bool challengeIsFeatureUnlockedByDefault(s32 featurenum)
{
	if (featurenum) {
		return false;
	}

	return true;
}
