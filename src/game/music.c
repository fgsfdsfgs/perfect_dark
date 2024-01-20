#include <ultra64.h>
#include "constants.h"
#include "game/menu.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/options.h"
#include "game/stagemusic.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/music.h"
#include "data.h"
#include "types.h"

#define FADETYPE_STOP  0
#define FADETYPE_PAUSE 1

s32 g_MusicStageNum;
struct musicevent g_MusicEventQueue[40];
struct seqchannel g_SeqChannels[3];
u32 g_AudioXReasonsActive[4];
s32 g_MusicXReasonMinDurations[4];
s32 g_MusicXReasonMaxDurations[4];

s32 g_MenuTrack = -1;
s32 g_MusicEventQueueLength = 0;
s32 g_TemporaryPrimaryTrack = -1;
s32 g_TemporaryAmbientTrack = -1;

#if VERSION >= VERSION_NTSC_1_0
s32 g_MusicSavedInterval240 = -1;
#endif

u32 g_MusicNextEventId = 0;
bool g_MusicNrgIsActive = false;
bool g_MusicMpDeathIsPlaying = false;
s32 g_MusicInterval240 = 15;
s32 g_MusicSleepRemaining240 = 0;
bool g_MusicSoloDeathIsPlaying = false;

#if VERSION >= VERSION_NTSC_1_0
u16 g_MusicVolume = 0x5000;
#endif

s32 g_MusicDeathTimer240 = 0;   // Counts down 5 seconds while death music plays
s32 g_MusicAge60 = 0;           // The current age of the MP track being played
s32 g_MusicLife60 = TICKS(120); // The max age of any MP track (this value is changed in MP code)
s32 g_MusicSilenceTimer60 = 0;  // Counts down the 2 second silence between MP track changes

#ifndef PLATFORM_N64
s32 g_MusicDisableMpDeath = false;
#endif

#if VERSION < VERSION_NTSC_1_0
const char var7f1b2030nb[] = "MUSIC : musicPlayLevel\n";
const char var7f1b2048nb[] = "MUSIC : SWITCHING TO CORRECT AMBIENT TUNE\n";
const char var7f1b2074nb[] = "musicStartLevel\n";
const char var7f1b2088nb[] = "musicEndLevel\n";
const char var7f1b2098nb[] = "musicStartNRG\n";
const char var7f1b20a8nb[] = "musicEndNRG\n";
const char var7f1b20b8nb[] = "musicStartWatch start\n";
const char var7f1b20d0nb[] = "musicEndWatch start\n";
const char var7f1b20e8nb[] = "musicEndWatch end\n";
const char var7f1b20fcnb[] = "musicStartDead\n";
const char var7f1b210cnb[] = "musicStartTemporary\n";
const char var7f1b2124nb[] = "musicEndTemporary\n";
const char var7f1b2138nb[] = "musicStartCutscene\n";
const char var7f1b214cnb[] = "musicEndCutscene\n";
const char var7f1b2160nb[] = "musicStartAmbient : Tune=%d\n";
const char var7f1b2180nb[] = "musicEndAmbient\n";
#endif

char *var80084100[] = {
	"null",
	"leveltune",
	"nrgtune,watchtune,mpdeathtune",
	"ambience",
	"NULL",
	"playing",
	"paused",
};

#if VERSION >= VERSION_PAL_FINAL
u32 var8008465cpf = 0;
#endif

const char var7f1b7918[] = "MUSIC : activedeath=%d\n";

u16 musicGetVolume(void)
{
#if VERSION >= VERSION_NTSC_1_0
	u32 volume;

	if (g_Vars.stagenum == STAGE_CREDITS) {
		return 0x5000;
	}

	if (g_MusicVolume < 0x5000) {
		volume = g_MusicVolume;
	} else {
		volume = 0x5000;
	}

	return volume;
#else
	return optionsGetMusicVolume();
#endif
}

void musicSetVolume(u16 volume)
{
	s32 i;

#if VERSION >= VERSION_NTSC_1_0
	if (volume > 0x5000) {
		volume = 0x5000;
	}
#endif

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype != TRACKTYPE_NONE && g_SeqChannels[i].tracktype != TRACKTYPE_AMBIENT) {
			seqSetVolume(&g_SeqInstances[i], volume);
		}
	}

