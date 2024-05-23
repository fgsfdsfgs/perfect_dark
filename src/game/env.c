#include <ultra64.h>
#include "constants.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/bg.h"
#include "game/env.h"
#include "bss.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"

bool g_FogEnabled;
bool g_EnvHasTransparency;
struct distfadesettings *g_EnvDistFadeSettingsPtr;
struct distfadesettings g_EnvDistFadeSettings;
u32 var800a65fc;

struct shadesettings g_EnvShadeSettings;
f32 g_EnvFogMaxFrac;
f32 g_EnvFogMinFrac;
struct fogenvironment *g_EnvOrigFogEnvironment;
struct fogenvironment *g_EnvTransitionFrom;
struct fogenvironment *g_EnvTransitionTo;

f32 g_EnvFogMax = MAXFLOAT;
f32 g_EnvFogMin = 0;
struct environment g_Env = {900, 1000};
u32 var800810a8 = 0;
u32 var800810ac = 0;

#define RGB(col) col >> 16, (col >> 8) & 0xff, col & 0xff
#define NO_SUNS 0, NULL
#define SUNS(arr) ARRAYCOUNT(arr), arr

//                   lensflare,        colour,        x,       y,        z, texture size, orb size
struct sun suns_00[]     = { 1, RGB(0xffffff), -1000000,  200000,    50000, 20, 48 };
struct sun suns_area51[] = { 0, RGB(0xffe080),  -200000,  310000,  1000000, 20, 48 };
struct sun suns_villa[]  = { 1, RGB(0xffffff), -1000000,  200000,    50000, 20, 48 };
struct sun suns_ci[]     = { 1, RGB(0xffffff),   400000,  600000, -1000000, 20, 48 };

struct sun suns_skedar[] = {
	{ 1, RGB(0xffffff),  -400000,  600000,  1000000, 20, 48 },
	{ 1, RGB(0xffffff),        0, 1000000,  1000000, 25, 60 },
	{ 1, RGB(0xffffff),   400000,  600000,  1000000, 15, 36 },
};

struct sun suns_crashsite[] = { 1, RGB(0xffd7f2),  1900000,  300000, -1400000, 22, 48 };
struct sun suns_airbase[]   = { 1, RGB(0xffd7f2), -1200000,  200000,   150000, 30, 60 };

