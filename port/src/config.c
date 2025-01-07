#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <PR/ultratypes.h>
#include "fs.h"
#include "config.h"
#include "system.h"
#include "utils.h"
#include "types.h"
#include "bss.h"

struct configentry settings[CONFIG_MAX_SETTINGS];
static s32 numSettings = 0;

static inline s32 configClampInt(s32 val, s32 min, s32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline u32 configClampUInt(u32 val, u32 min, u32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline f32 configClampFloat(f32 val, f32 min, f32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline struct configentry *configFindEntry(const char *key)
{
	for (s32 i = 0; i < numSettings; ++i) {
		if (!strncasecmp(settings[i].key, key, CONFIG_MAX_KEYNAME)) {
			return &settings[i];
		}
	}
	return NULL;
}

static inline struct configentry *configAddEntry(const char *key)
{
	if (numSettings < CONFIG_MAX_SETTINGS) {
		struct configentry *cfg = &settings[numSettings++];
		strncpy(cfg->key, key, CONFIG_MAX_KEYNAME);
		const char *delim = strrchr(cfg->key, '.');
		cfg->seclen = delim ? (delim - cfg->key) : 0;
		return cfg;
	}
	printf("configAddEntry: too many settings: %s\n", key);
	return NULL;
}

static inline struct configentry *configFindOrAddEntry(const char *key)
{
	for (s32 i = 0; i < numSettings; ++i) {
		if (!strncasecmp(settings[i].key, key, CONFIG_MAX_KEYNAME)) {
			return &settings[i];
		}
	}
	return configAddEntry(key);
}

static inline const char *configGetSection(char *sec, const struct configentry *cfg)
{
	if (!cfg->seclen || cfg->seclen > CONFIG_MAX_SECNAME) {
		strncpy(sec, cfg->key, CONFIG_MAX_SECNAME);
		sec[CONFIG_MAX_SECNAME] = '\0';
		return sec;
	}

	memcpy(sec, cfg->key, cfg->seclen);
	sec[cfg->seclen] = '\0';

	return sec;
}

void configRegisterInt(const char *key, s32 *var, s32 min, s32 max)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_S32;
		cfg->ptr = var;
		cfg->min_s32 = min;
		cfg->max_s32 = max;
	}
}

void configRegisterUInt(const char* key, u32* var, u32 min, u32 max)
{
	struct configentry* cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_U32;
		cfg->ptr = var;
		cfg->min_u32 = min;
		cfg->max_u32 = max;
	}
}

void configRegisterU8Int(const char* key, u8* var, u32 min, u32 max)
{
	struct configentry* cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_U8;
		cfg->ptr = var;
		cfg->min_u32 = min;
		cfg->max_u32 = max;
	}
}

void configRegisterFloat(const char *key, f32 *var, f32 min, f32 max)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_F32;
		cfg->ptr = var;
		cfg->min_f32 = min;
		cfg->max_f32 = max;
	}
}

void configRegisterString(const char *key, char *var, u32 maxstr)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_STR;
		cfg->ptr = var;
		cfg->max_str = maxstr;
	}
}

void configSet(struct configentry *cfg, const char *val) {
	s32 tmp_s32;
	f32 tmp_f32;
	u32 tmp_u32;
	u8  tmp_u8;
	if (!cfg->ptr) {
		return;
	}
	switch (cfg->type) {
		case CFG_S32:
			tmp_s32 = strtol(val, NULL, 0);
			if (cfg->min_s32 < cfg->max_s32) {
				tmp_s32 = configClampInt(tmp_s32, cfg->min_s32, cfg->max_s32);
			}
			*(s32 *)cfg->ptr = tmp_s32;
			break;
		case CFG_F32:
			tmp_f32 = strtof(val, NULL);
			if (cfg->min_f32 < cfg->max_f32) {
				tmp_f32 = configClampFloat(tmp_f32, cfg->min_f32, cfg->max_f32);
			}
			*(f32 *)cfg->ptr = tmp_f32;
			break;
		case CFG_U32:
			tmp_u32 = strtoul(val, NULL, 0);
			if (cfg->min_u32 < cfg->max_u32) {
				tmp_u32 = configClampUInt(tmp_u32, cfg->min_u32, cfg->max_u32);
			}
			*(u32*)cfg->ptr = tmp_u32;
			break;
		case CFG_U8:
			tmp_u8 = strtoul(val, NULL, 0);
			if (cfg->min_u8 < cfg->max_u8) {
				tmp_u8 = configClampUInt(tmp_u8, cfg->min_u8, cfg->max_u8);
			}
			*(u8*)cfg->ptr = tmp_u8;
			break;
		case CFG_STR:
			strncpy(cfg->ptr, val, cfg->max_str ? cfg->max_str - 1 : 4096);
			break;
		default:
			break;
	}
}

static inline void configSetFromString(const char *key, const char *val)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (!cfg) {
		return;
	}

	configSet(cfg, val);
}