#if VERSION >= VERSION_NTSC_1_0
	g_MusicVolume = volume;
#endif
}

bool musicIsTrackState(s32 tracktype, s32 state)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			switch (state) {
			case AL_STOPPED:
				return !g_SeqChannels[i].inuse;
			case AL_PLAYING:
				return g_SeqChannels[i].inuse;
			case AL_STOPPING:
				return g_SeqChannels[i].keepafterfade;
			}
		}
	}

	return false;
}

s32 musicGetTrackState(s32 tracktype)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			if (g_SeqChannels[i].keepafterfade) {
				return AL_STOPPING;
			}

			if (g_SeqChannels[i].inuse) {
				return AL_PLAYING;
			}
		}
	}

	return AL_STOPPED;
}

s32 musicGetChannelByTrackType(s32 tracktype)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			return i;
		}
	}

	return -1;
}

void musicQueueStartEvent(u32 tracktype, u32 tracknum, f32 arg2, u16 volume)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].tracknum = tracknum;
		g_MusicEventQueue[g_MusicEventQueueLength].unk0c = arg2;
		g_MusicEventQueue[g_MusicEventQueueLength].volume = volume;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_PLAY;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicQueueStopEvent(s32 tracktype)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_STOP;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicQueueFadeEvent(s32 tracktype, f32 arg1, bool keepafterfade)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].unk0c = arg1;
		g_MusicEventQueue[g_MusicEventQueueLength].keepafterfade = keepafterfade;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_FADE;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicReset(void)
{
	s32 i;

	if (!g_SndDisabled) {
		for (i = 0; i < ARRAYCOUNT(g_AudioXReasonsActive); i++) {
			g_AudioXReasonsActive[i] = 0;
			g_MusicXReasonMinDurations[i] = 0;
			g_MusicXReasonMaxDurations[i] = 0;
		}

#if VERSION >= VERSION_NTSC_1_0
		musicSaveInterval();
		musicQueueStopAllEvent();
		musicRestoreInterval();
#else
		musicQueueStopAllEvent();
#endif

		g_MusicSoloDeathIsPlaying = false;
		g_MusicDeathTimer240 = 0;
		g_MenuTrack = -1;
		g_TemporaryPrimaryTrack = -1;
		g_TemporaryAmbientTrack = -1;
		g_MusicNrgIsActive = false;
	}
}

void musicQueueStopAllEvent(void)
{
#if VERSION >= VERSION_NTSC_1_0
	g_MusicEventQueue[0].tracktype = TRACKTYPE_6;
#endif

	g_MusicEventQueue[0].eventtype = MUSICEVENTTYPE_STOPALL;
	g_MusicEventQueue[0].id = g_MusicNextEventId++;
	g_MusicEventQueue[0].numattempts = 0;
	g_MusicEventQueue[0].failcount = 0;

	g_MusicEventQueueLength = 1;

	musicTickEvents();
}

#if VERSION >= VERSION_NTSC_1_0
void musicSaveInterval(void)
{
	g_MusicSavedInterval240 = g_MusicInterval240;
	g_MusicInterval240 = 0;
}

void musicRestoreInterval(void)
{
	g_MusicEventQueue[g_MusicEventQueueLength].tracktype = TRACKTYPE_6;
	g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_SETINTERVAL;
	g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
	g_MusicEventQueue[g_MusicEventQueueLength].timer240 = g_MusicSavedInterval240;
	g_MusicEventQueueLength++;

	// @bug: This should be modifying the interval queue item, not the first queue item
	g_MusicEventQueue[0].numattempts = 0;
	g_MusicEventQueue[0].failcount = 0;
}
#endif

#define PRIMARYTRACK() (g_TemporaryPrimaryTrack != -1 ? g_TemporaryPrimaryTrack : stageGetPrimaryTrack(g_MusicStageNum))
#define AMBIENTTRACK() (g_TemporaryAmbientTrack != -1 ? g_TemporaryAmbientTrack : stageGetAmbientTrack(g_MusicStageNum))