struct fogenvironment g_FogEnvironments[] = {
	//                                   |- distfade --|  |- fog -|                                       |---------- clouds ---------|  |--------- water ---------|
	// stage                near    far  opa%  xlu%  ref  min   max  sky colour                           e  scale  type  colour         e  scale type colour        clouds_height
	{ STAGE_CRASHSITE,        15, 10000,    0,    0,   0, 994, 1000, RGB(0x9b2d1e), SUNS(suns_crashsite), 1,  1500,    0, RGB(0xfafa00), 0, -5000, 0, RGB(0x000000), 0  },
	{ STAGE_PELAGIC,          15, 15000, 3333, 4444, 600, 995, 1000, RGB(0x2d3e60), NO_SUNS,              1,  5000,    0, RGB(0xf0f0f0), 0, -2000, 0, RGB(0x14212b), 0  },
	{ STAGE_VILLA,            15, 20000,    0,    0,   0, 981, 1047, RGB(0x46a0ff), SUNS(suns_villa),     1,  5000,    0, RGB(0xffffff), 0, -1850, 1, RGB(0x00ffff), 0  },
	{ STAGE_PELAGIC + 900,    30, 15000, 3333, 4444, 600, 995, 1000, RGB(0x103060), NO_SUNS,              1,  5000,    0, RGB(0xffffff), 0, -1000, 0, RGB(0x000000), 0  },
	{ STAGE_24,                2, 25000,    0,    0,   0, 996, 1000, RGB(0x000000), NO_SUNS,              1, 10000,    0, RGB(0xffffff), 0,     0, 1, RGB(0x00ffff), 7  },
	{ STAGE_2B,                2,  2000, 2500, 3055, 750, 957, 1000, RGB(0x201010), NO_SUNS,              1,  5000,    0, RGB(0x3a1100), 0, -1000, 1, RGB(0x00ffff), 20 },
	{ STAGE_2B + 900,          2,  8000, 6000, 8000, 800, 991, 1000, RGB(0x201010), NO_SUNS,              1,  5000,    0, RGB(0x3a1100), 0, -1000, 1, RGB(0x00ffff), 0  },
	{ 238,                    10,  6000,    0,    0,   0, 996, 1000, RGB(0x181818), NO_SUNS,              1, 10000,    0, RGB(0x787878), 0,  -500, 0, RGB(0x000000), 0  },
	{ 338,                    10,  6000,    0,    0,   0, 996, 1000, RGB(0x181818), NO_SUNS,              1, 10000,    0, RGB(0x787878), 0,  -500, 0, RGB(0x000000), 0  },
	{ 438,                    10,  6000,    0,    0,   0, 996, 1000, RGB(0x181818), NO_SUNS,              1, 10000,    0, RGB(0x787878), 0,  -500, 0, RGB(0x000000), 0  },
	{ 200,                    10,  7500,    0,    0,   0, 996, 1000, RGB(0x000000), NO_SUNS,              0,     0,    0, RGB(0x000000), 0,     0, 0, RGB(0x000000), 0  },
	{ 300,                    10,  6000,    0,    0,   0, 996, 1000, RGB(0x000000), NO_SUNS,              0,     0,    0, RGB(0x000000), 0,     0, 0, RGB(0x000000), 0  },
	{ 400,                    10,  5000,    0,    0,   0, 996, 1000, RGB(0x000000), NO_SUNS,              0,     0,    0, RGB(0x000000), 0,     0, 0, RGB(0x000000), 0  },
	{ 0,                       0,     0,    0,    0,   0,   0,    0, RGB(0x000000), NO_SUNS,              0,     0,    0, RGB(0x000000), 0,     0, 0, RGB(0x000000), 0  },
};

