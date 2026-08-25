#include <stdlib.h>
#include <string.h>
#include <PR/ultratypes.h>
#include "platform.h"
#include "system.h"
#include "fs.h"
#include "utils.h"
#include "romdata.h"
#include "mod.h"
#include "data.h"
#include "game/stagetable.h"

#define MOD_TEXTURES_DIR "textures"
#define MOD_ANIMATIONS_DIR "animations"
#define MOD_SEQUENCES_DIR "sequences"

extern struct stagemusic g_StageTracks[];
extern struct stageallocation g_StageAllocations8Mb[];

#define PARSE_STAGE_FLOAT(sec, name, v, min, max) \
	p = modConfigParseFloatValue(p, token, &v); \
	if (!p || v < (min) || v > (max)) { \
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: " sec " invalid " name " value: %s", stagenum, token); \
		return NULL; \
	}

#define PARSE_STAGE_INT(sec, name, v, min, max) \
	p = modConfigParseIntValue(p, token, &v); \
	if (!p || v < (min) || v > (max)) { \
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: " sec " invalid " name " value: %s", stagenum, token); \
		return NULL; \
	}

#define PARSE_STAGE_FILENAME(sec, name, v) \
	p = modConfigParseFileValue(p, token, &v); \
	if (!p) { \
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: " sec " invalid " name " value: %s", stagenum, token); \
		return NULL; \
	}

#define PARSE_STAGE_STRING(sec, name, v) \
	p = strParseToken(p, token, NULL); \
	if (!p) { \
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: " sec " invalid " name " value: %s", stagenum, token); \
		return NULL; \
	} \
	v = strUnquote(token);

#define PARSE_INT(sec, name, v, min, max, ret) \
	p = modConfigParseIntValue(p, token, &v); \
	if (!p || v < (min) || v > (max)) { \
		sysLogPrintf(LOG_ERROR, "mod: %s: invalid " name " value: %s", sec, token); \
		return ret; \
	}

static inline char *modConfigParseFileValue(char *p, char *token, s32 *filenum)
{
	p = strParseToken(p, token, NULL);
	if (!token[0]) {
		return NULL; // empty 
	}
	// check if it is a number already
	s32 num = strtol(token, NULL, 0);
	if (num > 0 && romdataFileGetName(num)) {
		*filenum = num;
		return p;
	}
	// it's a filename
	num = romdataFileGetNumForName(strUnquote(token));
	if (num >= 0) {
		*filenum = num;
		return p;
	}
	// the filename was invalid
	return NULL;
}

static inline char *modConfigParseIntValue(char *p, char *token, s32 *out)
{
	p = strParseToken(p, token, NULL);
	if (!token[0]) {
		return NULL; // empty 
	}
	char *endp = token;
	const s32 num = strtol(token, &endp, 0);
	if (num == 0 && (endp == token || *endp != '\0')) {
		return NULL;
	}
	*out = num;
	return p;
}

static inline char *modConfigParseFloatValue(char *p, char *token, f32 *out)
{
	p = strParseToken(p, token, NULL);
	if (!token[0]) {
		return NULL; // empty 
	}
	char *endp = token;
	const f32 num = strtof(token, &endp);
	if (num == 0.f && (endp == token || *endp != '\0')) {
		return NULL;
	}
	*out = num;
	return p;
}

static char *modConfigParseStageMusic(char *p, char *token, s32 stagenum)
{
	struct stagemusic *smus = NULL;
	for (struct stagemusic *p = g_StageTracks; p->stagenum; ++p) {
		if (p->stagenum == stagenum) {
			smus = p;
			break;
		}
	}

	if (!smus) {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: music can't be changed for this stage", stagenum);
		return NULL;
	}

	// eat opening bracket
	p = strParseToken(p, token, NULL);
	if (token[0] != '{' || token[1] != '\0') {
		return NULL;
	}

	// parse keyvalues until } is reached
	s32 tmp = 0;
	p = strParseToken(p, token, NULL);
	while (p && token[0] && strcmp(token, "}") != 0) {
		if (!strcmp(token, "primarytrack")) {
			PARSE_STAGE_INT("music:", "primarytrack", tmp, 0, 128);
			smus->primarytrack = tmp;
		} else if (!strcmp(token, "ambienttrack")) {
			PARSE_STAGE_INT("music:", "ambienttrack", tmp, 0, 128);
			smus->ambienttrack = tmp;
		} else if (!strcmp(token, "xtrack")) {
			PARSE_STAGE_INT("music:", "xtrack", tmp, 0, 128);
			smus->xtrack = tmp;
		} else {
			sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: music: invalid key: %s", stagenum, token);
			return NULL;
		}
		p = strParseToken(p, token, NULL);
	}

	if (token[0] != '}') {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: unterminated music block", stagenum);
		return NULL;
	}

	return p;
}

static char *modConfigParseStageWeatherRooms(char *p, char *token, s32 stagenum, struct weathercfg *wcfg)
{
	// determine where we can start adding rooms
	s32 idx;
	for (idx = 0; idx < WEATHERCFG_MAX_SKIPROOMS && wcfg->skiprooms[idx]; ++idx);

	// eat opening bracket
	p = strParseToken(p, token, NULL);
	if (token[0] != '{' || token[1] != '\0') {
		return NULL;
	}

	// check if user wants to clear the whole list
	p = strParseToken(p, token, NULL);
	if (!strcmp(token, "clear")) {
		memset(wcfg->skiprooms, 0, sizeof(wcfg->skiprooms));
		idx = 0;
		p = strParseToken(p, token, NULL);
	}

	s32 tmp = 0;
	while (p && token[0] && strcmp(token, "}") != 0) {
		if (token[0] == ',' && !token[1]) {
			p = strParseToken(p, token, NULL);
			continue;
		}

		tmp = strtol(token, NULL, 0);
		if (tmp <= 0 || tmp > 32767) {
			sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: weather: rooms: invalid room %s", stagenum, token);
			return NULL;
		}

		if (idx < WEATHERCFG_MAX_SKIPROOMS) {
			wcfg->skiprooms[idx++] = tmp;
		}

		p = strParseToken(p, token, NULL);
	}

	if (token[0] != '}') {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: weather: unterminated rooms block", stagenum);
		return NULL;
	}

	return p;
}

static char *modConfigParseStageWeather(char *p, char *token, s32 stagenum)
{
	s32 wi;
	struct weathercfg *wcfg = NULL;
	for (wi = 0; wi < ARRAYCOUNT(g_WeatherConfig) && g_WeatherConfig[wi].stagenum; ++wi) {
		if (g_WeatherConfig[wi].stagenum == stagenum) {
			break;
		}
	}

	if (wi >= WEATHERCFG_MAX_STAGES) {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: no more space for weather config", stagenum);
		return NULL;
	}

	wcfg = &g_WeatherConfig[wi];

	if (!wcfg->stagenum) {
		// new weather config; initialize with defaults
		*wcfg = g_DefaultWeatherConfig;
		wcfg->stagenum = stagenum;
	} else {
		// flags have to be re-specified
		wcfg->flags = 0;
	}

	// eat opening bracket
	p = strParseToken(p, token, NULL);
	if (token[0] != '{' || token[1] != '\0') {
		return NULL;
	}

	// parse keyvalues until } is reached
	s32 tmpi = 0;
	f32 tmpf = 0.f;
	p = strParseToken(p, token, NULL);
	while (p && token[0] && strcmp(token, "}") != 0) {
		if (!strcmp(token, "include_rooms") || !strcmp(token, "exclude_rooms")) {
			// include_rooms | exclude_rooms { ROOM_NUMBERS... }
			const s32 include = (token[0] == 'i');
			p = modConfigParseStageWeatherRooms(p, token, stagenum, wcfg);
			if (!p) {
				return NULL;
			}
			if (wcfg->skiprooms[0] && include) {
				wcfg->flags |= WEATHERFLAG_INCLUDE;
			}
		} else if (!strcmp(token, "cutscene_only")) {
			wcfg->flags |= WEATHERFLAG_CUTSCENE_ONLY;
		} else if (!strcmp(token, "constant_wind")) {
			PARSE_STAGE_FLOAT("weather:", "constant_wind (0)", wcfg->windanglerad, -M_TAU, M_TAU);
			PARSE_STAGE_FLOAT("weather:", "constant_wind (1)", wcfg->windspeedx, -1024.f, 1024.f);
			PARSE_STAGE_FLOAT("weather:", "constant_wind (2)", wcfg->windspeedz, -1024.f, 1024.f);
			wcfg->flags |= WEATHERFLAG_FORCE_WINDDIR;
		} else if (!strcmp(token, "windspeed")) {
			PARSE_STAGE_FLOAT("weather:", "windspeed", tmpf, -1024.f, 1024.f);
			wcfg->windspeed = tmpf;
		} else if (!strcmp(token, "ymin")) {
			PARSE_STAGE_FLOAT("weather:", "ymin", tmpf, -65536.f, 65536.f);
			wcfg->ymin = tmpf;
		} else if (!strcmp(token, "ymax")) {
			PARSE_STAGE_FLOAT("weather:", "ymax", tmpf, -65536.f, 65536.f);
			wcfg->ymax = tmpf;
		} else if (!strcmp(token, "zmax")) {
			PARSE_STAGE_FLOAT("weather:", "zmax", tmpf, -65536.f, 65536.f);
			wcfg->zmax = tmpf;
		} else {
			sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: weather: invalid key: %s", stagenum, token);
			return NULL;
		}
		p = strParseToken(p, token, NULL);
	}

	if (token[0] != '}') {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: unterminated weather block", stagenum);
		return NULL;
	}

	return p;
}

static char *modConfigParseStage(char *p, char *token)
{
	// stage number
	p = strParseToken(p, token, NULL);
	const s32 stagenum = strtol(token, NULL, 0);
	if (stagenum <= 0x01 || stagenum > 0x50) {
		return NULL;
	}

	// eat opening bracket
	p = strParseToken(p, token, NULL);
	if (token[0] != '{' || token[1] != '\0') {
		return NULL;
	}

	// find the stage table pointers this corresponds to
	struct stagetableentry *stab = NULL;
	struct stageallocation *salloc = NULL;
	const s32 sidx = stageGetIndex(stagenum);
	if (sidx >= 0) {
		stab = &g_Stages[sidx];
	} else {
		sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: unknown stage number", stagenum);
		return NULL;
	}
	for (struct stageallocation *p = g_StageAllocations8Mb; p->stagenum; ++p) {
		if (p->stagenum == stagenum) {
			salloc = p;
			break;
		}
	}

	// parse keyvalues until } is reached
	s32 tmp = 0;
	char *tmps = NULL;
	p = strParseToken(p, token, NULL);
	while (p && token[0] && strcmp(token, "}") != 0) {
		if (!strcmp(token, "bgfile")) {
			// bg FILE_NAME_OR_NUM
			PARSE_STAGE_FILENAME("", "bgfile", tmp);
			stab->bgfileid = tmp;
		} else if (!strcmp(token, "tilesfile")) {
			// tilesfile FILE_NAME_OR_NUM
			PARSE_STAGE_FILENAME("", "tilesfile", tmp);
			stab->tilefileid = tmp;
		} else if (!strcmp(token, "padsfile")) {
			// padsfile FILE_NAME_OR_NUM
			PARSE_STAGE_FILENAME("", "padsfile", tmp);
			stab->padsfileid = tmp;
		} else if (!strcmp(token, "setupfile")) {
			// setupfile FILE_NAME_OR_NUM
			PARSE_STAGE_FILENAME("", "setupfile", tmp);
			stab->setupfileid = tmp;
		} else if (!strcmp(token, "mpsetupfile")) {
			// mpsetupfile FILE_NAME_OR_NUM
			PARSE_STAGE_FILENAME("", "mpsetupfile", tmp);
			stab->mpsetupfileid = tmp;
		} else if (!strcmp(token, "alarm")) {
			PARSE_STAGE_INT("", "alarm", tmp, 1, 0xFFFF);
			stab->alarm = tmp;
		} else if (!strcmp(token, "extragunmem")) {
			PARSE_STAGE_INT("", "extragunmem", tmp, 0, 0xFFFF);
			stab->extragunmem = tmp;
		}  else if (!strcmp(token, "allocation")) {
			// allocation "ALLOCSTRING"
			PARSE_STAGE_STRING("", "allocation", tmps);
			// FIXME: this leaks
			tmps = strDuplicate(tmps);
			if (tmps) {
				salloc->string = tmps;
			}
		}	else if (!strcmp(token, "music")) {
			// music { KEYVALUES... }
			p = modConfigParseStageMusic(p, token, stagenum);
			if (!p) {
				return NULL;
			}
		} else if (!strcmp(token, "weather")) {
			// weather { KEYVALUES... }
			p = modConfigParseStageWeather(p, token, stagenum);
			if (!p) {
				return NULL;
			}
		} else {
			sysLogPrintf(LOG_ERROR, "modconfig: stage 0x%02x: invalid key: %s", stagenum, token);
			return NULL;
		}
		p = strParseToken(p, token, NULL);
	}

	if (token[0] != '}') {
		sysLogPrintf(LOG_ERROR, "modconfig: unterminated stage 0x%02x block", stagenum);
		return NULL;
	}

	return p;
}

s32 modConfigLoad(const char *fname)
{
	u32 dataLen = 0;
	char *data = fsFileLoad(fname, &dataLen);
	if (!data) {
		return false;
	}

	s32 success = true;
	char token[UTIL_MAX_TOKEN + 1] = { 0 };
	char *end = data + dataLen;
	char *p = strParseToken(data, token, NULL);
	while (p && token[0]) {
		if (!strcmp(token, "stage")) {
			// stage NUMBER { KEYVALUES... }
			char *prev = p;
			p = modConfigParseStage(p, token);
			if (!p) {
				sysLogPrintf(LOG_ERROR, "modconfig: malformed stage block at offset %d", prev - data);
				success = false;
				break;
			}
		} else {
			// garbage
			sysLogPrintf(LOG_ERROR, "modconfig: unexpected %s at offset %d", token[0] ? token : "end of file", p - data);
			success = false;
			break;
		}
		p = strParseToken(p, token, NULL);
	}

	sysMemFree(data);
	return success;
}

s32 modTextureLoad(u16 num, void *dst, u32 dstSize)
{
	static s32 dirExists = -1;
	if (dirExists < 0) {
		dirExists = (fsFileSize(MOD_TEXTURES_DIR) >= 0);
	}

	if (!dirExists) {
		return -1;
	}

	char path[FS_MAXPATH + 1];
	snprintf(path, sizeof(path), MOD_TEXTURES_DIR "/%04x.bin", num);

	const s32 ret = fsFileLoadTo(path, dst, dstSize);
	if (ret > 0) {
		sysLogPrintf(LOG_NOTE, "mod: loaded external texture %04x", num);
	}

	return ret;
}

void *modSequenceLoad(u16 num, u32 *outSize)
{
	static s32 dirExists = -1;
	if (dirExists < 0) {
		dirExists = (fsFileSize(MOD_SEQUENCES_DIR) >= 0);
	}

	if (!dirExists) {
		return NULL;
	}

	char path[FS_MAXPATH + 1];
	snprintf(path, sizeof(path), MOD_SEQUENCES_DIR "/%04x.bin", num);
	if (fsFileSize(path) > 0) {
		void *ret = fsFileLoad(path, outSize);
		if (ret) {
			sysLogPrintf(LOG_NOTE, "mod: loaded external sequence %04x", num);
			return ret;
		}
	}

	return NULL;
}

void *modAnimationLoadData(u16 num)
{
	char path[FS_MAXPATH + 1];
	// load the animation data
	snprintf(path, sizeof(path), MOD_ANIMATIONS_DIR "/%04x.bin", num);
	void *data = fsFileLoad(path, NULL);
	if (!data) {
		sysFatalError("External animation %04x has no data file.\nEnsure that it is placed at %s or delete the descriptor.", num, path);
	}
	return data;
}

s32 modAnimationLoadDescriptor(u16 num, struct animtableentry *anim)
{
	static s32 dirExists = -1;
	if (dirExists < 0) {
		dirExists = (fsFileSize(MOD_ANIMATIONS_DIR) >= 0);
	}

	if (!dirExists) {
		return false;
	}

	char path[FS_MAXPATH + 1];

	// load the descriptor, if any
	snprintf(path, sizeof(path), MOD_ANIMATIONS_DIR "/%04x.txt", num);
	if (fsFileSize(path) <= 0) {
		return false;
	}

	char *desc = fsFileLoad(path, NULL);
	if (!desc) {
		return false;
	}

	// parse the descriptor
	char token[UTIL_MAX_TOKEN + 1] = { 0 };
	char *p = strParseToken(desc, token, NULL);
	s32 tmp = 0;
	while (p && token[0]) {
		if (!strcmp(token, "numframes")) {
			PARSE_INT(path, "numframes", tmp, 0, 0xFFFF, false);
			anim->numframes = tmp;
		} else if (!strcmp(token, "bytesperframe")) {
			PARSE_INT(path, "bytesperframe", tmp, 0, 0xFFFF, false);
			anim->bytesperframe = tmp;
		} else if (!strcmp(token, "headerlen")) {
			PARSE_INT(path, "headerlen", tmp, 0, 0xFFFF, false);
			anim->headerlen = tmp;
		} else if (!strcmp(token, "framelen")) {
			PARSE_INT(path, "framelen", tmp, 0, 0xFF, false);
			anim->framelen = tmp;
		} else if (!strcmp(token, "flags")) {
			PARSE_INT(path, "flags", tmp, 0, 0xFF, false);
			anim->flags = tmp;
		} else {
			sysLogPrintf(LOG_ERROR, "mod: %s: invalid key: %s", path, token);
			return false;
		}
		p = strParseToken(p, token, NULL);
	}

	sysMemFree(desc);

	sysLogPrintf(LOG_NOTE, "mod: loaded external animation %04x", num);

	return true;
}
