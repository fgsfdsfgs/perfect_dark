#include <ultra64.h>
#include "n_libaudio.h"
#include "constants.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/music.h"
#include "bss.h"
#include "naudio/n_sndp.h"
#include "lib/rzip.h"
#include "lib/args.h"
#include "lib/audiomgr.h"
#include "lib/crash.h"
#include "lib/dma.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/str.h"
#include "lib/lib_2fc60.h"
#include "lib/lib_317f0.h"
#include "lib/mp3.h"
#include "lib/lib_39c80.h"
#include "lib/speaker.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "system.h"
#include "preprocess.h"
#include "mod.h"
#endif

#define MAX_SEQ_SIZE_4MB 1024 * 14
#define MAX_SEQ_SIZE_8MB 1024 * 18

#define NUM_CACHE_SLOTS 45
#define NUM_KEYTHINGS 9

struct sndcache {
	/*0x0000*/ u16 *indexes; // indexed by sfxnum, value is cache index (0-44) or 0xffff
	/*0x0004*/ u8 refcounts[NUM_CACHE_SLOTS];
	/*0x0032*/ u16 ages[NUM_CACHE_SLOTS];
	/*0x008c*/ ALEnvelope envelopes[NUM_CACHE_SLOTS];
	/*0x035c*/ ALKeyMap keymaps[NUM_CACHE_SLOTS];
	/*0x046c*/ ALWaveTable wavetables[NUM_CACHE_SLOTS];
	/*0x07f0*/ ALADPCMBook books[NUM_CACHE_SLOTS];
	/*0x3658*/ ALADPCMloop loops[NUM_CACHE_SLOTS];
	/*0x3e14*/ ALSound sounds[NUM_CACHE_SLOTS];
};

struct curmp3 {
	union soundnumhack sfxref;
	u32 playing;
	u32 unk08;
	s32 responsetimer240;
	s32 prevwhisper;
	s32 prevacknowledge;
	s32 prevgreeting;
#ifdef PLATFORM_N64
	u32 romaddr;
#else
	uintptr_t romaddr;
#endif
	u32 romsize;
	s32 responsetype;
};

s32 g_NumSounds;
uintptr_t *g_ALSoundRomOffsets;
s32 g_SndMaxFxBusses;
u32 var80094eac;
struct curmp3 g_SndCurMp3;
struct seqinstance g_SeqInstances[3];
ALHeap g_SndHeap;
u32 var80095200;
ALBank *var80095204;
struct seqtable *g_SeqTable;
u32 g_SeqBufferSize;
ALIGNED16 struct sndcache g_SndCache;

const char g_SndGuardString[] = "RUSSES SOUND GUARD STRING";
const char var70053b3c[] = "Snd: SoundHeaderCacheInit\n";

bool g_SndDisabled = false;
u32 var8005dda4 = 0x00000000;

s32 g_SndNosediveVolume = 0;
s32 g_SndNosediveAge240 = -1;
s32 g_SndNosediveDuration240 = 0;

s32 g_SndUfoVolume = 0;
s32 g_SndUfoAge240 = -1;
s32 g_SndUfoDuration240 = 0;

struct sndstate *g_SndNosediveHandle = NULL;
struct sndstate *g_SndUfoHandle = NULL;

u16 g_SfxVolume = AL_VOL_FULL;
s32 g_SoundMode = (VERSION >= VERSION_NTSC_1_0 ? SOUNDMODE_STEREO : SOUNDMODE_SURROUND);
bool g_SndMp3Enabled = false;

#if VERSION >= VERSION_NTSC_1_0
s32 g_SndNumPlaying = 0;
s32 g_SndMostEverPlaying = 0;
s32 var8005dddc = 0;
#endif

u8 *g_SndGuardStringPtr = NULL;

uintptr_t* g_SeqRomAddrs;

enum audioconfig_e {
	AUDIOCONFIG_00,
	AUDIOCONFIG_01,
	AUDIOCONFIG_02,
	AUDIOCONFIG_03,
	AUDIOCONFIG_04,
	AUDIOCONFIG_05,
	AUDIOCONFIG_06,
	AUDIOCONFIG_07,
	AUDIOCONFIG_08,
	AUDIOCONFIG_09,
	AUDIOCONFIG_10,
	AUDIOCONFIG_11,
	AUDIOCONFIG_12,
	AUDIOCONFIG_13,
	AUDIOCONFIG_14,
	AUDIOCONFIG_15,
	AUDIOCONFIG_16,
	AUDIOCONFIG_17,
	AUDIOCONFIG_18,
	AUDIOCONFIG_19,
	AUDIOCONFIG_20,
	AUDIOCONFIG_21,
	AUDIOCONFIG_22,
	AUDIOCONFIG_23,
	AUDIOCONFIG_24,
	AUDIOCONFIG_25,
	AUDIOCONFIG_26,
	AUDIOCONFIG_27,
	AUDIOCONFIG_28,
	AUDIOCONFIG_29,
	AUDIOCONFIG_30,
	AUDIOCONFIG_31,
	AUDIOCONFIG_32,
	AUDIOCONFIG_33,
	AUDIOCONFIG_34,
	AUDIOCONFIG_35,
	AUDIOCONFIG_36,
	AUDIOCONFIG_37,
	AUDIOCONFIG_38,
	AUDIOCONFIG_39,
	AUDIOCONFIG_40,
	AUDIOCONFIG_41,
	AUDIOCONFIG_42,
	AUDIOCONFIG_43,
	AUDIOCONFIG_44,
	AUDIOCONFIG_45,
	AUDIOCONFIG_46,
	AUDIOCONFIG_47,
	AUDIOCONFIG_48,
	AUDIOCONFIG_49,
	AUDIOCONFIG_50,
	AUDIOCONFIG_51,
	AUDIOCONFIG_52,
	AUDIOCONFIG_53,
#if VERSION >= VERSION_NTSC_1_0
	AUDIOCONFIG_54,
#endif
	AUDIOCONFIG_55,
	AUDIOCONFIG_56,
	AUDIOCONFIG_57,
	AUDIOCONFIG_58,
	AUDIOCONFIG_59,
#if VERSION >= VERSION_NTSC_1_0
	AUDIOCONFIG_60,
	AUDIOCONFIG_61,
	AUDIOCONFIG_62,
	AUDIOCONFIG_63
#else
	AUDIOCONFIG_60
#endif
};