struct nofogenvironment g_NoFogEnvironments[] = {
	//                                                                                                                                              clouds_height
	//                                |distfade |                                     |--------- clouds ---------|  |----------- water ----------|  |  transparency
	// stage             near    far  opa xlu ref  sky colour                         e  colour         scale type  e  colour          scale  type  |  |
	{ -1,                  15, 10000,   0,  0,  0, RGB(0x001040), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 0 },
	{ STAGE_RESCUE,        15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_MAIANSOS,      15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_ATTACKSHIP,    15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 0 },
	{ STAGE_INFILTRATION,  15, 12000,   0,  0,  0, RGB(0x000000), SUNS(suns_area51),  0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_ESCAPE,        15, 10000,   0,  0,  0, RGB(0x000000), SUNS(suns_area51),  0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_TEST_ARCH,     15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 0 },
	{ STAGE_DEEPSEA,       15, 10000,   0,  0,  0, RGB(0x050000), NO_SUNS,            0, RGB(0x9b9b9b),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 0 },
	{ STAGE_TEST_DEST,     15, 10000,   0,  0,  0, RGB(0x103060), NO_SUNS,            1, RGB(0xe6e6e6),  3000,   0, 1, RGB(0xffff96),   -150,    2, 0, 0 },
	{ STAGE_TEST_LEN,      15, 10000,   0,  0,  0, RGB(0x304010), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 0 },
	{ STAGE_TEST_ASH,      15, 25000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_DEFENSE,       15, 10000,   0,  0,  0, RGB(0x65b2ff), SUNS(suns_ci),      0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_CITRAINING,    15, 10000,   0,  0,  0, RGB(0x65b2ff), SUNS(suns_ci),      0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_DUEL,          10, 10000,   0,  0,  0, RGB(0x65b2ff), SUNS(suns_ci),      0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_RETAKING,      15, 10000,   0,  0,  0, RGB(0x65b2ff), SUNS(suns_ci),      0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_DEFECTION,     10, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_MBR,           10, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_EXTRACTION,    10, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_SKEDARRUINS,   15, 10000,   0,  0,  0, RGB(0x6565ff), SUNS(suns_skedar),  0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_WAR,           15, 10000,   0,  0,  0, RGB(0x6565ff), SUNS(suns_skedar),  0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_TEST_RUN,      15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),      0,    0, 0, 1 },
	{ STAGE_MP_SKEDAR,     15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_TEST_SILO,     15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x3a1100),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_CHICAGO,       10, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x50280a),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_G5BUILDING,    15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x50280a),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_AIRFORCEONE,   15, 20000,   0,  0,  0, RGB(0x001040), NO_SUNS,            1, RGB(0xffffff),  5000,   0, 1, RGB(0xffffff),  -5000,    2, 0, 1 },
	{ STAGE_MP_RAVINE,     15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x50280a),  5000,   0, 0, RGB(0x000000), -31000,    0, 0, 1 },
	{ STAGE_AIRBASE,       15, 20000,   0,  0,  0, RGB(0x001040), SUNS(suns_airbase), 1, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_INVESTIGATION, 15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ STAGE_MP_PIPES,      15, 10000,   0,  0,  0, RGB(0x000008), NO_SUNS,            1, RGB(0x46c7ba),  4500,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_G5BUILDING, 15, 10000,   0,  0,  0, RGB(0x000008), NO_SUNS,            1, RGB(0x5a90a5),  4500,   0, 0, RGB(0x000000), -20000,    0, 0, 0 },
	{ STAGE_MP_TEMPLE,     15, 10000,   0,  0,  0, RGB(0x001080), NO_SUNS,            1, RGB(0xffffff),  5000,   0, 0, RGB(0x00ffff),  -1850,    1, 0, 1 },
	{ STAGE_MP_COMPLEX,    15, 10000,   0,  0,  0, RGB(0x020000), NO_SUNS,            1, RGB(0x82aac8),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_UFF,      10, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_OLD,      15, 30000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_LAM,      15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_BASE,       15, 20000,   0,  0,  0, RGB(0x040000), NO_SUNS,            1, RGB(0x82b464),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP2,      15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 }, // Custom: PD Plus Stack
	{ STAGE_MP_AREA52,     15, 10000,   0,  0,  0, RGB(0x000008), NO_SUNS,            1, RGB(0x46c7ba),  4500,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_WAREHOUSE,  15, 10000,   0,  0,  0, RGB(0x020000), NO_SUNS,            1, RGB(0x82aac8),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_CARPARK,    15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x64c886),  5500,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP6,      15, 20000,   0,  0,  0, RGB(0x001050), NO_SUNS,            1, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 }, // Custom: PD Plus Caves
	{ STAGE_TEST_MP7,      15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP8,      15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_RUINS,      15, 10000,   0,  0,  0, RGB(0x030000), NO_SUNS,            1, RGB(0x82e6aa),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_SEWERS,     15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x646464),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_FELICITY,   10, 10000,   0,  0,  0, RGB(0x040500), NO_SUNS,            1, RGB(0x64d282),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_FORTRESS,   15, 10000,   0,  0,  0, RGB(0x000008), NO_SUNS,            1, RGB(0x5a90a5),  5500,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_VILLA,      15, 10000,   0,  0,  0, RGB(0x8888dc), NO_SUNS,            1, RGB(0xffaa2a),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP14,     15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_MP_GRID,       15, 10000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP16,     15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP17,     15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            1, RGB(0x64c886), 10000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP18,     15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP19,     15, 20000,   0,  0,  0, RGB(0x000000), NO_SUNS,            0, RGB(0x1e1e1e),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 0 },
	{ STAGE_TEST_MP20,     15, 20000,   0,  0,  0, RGB(0x00103c), NO_SUNS,            1, RGB(0xffffff),  5000,   0, 0, RGB(0x000000),  -5000,    0, 0, 1 },
	{ 0 },
};

