#ifndef IN_GAME_MPLAYER_MPLAYER_H
#define IN_GAME_MPLAYER_MPLAYER_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

f32 mpHandicapToDamageScale(u8 value);
void func0f187838(struct mpchrconfig *mpchr);
void mpStartMatch(void);
void mpReset(void);
void mpCalculateTeamIsOnlyAi(void);
void func0f187fbc(s32 playernum);
void func0f187fec(void);
void mpPlayerSetDefaults(s32 playernum, bool autonames);
void func0f1881d4(s32 index);
void mpInit(void);
void mpGetTeamsWithDefaultName(u8 *mask);
void mpSetTeamNamesToDefault(u8 mask);
void mpSetDefaultNamesIfEmpty(void);
s32 mpCalculateTeamScoreLimit(void);
void mpApplyLimits(void);
s32 mpGetPlayerRankings(struct ranking *rankings);
s32 mpCalculateTeamScore(s32 arg0, s32 *arg1);
s32 mpGetTeamRankings(struct ranking *rankings);
s32 func0f188bcc(void);
s32 mpGetNumWeaponOptions(void);
char *mpGetWeaponLabel(s32 weaponnum);
void mpSetWeaponSlot(s32 slot, s32 mpweaponnum);
s32 mpGetWeaponSlot(s32 slot);
struct mpweapon *mpGetMpWeaponByLocation(s32 locationindex);
s32 mpCountWeaponSetThing(s32 weaponsetindex);
s32 func0f188f9c(s32 arg0);
s32 func0f189058(bool full);
s32 func0f189088(void);
char *mpGetWeaponSetName(s32 arg0);
void func0f18913c(void);
#ifndef PLATFORM_N64
void mpSetRandomWeapons(u8 weapons[]);
#endif
void mpApplyWeaponSet(void);
void mpSetWeaponSet(s32 weaponsetnum);
void func0f1895e8(void);
s32 mpGetWeaponSet(void);
bool mpIsPaused(void);
void mpSetPaused(u8 mode);
Gfx *mpRenderModalText(Gfx *gdl);
s32 mpFindMaxInt(s32 playercount, s32 val0, s32 val1, s32 val2, s32 val3);
s32 mpFindMinInt(s32 playercount, s32 val0, s32 val1, s32 val2, s32 val3);
s32 mpFindMaxFloat(s32 playercount, f32 val0, f32 val1, f32 val2, f32 val3);
s32 mpFindMinFloat(s32 playercount, f32 val0, f32 val1, f32 val2, f32 val3);
void mpCalculatePlayerTitle(struct mpplayerconfig *mpplayer);
void mpCalculateAwards(void);
void mpEndMatch(void);
s32 mpGetNumHeads2(void);
s32 mpGetNumHeads(void);
s32 mpGetHeadId(u8 headnum);
s32 mpGetHeadRequiredFeature(u8 headnum);
s32 mpGetBeauHeadId(u8 headnum);
u32 mpGetNumBodies(void);
s32 mpGetBodyId(u8 bodynum);
s32 mpGetMpbodynumByBodynum(u16 bodynum);
char *mpGetBodyName(u8 mpbodynum);
u8 mpGetBodyRequiredFeature(u8 bodynum);
s32 mpGetMpheadnumByMpbodynum(s32 bodynum);
s32 mpChooseRandomLockPlayer(void);
bool mpSetLock(s32 locktype, s32 playernum);
s32 mpGetLockType(void);
u32 mpGetLockPlayerNum(void);
bool mpIsPlayerLockedOut(s32 playernum);
void mpCalculateLockIfLastWinnerOrLoser(void);
bool mpIsTrackUnlocked(s32 tracknum);
s32 mpGetTrackSlotIndex(s32 tracknum);
s32 mpGetTrackNumAtSlotIndex(s32 slotindex);
s32 mpGetNumUnlockedTracks(void);
s32 mpGetTrackMusicNum(s32 slotindex);
char *mpGetTrackName(s32 slotindex);
void mpSetUsingMultipleTunes(bool enable);
bool mpGetUsingMultipleTunes(void);
bool mpIsMultiTrackSlotEnabled(s32 slot);
void mpSetMultiTrackSlotEnabled(s32 slot, bool enable);
void mpSetTrackSlotEnabled(s32 slot);
void mpEnableAllMultiTracks(void);
void mpDisableAllMultiTracks(void);
void mpRandomiseMultiTracks(void);
void mpSetTrackToRandom(void);
s32 mpGetCurrentTrackSlotNum(void);
s32 mpChooseTrack(void);
struct mpchrconfig *mpGetChrConfigBySlotNum(s32 slot);
s32 mpGetChrIndexBySlotNum(s32 slot);
s32 mpGetNumChrs(void);
u8 mpFindUnusedTeamNum(void);
void mpCreateBotFromProfile(s32 botnum, u8 difficulty);
void mpSetBotDifficulty(s32 botnum, s32 difficulty);
s32 mpGetSlotForNewBot(void);
void mpRemoveSimulant(s32 index);
#ifndef PLATFORM_N64
void mpCopySimulant(s32 index);
#endif
bool mpHasSimulants(void);
bool mpHasUnusedBotSlots(void);
bool mpIsSimSlotEnabled(s32 slot);
s32 mpFindBotProfile(s32 type, s32 difficulty);
void mpGenerateBotNames(void);
s32 mpPlayerGetIndex(struct chrdata *chr);
struct chrdata *mpGetChrFromPlayerIndex(s32 index);
s32 func0f18d074(s32 index);
s32 func0f18d0e8(s32 arg0);
void mpplayerfileLoadGunFuncs(struct savebuffer *buffer, s32 playernum);
void mpplayerfileSaveGunFuncs(struct savebuffer *buffer, s32 playernum);
void mpplayerfileLoadWad(s32 playernum, struct savebuffer *buffer, s32 arg2);
void mpplayerfileSaveWad(s32 playernum, struct savebuffer *buffer);
void mpplayerfileGetOverview(char *arg0, char *name, u32 *playtime);
s32 mpplayerfileSave(s32 playernum, s32 device, s32 fileid, u16 deviceserial);
s32 mpplayerfileLoad(s32 playernum, s32 device, s32 arg2, u16 deviceserial);
s32 mpGetNumPresets(void);
bool mpIsPresetUnlocked(s32 presetnum);
s32 mpGetNumUnlockedPresets(void);
char *mpGetPresetNameBySlot(s32 slot);
void mpApplyConfig(struct mpconfigfull *mpconfig);
void mp0f18dec4(s32 arg0);
void mpsetupfileLoadWad(struct savebuffer *buffer);
void mpsetupfileSaveWad(struct savebuffer *buffer);
void mpsetupfileGetOverview(char *arg0, char *filename, u16 *numsims, u16 *stagenum, u16 *scenarionum);
s32 mpsetupfileSave(s32 device, s32 filenum, u16 deviceserial);
s32 mpsetupfileLoad(s32 device, s32 filenum, u16 deviceserial);
void func0f18e558(void);
struct modeldef *func0f18e57c(s32 index, s32 *headnum);

#endif
