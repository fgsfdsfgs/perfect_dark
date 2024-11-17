#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/debug.h"
#include "game/chr.h"
#include "game/ceil.h"
#include "game/game_0b0fd0.h"
#include "game/playermgr.h"
#include "game/mplayer/setup.h"
#include "game/bot.h"
#include "game/botact.h"
#include "game/botcmd.h"
#include "game/botinv.h"
#include "game/mplayer/mplayer.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

struct aibotweaponpreference g_AibotWeaponPreferences[] = {
	//                             haspriammogoal
	//                             |  hassecammogoal
	//                             |  |  pridistconfig
	//                             |  |  |                          secdistconfig
	//                             |  |  |                          |                          targetammopri
	//                             |  |  |                          |                          |              targetammosec
	//                             |  |  |                          |                          |              |    criticalammopri
	//                             |  |  |                          |                          |              |    |   criticalammosec
	//                             |  |  |                          |                          |              |    |   |   reloaddelay (seconds)
	//                             |  |  |                          |                          |              |    |   |   |  allowpartialreloaddelay
	//                             |  |  |                          |                          |              |    |   |   |  |
	/*0x00*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  0, 0 }, // WEAPON_NONE
	/*0x01*/ { 13,  13,  13,  13,  1, 1, BOTDISTCFG_CLOSE,          BOTDISTCFG_CLOSE,          0,             0,   0,  0,  0, 0 }, // WEAPON_UNARMED
	/*0x02*/ { 56,  60,  84,  88,  1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_FALCON2
	/*0x03*/ { 52,  60,  80,  88,  1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_FALCON2_SILENCER
	/*0x04*/ { 60,  60,  88,  88,  1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_FALCON2_SCOPE
	/*0x05*/ { 76,  88,  104, 120, 1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_DEFAULT,        30,            30,  10, 10, 1, 0 }, // WEAPON_MAGSEC4
	/*0x06*/ { 64,  88,  92,  120, 1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_DEFAULT,        30,            30,  10, 10, 1, 0 }, // WEAPON_MAULER
	/*0x07*/ { 72,  76,  100, 120, 1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_DEFAULT,        30,            30,  10, 10, 2, 0 }, // WEAPON_PHOENIX
	/*0x08*/ { 68,  76,  96,  120, 1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   8,  0,  3, 0 }, // WEAPON_DY357MAGNUM
	/*0x09*/ { 180, 188, 184, 188, 1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          20,            0,   6,  0,  3, 0 }, // WEAPON_DY357LX
	/*0x0a*/ { 116, 128, 136, 152, 1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        100,           100, 30, 30, 2, 0 }, // WEAPON_CMP150
	/*0x0b*/ { 120, 128, 132, 140, 1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        150,           150, 50, 50, 2, 0 }, // WEAPON_CYCLONE
	/*0x0c*/ { 152, 176, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        100,           70,  25, 15, 2, 0 }, // WEAPON_CALLISTO
	/*0x0d*/ { 172, 188, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        300,           0,   40, 0,  2, 0 }, // WEAPON_RCP120
	/*0x0e*/ { 128, 140, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        100,           0,   30, 0,  3, 0 }, // WEAPON_LAPTOPGUN
	/*0x0f*/ { 124, 148, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        90,            0,   30, 0,  1, 0 }, // WEAPON_DRAGON
	/*0x10*/ { 156, 180, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        150,           0,   40, 0,  2, 0 }, // WEAPON_K7AVENGER
	/*0x11*/ { 148, 176, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        120,           0,   40, 0,  2, 0 }, // WEAPON_AR34
	/*0x12*/ { 164, 188, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_SHOOTEXPLOSIVE, 120,           20,  30, 6,  1, 0 }, // WEAPON_SUPERDRAGON
	/*0x13*/ { 140, 156, 0,   0,   1, 1, BOTDISTCFG_PISTOL,         BOTDISTCFG_PISTOL,         18,            18,  8,  8,  6, 1 }, // WEAPON_SHOTGUN
	/*0x14*/ { 144, 176, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_CLOSE,          400,           0,   80, 0,  3, 0 }, // WEAPON_REAPER
	/*0x15*/ { 28,  40,  0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        30,            30,  10, 10, 2, 0 }, // WEAPON_SNIPERRIFLE
	/*0x16*/ { 188, 188, 0,   0,   1, 0, BOTDISTCFG_SHOOTEXPLOSIVE, BOTDISTCFG_FARSIGHT,       16,            0,   4,  0,  2, 0 }, // WEAPON_FARSIGHT
	/*0x17*/ { 176, 188, 0,   0,   1, 1, BOTDISTCFG_SHOOTEXPLOSIVE, BOTDISTCFG_SHOOTEXPLOSIVE, 20,            20,  4,  4,  2, 0 }, // WEAPON_DEVASTATOR
	/*0x18*/ { 160, 188, 0,   0,   1, 1, BOTDISTCFG_SHOOTEXPLOSIVE, BOTDISTCFG_SHOOTEXPLOSIVE, 2,             2,   1,  1,  2, 0 }, // WEAPON_ROCKETLAUNCHER
	/*0x19*/ { 168, 188, 0,   0,   1, 1, BOTDISTCFG_SHOOTEXPLOSIVE, BOTDISTCFG_SHOOTEXPLOSIVE, 2,             2,   1,  1,  3, 0 }, // WEAPON_SLAYER
#if (VERSION == VERSION_JPN_FINAL) && defined(PLATFORM_N64)
	/*0x1a*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_COMBATKNIFE
#else
	/*0x1a*/ { 20,  40,  24,  40,  1, 1, BOTDISTCFG_CLOSE,          BOTDISTCFG_DEFAULT,        0,             5,   0,  1,  1, 0 }, // WEAPON_COMBATKNIFE
#endif
	/*0x1b*/ { 108, 176, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        15,            15,  5,  5,  4, 1 }, // WEAPON_CROSSBOW
	/*0x1c*/ { 48,  188, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_CLOSE,          20,            24,  6,  8,  1, 0 }, // WEAPON_TRANQUILIZER
	/*0x1d*/ { 112, 112, 0,   0,   1, 1, BOTDISTCFG_DEFAULT,        BOTDISTCFG_CLOSE,          0,             0,   0,  0,  1, 0 }, // WEAPON_LASER
	/*0x1e*/ { 36,  172, 0,   0,   1, 1, BOTDISTCFG_THROWEXPLOSIVE, BOTDISTCFG_THROWEXPLOSIVE, 6,             6,   2,  2,  1, 0 }, // WEAPON_GRENADE
	/*0x1f*/ { 32,  188, 0,   0,   1, 1, BOTDISTCFG_THROWEXPLOSIVE, BOTDISTCFG_THROWEXPLOSIVE, 3,             3,   1,  1,  1, 0 }, // WEAPON_NBOMB
	/*0x20*/ { 12,  12,  0,   0,   0, 0, BOTDISTCFG_THROWEXPLOSIVE, BOTDISTCFG_DEFAULT,        5,             5,   1,  1,  1, 0 }, // WEAPON_TIMEDMINE
	/*0x21*/ { 40,  176, 0,   0,   0, 0, BOTDISTCFG_THROWEXPLOSIVE, BOTDISTCFG_DEFAULT,        5,             5,   1,  1,  1, 0 }, // WEAPON_PROXIMITYMINE
	/*0x22*/ { 44,  156, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        5,             5,   2,  2,  1, 0 }, // WEAPON_REMOTEMINE
	/*0x23*/ { 8,   8,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_COMBATBOOST
#ifndef PLATFORM_N64 // add all classic weapons to multiplayer
	/*0x24*/ { 56,  60,  84,  88,  1, 0, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_PP9I
	/*0x25*/ { 56,  60,  84,  88,  1, 0, BOTDISTCFG_PISTOL,         BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_CC13
	/*0x26*/ { 56,  60,  84,  88,  1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_CLOSE,          30,            0,   10, 0,  1, 0 }, // WEAPON_KL01313
	/*0x27*/ { 124, 148, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        90,            0,   30, 0,  1, 0 }, // WEAPON_KF7SPECIAL
	/*0x28*/ { 116, 128, 136, 152, 1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        100,           0,   30, 0,  1, 0 }, // WEAPON_ZZT
	/*0x29*/ { 124, 148, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        90,            0,   30, 0,  1, 0 }, // WEAPON_DMC
	/*0x2a*/ { 156, 180, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        150,           0,   30, 0,  1, 0 }, // WEAPON_AR53
	/*0x2b*/ { 164, 188, 0,   0,   1, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        200,           0,   40, 0,  1, 0 }, // WEAPON_RCP45
#else
	/*0x24*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_PP9I
	/*0x25*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_CC13
	/*0x26*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KL01313
	/*0x27*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KF7SPECIAL
	/*0x28*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_ZZT
	/*0x29*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DMC
	/*0x2a*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_AR53
	/*0x2b*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_RCP45
#endif
	/*0x2c*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_PSYCHOSISGUN
	/*0x2d*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_NIGHTVISION
	/*0x2e*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_EYESPY
	/*0x2f*/ { 4,   4,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_XRAYSCANNER
	/*0x30*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_IRSCANNER
#if VERSION >= VERSION_PAL_FINAL
	/*0x31*/ { 218, 218, 0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        TICKS(1200),   0,   0,  0,  1, 0 }, // WEAPON_CLOAKINGDEVICE
#else
	/*0x31*/ { 218, 218, 0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        1200,          0,   0,  0,  1, 0 }, // WEAPON_CLOAKINGDEVICE
#endif
	/*0x32*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_HORIZONSCANNER
	/*0x33*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_TESTER
	/*0x34*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_ROCKETLAUNCHER_34
	/*0x35*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_ECMMINE
	/*0x36*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DATAUPLINK
	/*0x37*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_RTRACKER
	/*0x38*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_PRESIDENTSCANNER
	/*0x39*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DOORDECODER
	/*0x3a*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_AUTOSURGEON
	/*0x3b*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_EXPLOSIVES
	/*0x3c*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SKEDARBOMB
	/*0x3d*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_COMMSRIDER
	/*0x3e*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_TRACERBUG
	/*0x3f*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_TARGETAMPLIFIER
	/*0x40*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DISGUISE40
	/*0x41*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DISGUISE41
	/*0x42*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_FLIGHTPLANS
	/*0x43*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_RESEARCHTAPE
	/*0x44*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_BACKUPDISK
	/*0x45*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD45
	/*0x46*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD46
	/*0x47*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD47
	/*0x48*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD48
	/*0x49*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD49
	/*0x4a*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD4A
	/*0x4b*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD4B
	/*0x4c*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_KEYCARD4C
	/*0x4d*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SUITCASE
	/*0x4e*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_BRIEFCASE
#if VERSION >= VERSION_NTSC_1_0
	/*0x4f*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SHIELDTECHITEM
#endif
	/*0x50*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_NECKLACE
	/*0x51*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_HAMMER
	/*0x52*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SCREWDRIVER
	/*0x53*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_ROCKET
	/*0x54*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_HOMINGROCKET
	/*0x55*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_GRENADEROUND
	/*0x56*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_BOLT
	/*0x57*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_BRIEFCASE2
	/*0x58*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SKROCKET
	/*0x59*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_CHOPPERGUN
	/*0x5a*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_WATCHLASER
	/*0x5b*/ { 220, 220, 0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_MPSHIELD
	/*0x5c*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_DISABLED
	/*0x5d*/ { 0,   0,   0,   0,   0, 0, BOTDISTCFG_DEFAULT,        BOTDISTCFG_DEFAULT,        0,             0,   0,  0,  1, 0 }, // WEAPON_SUICIDEPILL
};

/**
 * Remove all items from the bot's inventory.
 */
void botinvClear(struct chrdata *chr)
{
	if (chr && chr->aibot) {
		s32 i = 0;

		for (i = 0; i < chr->aibot->maxitems; i++) {
			chr->aibot->items[i].type = -1;
		}

		dprint();
	}
}

/**
 * Return a free slot from the bot's inventory.
 *
 * In theory this should never fail. The inventory has 10 slots. 6 are for
 * weapons, and 4 are for scenario-specific items such as briefcases and the
 * data uplink.
 */
struct invitem *botinvGetFreeSlot(struct chrdata *chr)
{
	s32 i;

	if (!chr || !chr->aibot) {
		return NULL;
	}

	for (i = 0; i < chr->aibot->maxitems; i++) {
		if (chr->aibot->items[i].type == -1) {
			return &chr->aibot->items[i];
		}
	}

	dprint();

	return NULL;
}

/**
 * Retrieve an inventory item from the bot's inventory.
 */
struct invitem *botinvGetItem(struct chrdata *chr, s32 weaponnum)
{
	s32 i;

	if (!chr || !chr->aibot) {
		return NULL;
	}

	for (i = 0; i < chr->aibot->maxitems; i++) {
		struct invitem *item = &chr->aibot->items[i];

		if (item->type == -1) {
			continue;
		}

		if (item->type == INVITEMTYPE_WEAP || item->type == INVITEMTYPE_DUAL) {
			if (item->type_weap.weapon1 == weaponnum) {
				return item;
			}
		}
	}

	return NULL;
}

/**
 * Remove a weapon from the bot's inventory.
 */
void botinvRemoveItem(struct chrdata *chr, s32 weaponnum)
{
	s32 i;

	if (!chr || !chr->aibot) {
		return;
	}

	for (i = 0; i < chr->aibot->maxitems; i++) {
		struct invitem *item = &chr->aibot->items[i];

		if (item->type == -1) {
			continue;
		}

		if (item->type == INVITEMTYPE_WEAP || item->type == INVITEMTYPE_DUAL) {
			if (item->type_weap.weapon1 == weaponnum) {
				chr->aibot->items[i].type = -1;
				return;
			}
		}
	}
}

/**
 * Return the item type of the given weapon number.
 *
 * See the INVITEMTYPE constants.
 */
u32 botinvGetItemType(struct chrdata *chr, u32 weaponnum)
{
	struct invitem *item;

	if (!chr || !chr->aibot) {
		return 0;
	}

	item = botinvGetItem(chr, weaponnum);

	if (item) {
		return item->type;
	}

	return 0;
}

/**
 * Give a single weapon to the bot.
 *
 * There is no pickup pad, so this is likely for dropped items.
 */
bool botinvGiveSingleWeapon(struct chrdata *chr, u32 weaponnum)
{
	if (!chr || !chr->aibot) {
		return false;
	}

	if (!botinvGetItemType(chr, weaponnum)) {
		struct invitem *item = botinvGetFreeSlot(chr);

		if (item) {
			item->type = INVITEMTYPE_WEAP;
			item->type_weap.weapon1 = weaponnum;
			item->type_weap.pickuppad = -1;
		}

		return true;
	}

	return false;
}

/**
 * Grant a bot dual weapons. They must already have the single weapon in their
 * inventory.
 *
 * This works by converting the single item into a dual item, which makes it
 * impossible for the bot to single wield that weapon while they have dual.
 * This is different to how the player's inventory works, where there are
 * inventory items for both single and dual and the player can choose which one
 * they want to use.
 */
void botinvGiveDualWeapon(struct chrdata *chr, u32 weaponnum)
{
	struct invitem *item = botinvGetItem(chr, weaponnum);

	if (item) {
		item->type = INVITEMTYPE_DUAL;
	}
}

/**
 * Return the pad number that the given weapon was collected from.
 *
 * Usually when you pick up two of the same weapon you get to dual wield, but
 * this does not happen if the second weapon is from the same pad as the first
 * (ie. is the first weapon respawned).
 */
s16 botinvGetWeaponPad(struct chrdata *chr, u32 weaponnum)
{
	struct invitem *item = botinvGetItem(chr, weaponnum);

	if (item && item->type == INVITEMTYPE_WEAP) {
		return item->type_weap.pickuppad;
	}

	return -1;
}

/**
 * Give a weapon to the bot, either by direct weapon or by ammo crate.
 *
 * This function does not give any ammo to the bot.
 */
bool botinvGiveProp(struct chrdata *chr, struct prop *prop)
{
	bool result = false;
	struct defaultobj *obj;
	s32 i;

	if (!chr || !chr->aibot) {
		return false;
	}

	obj = prop->obj;

	if (prop->type == PROPTYPE_WEAPON) {
		if (obj->type == OBJTYPE_WEAPON) {
			struct weaponobj *weapon = prop->weapon;
			s32 weaponnum = weapon->weaponnum;
			result = botinvGiveSingleWeapon(chr, weaponnum);

			if (result) {
				struct invitem *item = botinvGetItem(chr, weaponnum);
				item->type_weap.pickuppad = obj->pad;
			}
		}
	} else if (obj->type == OBJTYPE_MULTIAMMOCRATE) {
		struct multiammocrateobj *multi = (struct multiammocrateobj *)prop->obj;

		for (i = 0; i < 19; i++) {
			if (multi->slots[i].quantity > 0) {
				s32 weaponnum = botactGetWeaponByAmmoType(i + 1);

				if (weaponnum > 0) {
					botinvGiveSingleWeapon(chr, weaponnum);
				}
			}
		}
	}

	return result;
}

void botinv0f198060(u32 arg0)
{
	// empty
}

/**
 * Score all weapons in the match's weaponset by themselves and write them to
 * the 3 array pointers, ordered by score1 descending.
 */
void botinvScoreAllWeapons(struct chrdata *chr, s32 *weaponnums, s32 *scores1, s32 *scores2)
{
	s32 i;
	s32 pri1;
	s32 pri2;
	s32 sec1;
	s32 sec2;

	// Gather scores for each weapon in the setup,
	// taking the higher score out of both gun functions
	for (i = 0; i < ARRAYCOUNT(g_MpSetup.weapons); i++) {
		s32 weaponnum = g_MpWeapons[g_MpSetup.weapons[i]].weaponnum;
		weaponnums[i] = weaponnum;

		botinvScoreWeaponByItself(chr, weaponnum, FUNC_PRIMARY, -1, false, &pri1, &pri2);
		botinvScoreWeaponByItself(chr, weaponnum, FUNC_SECONDARY, -1, false, &sec1, &sec2);

		scores1[i] = pri1 >= sec1 ? pri1 : sec1;
		scores2[i] = pri2 >= sec2 ? pri2 : sec2;
	}

	// Sort all three arrays by score1 descending
	for (i = 0; i < ARRAYCOUNT(g_MpSetup.weapons); i++) {
		s32 swapindex = i;
		s32 tmp;
		s32 j;

		for (j = i + 1; j < ARRAYCOUNT(g_MpSetup.weapons); j++) {
			if (scores1[j] > scores1[swapindex]) {
				swapindex = j;
			}
		}

		if (swapindex != i) {
			tmp = scores1[swapindex];
			scores1[swapindex] = scores1[i];
			scores1[i] = tmp;

			tmp = scores2[swapindex];
			scores2[swapindex] = scores2[i];
			scores2[i] = tmp;

			tmp = weaponnums[swapindex];
			weaponnums[swapindex] = weaponnums[i];
			weaponnums[i] = tmp;
		}
	}
}

/**
 * Return true if the match's weaponset contains a shield.
 */
bool mpHasShield(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpSetup.weapons); i++) {
		s32 weaponnum = g_MpWeapons[g_MpSetup.weapons[i]].weaponnum;

		if (weaponnum == WEAPON_MPSHIELD) {
			return true;
		}
	}

	return false;
}

/**
 * Get the weapon slot (0 to 5) by weapon number.
 */
s32 mpGetWeaponSlotByWeaponNum(s32 weaponnum)
{
	s32 result = -1;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_MpSetup.weapons); i++) {
		if (g_MpWeapons[g_MpSetup.weapons[i]].weaponnum == weaponnum && i < ARRAYCOUNT(g_MpSetup.weapons)) {
			result = i;
			break;
		}
	}

	return result;
}

/**
 * Calculate scores for the given weapon and function and write them to the
 * dst1 and dst2 pointers.
 *
 * Weapon scoring is used to determine if a weapon is better than another,
 * which affects whether the bot engages in combat or seeks a better weapon.
 */
void botinvScoreWeapon(struct chrdata *chr, s32 weaponnum, s32 funcnum, s32 arg3, bool arg4, s32 *dst1, s32 *dst2, bool comparewithtarget, bool arg8)
{
	s32 score1 = 0;
	s32 score2 = 0;
	s32 extra = 0;

	// @dangerous: Array overflow can occur if more weapons are added to the
	// game without extending the preferences table
	if (arg3 < 0
			|| (!funcnum && arg3 == g_AibotWeaponPreferences[weaponnum].haspriammogoal)
			|| (funcnum && arg3 == g_AibotWeaponPreferences[weaponnum].hassecammogoal)) {
		if (arg4) {
			score1 = g_AibotWeaponPreferences[weaponnum].unk02;
			score2 = g_AibotWeaponPreferences[weaponnum].unk03;
		} else {
			score1 = g_AibotWeaponPreferences[weaponnum].unk00;
			score2 = g_AibotWeaponPreferences[weaponnum].unk01;
		}

		if (chr && chr->aibot) {
			// Add weighting for explosive weapons for rocket sims
			// and weighting for shields for shield sims, with extra
			// weighting if the sim is meat or easy.
			if (chr->aibot->config->difficulty == BOTDIFF_MEAT) {
				extra = 100;
			} else if (chr->aibot->config->difficulty == BOTDIFF_EASY) {
				extra = 50;
			}

			if (chr->aibot->config->type == BOTTYPE_ROCKET) {
				if (weaponnum == WEAPON_ROCKETLAUNCHER) {
					score1 = extra + 300;
				} else if (weaponnum == WEAPON_SLAYER) {
					score1 = extra + 299;
				} else if (weaponnum == WEAPON_DEVASTATOR) {
					score1 = extra + 280;
				} else if (weaponnum == WEAPON_SUPERDRAGON && funcnum != FUNC_PRIMARY) {
					score1 = extra + 279;
				} else if (weaponnum == WEAPON_PHOENIX && funcnum != FUNC_PRIMARY) {
					score1 = extra + 260;
				} else if (weaponnum == WEAPON_GRENADE) {
					score1 = extra + 240;
				}
			} else if (chr->aibot->config->type == BOTTYPE_SHIELD) {
				if (weaponnum == WEAPON_MPSHIELD) {
					score1 = extra + 300;
				}
			}
		}
	}

	// Missing from this list:
	// Mauler, K7 Avenger, AR34, sniper rifle, Farsight, laser, grenade
	switch (weaponnum) {
	case WEAPON_UNARMED:
		if (comparewithtarget && funcnum != FUNC_PRIMARY) {
			if (chr->target != -1
					&& botGetTargetsWeaponNum(chr) > WEAPON_UNARMED
					&& chr->aibot->config->difficulty > BOTDIFF_MEAT) {
				score1 = 26;
				score2 = 26;
			} else {
				score1 = 0;
				score2 = 0;
			}
		}
		break;
	case WEAPON_FALCON2:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 15;
			score2 = 15;
		}
		break;
	case WEAPON_FALCON2_SILENCER:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 14;
			score2 = 14;
		}
		break;
	case WEAPON_FALCON2_SCOPE:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 16;
			score2 = 16;
		}
		break;
	case WEAPON_MAGSEC4:
		if (funcnum == FUNC_PRIMARY) {
			score1 = arg4 ? 91 : 63;
		}
		break;
	case WEAPON_PHOENIX:
		if (chr->aibot->config->type != BOTDIFF_HARD) {
			if (funcnum != FUNC_PRIMARY) {
				if ((g_MpSetup.options & MPOPTION_ONEHITKILLS)
						&& chr->aibot->config->difficulty >= BOTDIFF_NORMAL) {
					score1 = 110;
					score2 = 150;
				}
			} else {
				score1 = arg4 ? 90 : 62;
			}
		}
		break;
	case WEAPON_DY357MAGNUM:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 17;
			score2 = 17;
		}
		break;
	case WEAPON_DY357LX:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 18;
			score2 = 18;
		}
		break;
	case WEAPON_CYCLONE:
		if (funcnum == (chr->aibot->random1 % 2)) {
			score1--;
		}
		break;
	case WEAPON_CALLISTO:
		if (funcnum == (chr->aibot->random1 % 2)) {
			score1--;
		}
		break;
	case WEAPON_RCP120:
		if (chr->aibot->cloakdeviceenabled == false
				&& botactGetAmmoQuantityByWeapon(chr->aibot, WEAPON_RCP120, FUNC_PRIMARY, true) > 500
				&& chr->aibot->config->difficulty > BOTDIFF_MEAT) {
			score1 += chr->aibot->random1 % 10;
			score2 += chr->aibot->random1 % 10;
		}
		break;
	case WEAPON_LAPTOPGUN:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 0;
			score2 = 0;
		}
		break;
	case WEAPON_DRAGON:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 0;
			score2 = 0;
		}
		break;
	case WEAPON_SUPERDRAGON:
		if (chr->aibot->config->type != BOTDIFF_HARD
				&& (chr->aibot->random1 % 2) == funcnum) {
			score1 -= 15;
		}
		break;
	case WEAPON_SHOTGUN:
		if (funcnum == (chr->aibot->random1 % 2)) {
			score1--;
		}
		break;
	case WEAPON_REAPER:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 19;
			score2 = 80;
		}
		break;
	case WEAPON_DEVASTATOR:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 0;
			score2 = 0;
		}
		break;
	case WEAPON_ROCKETLAUNCHER:
		if (funcnum == (chr->aibot->random1 % 2)) {
			score1--;
		}
		break;
	case WEAPON_SLAYER:
		if (chr->aibot->config->type == BOTDIFF_HARD) {
			if (funcnum != FUNC_PRIMARY) {
				if (chr->aibot->config->difficulty > BOTDIFF_MEAT) {
					if (comparewithtarget) {
						if (chr->target != -1
								&& chr->aibot->chrsinsight[mpPlayerGetIndex(chrGetTargetProp(chr)->chr)] == 0
								&& (chr->aibot->random1 % 2) == 0) {
							score1 += 10;
						} else {
							score1 -= 10;
						}
					}
				} else {
					score1 = 0;
					score2 = 0;
				}
			}
		} else {
			if (funcnum != FUNC_PRIMARY) {
				if (chr->aibot->config->difficulty >= BOTDIFF_NORMAL) {
					if (comparewithtarget) {
						if (chr->target != -1
								&& chr->aibot->chrsinsight[mpPlayerGetIndex(chrGetTargetProp(chr)->chr)] == 0
								&& (chr->aibot->random1 % 2) == 0) {
							score1 = 178;
							score2 = 188;
						} else {
							score1 -= 15;
							score2 -= 15;
						}
					}
				} else {
					score1 = 0;
					score2 = 0;
				}
			}
		}
		break;
	case WEAPON_COMBATKNIFE:
		if (funcnum != FUNC_PRIMARY) {
			score1 = 0;
			score2 = 0;
		}
		break;
	case WEAPON_CROSSBOW:
		if (funcnum != FUNC_PRIMARY) {
			if (chr->aibot->config->difficulty > BOTDIFF_MEAT) {
				score1 = 158;
				score2 = 176;
			} else {
				score1 = 0;
				score2 = 0;
			}
		} else {
			if (comparewithtarget && chr->target != -1 && chrGetTargetProp(chr)->chr->blurdrugamount > TICKS(3500)) {
				score1 = 0;
				score2 = 0;
			} else {
				score1 = 49;
				score2 = 188;
			}
		}
		break;
	case WEAPON_TRANQUILIZER:
		if (comparewithtarget) {
			s32 bluramount = 0;

			if (chr->target != -1) {
				bluramount = chrGetTargetProp(chr)->chr->blurdrugamount;
			}

			if (funcnum != FUNC_PRIMARY) {
				if (chr->aibot->config->difficulty <= BOTDIFF_MEAT) {
					score1 = 0;
					score2 = 0;
				} else if (bluramount > TICKS(3500) && (chr->aibot->random1 % 2) == 0) {
					score1 = chr->aibot->random1 % 140 + 48;
					score2 = 188;
				} else if (chr->aibot->random1 % 10 == 0) {
					score1 = chr->aibot->random1 % 140 + 48;
					score2 = 188;
				} else {
					score1 = 0;
					score2 = 0;
				}
			} else {
				if (bluramount >= TICKS(5000)) {
					score2 = 48;

					if (chr->aibot->random1 % 2) {
						score1 = 0;
						score2 = 0;
					}
				} else if (bluramount > TICKS(3500)) {
					u32 value = (-bluramount * 16 + (PAL ? 66656 : 80000)) / TICKS(1500);

					if (value > 15) {
						value = 15;
					}

					value *= value;
					value *= value;
					value *= value;

					if (value < chr->aibot->random1) {
						score2 = 48;

						if (chr->aibot->random1 % 2) {
							score1 = 0;
							score2 = 0;
						}
					}
				}
			}
		}
		break;
	case WEAPON_NBOMB:
		if (comparewithtarget && chr->target != -1) {
			botGetTargetsWeaponNum(chr);
		}
		break;
	}

	if (arg8) {
		s32 weaponindex;
		s32 extra = 0;
		f32 float1;
		f32 killrate = 1;
		f32 float2;

		if (g_Vars.lvframe60 > 0) {
			killrate = g_Vars.totalkills * TICKS(3600.0f) / (f32)(g_Vars.lvframe60 * g_MpNumChrs);

			if (killrate < 1) {
				killrate = 1;
			}
		}

		weaponindex = mpGetWeaponSlotByWeaponNum(weaponnum);

		if (weaponindex >= 0) {
			float2 = ceilf(chr->aibot->equipdurations60[weaponindex][funcnum] * (1.0f / TICKS(3600.0f)));

			if (float2 > 0) {
				float1 = chr->aibot->killsbygunfunc[weaponindex][funcnum];

				if (chr->aibot->config->difficulty >= BOTDIFF_NORMAL) {
					float1 -= 3.0f * chr->aibot->suicidesbygunfunc[weaponindex][funcnum];
				}

				float1 = float1 / float2;

				extra = float1 * 10.0f / killrate;
			}

			if (extra > 30) {
				extra = 30;
			}

			extra += chr->aibot->equipextrascores[weaponindex];

			score1 += extra;
			score2 += extra;

			if (score1 < 0) {
				score1 = 0;
			}

			if (score2 < 0) {
				score2 = 0;
			}
		}
	}

	*dst1 = score1;
	*dst2 = score2;
}