void musicStartPrimaryWithReason(f32 arg0, s8 reason)
{
	/*
	 * reason: 0 - vanilla behavior
	 * reason: 1 - return from training
	*/
	if (reason == 1) {
		musicQueueStartEvent(TRACKTYPE_PRIMARY, PRIMARYTRACK(), arg0, musicGetVolume());
		return;
	}
	if (g_Vars.stagenum == STAGE_CITRAINING) {
		if (g_StageFlags & ~STAGEFLAG_CI_IN_TRAINING) return;
	}
	if (PRIMARYTRACK() >= 0 && !menuGetRoot()) {
		musicQueueStartEvent(TRACKTYPE_PRIMARY, PRIMARYTRACK(), arg0, musicGetVolume());
	}
	return;
}
void musicStartPrimary(f32 arg0)
{
	musicStartPrimaryWithReason(arg0, 0);
}

void musicStartAmbient(f32 arg0)
{
	s32 pass = false;

	if (AMBIENTTRACK() >= 0) {
		if (g_TemporaryAmbientTrack != -1) {
			pass = true;
		} else if (musicIsAnyPlayerInAmbientRoom()) {
			if (g_Vars.tickmode != TICKMODE_CUTSCENE && AMBIENTTRACK() != stageGetAmbientTrack(g_MusicStageNum)) {
				musicQueueStopEvent(TRACKTYPE_AMBIENT);
				musicStartTemporaryAmbient(stageGetAmbientTrack(g_MusicStageNum));
				return;
			}

			pass = true;
		}
	}

	if (pass) {
		switch (musicGetTrackState(TRACKTYPE_AMBIENT)) {
		case AL_STOPPED:
		case AL_STOPPING:
			musicQueueStartEvent(TRACKTYPE_AMBIENT, AMBIENTTRACK(), arg0, VOLUME(g_SfxVolume));
			break;
		}
	}
}

bool musicIsAnyPlayerInAmbientRoom(void)
{
	s32 i;

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		return false;
	}

	if (lvIsPaused()) {
		return false;
	}

	if (g_MusicSoloDeathIsPlaying) {
		return false;
	}

	if (g_MusicNrgIsActive && g_MusicMpDeathIsPlaying) {
		return false;
	}

	for (i = 0; i < PLAYERCOUNT(); i++) {
		if (g_Vars.players[i]->prop
				&& g_Vars.players[i]->prop->rooms
				&& g_Vars.players[i]->prop->rooms[0] != -1) {
			bool hasflag;

			if (g_Rooms[g_Vars.players[i]->prop->rooms[0]].flags & ROOMFLAG_PLAYAMBIENTTRACK) {
				hasflag = true;
			} else {
				hasflag = false;
			}

			if (hasflag) {
				return true;
			}
		}
	}

	return false;
}

void musicStartNrg(f32 arg0)
{
	musicQueueStartEvent(TRACKTYPE_NRG, stageGetNrgTrack(g_MusicStageNum), arg0, musicGetVolume());
}

/**
 * Not called.
 */
void musicStartWatch(f32 arg0)
{
	musicQueueStartEvent(TRACKTYPE_MENU, menuChooseMusic(), arg0, musicGetVolume());
}

/**
 * Play a specific track as a menu track.
 *
 * Used in credits and the soundtrack dialog in MP setup.
 */
void musicStartTrackAsMenu(s32 tracknum)
{
	if (tracknum != g_MenuTrack) {
		musicQueueStopEvent(TRACKTYPE_MENU);
		musicQueueStopEvent(TRACKTYPE_DEATH);
		musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.5f, FADETYPE_PAUSE);
		musicQueueFadeEvent(TRACKTYPE_NRG, 0.5f, FADETYPE_PAUSE);
		musicQueueFadeEvent(TRACKTYPE_AMBIENT, 0.5f, FADETYPE_PAUSE);
		musicQueueStartEvent(TRACKTYPE_MENU, tracknum, 0, musicGetVolume());
	}

	g_MenuTrack = tracknum;
}

/**
 * Used when starting combat simulator matches.
 */
void musicSetStageAndStartMusic(s32 stagenum)
{
	g_MusicStageNum = stagenum;

	musicStartPrimary(0);

	if (stageGetAmbientTrack(g_MusicStageNum) >= 0) {
		musicStartAmbient(0);
	}
}

/**
 * Used for solo missions.
 */
void musicSetStage(s32 stagenum)
{
	g_MusicStageNum = stagenum;
}