struct audiorussmapping g_AudioRussMappings[] = {
	/*0x0000*/ { 0x85ba, AUDIOCONFIG_59 },
	/*0x0001*/ { 0x85a5, AUDIOCONFIG_15 },
	/*0x0002*/ { 0x85a6, AUDIOCONFIG_15 },
	/*0x0003*/ { 0x80bb, AUDIOCONFIG_14 },
	/*0x0004*/ { 0x80bc, AUDIOCONFIG_14 },
	/*0x0005*/ { 0x8007, AUDIOCONFIG_14 },
	/*0x0006*/ { 0x8007, AUDIOCONFIG_14 },
	/*0x0007*/ { 0x80be, AUDIOCONFIG_14 },
	/*0x0008*/ { 0x80bf, AUDIOCONFIG_14 },
	/*0x0009*/ { 0x80bf, AUDIOCONFIG_14 },
	/*0x000a*/ { 0x80c0, AUDIOCONFIG_14 },
	/*0x000b*/ { 0x8007, AUDIOCONFIG_14 },
	/*0x000c*/ { 0x80c2, AUDIOCONFIG_14 },
	/*0x000d*/ { 0x80c3, AUDIOCONFIG_14 },
	/*0x000e*/ { 0x80c4, AUDIOCONFIG_14 },
	/*0x000f*/ { 0x80c5, AUDIOCONFIG_14 },
	/*0x0010*/ { 0x8007, AUDIOCONFIG_14 },
	/*0x0011*/ { 0x80c7, AUDIOCONFIG_14 },
	/*0x0012*/ { 0x80c8, AUDIOCONFIG_14 },
	/*0x0013*/ { 0x80c9, AUDIOCONFIG_14 },
	/*0x0014*/ { 0x80ca, AUDIOCONFIG_15 },
	/*0x0015*/ { 0x80cb, AUDIOCONFIG_15 },
	/*0x0016*/ { 0x80cc, AUDIOCONFIG_15 },
	/*0x0017*/ { 0x8007, AUDIOCONFIG_15 },
	/*0x0018*/ { 0x8007, AUDIOCONFIG_15 },
	/*0x0019*/ { 0x8007, AUDIOCONFIG_15 },
	/*0x001a*/ { 0x80d2, AUDIOCONFIG_15 },
	/*0x001b*/ { 0x80d3, AUDIOCONFIG_15 },
	/*0x001c*/ { 0x8007, AUDIOCONFIG_15 },
	/*0x001d*/ { 0x80c6, AUDIOCONFIG_14 },
	/*0x001e*/ { 0x80d6, AUDIOCONFIG_15 },
	/*0x001f*/ { 0x80d7, AUDIOCONFIG_15 },
	/*0x0020*/ { 0x80d8, AUDIOCONFIG_15 },
	/*0x0021*/ { 0x80db, AUDIOCONFIG_14 },
	/*0x0022*/ { 0x80da, AUDIOCONFIG_14 },
	/*0x0023*/ { 0x80bf, AUDIOCONFIG_14 },
	/*0x0024*/ { 0x80bf, AUDIOCONFIG_14 },
	/*0x0025*/ { 0x80de, AUDIOCONFIG_14 },
	/*0x0026*/ { 0x80e1, AUDIOCONFIG_15 },
	/*0x0027*/ { 0x80e2, AUDIOCONFIG_15 },
	/*0x0028*/ { 0x977a, AUDIOCONFIG_60 }, // Carrington: "Hello Joanna"
	/*0x0029*/ { 0x97b0, AUDIOCONFIG_60 }, // Carrington: "Joanna, it's good to see you"
	/*0x002a*/ { 0x97b1, AUDIOCONFIG_60 }, // Carrington: "Good day to you, Jo"
	/*0x002b*/ { 0x977c, AUDIOCONFIG_60 }, // Female: "Hi there"
	/*0x002c*/ { 0x9786, AUDIOCONFIG_60 }, // Grimshaw: "Welcome to hacker central"
	/*0x002d*/ { 0x9787, AUDIOCONFIG_60 }, // Grimshaw: "Hi there"
	/*0x002e*/ { 0x9788, AUDIOCONFIG_60 }, // Grimshaw: "Um, er, hi"
	/*0x002f*/ { 0x978c, AUDIOCONFIG_60 }, // Holo girl: "Hi. What can I do for you?"
	/*0x0030*/ { 0x9794, AUDIOCONFIG_60 }, // Hangar guy: "What do you want?"
	/*0x0031*/ { 0x9784, AUDIOCONFIG_60 }, // Foster: "Ah, our star agent"
	/*0x0032*/ { 0x978e, AUDIOCONFIG_60 }, // Male: "Hey there"
	/*0x0033*/ { 0x978f, AUDIOCONFIG_60 }, // Male: "Hi"
	/*0x0034*/ { 0x9790, AUDIOCONFIG_60 }, // Male "How's it going?"
	/*0x0035*/ { 0x977e, AUDIOCONFIG_60 }, // Female: "Hello"
	/*0x0036*/ { 0x977f, AUDIOCONFIG_60 }, // Holo girl: "Hi Jo"
	/*0x0037*/ { 0x9780, AUDIOCONFIG_60 }, // Holo girl: "How's it going?"
	/*0x0038*/ { 0x802e, AUDIOCONFIG_32 },
	/*0x0039*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x003a*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x003b*/ { 0x806a, AUDIOCONFIG_33 },
	/*0x003c*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x003d*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x003e*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x003f*/ { 0x806d, AUDIOCONFIG_33 },
	/*0x0040*/ { 0x806e, AUDIOCONFIG_33 },
	/*0x0041*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0042*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0043*/ { 0x8070, AUDIOCONFIG_33 },
	/*0x0044*/ { 0x8071, AUDIOCONFIG_33 },
	/*0x0045*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0046*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0047*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0048*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0049*/ { 0x8075, AUDIOCONFIG_33 },
	/*0x004a*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x004b*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x004c*/ { 0x8079, AUDIOCONFIG_31 },
	/*0x004d*/ { 0x8066, AUDIOCONFIG_31 },
	/*0x004e*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x004f*/ { 0x8032, AUDIOCONFIG_31 },
	/*0x0050*/ { 0x8037, AUDIOCONFIG_35 },
	/*0x0051*/ { 0x807a, AUDIOCONFIG_35 },
	/*0x0052*/ { 0x8059, AUDIOCONFIG_31 },
	/*0x0053*/ { 0x8001, AUDIOCONFIG_35 },
	/*0x0054*/ { 0x847e, AUDIOCONFIG_32 },
	/*0x0055*/ { 0x847d, AUDIOCONFIG_33 },
	/*0x0056*/ { 0x84f1, AUDIOCONFIG_32 },
	/*0x0057*/ { 0x84f2, AUDIOCONFIG_32 },
	/*0x0058*/ { 0x84f3, AUDIOCONFIG_32 },
	/*0x0059*/ { 0x84f4, AUDIOCONFIG_33 },
	/*0x005a*/ { 0x84f6, AUDIOCONFIG_33 },
	/*0x005b*/ { 0x8506, AUDIOCONFIG_31 },
	/*0x005c*/ { 0x84fd, AUDIOCONFIG_33 },
	/*0x005d*/ { 0x8504, AUDIOCONFIG_33 },
	/*0x005e*/ { 0x84fe, AUDIOCONFIG_33 },
	/*0x005f*/ { 0x8503, AUDIOCONFIG_33 },
	/*0x0060*/ { 0x8500, AUDIOCONFIG_32 },
	/*0x0061*/ { 0x8501, AUDIOCONFIG_32 },
	/*0x0062*/ { 0x8502, AUDIOCONFIG_32 },
	/*0x0063*/ { 0x85ca, AUDIOCONFIG_31 },
	/*0x0064*/ { 0x85cb, AUDIOCONFIG_31 },
	/*0x0065*/ { 0x85d2, AUDIOCONFIG_31 },
	/*0x0066*/ { 0x8074, AUDIOCONFIG_33 },
	/*0x0067*/ { 0x85eb, AUDIOCONFIG_35 },
	/*0x0068*/ { 0x85ec, AUDIOCONFIG_35 },
	/*0x0069*/ { 0x85ed, AUDIOCONFIG_31 },
	/*0x006a*/ { 0x85ee, AUDIOCONFIG_31 },
	/*0x006b*/ { 0x85ef, AUDIOCONFIG_33 },
	/*0x006c*/ { 0x85f0, AUDIOCONFIG_33 },
	/*0x006d*/ { 0x85f1, AUDIOCONFIG_33 },
	/*0x006e*/ { 0x85f2, AUDIOCONFIG_33 },
	/*0x006f*/ { 0x85f3, AUDIOCONFIG_33 },
	/*0x0070*/ { 0x85f4, AUDIOCONFIG_33 },
	/*0x0071*/ { 0x8508, AUDIOCONFIG_31 },
	/*0x0072*/ { 0x850a, AUDIOCONFIG_31 },
	/*0x0073*/ { 0x8076, AUDIOCONFIG_33 },
	/*0x0074*/ { 0x85f5, AUDIOCONFIG_30 },
	/*0x0075*/ { 0x8509, AUDIOCONFIG_33 },
	/*0x0076*/ { 0x8045, AUDIOCONFIG_45 },
	/*0x0077*/ { 0x8046, AUDIOCONFIG_45 },
	/*0x0078*/ { 0x8047, AUDIOCONFIG_25 },
	/*0x0079*/ { 0x8048, AUDIOCONFIG_45 },
	/*0x007a*/ { 0x80f0, AUDIOCONFIG_45 },
	/*0x007b*/ { 0x80ef, AUDIOCONFIG_45 },
	/*0x007c*/ { 0x8049, AUDIOCONFIG_45 },
	/*0x007d*/ { 0x804a, AUDIOCONFIG_45 },
	/*0x007e*/ { 0x804b, AUDIOCONFIG_45 },
	/*0x007f*/ { 0x80d9, AUDIOCONFIG_45 },
	/*0x0080*/ { 0x804c, AUDIOCONFIG_45 },
	/*0x0081*/ { 0x80e7, AUDIOCONFIG_45 },
	/*0x0082*/ { 0x80e6, AUDIOCONFIG_45 },
	/*0x0083*/ { 0x80e0, AUDIOCONFIG_45 },
	/*0x0084*/ { 0x80dc, AUDIOCONFIG_45 },
	/*0x0085*/ { 0x80b9, AUDIOCONFIG_45 },
	/*0x0086*/ { 0x80d4, AUDIOCONFIG_45 },
	/*0x0087*/ { 0x80d0, AUDIOCONFIG_45 },
	/*0x0088*/ { 0x80d1, AUDIOCONFIG_45 },
	/*0x0089*/ { 0x8078, AUDIOCONFIG_45 },
	/*0x008a*/ { 0x80dd, AUDIOCONFIG_45 },
	/*0x008b*/ { 0x8003, AUDIOCONFIG_45 },
	/*0x008c*/ { 0x802d, AUDIOCONFIG_21 },
	/*0x008d*/ { 0x807b, AUDIOCONFIG_21 },
	/*0x008e*/ { 0x807c, AUDIOCONFIG_21 },
	/*0x008f*/ { 0x807d, AUDIOCONFIG_21 },
	/*0x0090*/ { 0x807e, AUDIOCONFIG_21 },
	/*0x0091*/ { 0x807f, AUDIOCONFIG_21 },
	/*0x0092*/ { 0x8080, AUDIOCONFIG_21 },
	/*0x0093*/ { 0x8081, AUDIOCONFIG_21 },
	/*0x0094*/ { 0x8082, AUDIOCONFIG_21 },
	/*0x0095*/ { 0x8083, AUDIOCONFIG_21 },
	/*0x0096*/ { 0x8084, AUDIOCONFIG_21 },
	/*0x0097*/ { 0x8085, AUDIOCONFIG_21 },
	/*0x0098*/ { 0x80b4, AUDIOCONFIG_18 },
	/*0x0099*/ { 0x80b5, AUDIOCONFIG_18 },
	/*0x009a*/ { 0x80b6, AUDIOCONFIG_17 },
	/*0x009b*/ { 0x80a9, AUDIOCONFIG_18 },
	/*0x009c*/ { 0x80aa, AUDIOCONFIG_18 },
	/*0x009d*/ { 0x80ab, AUDIOCONFIG_17 },
	/*0x009e*/ { 0x80ac, AUDIOCONFIG_18 },
	/*0x009f*/ { 0x80ad, AUDIOCONFIG_18 },
	/*0x00a0*/ { 0x80ae, AUDIOCONFIG_17 },
	/*0x00a1*/ { 0x80b1, AUDIOCONFIG_18 },
	/*0x00a2*/ { 0x80b2, AUDIOCONFIG_18 },
	/*0x00a3*/ { 0x80b3, AUDIOCONFIG_18 },
	/*0x00a4*/ { 0x818a, AUDIOCONFIG_19 },
	/*0x00a5*/ { VERSION >= VERSION_NTSC_1_0 ? 0xefdd : 0x8190, AUDIOCONFIG_19 },
	/*0x00a6*/ { 0x805f, AUDIOCONFIG_32 },
	/*0x00a7*/ { 0x8060, AUDIOCONFIG_32 },
	/*0x00a8*/ { 0x8061, AUDIOCONFIG_32 },
	/*0x00a9*/ { 0x8004, AUDIOCONFIG_32 },
	/*0x00aa*/ { 0x80f1, AUDIOCONFIG_32 },
	/*0x00ab*/ { 0x80f3, AUDIOCONFIG_32 },
	/*0x00ac*/ { 0x8055, AUDIOCONFIG_16 },
	/*0x00ad*/ { 0x8056, AUDIOCONFIG_16 },
	/*0x00ae*/ { 0x8057, AUDIOCONFIG_16 },
	/*0x00af*/ { 0x85f7, AUDIOCONFIG_52 },
	/*0x00b0*/ { 0x8127, AUDIOCONFIG_29 },
	/*0x00b1*/ { 0x8128, AUDIOCONFIG_29 },
	/*0x00b2*/ { 0x8129, AUDIOCONFIG_29 },
	/*0x00b3*/ { 0x812a, AUDIOCONFIG_29 },
	/*0x00b4*/ { 0x812b, AUDIOCONFIG_29 },
	/*0x00b5*/ { 0x812c, AUDIOCONFIG_29 },
	/*0x00b6*/ { 0x812d, AUDIOCONFIG_50 },
	/*0x00b7*/ { 0x812e, AUDIOCONFIG_50 },
	/*0x00b8*/ { 0x8138, AUDIOCONFIG_50 },
	/*0x00b9*/ { 0x8138, AUDIOCONFIG_28 },
	/*0x00ba*/ { 0x812f, AUDIOCONFIG_51 },
	/*0x00bb*/ { 0x8151, AUDIOCONFIG_51 },
	/*0x00bc*/ { 0x8154, AUDIOCONFIG_51 },
	/*0x00bd*/ { 0x8157, AUDIOCONFIG_51 },
	/*0x00be*/ { 0x815a, AUDIOCONFIG_51 },
	/*0x00bf*/ { 0x8132, AUDIOCONFIG_51 },
	/*0x00c0*/ { 0x8139, AUDIOCONFIG_51 },
	/*0x00c1*/ { 0x813f, AUDIOCONFIG_51 },
	/*0x00c2*/ { 0x8145, AUDIOCONFIG_51 },
	/*0x00c3*/ { 0x814b, AUDIOCONFIG_51 },
	/*0x00c4*/ { 0x8161, AUDIOCONFIG_04 },
	/*0x00c5*/ { 0x8162, AUDIOCONFIG_04 },
	/*0x00c6*/ { 0x8163, AUDIOCONFIG_04 },
	/*0x00c7*/ { 0x8164, AUDIOCONFIG_04 },
	/*0x00c8*/ { 0x8165, AUDIOCONFIG_04 },
	/*0x00c9*/ { 0x8166, AUDIOCONFIG_04 },
	/*0x00ca*/ { 0x8167, AUDIOCONFIG_04 },
	/*0x00cb*/ { 0x8168, AUDIOCONFIG_04 },
	/*0x00cc*/ { 0x8169, AUDIOCONFIG_04 },
	/*0x00cd*/ { 0x816a, AUDIOCONFIG_04 },
	/*0x00ce*/ { 0x816b, AUDIOCONFIG_04 },
	/*0x00cf*/ { 0x816c, AUDIOCONFIG_04 },
	/*0x00d0*/ { 0x816d, AUDIOCONFIG_04 },
	/*0x00d1*/ { 0x816e, AUDIOCONFIG_04 },
	/*0x00d2*/ { 0x816f, AUDIOCONFIG_04 },
	/*0x00d3*/ { 0x8170, AUDIOCONFIG_04 },
	/*0x00d4*/ { 0x8171, AUDIOCONFIG_04 },
	/*0x00d5*/ { 0x8172, AUDIOCONFIG_04 },
	/*0x00d6*/ { 0x8173, AUDIOCONFIG_04 },
	/*0x00d7*/ { 0x8174, AUDIOCONFIG_04 },
	/*0x00d8*/ { 0x8175, AUDIOCONFIG_04 },
	/*0x00d9*/ { 0x8176, AUDIOCONFIG_04 },
	/*0x00da*/ { 0x8177, AUDIOCONFIG_04 },
	/*0x00db*/ { 0x8178, AUDIOCONFIG_04 },
	/*0x00dc*/ { 0x8179, AUDIOCONFIG_04 },
	/*0x00dd*/ { 0x817a, AUDIOCONFIG_04 },
	/*0x00de*/ { 0x817b, AUDIOCONFIG_04 },
	/*0x00df*/ { 0x817c, AUDIOCONFIG_04 },
	/*0x00e0*/ { 0x817d, AUDIOCONFIG_04 },
	/*0x00e1*/ { 0x817e, AUDIOCONFIG_04 },
	/*0x00e2*/ { 0x817f, AUDIOCONFIG_04 },
	/*0x00e3*/ { 0x8180, AUDIOCONFIG_04 },
	/*0x00e4*/ { 0x804d, AUDIOCONFIG_04 },
	/*0x00e5*/ { 0x804e, AUDIOCONFIG_04 },
	/*0x00e6*/ { 0x85f8, AUDIOCONFIG_05 },
	/*0x00e7*/ { 0x85f9, AUDIOCONFIG_05 },
	/*0x00e8*/ { 0x85fa, AUDIOCONFIG_05 },
	/*0x00e9*/ { 0x85fb, AUDIOCONFIG_05 },
	/*0x00ea*/ { 0x85fc, AUDIOCONFIG_06 },
	/*0x00eb*/ { 0x85fd, AUDIOCONFIG_06 },
	/*0x00ec*/ { 0x85fe, AUDIOCONFIG_06 },
	/*0x00ed*/ { 0x85ff, AUDIOCONFIG_06 },
	/*0x00ee*/ { 0x8600, AUDIOCONFIG_04 },
	/*0x00ef*/ { 0x8601, AUDIOCONFIG_04 },
	/*0x00f0*/ { 0x8602, AUDIOCONFIG_04 },
	/*0x00f1*/ { 0x8603, AUDIOCONFIG_04 },
	/*0x00f2*/ { 0x8604, AUDIOCONFIG_04 },
	/*0x00f3*/ { 0x8605, AUDIOCONFIG_04 },
	/*0x00f4*/ { 0x8606, AUDIOCONFIG_04 },
	/*0x00f5*/ { 0x8606, AUDIOCONFIG_04 },
#if VERSION >= VERSION_NTSC_1_0
	/*0x00f6*/ { 0x81d9, AUDIOCONFIG_61 },
#endif
	/*0x00f7*/ { 0x8267, AUDIOCONFIG_02 },
	/*0x00f8*/ { 0x8268, AUDIOCONFIG_02 },
	/*0x00f9*/ { 0x8286, AUDIOCONFIG_02 },
	/*0x00fa*/ { 0x8288, AUDIOCONFIG_02 },
	/*0x00fb*/ { 0x8291, AUDIOCONFIG_02 },
	/*0x00fc*/ { 0x829f, AUDIOCONFIG_02 },
	/*0x00fd*/ { 0x82a1, AUDIOCONFIG_02 },
#if VERSION >= VERSION_NTSC_1_0
	/*0x00fe*/ { 0x8af9, AUDIOCONFIG_47 }, // Cass: "You've become quite an annoyance"
	/*0x00ff*/ { 0x8afa, AUDIOCONFIG_47 }, // Cass: "We meet again, girl"
	/*0x0100*/ { 0x8afb, AUDIOCONFIG_47 }, // Cass: "Good night, Miss Dark"
#else
	/*0x00fe*/ { 0x92f9, AUDIOCONFIG_47 }, // Cass: "You've become quite an annoyance"
	/*0x00ff*/ { 0x92fa, AUDIOCONFIG_47 }, // Cass: "We meet again, girl"
	/*0x0100*/ { 0x92fb, AUDIOCONFIG_47 }, // Cass: "Good night, Miss Dark"
#endif
	/*0x0101*/ { 0x92fc, AUDIOCONFIG_47 }, // Cass: "Get her!"
	/*0x0102*/ { 0x92fd, AUDIOCONFIG_47 }, // Cass: (laughing)
	/*0x0103*/ { 0x9285, AUDIOCONFIG_02 }, // Scientist: "Who the hell are you?"
	/*0x0104*/ { 0x9306, AUDIOCONFIG_48 }, // Programmer: "Yes, yes, I agree. Personality..."
	/*0x0105*/ { 0x9307, AUDIOCONFIG_48 }, // Programmer: "No. I can start the process..."
	/*0x0106*/ { 0x9308, AUDIOCONFIG_48 }, // Programmer: "I'll begin at once. Goodbye"
	/*0x0107*/ { 0x9309, AUDIOCONFIG_48 }, // Programmer: "Security, help me!"
	/*0x0108*/ { 0x930a, AUDIOCONFIG_48 }, // Programmer: "Help! Help!"
	/*0x0109*/ { 0x8435, AUDIOCONFIG_49 },
	/*0x010a*/ { 0x81b5, AUDIOCONFIG_07 },
	/*0x010b*/ { 0x81b6, AUDIOCONFIG_08 },
	/*0x010c*/ { 0x81b5, AUDIOCONFIG_09 },
	/*0x010d*/ { 0x81b6, AUDIOCONFIG_10 },
	/*0x010e*/ { 0x85f3, AUDIOCONFIG_11 },
	/*0x010f*/ { 0x81b5, AUDIOCONFIG_12 },
	/*0x0110*/ { 0x81b6, AUDIOCONFIG_13 },
	/*0x0111*/ { 0x81b7, AUDIOCONFIG_20 },
	/*0x0112*/ { 0x81b8, AUDIOCONFIG_20 },
	/*0x0113*/ { 0x81c7, AUDIOCONFIG_20 },
	/*0x0114*/ { 0x8037, AUDIOCONFIG_20 },
	/*0x0115*/ { 0x81b7, AUDIOCONFIG_56 },
	/*0x0116*/ { 0x81b9, AUDIOCONFIG_00 },
	/*0x0117*/ { 0x81ba, AUDIOCONFIG_00 },
	/*0x0118*/ { 0x81bb, AUDIOCONFIG_00 },
	/*0x0119*/ { 0x81cb, AUDIOCONFIG_36 },
	/*0x011a*/ { 0x81cb, AUDIOCONFIG_37 },
	/*0x011b*/ { 0x81cb, AUDIOCONFIG_38 },
#if VERSION >= VERSION_NTSC_1_0
	/*0x011c*/ { 0x81c6, AUDIOCONFIG_55 },
#else
	/*0x011c*/ { 0x81c6, AUDIOCONFIG_55 },
#endif
	/*0x011d*/ { 0x9278, AUDIOCONFIG_47 }, // "That's not how it goes"
	/*0x011e*/ { 0x9279, AUDIOCONFIG_47 }, // "Give it to me. You're doing it wrong"
	/*0x011f*/ { 0x927a, AUDIOCONFIG_47 }, // "Oh no, we're too late"
	/*0x0120*/ { 0x927b, AUDIOCONFIG_47 }, // "Yes it is"
	/*0x0121*/ { 0x927c, AUDIOCONFIG_47 }, // "Stop annoying me"
	/*0x0122*/ { 0x927d, AUDIOCONFIG_47 }, // "She's here!"
	/*0x0123*/ { 0x9277, AUDIOCONFIG_47 }, // "Will you just hurry up?"
	/*0x0124*/ { 0x82fd, AUDIOCONFIG_02 }, // "Holy..."
	/*0x0125*/ { 0x82fe, AUDIOCONFIG_02 }, // "What the hell?"
	/*0x0126*/ { 0x8300, AUDIOCONFIG_02 }, // "Oh my god"
	/*0x0127*/ { 0x8303, AUDIOCONFIG_02 }, // "Oh god I'm hit"
	/*0x0128*/ { 0x830a, AUDIOCONFIG_02 }, // "Oh my god"
#if VERSION >= VERSION_NTSC_1_0
	/*0x0129*/ { 0x8313, AUDIOCONFIG_62 }, // "Noooo!"
#endif
	/*0x012a*/ { 0x8314, AUDIOCONFIG_02 }, // "Oh god he's dead!"
	/*0x012b*/ { 0x832e, AUDIOCONFIG_02 }, // "Bloody stupid gun"
	/*0x012c*/ { 0x8330, AUDIOCONFIG_02 }, // "Damn it"
	/*0x012d*/ { 0x8333, AUDIOCONFIG_02 }, // "Damn, she's good"
	/*0x012e*/ { 0x8334, AUDIOCONFIG_02 }, // "Cover my ass"
#if VERSION >= VERSION_NTSC_1_0
	/*0x012f*/ { 0x834c, AUDIOCONFIG_62 }, // Death scream
#endif
	/*0x0130*/ { 0x83a3, AUDIOCONFIG_02 }, // Female: "Damn she's good"
	/*0x0131*/ { 0x83b9, AUDIOCONFIG_02 }, // Female: "My god"
	/*0x0132*/ { 0x83c7, AUDIOCONFIG_02 }, // "Geez, that hurt"
	/*0x0133*/ { 0x83cf, AUDIOCONFIG_02 }, // "Damn it, I'm taking fire"
	/*0x0134*/ { 0x83d2, AUDIOCONFIG_02 }, // "God damn it"
	/*0x0135*/ { 0x83d4, AUDIOCONFIG_02 }, // "Holy moley"
	/*0x0136*/ { 0x83f3, AUDIOCONFIG_02 }, // "Damn, missed again"
	/*0x0137*/ { 0x83f5, AUDIOCONFIG_02 }, // "Damn"
	/*0x0138*/ { 0x83f6, AUDIOCONFIG_02 }, // "Hell, she's good"
	/*0x0139*/ { 0x8400, AUDIOCONFIG_02 }, // "Let's get the hell out of here"
#if VERSION >= VERSION_NTSC_1_0
	/*0x013a*/ { 0x8411, AUDIOCONFIG_62 }, // "Noooo!"
#endif
	/*0x013b*/ { 0x8413, AUDIOCONFIG_02 }, // "Shoot, damn it"
	/*0x013c*/ { 0x8414, AUDIOCONFIG_02 }, // "Oh god, I'm dying"
	/*0x013d*/ { 0x8422, AUDIOCONFIG_02 }, // "God, run!"
	/*0x013e*/ { 0x8432, AUDIOCONFIG_33 },
	/*0x013f*/ { 0x843c, AUDIOCONFIG_58 },
	/*0x0140*/ { 0x843d, AUDIOCONFIG_58 },
	/*0x0141*/ { 0x84ab, AUDIOCONFIG_58 },
	/*0x0142*/ { 0x843d, AUDIOCONFIG_58 },
	/*0x0143*/ { 0x81b7, AUDIOCONFIG_58 },
	/*0x0144*/ { 0x8477, AUDIOCONFIG_57 },
	/*0x0145*/ { 0x8478, AUDIOCONFIG_57 },
#if VERSION >= VERSION_NTSC_1_0
	/*0x0146*/ { 0x81c2, AUDIOCONFIG_54 },
	/*0x0147*/ { 0x8479, AUDIOCONFIG_54 },
	/*0x0148*/ { 0x81c3, AUDIOCONFIG_54 },
	/*0x0149*/ { 0x81c4, AUDIOCONFIG_54 },
#else
	/*0x0146*/ { 0x81c2, AUDIOCONFIG_55 },
	/*0x0147*/ { 0x8479, AUDIOCONFIG_55 },
	/*0x0148*/ { 0x81c3, AUDIOCONFIG_55 },
	/*0x0149*/ { 0x81c4, AUDIOCONFIG_55 },
#endif
	/*0x014a*/ { 0x8443, AUDIOCONFIG_02 }, // Civilian: "Oh my god"
	/*0x014b*/ { 0x844e, AUDIOCONFIG_02 }, // "What the hell was that?"
#if VERSION >= VERSION_NTSC_1_0
	/*0x014c*/ { 0x843c, AUDIOCONFIG_63 },
	/*0x014d*/ { 0x847c, AUDIOCONFIG_63 },
	/*0x014e*/ { 0x843d, AUDIOCONFIG_63 },
#endif
	/*0x014f*/ { 0xf2ad, AUDIOCONFIG_01 }, // "Joanna, this is Jonathan. You're in grave danger"
	/*0x0150*/ { 0xf2ae, AUDIOCONFIG_01 }, // "They've flooded the area with nerge gas"
	/*0x0151*/ { 0xf2af, AUDIOCONFIG_01 }, // "Get our friend to the containment lab. There's a hiding place there"
	/*0x0152*/ { 0xd2b0, AUDIOCONFIG_01 }, // "I'll contact you when you get there. Good luck"
	/*0x0153*/ { 0xf2b1, AUDIOCONFIG_01 }, // "Good work Jo. We need to meet up. They're getting suspicious of me"
	/*0x0154*/ { 0xd2b2, AUDIOCONFIG_01 }, // "Use the maintenance hatch I've opened in one of the containment labs. Hurry"
	/*0x0155*/ { 0x92b8, AUDIOCONFIG_02 }, // "Damn it Joanna, cover me!"
	/*0x0156*/ { 0x9b30, AUDIOCONFIG_02 }, // "Oh my god, a gun?!"
	/*0x0157*/ { 0x9b27, AUDIOCONFIG_47 }, // "So you got here at last"
	/*0x0158*/ { 0x9b28, AUDIOCONFIG_47 }, // "Hey, you're not, uh, security alert!"
	/*0x0159*/ { 0x9b29, AUDIOCONFIG_47 }, // "No guns allowed in here. Put your weapon away"
	/*0x015a*/ { 0x9b2a, AUDIOCONFIG_47 }, // "I won't tell you again. Lose the weapon"
	/*0x015b*/ { 0x9b2b, AUDIOCONFIG_47 }, // "Right. I'm calling security"
	/*0x015c*/ { 0x9b2c, AUDIOCONFIG_47 }, // "Everyone's been waiting for you"
	/*0x015d*/ { 0x9a9d, AUDIOCONFIG_60 }, // "How's things?"
	/*0x015e*/ { 0x9a9e, AUDIOCONFIG_60 }, // "Hey there"
	/*0x015f*/ { 0x9a9f, AUDIOCONFIG_60 }, // "Hi. How are you?"
	/*0x0160*/ { 0x8037, AUDIOCONFIG_04 },
	/*0x0161*/ { 0x9271, AUDIOCONFIG_60 }, // Female: "Good afternoon"
	/*0x0162*/ { 0x9275, AUDIOCONFIG_02 }, // Female: "Oh my god!"
	/*0x0163*/ { 0x9acc, AUDIOCONFIG_60 }, // "Hi there"
	/*0x0164*/ { 0x9acd, AUDIOCONFIG_60 }, // "How's things"
	/*0x0165*/ { 0x9ad4, AUDIOCONFIG_60 }, // "Hello"
	/*0x0166*/ { 0x9ad5, AUDIOCONFIG_60 }, // "Hey, what's up?"
	/*0x0167*/ { 0x84c8, AUDIOCONFIG_02 }, // "What the hell?"
	/*0x0168*/ { 0x84cd, AUDIOCONFIG_02 }, // "Geez"
	/*0x0169*/ { 0x84ad, AUDIOCONFIG_46 },
	/*0x016a*/ { 0x8507, AUDIOCONFIG_27 },
	/*0x016b*/ { 0x84ee, AUDIOCONFIG_16 },
	/*0x016c*/ { 0x84ef, AUDIOCONFIG_16 },
	/*0x016d*/ { 0x84f0, AUDIOCONFIG_16 },
	/*0x016e*/ { 0x92c8, AUDIOCONFIG_02 }, // President: "What the hell was that?"
	/*0x016f*/ { 0x92c9, AUDIOCONFIG_02 }, // Trent: "How the hell did you get on board?"
	/*0x0170*/ { 0xb3a0, AUDIOCONFIG_01 }, // Carrington: "We're getting a positive reading. The internal comms hub is nearby"
	/*0x0171*/ { 0xb3a1, AUDIOCONFIG_01 }, // Carrington: "You're on the same floor as Cassandra's office"
	/*0x0172*/ { 0xd3a3, AUDIOCONFIG_01 }, // Carrington: "The basement elevator must be around there somewhere"
	/*0x0173*/ { 0xd3a4, AUDIOCONFIG_01 }, // Carrington: "That's the highest security sector. Dr Caroll has got to be nearby"
	/*0x0174*/ { 0xd3a6, AUDIOCONFIG_01 }, // Grimshaw: "Reprogram the cleaning bot. It'll give you a way in"
	/*0x0175*/ { 0xd3a7, AUDIOCONFIG_01 }, // Carrington: "Get out Jo! The levels are too high. Use the camspy"
	/*0x0176*/ { 0xd3a8, AUDIOCONFIG_01 }, // Carrington: "They've locked down the ground floor. Get to the elevator"
	/*0x0177*/ { 0xd3a9, AUDIOCONFIG_01 }, // Carrington: "That copter needs to be taken out of the jumpship will get shut down"
	/*0x0178*/ { 0xd3ab, AUDIOCONFIG_01 }, // Carrington: "Don't keep the jumpship waiting. Get back to the landing pad"
	/*0x0179*/ { 0xd3b0, AUDIOCONFIG_01 }, // Carrington: "The only place we could secure the equipment was a storm drain"
	/*0x017a*/ { 0xd3b2, AUDIOCONFIG_01 }, // Grimshaw: "Reprogram that taxi. It's a mark 2 and should be compatible"
	/*0x017b*/ { 0xd3b3, AUDIOCONFIG_01 }, // Carrington: The carpark lift is your only point of ingress"
	/*0x017c*/ { 0xb3b6, AUDIOCONFIG_01 }, // Grimshaw: "The safe's heavily encrypted. The decoder's going to take a little longer"
	/*0x017d*/ { 0xd3b8, AUDIOCONFIG_01 }, // Carrington: "The air intercept radar is controlled from that bunker"
	/*0x017e*/ { 0xd3ba, AUDIOCONFIG_01 }, // Carrington: "The hangar lift is the other side of that huge door"
	/*0x017f*/ { 0xd3bb, AUDIOCONFIG_01 }, // Jon: "Careful with that trigger finger, Agent Dark. You're closing in on my position"
	/*0x0180*/ { 0xb3bd, AUDIOCONFIG_01 }, // Carrington: "The temperature's dropping. You must be close to the cryo room"
	/*0x0181*/ { 0xd3bf, AUDIOCONFIG_01 }, // Carrington: "There he is Jo. Hurry or you'll be too late"
	/*0x0182*/ { 0xb3c4, AUDIOCONFIG_01 }, // Carrington: "Now's your chance. Grab the stewardess's bag and get her uniform"
	/*0x0183*/ { 0xb3c5, AUDIOCONFIG_01 }, // Carrington: "Use that case to conceal your equipment"
	/*0x0184*/ { 0xb3c6, AUDIOCONFIG_01 }, // Carrington: "The case will get stopped unless you do something"
	/*0x0185*/ { 0xb3c9, AUDIOCONFIG_01 }, // Carrington: "The president's room is on this level, Joanna"
	/*0x0186*/ { 0xb3ca, AUDIOCONFIG_01 }, // Carrington: "Something's wrong. You have to get to the cockpit"
	/*0x0187*/ { 0x8594, AUDIOCONFIG_04 }, // Footstep
	/*0x0188*/ { 0x8595, AUDIOCONFIG_04 }, // Footstep
	/*0x0189*/ { 0x8596, AUDIOCONFIG_04 }, // Footstep
	/*0x018a*/ { 0x8597, AUDIOCONFIG_04 }, // Footstep
	/*0x018b*/ { 0x8598, AUDIOCONFIG_04 }, // Footstep
	/*0x018c*/ { 0x8599, AUDIOCONFIG_04 }, // Footstep
	/*0x018d*/ { 0x859a, AUDIOCONFIG_04 }, // Footstep
	/*0x018e*/ { 0x859b, AUDIOCONFIG_04 }, // Footstep
	/*0x018f*/ { 0x85a0, AUDIOCONFIG_04 }, // Footstep
	/*0x0190*/ { 0x85a1, AUDIOCONFIG_04 }, // Footstep
	/*0x0191*/ { 0x85a2, AUDIOCONFIG_04 }, // Footstep
	/*0x0192*/ { 0x85a3, AUDIOCONFIG_04 }, // Footstep
	/*0x0193*/ { 0xf310, AUDIOCONFIG_01 }, // Carrington: "Joanna. We're under attack"
	/*0x0194*/ { 0xd311, AUDIOCONFIG_01 }, // Carrington: "Get the guns back online. Hurry"
	/*0x0195*/ { 0xf312, AUDIOCONFIG_01 }, // Carrington: "The Skedar have taken hostages"
	/*0x0196*/ { 0xd313, AUDIOCONFIG_01 }, // Carrington: "Get up to the offices and save them"
	/*0x0197*/ { 0xf314, AUDIOCONFIG_01 }, // Carrington: "They're using a new form of shield technology"
	/*0x0198*/ { 0xd315, AUDIOCONFIG_01 }, // Carrington: "Foster was working on a new weapon which may be useful"
	/*0x0199*/ { 0xf316, AUDIOCONFIG_03 }, // Carrington: "Damn it, my office. If they get access..."
	/*0x019a*/ { 0xd317, AUDIOCONFIG_01 }, // Carrington: "Get there first, Jo and destroy the files"
	/*0x019b*/ { 0xf318, AUDIOCONFIG_01 }, // Carrington: "Things are desparate. They've planted a bomb"
	/*0x019c*/ { 0xd319, AUDIOCONFIG_01 }, // Carrington: "Find it and get it out of the building"
	/*0x019d*/ { 0xf31a, AUDIOCONFIG_01 }, // Carrington: "Well done Joanna. We're nearly clear"
	/*0x019e*/ { 0xd31b, AUDIOCONFIG_01 }, // Carrington: "The last dropship is waiting for you. Hurry"
	/*0x019f*/ { 0xf293, AUDIOCONFIG_01 }, // Grimshaw: "Joanna, it's Grimshaw. We've got hackers at..."
	/*0x01a0*/ { 0xf2df, AUDIOCONFIG_01 }, // Elvis: "We're outside, Joanna. Get the shields down and we can help you"
	/*0x01a1*/ { 0xf2e0, AUDIOCONFIG_01 }, // Elvis: "You've got to open the hanger doors so we can dock"
	/*0x01a2*/ { 0xd4ea, AUDIOCONFIG_01 }, // Carrington: "Stand back Joanna. We'll create your escape route"
	/*0x01a3*/ { VERSION >= VERSION_NTSC_1_0 ? 0x8d24 : 0x9524, AUDIOCONFIG_02 }, // Cass: "Get the hell out of my office"
	/*0x01a4*/ { 0x9d62, AUDIOCONFIG_47 }, // Elvis: "Joanna, what took you so long? Follow me. Let's get off this old tub"
	/*0x01a5*/ { 0x84aa, AUDIOCONFIG_43 },
	/*0x01a6*/ { 0x81cf, AUDIOCONFIG_41 },
	/*0x01a7*/ { 0x858f, AUDIOCONFIG_40 },
	/*0x01a8*/ { 0x80d5, AUDIOCONFIG_39 }, // Lift mechanical sound
	/*0x01a9*/ { 0x80ec, AUDIOCONFIG_39 }, // Lift mechanical sound
	/*0x01aa*/ { 0x80ed, AUDIOCONFIG_40 }, // Lift mechanical sound
	/*0x01ab*/ { 0x80d5, AUDIOCONFIG_41 }, // Lift mechanical sound
	/*0x01ac*/ { 0x80ec, AUDIOCONFIG_41 }, // Lift mechanical sound
	/*0x01ad*/ { 0x80ed, AUDIOCONFIG_42 }, // Lift mechanical sound
	/*0x01ae*/ { 0x80d5, AUDIOCONFIG_41 }, // Lift mechanical sound
	/*0x01af*/ { 0x80ec, AUDIOCONFIG_41 }, // Lift mechanical sound
	/*0x01b0*/ { 0x803a, AUDIOCONFIG_41 }, // Lift call
	/*0x01b1*/ { 0x84e8, AUDIOCONFIG_43 }, // Lift mechanical sound
	/*0x01b2*/ { 0x84e9, AUDIOCONFIG_43 }, // Lift mechanical sound
	/*0x01b3*/ { 0x80cd, AUDIOCONFIG_42 }, // Lift mechanical sound
	/*0x01b4*/ { 0x80ce, AUDIOCONFIG_42 }, // Lift mechanical sound
	/*0x01b5*/ { 0x8039, AUDIOCONFIG_42 }, // Lift mechanical sound
	/*0x01b6*/ { 0x80cd, AUDIOCONFIG_44 }, // Lift mechanical sound
	/*0x01b7*/ { 0x80db, AUDIOCONFIG_16 }, // Lift roller door
	/*0x01b8*/ { 0x80da, AUDIOCONFIG_16 }, // Lift roller door
	/*0x01b9*/ { 0xf41c, AUDIOCONFIG_02 }, // Joanna: "Oh my god"
	/*0x01ba*/ { 0xf434, AUDIOCONFIG_02 }, // Lab guy: "What the hell do you think you're doing? This is supposed..."
	/*0x01bb*/ { 0xf44f, AUDIOCONFIG_02 }, // President: "Damn it man. I say no and I mean no..."
	/*0x01bc*/ { 0x0000, AUDIOCONFIG_00 },
};