void botinvScoreWeaponAgainstTarget(struct chrdata *chr, s32 weaponnum, s32 funcnum, s32 arg3, bool arg4, s32 *dst1, s32 *dst2)
{
	botinvScoreWeapon(chr, weaponnum, funcnum, arg3, arg4, dst1, dst2, true, true);
}

void botinvScoreWeaponByItself(struct chrdata *chr, s32 weaponnum, s32 funcnum, s32 arg3, bool arg4, s32 *dst1, s32 *dst2)
{
	botinvScoreWeapon(chr, weaponnum, funcnum, arg3, arg4, dst1, dst2, false, true);
}

/**
 * Return the aibot's distance configuration index for the given weapon and
 * function.
 */
s32 botinvGetDistConfig(s32 weaponnum, s32 funcnum)
{
	if (funcnum != FUNC_PRIMARY) {
		return g_AibotWeaponPreferences[weaponnum].secdistconfig;
	}

	return g_AibotWeaponPreferences[weaponnum].pridistconfig;
}

/**
 * Check if the bot's personality permits it to use the given weapon and
 * function.
 */
bool botinvAllowsWeapon(struct chrdata *chr, s32 weaponnum, s32 funcnum)
{
	bool allow = true;

	if (chr->aibot->config->type == BOTTYPE_FIST) {
		if (funcnum != FUNC_PRIMARY) {
			if (g_AibotWeaponPreferences[weaponnum].secdistconfig != BOTDISTCFG_CLOSE) {
				allow = false;
			}
		} else {
			if (g_AibotWeaponPreferences[weaponnum].pridistconfig != BOTDISTCFG_CLOSE) {
				allow = false;
			}
		}
	}

	return allow;
}