void musicStop(void)
{
#if VERSION >= VERSION_NTSC_1_0
	musicSaveInterval();
	musicQueueStopAllEvent();
	musicRestoreInterval();
#else
	musicQueueStopAllEvent();
#endif
}

void musicActivateNrg(void)
{
#if VERSION >= VERSION_NTSC_1_0
	if (!g_MusicNrgIsActive)
#endif
	{
		if (stageGetNrgTrack(g_MusicStageNum) >= 0) {
			musicQueueStopEvent(TRACKTYPE_NRG);
			musicQueueStopEvent(TRACKTYPE_MENU);
			musicQueueStopEvent(TRACKTYPE_DEATH);
			musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.5, FADETYPE_PAUSE);
			musicStartNrg(0);

			g_MusicNrgIsActive = true;
		}
	}
}

void musicDeactivateNrg(void)
{
#if VERSION >= VERSION_NTSC_1_0
	if (g_MusicNrgIsActive)
#endif
	{
		musicQueueStopEvent(TRACKTYPE_MENU);
		musicQueueStopEvent(TRACKTYPE_DEATH);
		musicQueueFadeEvent(TRACKTYPE_NRG, 1, FADETYPE_STOP);

		if (g_Vars.dontplaynrg == false) {
			musicStartPrimary(0.5);
		}

		g_MusicNrgIsActive = false;
	}
}

/**
 * Called in many places when opening a pause menu.
 */
void musicStartMenu(void)
{
	musicStartTrackAsMenu(menuChooseMusic());
}

void musicEndMenu(void)
{
	musicQueueFadeEvent(TRACKTYPE_MENU, 1, FADETYPE_STOP);

	if (musicIsTrackState(TRACKTYPE_NRG, AL_PLAYING)) {
		musicStartNrg(1);
	} else {
		musicStartPrimary(1);
	}

	g_MenuTrack = -1;
}

void musicStartSoloDeath(void)
{
	g_MusicSoloDeathIsPlaying = true;

#if VERSION >= VERSION_NTSC_1_0
	musicSaveInterval();
#endif

	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicQueueStartEvent(TRACKTYPE_PRIMARY, MUSIC_DEATH_SOLO, 0, VOLUME(g_SfxVolume) > musicGetVolume() ? VOLUME(g_SfxVolume) : musicGetVolume());

#if VERSION >= VERSION_NTSC_1_0
	musicRestoreInterval();
#endif
}

void _musicStartMpDeath(f32 arg0)
{
#if VERSION >= VERSION_NTSC_1_0
	musicSaveInterval();
	musicQueueStartEvent(TRACKTYPE_DEATH, MUSIC_DEATH_MP, arg0, VOLUME(g_SfxVolume) > musicGetVolume() ? VOLUME(g_SfxVolume) : musicGetVolume());
	musicRestoreInterval();
#else
	musicQueueStartEvent(TRACKTYPE_DEATH, MUSIC_DEATH_MP, arg0, VOLUME(g_SfxVolume) > musicGetVolume() ? VOLUME(g_SfxVolume) : musicGetVolume());
#endif
}

void musicStartMpDeath(void)
{
#ifndef PLATFORM_N64
	if (g_MusicDisableMpDeath) {
		return;
	}
#endif

#if VERSION >= VERSION_NTSC_1_0
	musicSaveInterval();
#endif

	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);

	if (g_MusicNrgIsActive) {
		musicQueueFadeEvent(TRACKTYPE_NRG, 0.1f, FADETYPE_PAUSE);
	} else {
		musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.1f, FADETYPE_PAUSE);
	}

	_musicStartMpDeath(0);

	g_MusicDeathTimer240 = TICKS(1200);
	g_MusicMpDeathIsPlaying = true;

#if VERSION >= VERSION_NTSC_1_0
	musicRestoreInterval();
#endif
}

void musicEndDeath(void)
{
	musicQueueFadeEvent(TRACKTYPE_DEATH, 2, FADETYPE_STOP);

	if (g_MusicNrgIsActive) {
		musicStartNrg(2);
	} else {
		musicStartPrimary(2);
	}

	g_MusicMpDeathIsPlaying = false;
}