struct audioconfig g_AudioConfigs[] = {
	{ /* 0*/  200, 1000, 1200, -1, 100, -1,   0, 0 },
	{ /* 1*/  400, 2500, 3000, -1, 100, 48,   0, AUDIOCONFIGFLAG_20 },
	{ /* 2*/  400, 2500, 3000, -1, 100, -1,   0, AUDIOCONFIGFLAG_OFFENSIVE },
	{ /* 3*/  400, 2500, 3000, -1, 100, 48,   0, AUDIOCONFIGFLAG_OFFENSIVE | AUDIOCONFIGFLAG_20 },
	{ /* 4*/  200, 1000, 1200, -1, 100, -1,   0, 0 },
	{ /* 5*/  200, 1000, 1200, -1, 100, -1,   0, 0 },
	{ /* 6*/  200, 1000, 1200, -1, 100, -1,   0, 0 },
	{ /* 7*/ 1000, 3500, 4000, -1, 100, -1, 120, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /* 8*/ 1000, 2500, 3000, -1, 100, -1,  30, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /* 9*/ 1000, 3500, 4000, -1, 100, -1, 120, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*10*/ 1000, 2500, 3000, -1, 100, -1,  30, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*11*/ 1600, 3000, 3500, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*12*/  400, 1100, 1400, -1, 100, -1, 140, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*13*/  400, 1100, 1400, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*14*/  300, 1200, 1250, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*15*/  500, 1800, 1900, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*16*/ 1000, 2700, 2900, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*17*/ 1500, 3000, 4000, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*18*/ 2500, 4900, 5000, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*19*/ 5500, 5600, 5700, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_20 },
	{ /*20*/   20,  400,  500, -1, 100, -1,   0, 0 },
	{ /*21*/  300, 1700, 2000, -1, 100, -1,   0, 0 },
	{ /*22*/  600, 3500, 5000, -1, 100, -1,   0, 0 },
	{ /*23*/  600, 3500, 5000, -1, 100, -1,   0, 0 },
	{ /*24*/  600, 3500, 5000, -1, 100, -1,   0, 0 },
	{ /*25*/  600, 2500, 3000, -1, 100, -1,   0, 0 },
	{ /*26*/ 1500, 4900, 5500, -1, 100, -1, 100, AUDIOCONFIGFLAG_01 },
	{ /*27*/ 5500, 5740, 5800, -1, 100, -1, 100, AUDIOCONFIGFLAG_01 },
	{ /*28*/  250, 1000, 1300,  2, 100, -1, 120, AUDIOCONFIGFLAG_01 },
	{ /*29*/  200, 2600, 3000, -1, 100, -1,   0, 0 },
	{ /*30*/  200,  750,  850, -1, 100, -1,   0, 0 },
	{ /*31*/  600, 2000, 3000, -1, 100, -1,   0, 0 },
	{ /*32*/  500, 1500, 1800, -1, 100, -1,   0, 0 },
	{ /*33*/  800, 2500, 3000, -1, 100, -1,   0, 0 },
	{ /*34*/  400, 1500, 2000, -1, 100, -1,   0, 0 },
	{ /*35*/ 1000, 3500, 5000, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*36*/   50,  225,  250, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*37*/   50,  225,  250, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*38*/ 2000, 2500, 2700, -1,  50, -1,   0, AUDIOCONFIGFLAG_01 | AUDIOCONFIGFLAG_08 },
	{ /*39*/  300, 1000, 1200, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*40*/  300, 1000, 1200, -1, 100, -1, 100, AUDIOCONFIGFLAG_01 },
	{ /*41*/  600, 1500, 1800, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*42*/  600, 1500, 1800, -1, 100, -1, 100, AUDIOCONFIGFLAG_01 },
	{ /*43*/  800, 2600, 3500, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
	{ /*44*/  800, 2600, 3500, -1, 100, -1, 100, AUDIOCONFIGFLAG_01 },
	{ /*45*/  200,  800, 1000, -1, 100, -1,   0, 0 },
	{ /*46*/  200,  700, 1000, -1, 100, -1,   0, 0 },
	{ /*47*/ 1000, 2500, 3000, -1, 100, -1,   0, 0 },
	{ /*48*/  500, 1400, 1500, -1, 100, -1,   0, 0 },
	{ /*49*/  500, 2000, 2200, -1, 100, -1,   0, 0 },
	{ /*50*/  400, 2500, 3000, -1,  50, -1,   0, 0 },
	{ /*51*/  400, 2500, 3000, -1,  75, -1,   0, 0 },
	{ /*52*/  200,  500,  800, -1, 100, -1, 100, 0 },
	{ /*53*/ 1000, 1100, 1600, -1, 100, -1,   0, 0 },
#if VERSION >= VERSION_NTSC_1_0
	{ /*54*/  300, 1300, 1500, -1, 100, -1,   0, AUDIOCONFIGFLAG_01 },
#endif
	{ /*55*/  300, 1300, 1500, -1, 100, -1, 200, AUDIOCONFIGFLAG_01 },
	{ /*56*/  250,  600,  900, -1, 100, -1, 200, AUDIOCONFIGFLAG_01 },
	{ /*57*/   50,  180,  250, -1, 100, -1, 200, AUDIOCONFIGFLAG_01 },
	{ /*58*/  100,  400,  600, -1, 100, -1, 200, AUDIOCONFIGFLAG_01 },
	{ /*59*/  700, 1100, 1400, -1, 100, -1, 200, AUDIOCONFIGFLAG_01 },
	{ /*60*/  400, 2500, 3000, -1, 100, -1,   0, AUDIOCONFIGFLAG_RESPONDHELLO },
#if VERSION >= VERSION_NTSC_1_0
	{ /*61*/  300,  900, 1100, -1, 100, -1,   0, 0 },
	{ /*62*/ 1000, 2500, 3000, -1, 100, -1,   0, AUDIOCONFIGFLAG_08 },
	{ /*63*/  400, 1000, 1200, -1, 100, -1,   0, 0 },
#endif
};

u32 var8005ecd8 = 0x00000000;
u32 var8005ecdc = 0x00000000;
u32 var8005ece0 = 0x00000000;
u32 var8005ece4 = 0x00000000;
u32 var8005ece8 = 0x00000000;
u32 var8005ecec = 0x00000000;
u32 var8005ecf0 = 0x00000000;
u32 var8005ecf4 = 0x00000000;

s16 var8005ecf8[] = {
	0x6665,
	0x5998,
	0x5fff,
	0x4ccc,
	0x5fff,
	0x2ccc,
	0x5998,
	0x6665,
	0x4ccc,
	0x5998,
	0x5998,
	0x5998,
	0x5fff,
	0x6665,
	0x6665,
	0x5fff,
	0x5998,
	0x6665,
	0x5fff,
	0x5fff,
	0x5fff,
	0x6665,
	0x5fff,
	0x5fff,
	0x5fff,
	0x7332,
	0x5fff,
	0x6665,
	0x5fff,
#if VERSION >= VERSION_NTSC_1_0
	0x5998,
	0x5998,
#else
	0x5fff,
	0x5fff,
#endif
	0x5332,
	0x5332,
	0x5fff,
	0x3332,
#if VERSION >= VERSION_NTSC_1_0
	0x4ccc,
#else
	0x3fff,
#endif
	0x5fff,
	0x4ccc,
	0x3fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x4ccc,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x5fff,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x7332,
	0x7332,
	0x7332,
	0x4ccc,
#if VERSION >= VERSION_NTSC_1_0
	0x3c28,
#else
	0x4ccc,
#endif
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x7332,
	0x7332,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x6665,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x4ccc,
	0x6ccb,
	0x5fff,
	0x5998,
	0x7998,
	0x5998,
	0x5998,
	0x6665,
	0x5fff,
	0x5fff,
	0x7332,
	0x7332,
	0x7332,
	0x7332,
	0x7332,
	0x4ccc,
	0x6665,
	0x6665,
	0x6665,
	0x4ccc,
	0x6ccb,
	-1,
};

extern u8 EXT_SEG _sfxctlSegmentRomStart;
extern u8 EXT_SEG _sfxtblSegmentRomStart;
extern u8 EXT_SEG _seqctlSegmentRomStart;
extern u8 EXT_SEG _seqctlSegmentRomEnd;
extern u8 EXT_SEG _seqtblSegmentRomStart;
extern u8 EXT_SEG _sequencesSegmentRomStart;

bool sndIsPlayingMp3(void)
{
	return g_SndCurMp3.playing;
}

u16 snd0000e9dc(void)
{
#if VERSION >= VERSION_NTSC_1_0
	s32 result;

	if (func00033ec4(0) < 0x5000) {
		result = func00033ec4(0);
	} else {
		result = 0x5000;
	}

	return result;
#else
	return func00033ec4(0);
#endif
}

void sndSetSfxVolume(u16 volume)
{
	u8 i;

#if VERSION >= VERSION_NTSC_1_0
	if (volume > 0x5000) {
		volume = 0x5000;
	}
#endif

	for (i = 0; i < NUM_KEYTHINGS; i++) {
		func00033f44(i, volume);
	}

	g_SfxVolume = volume;
}

void snd0000ea80(u16 volume)
{
	u8 i;

#if VERSION >= VERSION_NTSC_1_0
	if (volume > 0x5000) {
		volume = 0x5000;
	}
#endif

	for (i = 0; i < NUM_KEYTHINGS; i++) {
		func00033f44(i, volume);
	}
}

void sndResetCurMp3(void)
{
	g_SndCurMp3.sfxref.id = 0;
	g_SndCurMp3.sfxref.mp3priority = 0;
	g_SndCurMp3.sfxref.unk02 = 0;
	g_SndCurMp3.sfxref.hasconfig = false;
	g_SndCurMp3.playing = false;
	g_SndCurMp3.unk08 = 0;
	g_SndCurMp3.responsetimer240 = -1;
	g_SndCurMp3.prevwhisper = -1;
	g_SndCurMp3.prevacknowledge = -1;
	g_SndCurMp3.prevgreeting = -1;
}

void sndLoadSfxCtl(void)
{
	s32 i;
	u8 unalignedbuffer[256 + 16];
	u8 *buffer;
	ALBankFile *file;
	ALBank *bank;
	romptr_t romaddr;
	u32 size;

	g_ALSoundRomOffsets = NULL;
	buffer = (u8 *) ALIGN16((uintptr_t)unalignedbuffer);

	// Load the first 256 bytes of the ctl file.
	size = 256;
	dmaExec(buffer, (romptr_t) REF_SEG _sfxctlSegmentRomStart, size);

	// Get the ROM address of the first (and only) bank,
	// then load the first 256 bytes of the bank.
	file = (ALBankFile *) buffer;
	romaddr = (romptr_t) REF_SEG _sfxctlSegmentRomStart;
	romaddr += (uintptr_t)file->bankArray[0];
	dmaExec(buffer, romaddr, size);

	// Get the ROM address of the first (and only) instrument,
	// then load the first 256 bytes of the instrument.
	bank = (ALBank *) buffer;
	romaddr = (romptr_t) REF_SEG _sfxctlSegmentRomStart;
	romaddr += (uintptr_t)bank->instArray[0];
	dmaExec(buffer, romaddr, size);

	// Get the soundCount (spoiler: there's 1545+1).
	// The final one might be a null terminator?
	// Or accounting for 1-based indexing of soundnums.
	g_NumSounds = ((ALInstrument *)buffer)->soundCount + 1;

	// Calculate the size of the ALInstrument and load it. The pointer is then
	// shifted forward to point to the instrument's ALSound array. This leaks
	// some memory but this is initialisation code so it's not much of an issue.
	size = g_NumSounds * sizeof(uintptr_t) + 20;
	size = ALIGN16(size);
	g_ALSoundRomOffsets = alHeapAlloc(&g_SndHeap, 1, size);
	dmaExec(g_ALSoundRomOffsets, romaddr, size);

	*(uintptr_t *)&g_ALSoundRomOffsets += 0x10;

	// Convert ctl-local offsets to ROM offsets
	for (i = 0; i < g_NumSounds; i++) {
		g_ALSoundRomOffsets[i] += (romptr_t) REF_SEG _sfxctlSegmentRomStart;
	}

	// Allocate and initialise cache
	g_SndCache.indexes = alHeapAlloc(&g_SndHeap, sizeof(u16), g_NumSounds);

	for (i = 0; i < (u32)g_NumSounds; i++) {
		g_SndCache.indexes[i] = -1;
	}

	for (i = 0; i < NUM_CACHE_SLOTS; i++) {
#if VERSION >= VERSION_NTSC_1_0
		g_SndCache.ages[i] = 1;
#else
		g_SndCache.ages[i] = g_Vars.updateframe;
#endif

		g_SndCache.refcounts[i] = 0;
	}
}

#if VERSION >= VERSION_NTSC_1_0
void sndIncrementAges(void)
{
	s32 i;

	for (i = 0; i < NUM_CACHE_SLOTS; i++) {
		if (g_SndCache.refcounts[i] == 0 && g_SndCache.ages[i] < 32000) {
			g_SndCache.ages[i]++;
		}
	}
}
#endif

ALEnvelope *sndLoadEnvelope(uintptr_t offset, u16 cacheindex)
{
#if VERSION >= VERSION_NTSC_1_0
	u8 spaf[0x90];
	u8 sp5f[0x90];
	ALEnvelope *s2 = (ALEnvelope *)ALIGN16((uintptr_t)spaf);
	ALEnvelope *s1 = (ALEnvelope *)ALIGN16((uintptr_t)sp5f);
	s32 i;
	s32 sum1;
	s32 sum2;

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	do {
		dmaExecHighPriority(s2, offset, 16 * sizeof(uintptr_t));
		sum1 = 0;

		for (i = 0; i < 16U; i++) {
			sum1 += ((u32 *)s2)[i];
		}

		dmaExecHighPriority(s1, offset, 16 * sizeof(uintptr_t));
		sum2 = 0;

		for (i = 0; i < 16U; i++) {
			sum2 += ((u32 *)s1)[i];
		}

		if (1);
	} while (sum1 != sum2);
#else
	u8 sp5f[0x50];
	ALEnvelope *s1 = (ALEnvelope *)ALIGN16((uintptr_t)sp5f);

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	dmaExecHighPriority(s1, offset, 0x40);
#endif

	g_SndCache.envelopes[cacheindex] = *s1;

	s1 = g_SndCache.envelopes;
	s1 += cacheindex;

	return s1;
}

ALKeyMap *sndLoadKeymap(uintptr_t offset, u16 cacheindex)
{
#if VERSION >= VERSION_NTSC_1_0
	u8 spaf[0x90];
	u8 sp5f[0x90];
	ALKeyMap *s2 = (ALKeyMap *)ALIGN16((uintptr_t)spaf);
	ALKeyMap *s1 = (ALKeyMap *)ALIGN16((uintptr_t)sp5f);
	s32 i;
	s32 sum1;
	s32 sum2;

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	do {
		dmaExecHighPriority(s2, offset, 16 * sizeof(uintptr_t));
		sum1 = 0;

		for (i = 0; i < 16U; i++) {
			sum1 += ((u32 *)s2)[i];
		}

		dmaExecHighPriority(s1, offset, 16 * sizeof(uintptr_t));
		sum2 = 0;

		for (i = 0; i < 16U; i++) {
			sum2 += ((u32 *)s1)[i];
		}

		if (1);
	} while (sum1 != sum2);
#else
	u8 sp5f[0x50];
	ALKeyMap *s1 = (ALKeyMap *)ALIGN16((uintptr_t)sp5f);

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	dmaExecHighPriority(s1, offset, 0x40);
#endif

	g_SndCache.keymaps[cacheindex] = *s1;

	s1 = g_SndCache.keymaps;
	s1 += cacheindex;

	return s1;
}

ALADPCMBook *sndLoadAdpcmBook(uintptr_t offset, u16 cacheindex)
{
#if VERSION >= VERSION_NTSC_1_0
	u8 spaf[0x150];
	u8 sp5f[0x150];
	ALADPCMBook *s2 = (ALADPCMBook *)ALIGN16((uintptr_t)spaf);
	ALADPCMBook *s1 = (ALADPCMBook *)ALIGN16((uintptr_t)sp5f);
	s32 i;
	s32 sum1;
	s32 sum2;

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	do {
		dmaExecHighPriority(s2, offset, 0x140);
		sum1 = 0;

		for (i = 0; i < 80U; i++) {
			sum1 += ((u32 *)s2)[i];
		}

		dmaExecHighPriority(s1, offset, 0x140);
		sum2 = 0;

		for (i = 0; i < 80U; i++) {
			sum2 += ((u32 *)s1)[i];
		}

		if (1);
	} while (sum1 != sum2);
#else
	u8 sp5f[0x150];
	ALADPCMBook *s1 = (ALADPCMBook *)ALIGN16((uintptr_t)sp5f);

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	dmaExecHighPriority(s1, offset, 0x140);
#endif

	g_SndCache.books[cacheindex] = *s1;

	s1 = g_SndCache.books;
	s1 += cacheindex;

	return s1;
}

ALADPCMloop *sndLoadAdpcmLoop(uintptr_t offset, u16 cacheindex)
{
#if VERSION >= VERSION_NTSC_1_0
	u8 spaf[0x90];
	u8 sp5f[0x90];
	ALADPCMloop *s2 = (ALADPCMloop *)ALIGN16((uintptr_t)spaf);
	ALADPCMloop *s1 = (ALADPCMloop *)ALIGN16((uintptr_t)sp5f);
	s32 i;
	s32 sum1;
	s32 sum2;

	if (offset == 0) {
		return NULL;
	}

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	do {
		dmaExecHighPriority(s2, offset, 16 * sizeof(uintptr_t));
		sum1 = 0;

		for (i = 0; i < 16U; i++) {
			sum1 += ((u32 *)s2)[i];
		}

		dmaExecHighPriority(s1, offset, 16 * sizeof(uintptr_t));
		sum2 = 0;

		for (i = 0; i < 16U; i++) {
			sum2 += ((u32 *)s1)[i];
		}

		if (1);
	} while (sum1 != sum2);
#else
	u8 sp5f[0x50];
	ALADPCMloop *s1 = (ALADPCMloop *)ALIGN16((uintptr_t)sp5f);

	if (offset == 0) {
		return NULL;
	}

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	dmaExecHighPriority(s1, offset, 0x40);
#endif

	g_SndCache.loops[cacheindex] = *s1;

	s1 = g_SndCache.loops;
	s1 += cacheindex;

	return s1;
}

ALWaveTable *sndLoadWavetable(uintptr_t offset, u16 cacheindex)
{
#if VERSION >= VERSION_NTSC_1_0
	u8 spaf[0x90];
	u8 sp5f[0x90];
	ALWaveTable *s2 = (ALWaveTable *)ALIGN16((uintptr_t)spaf);
	ALWaveTable *s1 = (ALWaveTable *)ALIGN16((uintptr_t)sp5f);
	s32 i;
	s32 sum1;
	s32 sum2;
	ALWaveTable *tmp;

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	do {
		dmaExecHighPriority(s2, offset, 16 * sizeof(uintptr_t));
		sum1 = 0;

		for (i = 0; i < 16U; i++) {
			sum1 += ((u32 *)s2)[i];
		}

		dmaExecHighPriority(s1, offset, 16 * sizeof(uintptr_t));
		sum2 = 0;

		for (i = 0; i < 16U; i++) {
			sum2 += ((u32 *)s1)[i];
		}

		if (1);
	} while (sum1 != sum2);
#else
	u8 sp5f[0x50];
	ALWaveTable *s1 = (ALWaveTable *)ALIGN16((uintptr_t)sp5f);
	ALWaveTable *tmp;

	offset += (romptr_t) REF_SEG _sfxctlSegmentRomStart;

	dmaExecHighPriority(s1, offset, 0x40);
#endif

	tmp = &g_SndCache.wavetables[cacheindex];

	*tmp = *s1;

	tmp->base += (romptr_t) REF_SEG _sfxtblSegmentRomStart;

	if (tmp->type == AL_ADPCM_WAVE) {
		tmp->waveInfo.adpcmWave.book = sndLoadAdpcmBook((uintptr_t)tmp->waveInfo.adpcmWave.book, cacheindex);
		tmp->waveInfo.adpcmWave.loop = sndLoadAdpcmLoop((uintptr_t)tmp->waveInfo.adpcmWave.loop, cacheindex);
	}

	return tmp;
}

void sndSetSoundMode(s32 mode)
{
	s32 i;

	g_SoundMode = mode;

	switch (mode) {
	case SOUNDMODE_MONO:
		speakersSetMode(SPEAKERMODE_MONO);
		break;
	case SOUNDMODE_STEREO:
		speakersSetMode(SPEAKERMODE_STEREO);
		break;
	case SOUNDMODE_HEADPHONE:
		speakersSetMode(SPEAKERMODE_HEADPHONE);
		break;
	case SOUNDMODE_SURROUND:
		speakersSetMode(SPEAKERMODE_SURROUND);
		break;
	}

	speaker00034104(0, 4);

	for (i = 1; i < g_SndMaxFxBusses; i++) {
		switch (mode) {
		case SOUNDMODE_STEREO:
		case SOUNDMODE_HEADPHONE:
		case SOUNDMODE_SURROUND:
			speaker00034104(i, 4);
			break;
		}
	}
}

ALSound *sndLoadSound(s16 soundnum)
{
	union soundnumhack tmp;
	u16 cacheindex;
	u16 *ptr;
	ALSound *sound;
	s32 oldestindex;
	u32 oldestage;
	s32 i;
	s16 sfxnum;
	u8 sp47[0x58];

	sound = (ALSound *)ALIGN16((uintptr_t)sp47);

	tmp.packed = soundnum;
	sfxnum = tmp.id;

	if (sndIsMp3(tmp.packed)) {
		return NULL;
	}

	// If this sound doesn't exist in the cache
	cacheindex = g_SndCache.indexes[sfxnum];

	if (cacheindex == 0xffff) {
		// Find the oldest cache item
		oldestindex = -1;
		oldestage = 0;

		for (i = 0; i < NUM_CACHE_SLOTS; i++) {
			if (g_SndCache.refcounts[i] == 0) {
#if VERSION >= VERSION_NTSC_1_0
				if (g_SndCache.ages[i] > oldestage) {
					oldestage = g_SndCache.ages[i];
					oldestindex = i;
				}
#else
				s32 age = g_Vars.updateframe - g_SndCache.ages[i] + 1;

				if (age > oldestage) {
					oldestage = age;
					oldestindex = i;
				}
#endif
			}
		}

		cacheindex = oldestindex;

		// Remove the old sound from the cacheindexes list
		ptr = g_SndCache.indexes;

		for (i = 0; i < (u32)g_NumSounds; i++) {
			if (*ptr == cacheindex) {
				*ptr = 0xffff;
			}

			ptr++;
		}

		// DMA the ALSound data
		dmaExecHighPriority(sound, g_ALSoundRomOffsets[sfxnum - 1], 0x40);

		// Promote segment offsets to pointers and load their child data
		sound->envelope = sndLoadEnvelope((uintptr_t)sound->envelope, cacheindex);
		sound->wavetable = sndLoadWavetable((uintptr_t)sound->wavetable, cacheindex);
		sound->keyMap = sndLoadKeymap((uintptr_t)sound->keyMap, cacheindex);

		// Save the ALSound into the cache
		g_SndCache.sounds[cacheindex] = *sound;

		g_SndCache.indexes[sfxnum] = cacheindex;
	}

	// Reset this cache item's age, even if it already existed in the cache
#if VERSION >= VERSION_NTSC_1_0
	g_SndCache.ages[cacheindex] = 1;
#else
	g_SndCache.ages[cacheindex] = g_Vars.updateframe;
#endif

	return &g_SndCache.sounds[cacheindex];
}

void seqInit(struct seqinstance *seq)
{
	u32 stack;
	ALSeqpConfig config;

	config.maxVoices = 44;
	config.maxEvents = 64;
	config.maxChannels = 16;
	config.debugFlags = 0;
	config.heap = &g_SndHeap;

	func00030c98(&config);

	if (IS4MB()) {
		g_SeqBufferSize = MAX_SEQ_SIZE_4MB;
	} else {
		g_SeqBufferSize = MAX_SEQ_SIZE_8MB;
	}

	seq->data = alHeapAlloc(&g_SndHeap, 1, g_SeqBufferSize);
	seq->seqp = alHeapAlloc(&g_SndHeap, 1, sizeof(N_ALCSPlayer));

	n_alCSPNew(seq->seqp, &config);

	n_alCSPSetBank(seq->seqp, var80095204);
}

void sndAddRef(ALSound *sound)
{
	if (sound >= &g_SndCache.sounds[0] && sound <= &g_SndCache.sounds[NUM_CACHE_SLOTS - 1]) {
		s32 cacheindex = sound - g_SndCache.sounds;
		g_SndCache.refcounts[cacheindex]++;
	}
}

void sndRemoveRef(ALSound *sound)
{
	if (sound >= &g_SndCache.sounds[0] && sound <= &g_SndCache.sounds[NUM_CACHE_SLOTS - 1]) {
		s32 cacheindex = sound - g_SndCache.sounds;
		g_SndCache.refcounts[cacheindex]--;
	}
}

void sndInit(void)
{
	ALSndpConfig sndpconfig;
	ALSynConfig synconfig;
#if VERSION >= VERSION_PAL_BETA
	u32 settings[3];
#endif

#if VERSION >= VERSION_JPN_FINAL
	u32 heaplen = 1024 * 528;
#elif VERSION >= VERSION_PAL_BETA
	u32 heaplen = 1024 * 533;
#elif VERSION >= VERSION_NTSC_1_0
	u32 heaplen = 1024 * 528;
#else
	u32 heaplen = 1024 * 525;
#endif

#ifdef PLATFORM_64BIT
	heaplen = 1024 * 745;
#endif

	g_Vars.langfilteron = false;

	if (IS4MB()) {
		g_SndMaxFxBusses = 1;

		heaplen -= 1024 * (PAL ? 6 : 38);
		heaplen -= 1024 * 137;
		heaplen -= 1024 * 12;
		heaplen -= 1024 * 23;

		g_SndMp3Enabled = false;
	} else {
		g_SndMp3Enabled = true;
		g_SndMaxFxBusses = 2;

		if (argFindByPrefix(1, "-nomp3")) {
			g_SndMp3Enabled = false;
		}
	}

	if (!g_SndDisabled) {
		// Allocate memory for the audio heap,
		// clear it and give it to the audio library
		u32 len = REF_SEG _seqctlSegmentRomEnd - REF_SEG _seqctlSegmentRomStart;
		u8 *ptr = mempAlloc(heaplen, MEMPOOL_PERMANENT);
		s32 i;
		u8 *heapstart = ptr;
		u8 *end = heapstart + heaplen;
		ALBankFile *bankfile;

		while (ptr < end) {
			*ptr = 0;
			ptr++;
		}

		alHeapInit(&g_SndHeap, heapstart, heaplen);

		// Allocate some space at the start of the heap for a string identifier.
		// This might be used to determine if the heap has overflowed.
		g_SndGuardStringPtr = alHeapAlloc(&g_SndHeap, 1, ALIGN16(sizeof(g_SndGuardString)));
		strcpy(g_SndGuardStringPtr, g_SndGuardString);

		// Load sfx.ctl
		sndLoadSfxCtl();

		// Load seq.ctl
		var80095200 = 0xffffffff;
		bankfile = alHeapAlloc(&g_SndHeap, 1, len);
		dmaExec(bankfile, (romptr_t) REF_SEG _seqctlSegmentRomStart, len);

		// Load seq.tbl
		alBnkfNew(bankfile, REF_SEG _seqtblSegmentRomStart);

		// Load the sequences table. To do this, load the header of the
		// sequences segment and read the number of sequences, then allocate
		// enough space for the table and load it.
		var80095204 = bankfile->bankArray[0];
		g_SeqTable = alHeapDBAlloc(0, 0, &g_SndHeap, 1, 0x10);
		dmaExec(g_SeqTable, (romptr_t) REF_SEG _sequencesSegmentRomStart, 0x10);

		len = g_SeqTable->count * sizeof(struct seqtableentry) + 4;
		g_SeqTable = alHeapDBAlloc(0, 0, &g_SndHeap, 1, len);
		dmaExec(g_SeqTable, (romptr_t) REF_SEG _sequencesSegmentRomStart, (len + 0xf) & ~0xf);

		// Promote segment-relative offsets to ROM addresses
		g_SeqRomAddrs = mempAlloc(g_SeqTable->count * sizeof(uintptr_t), MEMPOOL_PERMANENT);
		
		for (i = 0; i < g_SeqTable->count; i++) {
			g_SeqRomAddrs[i] = g_SeqTable->entries[i].romaddr + (romptr_t) REF_SEG _sequencesSegmentRomStart;
		}

		synconfig.maxVVoices = 44;
		synconfig.maxPVoices = 30;
		synconfig.maxUpdates = 64;
		synconfig.dmaproc = NULL;
		synconfig.outputRate = 0;
		synconfig.heap = &g_SndHeap;
		synconfig.maxFXbusses = g_SndMaxFxBusses;

		for (i = 0; i < g_SndMaxFxBusses; i++) {
			synconfig.fxTypes[i] = 6;
		}

		sndpconfig.maxEvents = 64;
		sndpconfig.maxStates = 64;
		sndpconfig.maxSounds = 20;
		sndpconfig.unk10 = NUM_KEYTHINGS;
		sndpconfig.heap = &g_SndHeap;

#if VERSION >= VERSION_PAL_BETA
		settings[0] = 22020;
		settings[1] = 1;
		settings[2] = 2000;

		amgrCreate(&synconfig, settings);
#else
		amgrCreate(&synconfig);
#endif

		if (g_SndMp3Enabled) {
			osSyncPrintf("RWI : Initialising the new and improved MP3 player\n");

			mp3Init(&g_SndHeap);
			func00037f08(0x7fff, 1);
			func00037f5c(0, true);

			osSyncPrintf("RWI : MP3 player Initialising Done\n");
		}

		for (i = 0; i < ARRAYCOUNT(g_SeqInstances); i++) {
			seqInit(&g_SeqInstances[i]);
		}

		osSyncPrintf("gsSndpNew\n");

		n_alSndpNew(&sndpconfig);

		osSyncPrintf("Set the sample callbacks\n");

		sndpSetAddRefCallback(sndAddRef);
		sndpSetRemoveRefCallback(sndRemoveRef);

		amgrStartThread();

		sndSetSoundMode(g_SoundMode);
	}
}

bool sndIsMp3(s16 soundnum)
{
	union soundnumhack tmp;
	tmp.packed = soundnum;

	return tmp.mp3priority != 0;
}

bool sndStopMp3(s16 arg0)
{
	if (!g_SndDisabled && g_SndMp3Enabled) {
		if (func00037ea4() && g_SndCurMp3.unk08 != 0) {
			return false;
		}

		func00037e1c();

		g_SndCurMp3.playing = false;
		g_SndCurMp3.responsetimer240 = -1;
	}

	return true;
}

void snd0000fc40(s32 arg0)
{
	// empty
}

bool seqPlay(struct seqinstance *seq, s32 tracknum)
{
	u32 stack;
	s32 binlen;
	u8 *binstart;
	u8 *zipstart;
	s32 ziplen;
	u8 scratch[1024 * 5];

	s32 state = n_alCSPGetState(seq->seqp);

	if (g_SndDisabled) {
		return false;
	}

	seq->tracknum = tracknum;

	if (g_SeqTable && tracknum);

	if (state != AL_STOPPED) {
		return false;
	}

	if (g_SeqRomAddrs[seq->tracknum] < 0x10000) {
		return false;
	}

#ifndef PLATFORM_N64
	// try to load external replacement, which can be either compressed or not
	u32 extlen = 0;
	u8 *extseq = modSequenceLoad(seq->tracknum, &extlen);
	if (extseq) {
		if (extlen > 2 && rzipIs1173(extseq)) {
			// sequence is compressed; uncompress
			binlen = ((u32)extseq[2] << 16) | ((u32)extseq[3] << 8) | (u32)extseq[4];
			binlen = ALIGN16(binlen) + 0x40;
			if (binlen >= g_SeqBufferSize) {
				return false;
			}
			ziplen = ALIGN16(extlen);
			binstart = seq->data;
			zipstart = binstart + binlen - ziplen;
			ziplen = rzipInflate(zipstart, binstart, scratch);
		} else {
			// sequence is uncompressed; just load as is
			binlen = ALIGN16(extlen) + 0x40;
			if (binlen >= g_SeqBufferSize) {
				return false;
			}
			ziplen = extlen;
			binstart = seq->data;
			zipstart = NULL;
			dmaExec(binstart, (romptr_t)extseq, extlen);
		}
		sysMemFree(extseq);
	} else
#endif
	{
		binlen = ALIGN16(g_SeqTable->entries[seq->tracknum].binlen) + 0x40;

		if (binlen >= g_SeqBufferSize) {
			return false;
		}

		ziplen = ALIGN16(g_SeqTable->entries[seq->tracknum].ziplen);
#if VERSION < VERSION_NTSC_1_0
		if (seq->data);
#endif

		binstart = seq->data;
		zipstart = binstart + binlen - ziplen;
	
		dmaExec(zipstart, g_SeqRomAddrs[seq->tracknum], ziplen);
		ziplen = rzipInflate(zipstart, binstart, scratch);
	}

#ifndef PLATFORM_N64
	preprocessALCMidiHdr(binstart, ziplen, NULL);
#endif

#if VERSION < VERSION_NTSC_1_0
	if (ziplen == 0) {
		char message[128];
		sprintf(message, "DMA-Crash %s %d Ram: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				"snd.c", 1676,
				zipstart[0], zipstart[1], zipstart[2], zipstart[3],
				zipstart[4], zipstart[5], zipstart[6], zipstart[7],
				zipstart[8], zipstart[9], zipstart[10], zipstart[11],
				zipstart[12], zipstart[13], zipstart[14], zipstart[15]);
		crashSetMessage(message);
		CRASH();
	}
#endif

#if AVOID_UB
	// To avoid undefined behaviour, we must change the sequence player's state
	// from AL_STOPPED to something else. Otherwise a race condition can occur
	// where the same sequence player is used for two sequences if the audio
	// thread hasn't run between the two calls and updated its state.
	seq->seqp->state = AL_STARTING;
#endif

	n_alCSeqNew(&seq->seq, seq->data);
	n_alCSPSetSeq(seq->seqp, &seq->seq);
	seqSetVolume(seq, seqGetVolume(seq));
	n_alCSPPlay(seq->seqp);

	return true;
}

u16 seqGetVolume(struct seqinstance *seq)
{
	return g_SndDisabled ? AL_VOL_FULL : seq->volume;
}

void seqSetVolume(struct seqinstance *seq, u16 volume)
{
	if (!g_SndDisabled) {
		u32 tmp = var8005ecf8[seq->tracknum] * volume;
		tmp >>=	15;

		seq->volume = volume;

		if (tmp > AL_VOL_FULL) {
			tmp = AL_VOL_FULL;
		}

		n_alCSPSetVol(seq->seqp, tmp);
	}
}

void sndHandleRetrace(void)
{
	// empty
}

void snd0000fe20(void)
{
	if (g_SndMp3Enabled) {
		func00037e38();
	}
}

void snd0000fe50(void)
{
	if (g_SndMp3Enabled) {
		func00037e68();
	}
}

void snd0000fe80(void)
{
	// empty
}

void sndTick(void)
{
#if VERSION >= VERSION_NTSC_1_0
	struct sndstate *stateptrs[64];
	struct sndstate states[64];
	s32 i;
	s32 curtime;
	struct sndstate *state;
#endif
	OSPri prevpri;
	s32 s0;
	union soundnumhack sp50;
	s32 index;
	s32 stack;

#if VERSION >= VERSION_NTSC_1_0
	static s32 g_SndMostEverPlaying2 = -1;

	sndIncrementAges();

	prevpri = osGetThreadPri(NULL);
	osSetThreadPri(0, osGetThreadPri(&g_AudioManager.thread) + 1);

	curtime = sndpGetCurTime();
	state = sndpGetHeadState();

	g_SndNumPlaying = 0;
	i = 0;

	while (state) {
		stateptrs[i] = state;
		states[i] = *state;

		g_SndNumPlaying++;

		if ((state->flags & SNDSTATEFLAG_02) == 0
				&& stateptrs[i]->state == AL_PLAYING
				&& stateptrs[i]->unk48 > 0
				&& stateptrs[i]->unk48 < curtime) {
			state->unk48 = 0;
			func00033db0();
			break;
		}

		state = (struct sndstate *)state->node.next;
		i++;
	}

	osSetThreadPri(0, prevpri);

	if (g_SndNumPlaying > g_SndMostEverPlaying) {
		g_SndMostEverPlaying = g_SndNumPlaying;
	}

	if (g_SndMostEverPlaying != g_SndMostEverPlaying2) {
		g_SndMostEverPlaying2 = g_SndMostEverPlaying;
	}
#endif

	if (!g_SndDisabled && g_SndMp3Enabled) {
		if (g_Vars.stagenum == STAGE_AIRFORCEONE) {
			sndTickNosedive();
		} else if (g_Vars.stagenum == STAGE_ESCAPE) {
			sndTickUfo();
		}

		if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
			s0 = musicGetVolume() > g_SfxVolume ? musicGetVolume() : g_SfxVolume;

			if (s0 != snd0000e9dc()) {
				snd0000ea80(s0);
			}
		} else {
			if (g_SfxVolume != snd0000e9dc()) {
				snd0000ea80(g_SfxVolume);
			}
		}

		if (g_SndGuardStringPtr != NULL) {
			if (strcmp(g_SndGuardStringPtr, g_SndGuardString) != 0) {
#if VERSION < VERSION_NTSC_1_0
				crashSetMessage("Snd Heap Check FAILED");
				CRASH();
#endif
			}
		}

		if (func00037ea4() == 0 && g_SndCurMp3.playing) {
			if (g_SndCurMp3.unk08) {
				mp3PlayFile(g_SndCurMp3.romaddr, g_SndCurMp3.romsize);
				return;
			}

			switch (g_SndCurMp3.responsetype) {
			case MP3RESPONSETYPE_ACKNOWLEDGE:
			case MP3RESPONSETYPE_WHISPER:
				g_SndCurMp3.responsetimer240 = TICKS(60);
				break;
			case MP3RESPONSETYPE_GREETING:
				g_SndCurMp3.responsetimer240 = 1;
				break;
			}

			g_SndCurMp3.playing = false;
			return;
		}

		if (g_SndCurMp3.responsetimer240 > 0) {
			g_SndCurMp3.responsetimer240 -= g_Vars.lvupdate240;

			if (g_SndCurMp3.responsetimer240 <= 0) {
				if (g_SndCurMp3.responsetype == MP3RESPONSETYPE_WHISPER) {
					do {
						index = random() % 4;
					} while (index == g_SndCurMp3.prevwhisper);

					g_SndCurMp3.prevwhisper = index;

					switch (index) {
					case 0: sp50.packed = MP3_JO_WHISPER_RECEIVED; break;
					case 1: sp50.packed = MP3_JO_WHISPER_UNDERSTOOD; break;
					case 2: sp50.packed = MP3_JO_WHISPER_AFFIRMATIVE; break;
					case 3: sp50.packed = MP3_JO_WHISPER_CONFIRMED; break;
					}
				} else if (g_SndCurMp3.responsetype == MP3RESPONSETYPE_ACKNOWLEDGE) {
					do {
						index = random() % 4;
					} while (index == g_SndCurMp3.prevacknowledge);

					g_SndCurMp3.prevacknowledge = index;

					switch (index) {
					case 0: sp50.packed = MP3_JO_ACKNOWLEDGE_CONFIRMED; break;
					case 1: sp50.packed = MP3_JO_ACKNOWLEDGE_UNDERSTOOD; break;
					case 2: sp50.packed = MP3_JO_ACKNOWLEDGE_AFFIRMATIVE; break;
					case 3: sp50.packed = MP3_JO_ACKNOWLEDGE_RECEIVED; break;
					}
				} else if (g_SndCurMp3.responsetype == MP3RESPONSETYPE_GREETING) {
					do {
						index = random() % 4;
					} while (index == g_SndCurMp3.prevgreeting);

					g_SndCurMp3.prevgreeting = index;

					switch (index) {
					case 0: sp50.packed = MP3_JO_GREETING_HELLO; break;
					case 1: sp50.packed = MP3_JO_GREETING_HI; break;
					case 2: sp50.packed = MP3_JO_GREETING_HEY; break;
					case 3: sp50.packed = MP3_JO_GREETING_HIYA; break;
					}
				}

				g_SndCurMp3.responsetimer240 = -1;
				sndStart(0, sp50.packed, 0, -1, -1, -1.0f, -1, -1);
			}
		}
	}
}

