#pragma once

#include <PR/ultratypes.h>

#define CONFIG_FNAME "pd.ini"
#define CONFIG_PATH "$S/" CONFIG_FNAME


#define CONFIG_MAX_SECNAME 128
#define CONFIG_MAX_KEYNAME 256
#define CONFIG_MAX_SETTINGS 256 * 8

typedef enum {
	CFG_NONE,
	CFG_S32,
	CFG_F32,
	CFG_U32,
	CFG_STR,
	CFG_U8
} configtype;

struct configentry {
	char key[CONFIG_MAX_KEYNAME + 1];
	s32 seclen;
	configtype type;
	void *ptr;
	union {
		struct { f32 min_f32, max_f32; };
		struct { s32 min_s32, max_s32; };
		struct { u32 min_u32, max_u32; };
		struct { u8 min_u8, max_u8; };
		u32 max_str;
	};
};

extern struct configentry settings[CONFIG_MAX_SETTINGS];

void configInit(void);

// loads config from file (path extensions such as ! apply)
s32 configLoad(const char *fname);
s32 configLoadKey(const char *fname, char *key);

// saves config to file (path extensions such as ! apply)
s32 configSave(const char *fname);

// registers a variable in the config file
// this should be done before configInit() is called, preferably in a module constructor
void configRegisterInt(const char *key, s32 *var, s32 min, s32 max);
void configRegisterUInt(const char* key, u32* var, u32 min, u32 max);
void configRegisterFloat(const char *key, f32 *var, f32 min, f32 max);
void configRegisterString(const char *key, char *var, u32 maxstr);

// player save stuff
struct configentry *configFindPlayerEntry(s32 player, const char *key);
struct configentry *configFindOrAddPlayerEntry(s32 player, const char *key);
void configSetPlayerEntry(s32 player, const char *key, const char *val);
s32 getPlayerConfigSlug(s32 playernum, char *out, char* key);
void configRegisterU8Int(const char *key, u8 *var, u32 min, u32 max);

struct configentry *configFindEntryByPtr(void *ptr);

s32 getConfigIndexFromDB(u16 deviceserial, s32 fileid);