/**
 * Stop all other music and play the given track.
 *
 * It's used by the AI scripting language, specifically for CI training and
 * the Skedar King battle.
 *
 * The track type used is primary.
 */
void musicPlayTrackIsolated(s32 tracknum)
{
#if VERSION >= VERSION_NTSC_1_0
	musicSaveInterval();
#endif

	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicQueueStartEvent(TRACKTYPE_PRIMARY, tracknum, 0, musicGetVolume());

#if VERSION >= VERSION_NTSC_1_0
	musicRestoreInterval();
#endif
}

/**
 * Restart the level's default tracks after using the isolated track above.
 *
 * It's used by the AI scripting language, specifically when ending CI training.
 */
void musicPlayDefaultTracks(void)
{
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicStartPrimaryWithReason(0.5f, 1);
}

/**
 * Used by the title screen, as well as AF1's NRG theme which never ends.
 */
void musicStartTemporaryPrimary(s32 tracknum)
{
	musicQueueStopEvent(TRACKTYPE_PRIMARY);

	g_TemporaryPrimaryTrack = tracknum;

	musicStartPrimary(0.5f);
}

/**
 * Used by AI scripting on each stage.
 *
 * The cutscene track is played with a primary tracktype.
 *
 * The NRG theme will not play while a cutscene theme is active.
 */
void musicStartCutscene(s32 tracknum)
{
	u32 volume;

	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);

	if (g_SfxVolume < musicGetVolume()) {
		volume = musicGetVolume();
	} else {
		volume = g_SfxVolume;
	}

	musicQueueStartEvent(TRACKTYPE_PRIMARY, tracknum, 0, volume);

	g_Vars.dontplaynrg = true;
}

/**
 * Used by AI scripting on each stage.
 */
void musicEndCutscene(void)
{
	g_Vars.dontplaynrg = false;

	if (!g_IsTitleDemo) {
		musicQueueStopEvent(TRACKTYPE_PRIMARY);
		musicQueueStopEvent(TRACKTYPE_AMBIENT);
		musicStartPrimary(0.5f);
	}
}

/**
 * Used by AI scripting, and only to set the ambient track during the Defection
 * intro and Extraction outro to traffic noises.
 */
void musicStartTemporaryAmbient(s32 tracknum)
{
	g_TemporaryAmbientTrack = tracknum;
	musicQueueStopEvent(TRACKTYPE_AMBIENT);

	musicQueueStartEvent(TRACKTYPE_AMBIENT, tracknum, 0, VOLUME(g_SfxVolume));
}

void musicEndTemporaryAmbient(void)
{
	g_TemporaryAmbientTrack = -1;
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
}

void musicSetXReason(s32 reason, u32 minsecs, u32 maxsecs)
{
	if (g_AudioXReasonsActive[reason] == false) {
		g_AudioXReasonsActive[reason] = true;
		g_MusicXReasonMinDurations[reason] = minsecs * TICKS(240);
		g_MusicXReasonMaxDurations[reason] = maxsecs * TICKS(240);
	}
}

void musicUnsetXReason(s32 reason)
{
	s32 i;

	if (reason >= 0) {
		g_AudioXReasonsActive[reason] = false;
	} else {
		for (i = 0; i < ARRAYCOUNT(g_AudioXReasonsActive); i++) {
			g_AudioXReasonsActive[i] = false;
			g_MusicXReasonMinDurations[i] = 0;
			g_MusicXReasonMaxDurations[i] = 0;
		}

#if VERSION >= VERSION_NTSC_1_0
		if (g_MusicNrgIsActive) {
			musicDeactivateNrg();
		}
#endif
	}
}

/**
 * Called by musicTick every 0.25 seconds.
 */
void musicTickAmbient(void)
{
	if (g_TemporaryAmbientTrack == -1) {
		if (musicIsAnyPlayerInAmbientRoom()) {
			musicStartAmbient(1);
		} else if (musicGetTrackState(TRACKTYPE_AMBIENT) == AL_PLAYING) {
			musicQueueFadeEvent(TRACKTYPE_AMBIENT, 1, FADETYPE_PAUSE);
		}
	} else if (stageGetAmbientTrack(g_MusicStageNum) >= 0) {
		musicStartAmbient(1);
	}
}

void func0f16e1cc(void)
{
	// empty
}