s16 snd0001034c(s16 sfxnum)
{
	union soundnumhack sfxref;

	sfxref.packed = sfxnum;
	sfxnum = sfxref.id;

	if (sfxref.id);

	return sfxnum;
}

bool sndIsDisabled(void)
{
	return g_SndDisabled;
}

void sndStartMp3ByFilenum(u32 filenum)
{
	union soundnumhack sfxref;

	if (!g_SndDisabled && g_SndMp3Enabled) {
		sfxref.packed = 0;
		sfxref.unk02 = 0;
		sfxref.mp3priority = 1; // high priority
		sfxref.id = filenum;

		sndStart(0, sfxref.packed, NULL, -1, -1, -1, -1, -1);
	}
}

/**
 * Return true if the player has the language filter enabled
 * and the given audio ID is one that should be filtered out.
 */
bool sndIsFiltered(s32 audio_id)
{
	if (g_Vars.langfilteron) {
		union soundnumhack sfxref;
		sfxref.packed = audio_id;

		if (sfxref.hasconfig) {
			if (g_AudioConfigs[g_AudioRussMappings[sfxref.confignum].audioconfig_index].flags & AUDIOCONFIGFLAG_OFFENSIVE) {
				return true;
			}
		}
#if VERSION >= VERSION_NTSC_1_0
		else {
			// @bug: The masking here makes it impossible to match hangar guy's
			// audio ID, so his phrase can be said even with the lang filter on.
			// The switch value resolves to sfxref & 0x7ff, while hangar guy's
			// reference is 0x17ad.
			switch (sfxref.id) {
			case SFX_ELVIS_KISS_MY_ALIEN_BUTT:
			case SFX_ELVIS_ILL_KICK_YOUR_ASS:
			case MP3_HANGERGUY_DONT_SCREW_UP:
				return true;
			}
		}
#endif
	}

	return false;
}