static void configSaveEntry(struct configentry *cfg, FILE *f)
{
	if (!cfg->ptr) {
		return;
	}
	switch (cfg->type) {
		case CFG_S32:
			if (cfg->min_s32 < cfg->max_s32) {
				*(s32 *)cfg->ptr = configClampInt(*(s32 *)cfg->ptr, cfg->min_s32, cfg->max_s32);
			}
			fprintf(f, "%s=%d\n", cfg->key + cfg->seclen + 1, *(s32 *)cfg->ptr);
			break;
		case CFG_F32:
			if (cfg->min_f32 < cfg->max_f32) {
				*(f32 *)cfg->ptr = configClampFloat(*(f32 *)cfg->ptr, cfg->min_f32, cfg->max_f32);
			}
			fprintf(f, "%s=%f\n", cfg->key + cfg->seclen + 1, *(f32 *)cfg->ptr);
			break;
		case CFG_U32:
			if (cfg->min_u32 < cfg->max_u32) {
				*(u32*)cfg->ptr = configClampUInt(*(u32*)cfg->ptr, cfg->min_u32, cfg->max_u32);
			}
			fprintf(f, "%s=%u\n", cfg->key + cfg->seclen + 1, *(u32 *)cfg->ptr);
			break;
		case CFG_U8:
			if (cfg->min_u8 < cfg->max_u8) {
				*(u8*)cfg->ptr = configClampUInt(*(u8*)cfg->ptr, cfg->min_u8, cfg->max_u8);
			}
			fprintf(f, "%s=%u\n", cfg->key + cfg->seclen + 1, *(u8 *)cfg->ptr);
			break;
		case CFG_STR:
			fprintf(f, "%s=%s\n", cfg->key + cfg->seclen + 1, (char *)cfg->ptr);
			break;
		default:
			break;
	}
}

struct configentry *configFindEntryByPtr(void *ptr)
{
	for (s32 i = 0; i < numSettings; ++i) {
		if (settings[i].ptr == ptr) {
			return &settings[i];
		}
	}
	return NULL;
}

s32 configSave(const char *fname)
{
	FILE *f = fsFileOpenWrite(fname);
	if (!f) {
		return 0;
	}

	char tmpSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	char curSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	configGetSection(curSec, &settings[0]);
	fprintf(f, "[%s]\n", curSec);

	for (s32 i = 0; i < numSettings; ++i) {
		struct configentry *cfg = &settings[i];
		configGetSection(tmpSec, cfg);
		if (strncmp(curSec, tmpSec, CONFIG_MAX_SECNAME) != 0) {
			fprintf(f, "\n[%s]\n", tmpSec);
			strncpy(curSec, tmpSec, CONFIG_MAX_SECNAME);
		}
		configSaveEntry(cfg, f);
	}

	fsFileFree(f);
	return 1;
}

static inline s32 configLoadFileIdFromSection(const char *sec, u16 *deviceserial, s32 *fileid)
{
    u32 tmp_deviceserial = 0;
    s32 tmp_fileid = 0;

    // Ensure sscanf matches both fields
    if (sscanf(sec, "MpPlayer.%x-%x", &tmp_deviceserial, &tmp_fileid) == 2) {
        *deviceserial = tmp_deviceserial;
        *fileid = tmp_fileid;
        return 1;
    }

    // If sscanf did not match both fields, return 0
    return 0;
}

s32 configLoadKey(const char *fname, char *key)
{
	FILE *f = fsFileOpenRead(fname);
	if (!f) {
		return 0;
	}

	char curSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	char keyBuf[CONFIG_MAX_SECNAME * 2 + 2] = { 0 }; // SECTION + . + KEY + \0
	char token[UTIL_MAX_TOKEN + 1] = { 0 };
	char lineBuf[2048] = { 0 };
	char *line = lineBuf;
	s32 lineLen = 0;

	while (fgets(lineBuf, sizeof(lineBuf), f)) {
		line = lineBuf;

		line = strParseToken(line, token, NULL);

		if (token[0] == '[' && token[1] == '\0') {
			// section; get name
			line = strParseToken(line, token, NULL);
			if (!token[0]) {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed section line: %s", lineBuf);
				continue;
			}
			strncpy(curSec, token, CONFIG_MAX_SECNAME);
			// detect if curSec has a fileguid in it
			u16 deviceserial = 0;
			s32 fileid = 0;
			s32 configindex = -1;
			if (!key && configLoadFileIdFromSection(curSec, &deviceserial, &fileid)) {
				g_GuidsToProcess[g_NumGuidsToProcess++] = (struct fileguid) { fileid, deviceserial };
			}
			// eat ]
			line = strParseToken(line, token, NULL);
			if (token[0] != ']' || token[1] != '\0') {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed section line: %s", lineBuf);
			}
		} else if (token[0]) {
			// probably a key=value pair; append key name to section name
			snprintf(keyBuf, sizeof(keyBuf) - 1, "%s.%s", curSec, token);
			// eat =
			line = strParseToken(line, token, NULL);
			if (token[0] != '=' || token[1] != '\0') {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed keyvalue line: %s", lineBuf);
				continue;
			}
			// the rest of the line is the value
			line = strTrim(line);
			if (line[0] == '"') {
				line = strUnquote(line);
			}
			if (!key || (key && strcmp(keyBuf, key) == 0)) {
				configSetFromString(keyBuf, line);
			}
		}
	}

	fsFileFree(f);

	return 1;

}
s32 configLoad(const char *fname)
{
	return configLoadKey(fname, 0);
}

void configInit(void)
{
	if (fsFileSize(CONFIG_PATH) > 0) {
		configLoad(CONFIG_PATH);
	}
}

s32 getPlayerConfigSlug(s32 playernum, char *out, char *key)
{
	struct fileguid *guid = &(g_PlayerConfigsArray[playernum].fileguid);
	if (guid && key) {
		sprintf(out, "MpPlayer.%x-%x.%s\0",  guid->deviceserial, guid->fileid, key);
		return 1;
	} else if (guid) {
		sprintf(out, "MpPlayer.%x-%x\0",  guid->deviceserial, guid->fileid);
		return 1;
	}
	return 0;
}

s32 getConfigIndexFromDB(u16 deviceserial, s32 fileid) {
	s32 retval = -1;
	for (s32 i = 0; i < numSettings; ++i) {
		if (strncmp(settings[i].key, "MpPlayer.", 9) != 0) {
			continue;
		}
	}
	return retval;
}