/**
 * Tick the bot's inventory.
 *
 * The logic for deciding when to switch weapons is here.
 */
void botinvTick(struct chrdata *chr)
{
	s32 newweaponnum = WEAPON_UNARMED;
	s32 newfuncnum = FUNC_PRIMARY;
	s32 weaponindex;
	struct aibot *aibot;
	bool keepcurrentweapon = false;
	s32 i;

	if (!chr || !chr->aibot) {
		return;
	}

	aibot = chr->aibot;
	weaponindex = mpGetWeaponSlotByWeaponNum(aibot->weaponnum);

	if (weaponindex >= 0) {
		aibot->equipdurations60[weaponindex][aibot->gunfunc] += g_Vars.lvupdate60;
	}

	// Every 60-61 seconds, decrease suicide values. These values are used when
	// determining which gun to use. The more suicides with a particular gun and
	// function, the less likely the aibot is to use it. This value cools off.
	aibot->dampensuicidesttl60 -= g_Vars.lvupdate60;

	if (aibot->dampensuicidesttl60 < 0) {
		aibot->dampensuicidesttl60 = TICKS(3600) + rngRandom() % TICKS(60);

		for (i = 0; i < ARRAYCOUNT(aibot->suicidesbygunfunc); i++) {
			aibot->suicidesbygunfunc[i][0] *= 0.9f;
			aibot->suicidesbygunfunc[i][1] *= 0.9f;
		}
	}

	// Every 10-60 seconds, generate new equipextrascores
	aibot->equipextrascorestimer60 -= g_Vars.lvupdate60;

	if (aibot->equipextrascorestimer60 < 0) {
		aibot->equipextrascorestimer60 = TICKS(600) + rngRandom() % TICKS(3000);

		for (i = 0; i < ARRAYCOUNT(aibot->equipextrascores); i++) {
			if (aibot->config->difficulty == BOTDIFF_MEAT) {
				aibot->equipextrascores[i] = rngRandom() % 200 - 100; // -100 to +100
			} else if (aibot->config->difficulty == BOTDIFF_EASY) {
				aibot->equipextrascores[i] = rngRandom() % 100 - 50; // -50 to +50
			} else {
				aibot->equipextrascores[i] = rngRandom() % 30 - 15; // -15 to +15
			}
		}
	}

	// Every 2-12 seconds, generate a new random value which is used as a factor
	// in various aibot logic.
	aibot->random1ttl60 -= g_Vars.lvupdate60;

	if (aibot->random1ttl60 < 0) {
		aibot->random1ttl60 = TICKS(120) + rngRandom() % TICKS(600);
		aibot->random1 = rngRandom();
	}

	if (aibot->cyclonedischarging[HAND_LEFT] == 0
			&& aibot->cyclonedischarging[HAND_RIGHT] == 0
			&& aibot->burstsdone[HAND_LEFT] <= 0
			&& aibot->burstsdone[HAND_RIGHT] <= 0
			&& aibot->reaperspeed[HAND_LEFT] <= 0
			&& aibot->reaperspeed[HAND_RIGHT] <= 0
			&& aibot->skrocket == NULL) {
		if (chr->myaction == MA_AIBOTDOWNLOAD) {
			keepcurrentweapon = true;
		}

		if (!keepcurrentweapon && aibot->config->type == BOTTYPE_PEACE) {
			newfuncnum = FUNC_SECONDARY;
			keepcurrentweapon = true;
		}

		if (!keepcurrentweapon) {
			s32 bestscore = 0;
			s32 score1;
			s32 score2;
			s32 canuse;
			s32 j;

			for (i = -1; i < aibot->maxitems; i++) {
				s32 weaponnum = -1;
				struct invitem *item = NULL;

				if (i < 0) {
					weaponnum = WEAPON_UNARMED;
				} else {
					item = &aibot->items[i];

					if (item->type == INVITEMTYPE_WEAP || item->type == INVITEMTYPE_DUAL) {
						weaponnum = item->type_weap.weapon1;
					}
				}

				if (weaponnum >= 0) {
					for (j = 1; j >= 0; j--) {
						if (j != FUNC_PRIMARY) {
							canuse = g_AibotWeaponPreferences[weaponnum].hassecammogoal;
						} else {
							canuse = g_AibotWeaponPreferences[weaponnum].haspriammogoal;
						}

						if (canuse && botinvAllowsWeapon(chr, weaponnum, j)) {
							botinvScoreWeaponAgainstTarget(chr, weaponnum, j, 1, item && item->type == INVITEMTYPE_DUAL, &score1, &score2);

							if (score1 >= bestscore) {
								if (!botactGetAmmoTypeByFunction(weaponnum, j)
										|| botactGetAmmoQuantityByWeapon(aibot, weaponnum, j, true) > 0) {
									bestscore = score1;
									newweaponnum = weaponnum;
									newfuncnum = j;
								}
							}
						}
					}
				}
			}
		}

		// Consider setting knives to secondary function (throw)
		if (newweaponnum == WEAPON_COMBATKNIFE
				&& botactGetAmmoQuantityByWeapon(aibot, WEAPON_COMBATKNIFE, FUNC_SECONDARY, true) >= 2
				&& chr->target != -1
				&& chr->aibot->chrdistances[mpPlayerGetIndex(chrGetTargetProp(chr)->chr)] > 200
				&& chr->aibot->chrdistances[mpPlayerGetIndex(chrGetTargetProp(chr)->chr)] < 1500) {
			newfuncnum = FUNC_SECONDARY;
		}

		// Consider setting Phoenix and SuperDragon to their explosive functions
		if (aibot->config->type == BOTTYPE_ROCKET) {
			if (newweaponnum == WEAPON_PHOENIX && botactGetAmmoQuantityByWeapon(aibot, WEAPON_PHOENIX, FUNC_SECONDARY, true) > 0) {
				newfuncnum = FUNC_SECONDARY;
			} else if (newweaponnum == WEAPON_SUPERDRAGON && botactGetAmmoQuantityByWeapon(aibot, WEAPON_SUPERDRAGON, FUNC_SECONDARY, true) > 0) {
				newfuncnum = FUNC_SECONDARY;
			}
		}

		botinvSwitchToWeapon(chr, newweaponnum, newfuncnum);
	}
}