void sndAdjust(struct sndstate **handle, bool ismp3, s32 vol, s32 pan, s32 soundnum, f32 pitch, s32 fxbus, s32 fxmixarg, bool forcefxmix)
{
	s32 fxmix = -1;
	union soundnumhack sp20;
	union soundnumhack sp1c;
	struct audioconfig *config;

	if (forcefxmix || fxmixarg != -1) {
		if (fxmixarg != -1) {
			fxmix = fxmixarg;
		} else {
			fxmix = 0;
		}

#if VERSION >= VERSION_NTSC_1_0
		if (pan != -1 && g_SoundMode == SOUNDMODE_SURROUND && (pan & 0x80)) {
			fxmix += 128;
		}
#else
		if (g_SoundMode == SOUNDMODE_SURROUND && (pan & 0x80)) {
			fxmix += 128;
		}
#endif
	}

	if (soundnum > 0) {
		sp20.packed = soundnum;

		if (sp20.hasconfig) {
			s32 index = g_AudioRussMappings[sp20.confignum].audioconfig_index;

			sp1c.packed = g_AudioRussMappings[sp20.confignum].soundnum;
			sp1c.hasconfig = false;

			soundnum = sp1c.packed;

			config = &g_AudioConfigs[index];

			if (config->volpercentage != 100) {
				vol = vol * config->volpercentage / 100;
			}

			if (config->pan != -1) {
				pan = config->pan;
			}

			if (g_Vars.langfilteron && (config->flags & AUDIOCONFIGFLAG_OFFENSIVE)) {
				vol = 0;
			}
		}
	}

	if (ismp3) {
		if (vol != -1) {
			vol = vol * snd0000e9dc() / AL_VOL_FULL;
			func00037f08(vol, true);
		}

		if (pan != -1) {
			func00037f5c(pan, true);
		}
	}

	if (*handle != NULL) {
		if (vol != -1) {
			audioPostEvent(*handle, AL_SNDP_VOL_EVT, vol);
		}

		if (pan != -1) {
			audioPostEvent(*handle, AL_SNDP_PAN_EVT, pan & 0x7f);
		}

		if (pitch != -1.0f) {
			audioPostEvent(*handle, AL_SNDP_PITCH_EVT, *(s32 *)&pitch);
		}

		if (fxmix != -1) {
			audioPostEvent(*handle, AL_SNDP_FX_EVT, fxmix);
		}
	}
}