void env0f1657c0(s32 arg0, s32 arg1)
{
	// empty
}

struct environment *envGetCurrent(void)
{
	return &g_Env;
}

f32 envGetFogMax(void)
{
	return g_EnvFogMax;
}

f32 envGetSquaredFogMax(void)
{
	return g_EnvFogMax * g_EnvFogMax;
}

void envTick(void)
{
	struct zrange zrange;
	f32 scale;
	f32 zfar;
	f32 znear;
	f32 sp28;
	f32 sp24;

	if (!g_FogEnabled) {
		return;
	}

	viGetZRange(&zrange);

	scale = bgGetScaleBg2Gfx();

	zrange.near /= scale;
	zrange.far /= scale;

	// g_Env.fogmin and fogmax are a percentage of the draw distance, where a
	// value of 1000 is 100%. So this is changing it back to a frac (0-1).
	g_EnvFogMinFrac = g_Env.fogmin * 0.001f;
	g_EnvFogMaxFrac = g_Env.fogmax * 0.001f;

	g_EnvFogMax = (zrange.far - zrange.near) * g_EnvFogMaxFrac + zrange.f[0];
	g_EnvFogMin = (zrange.far - zrange.near) * g_EnvFogMinFrac + zrange.f[0];

	g_EnvShadeSettings.znear = zrange.near;
	g_EnvShadeSettings.zfar = zrange.far;

	zfar = g_EnvShadeSettings.zfar;
	znear = g_EnvShadeSettings.znear;
	sp28 = 128.0f / (g_EnvFogMaxFrac - g_EnvFogMinFrac);
	sp24 = (0.5f - g_EnvFogMinFrac) * 256.0f / (g_EnvFogMaxFrac - g_EnvFogMinFrac);

	g_EnvShadeSettings.alphafar = -sp28 * zfar * (znear + 1.0f) / (zfar - znear) / 255.0f;
	g_EnvShadeSettings.alphanear = (sp28 * (zfar + 1.0f) / (zfar - znear) + sp24) / 255.0f;
}

void envApplyFogEnvironment(struct fogenvironment *env)
{
	g_FogEnabled = true;
	g_EnvHasTransparency = false;

	viSetZRange(env->near, env->far);

	g_Env.fogmin = env->fogmin;
	g_Env.fogmax = env->fogmax;

	g_Env.sky_r = env->sky_r;
	g_Env.sky_g = env->sky_g;
	g_Env.sky_b = env->sky_b;
	g_Env.skyredfrac = g_Env.sky_r / 255.0f;
	g_Env.skygreenfrac = g_Env.sky_g / 255.0f;
	g_Env.skybluefrac = g_Env.sky_b / 255.0f;

	g_Env.numsuns = env->numsuns;
	g_Env.suns = env->suns;

	g_Env.clouds_enabled = env->clouds_enabled;
	g_Env.clouds_scale = env->clouds_scale;
	g_Env.clouds_type = env->clouds_type;
	g_Env.clouds_r = env->clouds_r;
	g_Env.clouds_g = env->clouds_g;
	g_Env.clouds_b = env->clouds_b;

	g_Env.water_enabled = env->water_enabled;
	g_Env.water_scale = env->water_scale;
	g_Env.water_type = env->water_type;
	g_Env.water_r = env->water_r;
	g_Env.water_g = env->water_g;
	g_Env.water_b = env->water_b;
	g_Env.clouds_height = env->clouds_height;

	if (!env->opaperc) {
		g_EnvDistFadeSettingsPtr = NULL;
	} else {
		g_EnvDistFadeSettings.opaperc = env->opaperc;
		g_EnvDistFadeSettings.xluperc = env->xluperc;
		g_EnvDistFadeSettings.refdist = env->refdist;
		g_EnvDistFadeSettingsPtr = &g_EnvDistFadeSettings;
	}

	envTick();
}