/**
 * Equip the given weapon and switch to the given function.
 *
 * The weapon must already exist in the bot's inventory,
 * otherwise unarmed will be equipped instead.
 */
bool botinvSwitchToWeapon(struct chrdata *chr, s32 weaponnum, s32 funcnum)
{
	struct invitem *item;
	struct weaponfunc *func;
	struct aibot *aibot;
	s32 i;
	s32 modelnum;
	bool changinggun;
	bool changingfunc;

	if (!chr || !chr->aibot) {
		return false;
	}

	aibot = chr->aibot;

	if (weaponnum == WEAPON_BRIEFCASE2) {
		return true;
	}

	// If changing to anything other than unarmed, make sure the aibot has the
	// weapon in their inventory. Otherwise make them switch to unarmed.
	if (weaponnum != WEAPON_UNARMED) {
		item = botinvGetItem(chr, weaponnum);

		if (!item) {
			weaponnum = WEAPON_UNARMED;
			funcnum = FUNC_PRIMARY;
		}
	}

	changinggun = weaponnum != aibot->weaponnum;
	changingfunc = funcnum != aibot->gunfunc;

	if (changinggun) {
		aibot->changeguntimer60 = TICKS(60);

		for (i = 0; i < 2; i++) {
			aibot->cyclonedischarging[i] = false;
			aibot->burstsdone[i] = 0;
			aibot->reaperspeed[i] = 0;

			if (chr->weapons_held[i]) {
				chr->weapons_held[i]->obj->hidden |= OBJHFLAG_DELETING;
				chr->weapons_held[i] = NULL;
			}
		}
	}

	// Return any loaded ammo to reserve
	if (changingfunc || changinggun) {
		for (i = 0; i < 2; i++) {
			if (aibot->loadedammo[i] > 0) {
				botactGiveAmmoByWeapon(aibot, aibot->weaponnum, aibot->gunfunc, aibot->loadedammo[i]);
				aibot->loadedammo[i] = 0;
			}
		}
	}

	// Assign new weapon and function
	aibot->gunfunc = funcnum;
	aibot->weaponnum = weaponnum;

	// Load ammo from reserve into new weapon
	if (changingfunc && !changinggun) {
		for (i = 0; i < 2; i++) {
			if (chr->weapons_held[i]) {
				botactReload(chr, i, false);
			}
		}
	}

	if (!changinggun) {
		modelnum = playermgrGetModelOfWeapon(weaponnum);

		// @dangerous: item is uninitialised if weaponnum is WEAPON_UNARMED.
		// This function assumes playermgrGetModelOfWeapon returns a negative value for
		// WEAPON_UNARMED which is a dangerous assumption to make, but correct.
		if (modelnum >= 0 && item && item->type == INVITEMTYPE_DUAL && chr->weapons_held[HAND_LEFT] == NULL) {
			chrGiveWeapon(chr, modelnum, weaponnum, OBJFLAG_WEAPON_LEFTHANDED);
			botactReload(chr, HAND_LEFT, false);
		}
	}

	func = weaponGetFunctionById(weaponnum, funcnum);

	aibot->ismeleeweapon = func && func->type == INVENTORYFUNCTYPE_MELEE;

	for (i = 0; i < 2; i++) {
		if (chr->weapons_held[i]) {
			chr->weapons_held[i]->weapon->gunfunc = chr->aibot->gunfunc;
		}
	}

	return true;
}