struct sndstate *snd00010718(struct sndstate **handle, s32 flags, s32 volume, s32 pan, s32 soundnum, f32 pitch, s32 fxbus, s32 fxmixarg, bool forcefxmix)
{
	OSPri prevpri = osGetThreadPri(NULL);
	s32 fxmix = -1;
	struct sndstate *state;
	union soundnumhack sp30;
	union soundnumhack sp2c;
	struct audioconfig *config;

	osSetThreadPri(0, osGetThreadPri(&g_AudioManager.thread) + 1);

	if (forcefxmix || fxmixarg != -1) {
		if (fxmixarg != -1) {
			fxmix = fxmixarg;
		} else {
			fxmix = 0;
		}

#if VERSION >= VERSION_NTSC_1_0
		if (pan != -1 && g_SoundMode == SOUNDMODE_SURROUND && (pan & 0x80)) {
			fxmix += 128;
		}
#else
		if (g_SoundMode == SOUNDMODE_SURROUND && (pan & 0x80)) {
			fxmix += 128;
		}
#endif
	}

	if (soundnum) {
		sp30.packed = soundnum;

		if (sp30.hasconfig) {
			s32 index = g_AudioRussMappings[sp30.confignum].audioconfig_index;

			sp2c.packed = g_AudioRussMappings[sp30.confignum].soundnum;
			sp2c.hasconfig = false;

			soundnum = sp2c.packed;

			config = &g_AudioConfigs[index];

			if (config->volpercentage != 100) {
				volume = volume * config->volpercentage / 100;
			}

			if (config->pan != -1) {
				pan = config->pan;
			}

			if (g_Vars.langfilteron && (config->flags & AUDIOCONFIGFLAG_OFFENSIVE)) {
				volume = 0;
			}
		}
	}