void envApplyNoFogEnvironment(struct nofogenvironment *env)
{
	struct zrange zrange;

	viSetZRange(env->near, env->far);

	g_Env.sky_r = env->sky_r;
	g_Env.sky_g = env->sky_g;
	g_Env.sky_b = env->sky_b;

	g_Env.skyredfrac = g_Env.sky_r / 255.0f;
	g_Env.skygreenfrac = g_Env.sky_g / 255.0f;
	g_Env.skybluefrac = g_Env.sky_b / 255.0f;

	g_Env.numsuns = env->numsuns;
	g_Env.suns = env->suns;
	g_Env.clouds_enabled = env->clouds_enabled;
	g_Env.clouds_scale = env->clouds_scale;
	g_Env.clouds_type = env->clouds_type;

	g_Env.clouds_r = env->clouds_r;
	g_Env.clouds_g = env->clouds_g;
	g_Env.clouds_b = env->clouds_b;

	g_Env.water_enabled = env->water_enabled;
	g_Env.water_scale = env->water_scale;
	g_Env.water_type = env->water_type;

	g_Env.water_r = env->water_r;
	g_Env.water_g = env->water_g;
	g_Env.water_b = env->water_b;
	g_Env.clouds_height = env->clouds_height;

	if (!env->opaperc) {
		g_EnvDistFadeSettingsPtr = NULL;
	} else {
		g_EnvDistFadeSettings.opaperc = env->opaperc;
		g_EnvDistFadeSettings.xluperc = env->xluperc;
		g_EnvDistFadeSettings.refdist = env->refdist;
		g_EnvDistFadeSettingsPtr = &g_EnvDistFadeSettings;
	}

	g_FogEnabled = false;

	g_EnvHasTransparency = env->transparency;
}

void envDisableSky(void)
{
	g_Env.sky_r = 0;
	g_Env.sky_g = 0;
	g_Env.sky_b = 0;

	g_Env.skyredfrac = 0;
	g_Env.skygreenfrac = 0;
	g_Env.skybluefrac = 0;
}

void envSetStageNum(s32 stagenum)
{
	// empty
}

void envChooseAndApply(s32 stagenum, bool allowoverride)
{
	struct nofogenvironment *finalenv = NULL;
	struct nofogenvironment *env2;
	struct fogenvironment *env1;

	if (PLAYERCOUNT());

	g_EnvFogMax = MAXFLOAT;
	g_EnvFogMin = 0;

	// If allowoverride is set, try to find an env1 with stage + 900.
	// But allowoverride is never set, so this never happens.
	if (allowoverride) {
		for (env1 = &g_FogEnvironments[0]; env1->stage != 0; env1++) {
			if (env1->stage == stagenum + 900) {
				g_EnvOrigFogEnvironment = env1;
				g_EnvTransitionFrom = env1;
				g_EnvTransitionTo = env1 + 1;
				envApplyFogEnvironment(g_EnvOrigFogEnvironment);
				return;
			}
		}
	}

	// Try to find an env1
	for (env1 = &g_FogEnvironments[0]; env1->stage != 0; env1++) {
		if (env1->stage == stagenum) {
			g_EnvOrigFogEnvironment = env1;
			g_EnvTransitionFrom = env1;
			g_EnvTransitionTo = env1 + 1;
			envApplyFogEnvironment(g_EnvOrigFogEnvironment);
			return;
		}
	}

	// Try to find an env2
	for (env2 = &g_NoFogEnvironments[0]; env2->stage != 0; env2++) {
		if (env2->stage == stagenum) {
			finalenv = env2;
		}
	}

	if (env2);

	if (finalenv == NULL) {
		finalenv = &g_NoFogEnvironments[0];
	}

	envApplyNoFogEnvironment(finalenv);

	g_EnvOrigFogEnvironment = NULL;
}