/**
 * Drop one or all items. The item does not need to be the equipped item but it
 * must exist in the bot's inventory.
 *
 * dropall is used when the bot is killed.
 */
void botinvDrop(struct chrdata *chr, s32 weaponnum, u8 dropall)
{
	s32 i;

	if (!chr || !chr->aibot) {
		return;
	}

	for (i = 0; i < chr->aibot->maxitems; i++) {
		struct invitem *item = &chr->aibot->items[i];

		if (item->type == -1) {
			continue;
		}

		if ((item->type == INVITEMTYPE_WEAP || item->type == INVITEMTYPE_DUAL)
				&& (dropall || weaponnum == item->type_weap.weapon1)) {
			if (!weaponHasFlag(item->type_weap.weapon1, WEAPONFLAG_UNDROPPABLE)
					|| (g_Vars.normmplayerisrunning
						&& g_MpSetup.scenario == MPSCENARIO_HACKERCENTRAL
						&& item->type_weap.weapon1 == WEAPON_DATAUPLINK)) {
				s32 modelnum = playermgrGetModelOfWeapon(item->type_weap.weapon1);

				if (modelnum > 0) {
					struct prop *prop = weaponCreateForChr(chr, modelnum, item->type_weap.weapon1, OBJFLAG_WEAPON_AICANNOTUSE, NULL, NULL);

					if (prop) {
						objSetDropped(prop, DROPTYPE_DEFAULT);
						objDrop(prop, true);

						if (item->type_weap.weapon1 == WEAPON_BRIEFCASE2) {
							scenarioHandleDroppedToken(chr, prop);
						}
					}
				}
			}
		}
	}

	if ((dropall && weaponnum >= WEAPON_FALCON2)
			|| (!dropall && weaponnum == chr->aibot->weaponnum)) {
		botinvSwitchToWeapon(chr, WEAPON_UNARMED, FUNC_PRIMARY);
	}

	chr->hidden |= CHRHFLAG_DROPPINGITEM;

	if (!dropall) {
		botinvRemoveItem(chr, weaponnum);
	}
}

void botinvDropAll(struct chrdata *chr, u32 weaponnum)
{
	botinvDrop(chr, weaponnum, true);
}

void botinvDropOne(struct chrdata *chr, u32 weaponnum)
{
	botinvDrop(chr, weaponnum, false);
}