	state = sndStart(var80095200, soundnum, handle, volume, pan, pitch, fxbus, fxmix);

	osSetThreadPri(0, prevpri);

	return state;
}

struct sndstate *sndStart(s32 arg0, s16 sound, struct sndstate **handle, s32 volumearg, s32 panarg, f32 pitcharg, s32 fxbusarg, s32 fxmixarg)
{
	union soundnumhack sp44;
	union soundnumhack sp40;
	u8 fxmix;
	u8 fxbus;
	u8 pan;
	u16 volume;
	f32 pitch;

	fxmix = fxmixarg != -1 ? fxmixarg : 0;
	fxbus = fxbusarg != -1 ? fxbusarg : 1;
	pan = panarg != -1 ? panarg : AL_PAN_CENTER;
	volume = volumearg != -1 ? volumearg : AL_VOL_FULL;
	pitch = pitcharg > 0.0f ? pitcharg : 1.0f;

	sp44.packed = sound;

	if (g_SndDisabled) {
		return NULL;
	}

	sp40.packed = sp44.hasconfig ? g_AudioRussMappings[sp44.confignum].soundnum : sp44.packed;

	if (sp40.id == SFX_0037 || sp40.id == SFX_0009) {
		return NULL;
	}

	if (sndIsMp3(sp40.packed)) {
		sndStartMp3(sp40.packed, volume, pan, 0);

		if (handle != NULL) {
			*handle = NULL;
		}

		return NULL;
	}

#if VERSION >= VERSION_NTSC_1_0
	if (sp40.id < (u32)g_NumSounds) {
		return func00033820(arg0, sp40.id, volume, pan & 0x7f, pitch, fxmix, IS4MB() ? 0 : fxbus, handle);
	}