void envApplyTransitionFrac(f32 frac)
{
	static struct fogenvironment tmp;

	tmp = *g_EnvTransitionFrom;

	tmp.near = g_EnvTransitionFrom->near + frac * ((f32)g_EnvTransitionTo->near - (f32)g_EnvTransitionFrom->near);
	tmp.far = g_EnvTransitionFrom->far + frac * ((f32)g_EnvTransitionTo->far - (f32)g_EnvTransitionFrom->far);
	tmp.fogmin = g_EnvTransitionFrom->fogmin + frac * ((f32)g_EnvTransitionTo->fogmin - (f32)g_EnvTransitionFrom->fogmin);
	tmp.fogmax = g_EnvTransitionFrom->fogmax + frac * ((f32)g_EnvTransitionTo->fogmax - (f32)g_EnvTransitionFrom->fogmax);

	tmp.sky_r = g_EnvTransitionFrom->sky_r + frac * ((f32)g_EnvTransitionTo->sky_r - (f32)g_EnvTransitionFrom->sky_r);
	tmp.sky_g = g_EnvTransitionFrom->sky_g + frac * ((f32)g_EnvTransitionTo->sky_g - (f32)g_EnvTransitionFrom->sky_g);
	tmp.sky_b = g_EnvTransitionFrom->sky_b + frac * ((f32)g_EnvTransitionTo->sky_b - (f32)g_EnvTransitionFrom->sky_b);

	tmp.sky_r &= 0xf8;
	tmp.sky_g &= 0xf8;
	tmp.sky_b &= 0xf8;

	envApplyFogEnvironment(&tmp);
}

Gfx *envStartFog(Gfx *gdl, bool xlupass)
{
	if (!g_FogEnabled) {
		return gdl;
	}

	if (xlupass) {
		gDPSetFogColor(gdl++, g_Env.sky_r, g_Env.sky_g, g_Env.sky_b, 0xff);
		gSPFogPosition(gdl++, g_Env.fogmin, g_Env.fogmax);
	} else {
		gDPSetFogColor(gdl++, g_Env.sky_r, g_Env.sky_g, g_Env.sky_b, 0xff);
		gSPFogPosition(gdl++, g_Env.fogmin, g_Env.fogmax);
	}

	gSPSetGeometryMode(gdl++, G_FOG);
	gDPSetAlphaDither(gdl++, G_AD_NOISE);

	return gdl;
}

Gfx *envStopFog(Gfx *gdl)
{
	if (!g_FogEnabled) {
		return gdl;
	}

	gSPClearGeometryMode(gdl++, G_FOG);

	return gdl;
}

bool envIsPosInFogMaxDistance(struct coord *pos, f32 tolerance)
{
	struct coord sp24;
	Mtxf *mtx;
	struct coord *campos;
	f32 tmp;

	if (!g_FogEnabled) {
		return true;
	}

	campos = &g_Vars.currentplayer->cam_pos;

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		return true;
	}

	mtx = camGetWorldToScreenMtxf();

	sp24.x = pos->x - campos->x;
	sp24.y = pos->y - campos->y;
	sp24.z = pos->z - campos->z;

	tmp = sp24.f[0] * mtx->m[0][0] + sp24.f[1] * mtx->m[0][1] + sp24.f[2] * mtx->m[0][2];

	if (tmp > g_EnvFogMax + tolerance) {
		return false;
	}

	return true;
}

struct distfadesettings *envGetDistFadeSettings(void)
{
	return g_EnvDistFadeSettingsPtr;
}

s32 envGetObjShadeMode(struct prop *prop, f32 out[4])
{
	if (!g_FogEnabled) {
		return SHADEMODE_OPA;
	}

	if (prop->z < 0.0f) {
		return SHADEMODE_OPA;
	}

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		return SHADEMODE_OPA;
	}

	out[0] = g_Env.skyredfrac;
	out[1] = g_Env.skygreenfrac;
	out[2] = g_Env.skybluefrac;
	out[3] = g_EnvShadeSettings.alphanear + g_EnvShadeSettings.alphafar / prop->z;

	if (out[3] < 0.0f) {
		return SHADEMODE_OPA;
	}

	if (out[3] > 1.0f) {
		return SHADEMODE_XLU;
	}

	return SHADEMODE_FRAC;
}