	return NULL;
#else
	return func00033820(arg0, sp40.id, volume, pan & 0x7f, pitch, fxmix, IS4MB() ? 0 : fxbus, handle);
#endif
}

const char var70053be0[] = "Snd_Play_Universal : Overriding -> Link = %d\n";

#if VERSION < VERSION_NTSC_1_0
const char var700552f0nb[] = "Snd_Play_Mpeg : sndId=%d, vol=%d, pan=%d\n";
#endif

const char var70053c10[] = "Snd_Play_Mpeg : SYSTEM IS DISABLED\n";
const char var70053c34[] = "Snd_Play_Mpeg  : Lib called -> Adr=%x\n";
const char var70053c5c[] = "Snd_Play_Mpeg  : Chunk size -> Adr=%x\n";

void sndStartMp3(s16 soundnum, s32 volume, s32 pan, s32 responseflags)
{
	union soundnumhack sp24;
	union soundnumhack sp20;

	sp24.packed = soundnum;

	if (!g_SndDisabled) {
		if (sp24.hasconfig) {
			sp20.packed = g_AudioRussMappings[sp24.confignum].soundnum;
			sp20.hasconfig = false;
		} else {
			sp20.packed = soundnum;
		}

		if (!g_SndCurMp3.playing
				|| ((sp20.mp3priority != 1 || g_SndCurMp3.sfxref.mp3priority != 1)
				 && sp20.mp3priority <= g_SndCurMp3.sfxref.mp3priority)) {
			if (sp24.hasconfig) {
				if (g_AudioConfigs[sp24.confignum].volpercentage != -1) {
					volume = g_AudioConfigs[sp24.confignum].volpercentage * AL_VOL_FULL / 100;
				}

				if (g_AudioConfigs[sp24.confignum].pan != -1) {
					pan = g_AudioConfigs[sp24.confignum].pan;
				}

				// This is the same thing again
				if (g_AudioConfigs[sp24.confignum].pan != -1) {
					pan = g_AudioConfigs[sp24.confignum].pan;
				}

				if (g_Vars.langfilteron && (g_AudioConfigs[sp24.confignum].flags & AUDIOCONFIGFLAG_OFFENSIVE)) {
					volume = 0;
				}
			}

			volume = volume * snd0000e9dc() / AL_VOL_FULL;

			g_SndCurMp3.romaddr = fileGetRomAddress(sp20.id);
			g_SndCurMp3.romsize = fileGetRomSize(sp20.id);

			func00037f08(volume, true);
			func00037f5c(pan, true);

			mp3PlayFile(g_SndCurMp3.romaddr, g_SndCurMp3.romsize);

			func00037f08(volume, true);
			func00037f5c(pan, true);

			g_SndCurMp3.sfxref.packed = sp20.packed;
			g_SndCurMp3.playing = true;
			g_SndCurMp3.responsetimer240 = -1;
			g_SndCurMp3.responsetype = MP3RESPONSETYPE_NONE;

			if (g_SndCurMp3.sfxref.unk02 == 2) {
				g_SndCurMp3.responsetype = MP3RESPONSETYPE_ACKNOWLEDGE;
			} else if (g_SndCurMp3.sfxref.unk02 == 1) {
				g_SndCurMp3.responsetype = MP3RESPONSETYPE_WHISPER;
			}

			if ((sp24.hasconfig && (g_AudioConfigs[sp24.confignum].flags & AUDIOCONFIGFLAG_RESPONDHELLO)) || (responseflags & 1)) {
				g_SndCurMp3.responsetype = MP3RESPONSETYPE_GREETING;
			}
		}
	}
}

void sndPlayNosedive(s32 seconds)
{
	g_SndNosediveDuration240 = seconds * TICKS(240);
	g_SndNosediveAge240 = 0;
	g_SndNosediveVolume = 0;
	g_SndNosediveHandle = NULL;
}

void sndStopNosedive(void)
{
	if (g_SndNosediveAge240 != -1) {
		g_SndNosediveAge240 = g_SndNosediveDuration240 + 1;
	}
}

void sndTickNosedive(void)
{
	f32 percentage;

	if (g_SndNosediveAge240 != -1) {
		g_SndNosediveAge240 += g_Vars.lvupdate240;

		percentage = (f32)g_SndNosediveAge240 / (f32)g_SndNosediveDuration240;

		if (percentage < 1.0f) { // less than 100% complete
			percentage += 0.44f;

			if (lvIsPaused()) {
				// Fade out volume during pause instead of stopping abruptly
				if (g_SndNosediveVolume > 0) {
					g_SndNosediveVolume -= g_Vars.diffframe240 * PALUP(80);

					if (g_SndNosediveVolume < 0) {
						g_SndNosediveVolume = 0;
					}
				}

				if (g_SndNosediveVolume) {
					sndAdjust(&g_SndNosediveHandle, 0, g_SndNosediveVolume, AL_PAN_CENTER, -1, percentage, 0, -1, 1);
				} else if (g_SndNosediveHandle != NULL) {
					audioStop(g_SndNosediveHandle);
					g_SndNosediveHandle = NULL;
				}
			} else {
				// Not paused
				if (g_SndNosediveHandle == NULL) {
					sndStart(var80095200, SFX_NOSEDIVE, &g_SndNosediveHandle, -1, -1, -1, -1, -1);
				}

				// Fade in over about 2 seconds
				if (g_SndNosediveVolume < 20000) {
					g_SndNosediveVolume += g_Vars.diffframe240 * PALUP(40);

					if (g_SndNosediveVolume > 20000) {
						g_SndNosediveVolume = 20000;
					}
				}

				sndAdjust(&g_SndNosediveHandle, 0, g_SndNosediveVolume, AL_PAN_CENTER, -1, percentage, 0, -1, 1);
			}
		} else {
			// Reached the configured fade out point
			if (g_SndNosediveVolume > 0) {
				g_SndNosediveVolume -= g_Vars.diffframe240 * PALUP(80);

				if (g_SndNosediveVolume < 0) {
					g_SndNosediveVolume = 0;
				}
			}

			if (g_SndNosediveVolume) {
				sndAdjust(&g_SndNosediveHandle, 0, g_SndNosediveVolume, AL_PAN_CENTER, -1, -1, 0, -1, 1);
			} else if (g_SndNosediveHandle != NULL) {
				audioStop(g_SndNosediveHandle);
				g_SndNosediveHandle = NULL;
			}
		}
	}
}

void sndPlayUfo(s32 seconds)
{
	g_SndUfoDuration240 = seconds * TICKS(240);
	g_SndUfoAge240 = 0;
	g_SndUfoVolume = 0;
	g_SndUfoHandle = NULL;
}

void sndStopUfo(void)
{
	if (g_SndUfoAge240 != -1) {
		g_SndUfoAge240 = g_SndUfoDuration240 + 1;
	}
}

void sndTickUfo(void)
{
	f32 percentage;

	if (g_SndUfoAge240 != -1) {
		g_SndUfoAge240 += g_Vars.lvupdate240;

		percentage = (f32)g_SndUfoAge240 / (f32)g_SndUfoDuration240;

		if (percentage < 1.0f) { // less than 100% complete
			if (percentage < 0.65f) {
				percentage = percentage / 0.65f * 0.3f + 0.7f;
			} else {
				percentage = (percentage - 0.65f) / 0.35f * 0.8f + 1.0f;
			}

			if (lvIsPaused()) {
				// Fade out volume during pause instead of stopping abruptly
				if (g_SndUfoVolume > 0) {
					g_SndUfoVolume -= g_Vars.diffframe240 * PALUP(120);

					if (g_SndUfoVolume < 0) {
						g_SndUfoVolume = 0;
					}
				}

				if (g_SndUfoVolume) {
					sndAdjust(&g_SndUfoHandle, 0, g_SndUfoVolume, AL_PAN_CENTER, -1, percentage, 0, -1, 1);
				} else if (g_SndUfoHandle != NULL) {
					audioStop(g_SndUfoHandle);
					g_SndUfoHandle = NULL;
				}
			} else {
				// Not paused
				if (g_SndUfoHandle == NULL) {
					sndStart(var80095200, SFX_UFOHUM, &g_SndUfoHandle, -1, -1, -1, -1, -1);
				}

				// Fade in over about 2.4 seconds
				if (g_SndUfoVolume < AL_VOL_FULL) {
					g_SndUfoVolume += g_Vars.diffframe240 * PALUP(40);

					if (g_SndUfoVolume > AL_VOL_FULL) {
						g_SndUfoVolume = AL_VOL_FULL;
					}
				}

				sndAdjust(&g_SndUfoHandle, 0, g_SndUfoVolume, AL_PAN_CENTER, -1, percentage, 0, -1, 1);
			}
		} else {
			// Reached the configured fade out point
			if (g_SndUfoVolume > 0) {
				g_SndUfoVolume -= g_Vars.diffframe240 * PALUP(120);

				if (g_SndUfoVolume < 0) {
					g_SndUfoVolume = 0;
				}
			}

			if (g_SndUfoVolume) {
				sndAdjust(&g_SndUfoHandle, 0, g_SndUfoVolume, AL_PAN_CENTER, -1, -1, 0, -1, 1);
			} else if (g_SndUfoHandle != NULL) {
				audioStop(g_SndUfoHandle);
				g_SndUfoHandle = NULL;
			}
		}
	}
}
