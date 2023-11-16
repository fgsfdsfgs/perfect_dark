#include <ultra64.h>
#include "game/chraicommands.h"
#include "game/prop.h"
#include "game/inv.h"
#include "stagesetup.h"
#include "data.h"
#include "types.h"

/**
 * Noise settings
 *
 * - minradius
 * - maxradius
 * - incradius
 * - decbasespeed
 * - decremspeed
 *
 * incradius is how var the radius is incremented on each shot.
 *
 * decbasespeed controls how quickly the noise radius shrinks.
 * The value is the amount of seconds it takes to shrink by one increment.
 *
 * decremspeed also controls how quickly the noise radius shrinks.
 * The value is the amount of seconds it takes to shrink the total remaining.
 * Because the remaining amount changes on each frame, this property causes the
 * noise radius to shrink faster at high radiuses and slower at small radiuses.
 *
 * The highest speed out of both decbasespeed and decremspeed is used.
 */

struct noisesettings invnoisesettings_silent   = { 0, 0,  0,   1, 6 };
struct noisesettings invnoisesettings_silenced = { 0, 3,  1.4, 1, 6 };
struct noisesettings invnoisesettings_sniper   = { 0, 9,  2,   1, 6 };
struct noisesettings invnoisesettings_default  = { 0, 14, 2,   1, 6 };
struct noisesettings invnoisesettings_loud     = { 0, 18, 2,   1, 6 };
struct noisesettings invnoisesettings_louder   = { 0, 22, 3,   1, 6 };
struct noisesettings invnoisesettings_loudest  = { 0, 25, 15,  5, 6 };
struct noisesettings invnoisesettings_reaper   = { 0, 35, 1,   1, 6 };

/**
 * Recoil settings
 *
 * - xrange
 * - yrange
 * - zrange
 * - unused
 * - unused
 *
 * These settings affect the gun's position, but only on the frame that it's firing.
 * The distance moved is random within the range that is specified here.
 *
 * xrange is left to right.
 * yrange is bottom to top.
 * zrange is forward to back.
 */

struct recoilsettings invrecoilsettings_tester      = { 0,   0,   0,   0.2, 1 };
struct recoilsettings invrecoilsettings_laserstream = { 0.2, 0.2, 0.2, 0.2, 1 };
struct recoilsettings invrecoilsettings_default     = { 0.6, 0.6, 0.6, 0.2, 1 };
struct recoilsettings invrecoilsettings_reaper      = { 0.6, 0.6, 2.6, 0.2, 1 };

/**
 * Aim settings
 *
 * f32 zoomfov
 * f32 guntransup
 * f32 guntransdown
 * f32 guntransside
 * f32 aimdamppal
 * f32 aimdamp
 * u32 tracktype
 * u32 unused
 * u32 flags
 *
 * zoomfov is in degrees. The default field of view is 60. A zoomfov value of 30
 * means a 2x zoom is being used.
 *
 * guntransup, guntransdown and guntransside is how far the gun's position is
 * translated when aiming in that direction, either with R or by turning.
 *
 * aimdamp is how quickly the aim slows down as it reaches its target.
 * The aimdamp value is only used on NTSC, while aimdamppal is only used on PAL.
 *
 * tracktype is what kind of red box tracking is used when aiming.
 * See the SIGHTRACKTYPE constants for possible values.
 *
 * flags is a bitfield of INVAIMFLAG constants.
 */

struct invaimsettings invaimsettings_default = {
	0,                                                  // zoomfov
	3,                                                  // guntransup
	8,                                                  // guntransdown
	15,                                                 // guntransside
	0.9721,                                             // aimdamppal
	0.9767,                                             // aimdamp
	SIGHTTRACKTYPE_DEFAULT,                             // tracktype
	0,                                                  // unused
	INVAIMFLAG_AUTOAIM,                                 // flags
};

struct invaimsettings invaimsettings_smg = {
	0,                                                  // zoomfov
	3,                                                  // guntransup
	8,                                                  // guntransdown
	15,                                                 // guntransside
	0.9721,                                             // aimdamppal
	0.9767,                                             // aimdamp
	SIGHTTRACKTYPE_DEFAULT,                             // tracktype
	0,                                                  // unused
	INVAIMFLAG_AUTOAIM | INVAIMFLAG_ACCURATESINGLESHOT, // flags
};

struct invaimsettings invaimsettings_unarmed = {
	0,                                                  // zoomfov
	3,                                                  // guntransup
	8,                                                  // guntransdown
	15,                                                 // guntransside
	0.9721,                                             // aimdamppal
	0.9767,                                             // aimdamp
	SIGHTTRACKTYPE_NONE,                                // tracktype
	7,                                                  // unused
	INVAIMFLAG_AUTOAIM,                                 // flags
};

struct invaimsettings invaimsettings_falcon2scope = {
	30,                                                 // zoomfov
	3,                                                  // guntransup
	8,                                                  // guntransdown
	15,                                                 // guntransside
	0.9721,                                             // aimdamppal
	0.9767,                                             // aimdamp
	SIGHTTRACKTYPE_DEFAULT,                             // tracktype
	5,                                                  // unused
	INVAIMFLAG_AUTOAIM,                                 // flags
};

struct invaimsettings invaimsettings_heavy = {
	30,                                                 // zoomfov
	3,                                                  // guntransup
	8,                                                  // guntransdown
	15,                                                 // guntransside
	0.9721,                                             // aimdamppal
	0.9767,                                             // aimdamp
	SIGHTTRACKTYPE_DEFAULT,                             // tracktype
	5,                                                  // unused
	INVAIMFLAG_AUTOAIM | INVAIMFLAG_ACCURATESINGLESHOT, // flags
};

struct weaponfunc_shootsingle invfunc_00011160 = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8039, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_default = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_hammer = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	&invammo_default, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_000, // short name
	L_GUN_000, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_DUALWIELD,
};

struct weapon invitem_nothing = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_000, // short name
	L_GUN_000, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_DUALWIELD,
};

struct guncmd invanim_punch_type3[] = {
	gunscript_playanimation(ANIM_GUN_UNARMED_LEFTPUNCH, 0, 10000)
	gunscript_waittime(7, 2)
	gunscript_end
};

struct guncmd invanim_punch_type1[] = {
	gunscript_playanimation(ANIM_GUN_UNARMED_RIGHTPUNCH, 0, 10000)
	gunscript_waittime(8, 2)
	gunscript_end
};

struct guncmd invanim_punch_type2[] = {
	gunscript_playanimation(ANIM_GUN_UNARMED_RIGHTPUSH, 0, 10000)
	gunscript_waittime(7, 2)
	gunscript_end
};

struct guncmd invanim_punch_type4[] = {
	gunscript_playanimation(ANIM_GUN_UNARMED_DOUBLEPUNCH, 0, 10000)
	gunscript_waittime(8, 2)
	gunscript_waittime(18, 3)
	gunscript_end
};

struct guncmd invanim_punch[] = {
	gunscript_random(20, invanim_punch_type1)
	gunscript_random(40, invanim_punch_type2)
	gunscript_random(60, invanim_punch_type3)
	gunscript_include(0, invanim_punch_type4)
	gunscript_end
};

struct weaponfunc_melee invfunc_unarmed_punch = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_100, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	invanim_punch, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_BLUNTIMPACT | FUNCFLAG_NOSTUN | FUNCFLAG_00400000,
	0.5, // damage
	60, // range
	105, // unused
	105, // unused
	105, // unused
	7.5, // unused
	-35, // unused
	7.5, // unused
	-35, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct weaponfunc_melee invfunc_unarmed_disarm = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_101, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	invanim_punch, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_DISARM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_BLUNTIMPACT | FUNCFLAG_NOSTUN | FUNCFLAG_00400000,
	0.3, // damage
	60, // range
	105, // unused
	105, // unused
	105, // unused
	7.5, // unused
	-35, // unused
	7.5, // unused
	-35, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct weapon invitem_unarmed = {
	FILE_GCOMBATHANDSLOD, // hi model
	FILE_GCOMBATHANDSLOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{&invfunc_unarmed_punch, &invfunc_unarmed_disarm}, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_unarmed,
	1, // muzzlez
	7.5, // posx
	-41, // posy
	-16, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_006, // short name
	L_GUN_006, // name
	L_GUN_000, // manufacturer
	L_GUN_155, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_HIDEMENUMODEL | WEAPONFLAG_UNDROPPABLE,
};

struct modelpartvisibility invpartvisibility_falcon2[] = {
	{ MODELPART_GUN_MUZZLEFLASH1,  false },
	{ MODELPART_FALCON2_MAGAZINE1, false },
	{ MODELPART_FALCON2_MAGAZINE2, false },
	{ MODELPART_FALCON2_SCOPE,     false },
	{ MODELPART_FALCON2_SILENCER,  false },
	{ MODELPART_FALCON2_002E,      false },
	{ MODELPART_FALCON2_002F,      false },
	{ 255 },
};

struct modelpartvisibility invpartvisibility_falcon2scope[] = {
	{ MODELPART_GUN_MUZZLEFLASH1,  false },
	{ MODELPART_FALCON2_MAGAZINE1, false },
	{ MODELPART_FALCON2_MAGAZINE2, false },
	{ MODELPART_FALCON2_SILENCER,  false },
	{ MODELPART_FALCON2_002F,      false },
	{ 255 },
};

struct modelpartvisibility invpartvisibility_falcon2silencer[] = {
	{ MODELPART_GUN_MUZZLEFLASH1,  false },
	{ MODELPART_FALCON2_MAGAZINE1, false },
	{ MODELPART_FALCON2_MAGAZINE2, false },
	{ MODELPART_FALCON2_SCOPE,     false },
	{ MODELPART_FALCON2_002E,      false },
	{ 255 },
};

struct gunviscmd gunviscmds_falcon2[] = {
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE1)
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_FALCON2_SCOPE)
	gunviscmd_sethidden(MODELPART_FALCON2_SILENCER)
	gunviscmd_sethidden(MODELPART_FALCON2_002E)
	gunviscmd_sethidden(MODELPART_FALCON2_002F)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct gunviscmd gunviscmds_falcon2scope[] = {
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE1)
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_FALCON2_SILENCER)
	gunviscmd_sethidden(MODELPART_FALCON2_002E)
	gunviscmd_sethidden(MODELPART_FALCON2_002F)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct gunviscmd gunviscmds_falcon2silencer[] = {
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE1)
	gunviscmd_sethidden(MODELPART_FALCON2_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_FALCON2_SCOPE)
	gunviscmd_sethidden(MODELPART_FALCON2_002E)
	gunviscmd_sethidden(MODELPART_FALCON2_002F)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_falcon2_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_FALCON2_MAGAZINE2)
	gunscript_showpart(1, MODELPART_FALCON2_MAGAZINE1)
	gunscript_playsound(10, SFX_01D8)
	gunscript_hidepart(19, MODELPART_FALCON2_MAGAZINE1)
	gunscript_waittime(24, 1)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(24, SFX_80F6)
#else
	gunscript_playsound(24, SFX_01D9)
#endif
	gunscript_hidepart(24, MODELPART_FALCON2_MAGAZINE2)
	gunscript_playsound(53, SFX_01DB)
	gunscript_waittime(53, 3)
	gunscript_end
};

struct guncmd invanim_falcon2scope_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_RELOAD_SCOPE, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_FALCON2_MAGAZINE2)
	gunscript_showpart(1, MODELPART_FALCON2_MAGAZINE1)
	gunscript_playsound(10, SFX_01D8)
	gunscript_hidepart(19, MODELPART_FALCON2_MAGAZINE1)
	gunscript_waittime(24, 1)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(24, SFX_80F6)
#else
	gunscript_playsound(24, SFX_01D9)
#endif
	gunscript_hidepart(24, MODELPART_FALCON2_MAGAZINE2)
	gunscript_playsound(53, SFX_01DB)
	gunscript_waittime(53, 3)
	gunscript_end
};

struct guncmd invanim_falcon2_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_RELOAD_DUALWIELD, 0, 10000)
	gunscript_hidepart(1, MODELPART_FALCON2_MAGAZINE2)
	gunscript_showpart(1, MODELPART_FALCON2_MAGAZINE1)
	gunscript_playsound(6, SFX_01D8)
	gunscript_waittime(50, 1)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(50, SFX_80F6)
#else
	gunscript_playsound(50, SFX_01D9)
#endif
	gunscript_playsound(71, SFX_01DB)
	gunscript_end
};

struct guncmd invanim_falcon2_reload[] = {
	gunscript_include(1, invanim_falcon2_reload_dualwield)
	gunscript_include(0, invanim_falcon2_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_falcon2scope_reload[] = {
	gunscript_include(1, invanim_falcon2_reload_dualwield)
	gunscript_include(0, invanim_falcon2scope_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_falcon2_pistolwhip[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_WHIP, 0, 10000)
	gunscript_waittime(23, 2)
	gunscript_end
};

struct guncmd invanim_falcon2_equip[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_falcon2_unequip[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_falcon2_shoot[] = {
	gunscript_playanimation(ANIM_GUN_FALCON2_SHOOT, 0, 10000)
	gunscript_waittime(9, 5)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_falcon2_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_falcon2_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	16, // recoverytime60
	1, // damage
	1, // spread
	3, 5, 2, 0,
	10, // recoildist
	15, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_804D, // shootsound
	1, // penetration
};

struct weaponfunc_shootsingle invfunc_falcon2silenced_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_falcon2_shoot, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	&invrecoilsettings_default,
	16, // recoverytime60
	1, // damage
	1, // spread
	3, 5, 2, 0,
	10, // recoildist
	15, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_8054, // shootsound
	1, // penetration
};

struct weaponfunc_melee invfunc_falcon2_pistolwhip = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_094, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	invanim_falcon2_pistolwhip, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_BLUNTIMPACT | FUNCFLAG_NOSTUN | FUNCFLAG_00400000,
	0.9, // damage
	60, // range
	105, // unused
	105, // unused
	105, // unused
	-16, // unused
	0.8, // unused
	23.5, // unused
	-32.7, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct inventory_ammo invammo_falcon2 = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	8, // clip size
	invanim_falcon2_reload, // reload animation
	0, // flags
};

struct inventory_ammo invammo_falcon2scope = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	8, // clip size
	invanim_falcon2scope_reload, // reload animation
	0, // flags
};

struct weapon invitem_falcon2 = {
	FILE_GFALCON2, // hi model
	FILE_GFALCON2LOD, // lo model
	invanim_falcon2_equip, // equip animation
	invanim_falcon2_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_falcon2_singleshot, &invfunc_falcon2_pistolwhip }, // functions
	&invammo_falcon2, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	2, // muzzlez
	9, // posx
	-15.7, // posy
	-23.8, // posz
	1, // sway
	gunviscmds_falcon2, // gunviscmds
	invpartvisibility_falcon2, // part visibility
#if VERSION == VERSION_JPN_FINAL
	L_GUN_257, // short name
#else
	L_GUN_007, // short name
#endif
	L_GUN_007, // name
	L_GUN_150, // manufacturer
	L_GUN_156, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct weapon invitem_falcon2scope = {
	FILE_GFALCON2, // hi model
	FILE_GFALCON2LOD, // lo model
	invanim_falcon2_equip, // equip animation
	invanim_falcon2_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_falcon2_singleshot, &invfunc_falcon2_pistolwhip }, // functions
	&invammo_falcon2scope, // pri ammo
	NULL, // sec ammo
	&invaimsettings_falcon2scope,
	1, // muzzlez
	9, // posx
	-15.7, // posy
	-23.8, // posz
	1, // sway
	gunviscmds_falcon2scope, // gunviscmds
	invpartvisibility_falcon2scope, // part visibility
	L_GUN_077, // short name
	L_GUN_009, // name
	L_GUN_150, // manufacturer
	L_GUN_158, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct weapon invitem_falcon2silencer = {
	FILE_GFALCON2, // hi model
	FILE_GFALCON2LOD, // lo model
	invanim_falcon2_equip, // equip animation
	invanim_falcon2_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_falcon2silenced_singleshot, &invfunc_falcon2_pistolwhip }, // functions
	&invammo_falcon2, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	9, // posx
	-15.7, // posy
	-23.8, // posz
	1, // sway
	gunviscmds_falcon2silencer, // gunviscmds
	invpartvisibility_falcon2silencer, // part visibility
	L_GUN_078, // short name
	L_GUN_008, // name
	L_GUN_150, // manufacturer
	L_GUN_157, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct modelpartvisibility invpartvisibility_magsec[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_PISTOL_MAGAZINE1, false },
	{ MODELPART_PISTOL_MAGAZINE2, false },
	{ 255 },
};

struct gunviscmd gunviscmds_magsec[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_sethidden(MODELPART_PISTOL_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_PISTOL_MAGAZINE1)
	gunviscmd_end
};

struct guncmd invanim_magsec_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_MAGSEC4_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_PISTOL_MAGAZINE1)
	gunscript_setsoundspeed(3, 1300)
	gunscript_playsound(3, SFX_01D8)
	gunscript_showpart(10, MODELPART_PISTOL_MAGAZINE2)
	gunscript_hidepart(14, MODELPART_PISTOL_MAGAZINE1)
	gunscript_hidepart(22, MODELPART_PISTOL_MAGAZINE2)
	gunscript_showpart(22, MODELPART_PISTOL_MAGAZINE1)
	gunscript_setsoundspeed(27, 1300)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(27, SFX_80F6)
#else
	gunscript_playsound(27, SFX_01D9)
#endif
	gunscript_waittime(30, 1)
	gunscript_waittime(56, 3)
	gunscript_setsoundspeed(58, 1300)
	gunscript_playsound(58, SFX_01DB)
	gunscript_end
};

struct guncmd invanim_magsec_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_MAGSEC4_RELOAD_DUALWIELD, 0, 10000)
	gunscript_showpart(1, MODELPART_PISTOL_MAGAZINE1)
	gunscript_setsoundspeed(5, 1300)
	gunscript_playsound(5, SFX_01D8)
	gunscript_hidepart(25, MODELPART_PISTOL_MAGAZINE1)
	gunscript_setsoundspeed(47, 1300)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(47, SFX_80F6)
#else
	gunscript_playsound(47, SFX_01D9)
#endif
	gunscript_waittime(49, 1)
	gunscript_setsoundspeed(69, 1300)
	gunscript_playsound(69, SFX_01DB)
	gunscript_end
};

struct guncmd invanim_magsec_reload[] = {
	gunscript_include(1, invanim_magsec_reload_dualwield)
	gunscript_include(0, invanim_magsec_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_magsec_shoot[] = {
	gunscript_playanimation(ANIM_GUN_MAGSEC4_SHOOT, 0, 10000)
	gunscript_waittime(12, 5)
	gunscript_end
};

struct invaimsettings invaimsettings_magsec4 = {
	25,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_DEFAULT,
	1,
	INVAIMFLAG_AUTOAIM,
};

struct weaponfunc_shootsingle invfunc_magsec_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_magsec_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	16, // recoverytime60
	1.1, // damage
	6, // spread
	4, 8, 3, 0,
	5, // recoildist
	10, // recoilangle
	59.999996185303, // slidemax
	10, // impactforce
	0, // duration60
	SFX_804C, // shootsound
	1, // penetration
};

struct weaponfunc_shootsingle invfunc_magsec_burst = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_128, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_magsec_shoot, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	16, // recoverytime60
	1.1, // damage
	10, // spread
	9, 14, 0, 0,
	8, // recoildist
	12, // recoilangle
	59.999996185303, // slidemax
	10, // impactforce
	0, // duration60
	SFX_804C, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_magsec = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	9, // clip size
	invanim_magsec_reload, // reload animation
	0, // flags
};

struct weapon invitem_magsec = {
	FILE_GLEEGUN1, // hi model
	FILE_GMAGSECLOD, // lo model
	invanim_magsec_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_magsec_singleshot, &invfunc_magsec_burst }, // functions
	&invammo_magsec, // pri ammo
	NULL, // sec ammo
	&invaimsettings_magsec4,
	2, // muzzlez
	10.5, // posx
	-17.2, // posy
	-26.5, // posz
	1, // sway
	gunviscmds_magsec, // gunviscmds
	invpartvisibility_magsec, // part visibility
	L_GUN_010, // short name
	L_GUN_010, // name
	L_GUN_151, // manufacturer
	L_GUN_159, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct modelpartvisibility invpartvisibility_dy357[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_MAGNUM_CART1,     false },
	{ MODELPART_MAGNUM_CART2,     false },
	{ MODELPART_MAGNUM_CART3,     false },
	{ MODELPART_MAGNUM_CART4,     false },
	{ MODELPART_MAGNUM_CART5,     false },
	{ MODELPART_MAGNUM_CART6,     false },
	{ 255 },
};

struct gunviscmd gunviscmds_magnum[] = {
	gunviscmd_sethidden(MODELPART_MAGNUM_CART1)
	gunviscmd_sethidden(MODELPART_MAGNUM_CART2)
	gunviscmd_sethidden(MODELPART_MAGNUM_CART3)
	gunviscmd_sethidden(MODELPART_MAGNUM_CART4)
	gunviscmd_sethidden(MODELPART_MAGNUM_CART5)
	gunviscmd_sethidden(MODELPART_MAGNUM_CART6)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_dy357_shoot[] = {
	gunscript_playanimation(ANIM_GUN_DY357_SHOOT, 0, 10000)
	gunscript_waittime(12, 5)
	gunscript_waittime(12, 2)
	gunscript_end
};

struct guncmd invanim_dy357_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_DY357_RELOAD, 0, 10000)
	gunscript_playsound(50, SFX_05CF)
	gunscript_setsoundspeed(80, 1830)
	gunscript_playsound(80, SFX_01D8)
	gunscript_popoutsackofpills(80)
	gunscript_popoutsackofpills(81)
	gunscript_popoutsackofpills(82)
	gunscript_popoutsackofpills(83)
	gunscript_popoutsackofpills(84)
	gunscript_popoutsackofpills(85)
	gunscript_showpart(92, MODELPART_HAND_LEFT)
	gunscript_showpart(92, MODELPART_MAGNUM_CART1)
	gunscript_showpart(92, MODELPART_MAGNUM_CART2)
	gunscript_showpart(92, MODELPART_MAGNUM_CART3)
	gunscript_showpart(92, MODELPART_MAGNUM_CART4)
	gunscript_showpart(92, MODELPART_MAGNUM_CART5)
	gunscript_showpart(92, MODELPART_MAGNUM_CART6)
	gunscript_setsoundspeed(114, 1210)
	gunscript_playsound(114, SFX_01D8)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART1)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART2)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART3)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART4)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART5)
	gunscript_hidepart(121, MODELPART_MAGNUM_CART6)
	gunscript_waittime(123, 1)
	gunscript_playsound(147, SFX_05CD)
	gunscript_end
};

struct guncmd invanim_dy357_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_DY357_RELOAD_DUALWIELD, 0, 10000)
	gunscript_playsound(50, SFX_05CF)
	gunscript_setsoundspeed(80, 1830)
	gunscript_playsound(80, SFX_01D8)
	gunscript_popoutsackofpills(80)
	gunscript_popoutsackofpills(81)
	gunscript_popoutsackofpills(82)
	gunscript_popoutsackofpills(83)
	gunscript_popoutsackofpills(84)
	gunscript_popoutsackofpills(85)
	gunscript_setsoundspeed(114, 1210)
	gunscript_playsound(114, SFX_01D8)
	gunscript_waittime(123, 1)
	gunscript_playsound(147, SFX_05CD)
	gunscript_end
};

struct guncmd invanim_dy357_reload[] = {
	gunscript_include(1, invanim_dy357_reload_dualwield)
	gunscript_include(0, invanim_dy357_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_dy357_pistolwhip[] = {
	gunscript_playanimation(ANIM_GUN_DY357_WHIP, 0, 10000)
	gunscript_waittime(23, 2)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_dy357_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_dy357_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	20, // recoverytime60
	2, // damage
	0, // spread
	8, 16, 0, -1,
	12, // recoildist
	35, // recoilangle
	0, // slidemax
	6, // impactforce
	0, // duration60
	SFX_8066, // shootsound
	5, // penetration
};

struct weaponfunc_shootsingle invfunc_dy357lx_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_dy357_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	30, // recoverytime60
	200, // damage
	0, // spread
	8, 24, 0, -1,
	12, // recoildist
	35, // recoilangle
	0, // slidemax
	6, // impactforce
	0, // duration60
	SFX_8066, // shootsound
	5, // penetration
};

struct weaponfunc_melee invfunc_dy357_pistolwhip = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_094, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	invanim_dy357_pistolwhip, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_BLUNTIMPACT | FUNCFLAG_NOSTUN | FUNCFLAG_00400000,
	0.9, // damage
	60, // range
	105, // unused
	105, // unused
	105, // unused
	19, // unused
	-18.7, // unused
	19, // unused
	-18.7, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct inventory_ammo invammo_dy357 = {
	AMMOTYPE_MAGNUM,
	CASING_STANDARD,
	6, // clip size
	invanim_dy357_reload, // reload animation
	0, // flags
};

struct weapon invitem_dy357 = {
	FILE_GDY357, // hi model
	FILE_GDY357LOD, // lo model
	invanim_dy357_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_dy357_singleshot, &invfunc_dy357_pistolwhip }, // functions
	&invammo_dy357, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	2, // muzzlez
	9.5, // posx
	-18.2, // posy
	-25.5, // posz
	1, // sway
	gunviscmds_magnum, // gunviscmds
	invpartvisibility_dy357, // part visibility
	L_GUN_084, // short name
	L_GUN_012, // name
	L_GUN_149, // manufacturer
	L_GUN_161, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct weapon invitem_dy357lx = {
	FILE_GDY357TRENT, // hi model
	FILE_GDY357TRENTLOD, // lo model
	invanim_dy357_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_dy357lx_singleshot, &invfunc_dy357_pistolwhip }, // functions
	&invammo_dy357, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	2, // muzzlez
	9.5, // posx
	-18.2, // posy
	-25.5, // posz
	1, // sway
	gunviscmds_magnum, // gunviscmds
	invpartvisibility_dy357, // part visibility
	L_GUN_013, // short name
	L_GUN_013, // name
	L_GUN_149, // manufacturer
	L_GUN_162, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct modelpartvisibility invpartvisibility_phoenix[] = {
	{ MODELPART_PHOENIX_ORB, false },
	{ 255 },
};

struct gunviscmd gunviscmds_phoenix[] = {
	gunviscmd_sethidden(MODELPART_PHOENIX_ORB)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_phoenix_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_PHOENIX_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_PHOENIX_ORB)
	gunscript_setsoundspeed(44, 1510)
	gunscript_playsound(44, SFX_RELOAD_FARSIGHT)
	gunscript_hidepart(50, MODELPART_PHOENIX_ORB)
	gunscript_end
};

struct guncmd invanim_phoenix_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_PHOENIX_RELOAD_DUALWIELD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_PHOENIX_ORB)
	gunscript_setsoundspeed(44, 1510)
	gunscript_playsound(44, SFX_RELOAD_FARSIGHT)
	gunscript_hidepart(50, MODELPART_PHOENIX_ORB)
	gunscript_end
};

struct guncmd invanim_phoenix_equiporreload[] = {
	gunscript_include(1, invanim_phoenix_reload_dualwield)
	gunscript_include(0, invanim_phoenix_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_phoenix_shoot[] = {
	gunscript_playanimation(ANIM_GUN_PHOENIX_SHOOT, 0, 10000)
	gunscript_waittime(9, 5)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_phoenix_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_phoenix_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	16, // recoverytime60
	1.1, // damage
	3, // spread
	3, 5, 2, 0,
	10, // recoildist
	15, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_8071, // shootsound
	1, // penetration
};

struct weaponfunc_shootsingle invfunc_phoenix_explosiveshells = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_095, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_phoenix_shoot, // fire animation
	FUNCFLAG_EXPLOSIVESHELLS,
	&invrecoilsettings_default,
	16, // recoverytime60
	1.2, // damage
	5, // spread
	13, 15, 25, 0,
	15, // recoildist
	25, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_8072, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_phoenix = {
	AMMOTYPE_PISTOL,
	CASING_NONE,
	8, // clip size
	invanim_phoenix_equiporreload, // reload animation
	0, // flags
};

struct weapon invitem_phoenix = {
	FILE_GMAIANPISTOL, // hi model
	FILE_GMAYANPISTOLLOD, // lo model
	invanim_phoenix_equiporreload, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_phoenix_singleshot, &invfunc_phoenix_explosiveshells }, // functions
	&invammo_phoenix, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	9.5, // posx
	-16.2, // posy
	-23, // posz
	1, // sway
	gunviscmds_phoenix, // gunviscmds
	invpartvisibility_phoenix, // part visibility
	L_GUN_014, // short name
	L_GUN_014, // name
	L_GUN_153, // manufacturer
	L_GUN_163, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct modelpartvisibility invpartvisibility_mauler[] = {
	{ MODELPART_MAULER_MAGAZINE2, false },
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ 255 },
};

struct gunviscmd gunviscmds_mauler[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_sethidden(MODELPART_MAULER_MAGAZINE2)
	gunviscmd_end
};

struct guncmd invanim_mauler_shoot[] = {
	gunscript_playanimation(ANIM_GUN_MAULER_SHOOT, 0, 10000)
	gunscript_waittime(9, 5)
	gunscript_end
};

struct guncmd invanim_mauler_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_MAULER_RELOAD, 0, 10000)
	gunscript_showpart(0, MODELPART_HAND_LEFT)
	gunscript_setsoundspeed(4, 1200)
	gunscript_playsound(4, SFX_0053)
	gunscript_showpart(4, MODELPART_MAULER_MAGAZINE2)
	gunscript_hidepart(18, MODELPART_MAULER_MAGAZINE1)
	gunscript_setsoundspeed(20, 2500)
	gunscript_playsound(20, SFX_DOOR_8012)
	gunscript_waittime(25, 1)
	gunscript_showpart(25, MODELPART_MAULER_MAGAZINE1)
	gunscript_hidepart(25, MODELPART_MAULER_MAGAZINE2)
	gunscript_setsoundspeed(27, 2000)
	gunscript_playsound(27, SFX_05C5)
	gunscript_setsoundspeed(61, 1000)
	gunscript_playsound(61, SFX_01DB)
	gunscript_waittime(61, 3)
	gunscript_end
};

struct guncmd invanim_mauler_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_MAULER_RELOAD_DUALWIELD, 0, 10000)
	gunscript_setsoundspeed(14, 1200)
	gunscript_playsound(14, SFX_0053)
	gunscript_setsoundspeed(40, 2500)
	gunscript_playsound(40, SFX_DOOR_8012)
	gunscript_setsoundspeed(47, 2000)
	gunscript_playsound(47, SFX_05C5)
	gunscript_waittime(47, 1)
	gunscript_setsoundspeed(68, 1000)
	gunscript_playsound(68, SFX_01DB)
	gunscript_end
};

struct guncmd invanim_mauler_reload[] = {
	gunscript_include(1, invanim_mauler_reload_dualwield)
	gunscript_include(0, invanim_mauler_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_unused_8007c0bc[] = {
	gunscript_playanimation(ANIM_03F6, 0, 10000)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_mauler_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_mauler_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.2, // damage
	6, // spread
	3, 9, 32, 28,
	0, // recoildist
	0, // recoilangle
	59.999996185303, // slidemax
	4, // impactforce
	0, // duration60
	SFX_805B, // shootsound
	1, // penetration
};

struct weaponfunc_shootsingle invfunc_mauler_chargeshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_129, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_mauler_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.2, // damage
	6, // spread
	3, 9, 32, 28,
	0, // recoildist
	0, // recoilangle
	59.999996185303, // slidemax
	4, // impactforce
	0, // duration60
	SFX_805B, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_mauler = {
	AMMOTYPE_PISTOL,
	CASING_NONE,
	20, // clip size
	invanim_mauler_reload, // reload animation
	0, // flags
};

struct weapon invitem_mauler = {
	FILE_GSKPISTOL, // hi model
	FILE_GSKPISTOLLOD, // lo model
	invanim_mauler_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_mauler_singleshot, &invfunc_mauler_chargeshot }, // functions
	&invammo_mauler, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	11.5, // posx
	-17.5, // posy
	-20, // posz
	1, // sway
	gunviscmds_mauler, // gunviscmds
	invpartvisibility_mauler, // part visibility
	L_GUN_011, // short name
	L_GUN_011, // name
	L_GUN_152, // manufacturer
	L_GUN_160, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct modelpartvisibility invpartvisibility_cmp150[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_CMP150_MAGAZINE2, false },
	{ 255 },
};

struct gunviscmd gunviscmds_cmp150[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_sethidden(MODELPART_CMP150_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_GUN_CARTFLAPOPEN)
	gunviscmd_end
};

struct guncmd invanim_cmp150_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_CMP150_RELOAD, 0, 10000)
	gunscript_hidepart(23, MODELPART_CMP150_MAGAZINE1)
	gunscript_showpart(33, MODELPART_HAND_LEFT)
	gunscript_showpart(33, MODELPART_CMP150_MAGAZINE2)
	gunscript_setsoundspeed(9, 950)
	gunscript_playsound(9, SFX_01D8)
	gunscript_setsoundspeed(44, 950)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(44, SFX_80F6)
#else
	gunscript_playsound(44, SFX_01D9)
#endif
	gunscript_hidepart(45, MODELPART_CMP150_MAGAZINE2)
	gunscript_showpart(45, MODELPART_CMP150_MAGAZINE1)
	gunscript_waittime(45, 1)
	gunscript_playsound(86, SFX_0431)
	gunscript_end
};

struct guncmd invanim_cmp150_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_CMP150_RELOAD_DUALWIELD, 0, 10000)
	gunscript_hidepart(23, MODELPART_CMP150_MAGAZINE1)
	gunscript_showpart(33, MODELPART_HAND_LEFT)
	gunscript_showpart(33, MODELPART_CMP150_MAGAZINE2)
	gunscript_setsoundspeed(20, 950)
	gunscript_playsound(20, SFX_01D8)
	gunscript_setsoundspeed(61, 950)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(61, SFX_80F6)
#else
	gunscript_playsound(61, SFX_01D9)
#endif
	gunscript_hidepart(61, MODELPART_CMP150_MAGAZINE2)
	gunscript_showpart(61, MODELPART_CMP150_MAGAZINE1)
	gunscript_waittime(61, 1)
	gunscript_playsound(76, SFX_0431)
	gunscript_end
};

struct guncmd invanim_cmp150_reload[] = {
	gunscript_include(1, invanim_cmp150_reload_dualwield)
	gunscript_include(0, invanim_cmp150_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_cmp150_shoot[] = {
	gunscript_playanimation(ANIM_GUN_CMP150_SHOOT, 0, 10000)
	gunscript_waittime(14, 5)
	gunscript_waitforzreleased(14)
	gunscript_end
};

struct invaimsettings invaimsettings_cmp150 = {
	0,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_FOLLOWLOCKON,
	0,
	INVAIMFLAG_AUTOAIM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weaponfunc_shootauto invfunc_cmp150_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_cmp150_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	9, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	SFX_MENU_ERROR, // shootsound
	1, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootauto invfunc_cmp150_followlockon = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_102, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_cmp150_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	9, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	SFX_MENU_ERROR, // shootsound
	1, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_cmp150 = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	32, // clip size
	invanim_cmp150_reload, // reload animation
	0, // flags
};

struct weapon invitem_cmp150 = {
	FILE_GCMP150, // hi model
	FILE_GCMP150LOD, // lo model
	invanim_cmp150_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_cmp150_rapidfire, &invfunc_cmp150_followlockon }, // functions
	&invammo_cmp150, // pri ammo
	NULL, // sec ammo
	&invaimsettings_cmp150,
	3, // muzzlez
	13, // posx
	-17.7, // posy
	-27.5, // posz
	1, // sway
	gunviscmds_cmp150, // gunviscmds
	invpartvisibility_cmp150, // part visibility
	L_GUN_015, // short name
	L_GUN_015, // name
	L_GUN_149, // manufacturer
	L_GUN_164, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_04000000 | WEAPONFLAG_AIMTRACK,
};

struct modelpartvisibility invpartvisibility_cyclone[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_CYCLONE_MAGAZINE, false },
	{ 255 },
};

struct gunviscmd gunviscmds_cyclone[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_sethidden(MODELPART_CYCLONE_MAGAZINE)
	gunviscmd_end
};

struct guncmd invanim_cyclone_reload_singlewield[] = {
	gunscript_playanimation(ANIM_GUN_CYCLONE_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_CYCLONE_MAGAZINE)
	gunscript_playsound(60, SFX_05D3)
	gunscript_end
};

struct guncmd invanim_cyclone_reload_dualwield[] = {
	gunscript_playanimation(ANIM_GUN_CYCLONE_RELOAD_DUALWIELD, 0, 10000)
	gunscript_playsound(60, SFX_05D3)
	gunscript_end
};

struct guncmd invanim_cyclone_equiporreload[] = {
	gunscript_include(1, invanim_cyclone_reload_dualwield)
	gunscript_include(0, invanim_cyclone_reload_singlewield)
	gunscript_end
};

struct guncmd invanim_cyclone_shoot[] = {
	gunscript_playanimation(ANIM_GUN_CYCLONE_SHOOT, 0, 10000)
	gunscript_waittime(5, 5)
	gunscript_waitforzreleased(5)
	gunscript_end
};

struct weaponfunc_shootauto invfunc_cyclone_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_cyclone_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	0.8, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_805F, // shootsound
	1, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootauto invfunc_cyclone_magazinedischarge = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_097, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_cyclone_shoot, // fire animation
	FUNCFLAG_BURST50,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.4, // damage
	25, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8075, // shootsound
	1, // penetration
	2000, // initialrpm
	2000, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_cyclone = {
	AMMOTYPE_SMG,
	CASING_NONE,
	50, // clip size
	invanim_cyclone_equiporreload, // reload animation
	0, // flags
};

struct weapon invitem_cyclone = {
	FILE_GCYCLONE, // hi model
	FILE_GCYCLONELOD, // lo model
	invanim_cyclone_equiporreload, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_cyclone_rapidfire, &invfunc_cyclone_magazinedischarge }, // functions
	&invammo_cyclone, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	1, // muzzlez
	21.5, // posx
	-26.5, // posy
	-35, // posz
	1, // sway
	gunviscmds_cyclone, // gunviscmds
	invpartvisibility_cyclone, // part visibility
	L_GUN_020, // short name
	L_GUN_020, // name
	L_GUN_151, // manufacturer
	L_GUN_169, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct guncmd invanim_rcp120_reload[] = {
	gunscript_playanimation(ANIM_GUN_RCP120_RELOAD, 0, 10000)
	gunscript_setsoundspeed(18, 1100)
	gunscript_playsound(18, SFX_0053)
	gunscript_showpart(25, MODELPART_RCP120_MAGAZINE1)
	gunscript_showpart(50, MODELPART_RCP120_MAGAZINE2)
	gunscript_playsound(74, SFX_05C5)
	gunscript_hidepart(75, MODELPART_RCP120_MAGAZINE2)
	gunscript_showpart(75, MODELPART_RCP120_MAGAZINE1)
	gunscript_playsound(112, SFX_0431)
	gunscript_end
};

struct guncmd invanim_rcp120_shoot[] = {
	gunscript_playanimation(ANIM_GUN_RCP120_SHOOT, 0, 10000)
	gunscript_waittime(10, 5)
	gunscript_waitforzreleased(10)
	gunscript_end
};

struct modelpartvisibility invpartvisibility_rcp120[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_RCP120_MAGAZINE2, false },
	{ 255 },
};

struct gunviscmd gunviscmds_rcp120[] = {
	gunviscmd_sethidden(MODELPART_RCP120_MAGAZINE2)
	gunviscmd_end
};

struct weaponfunc_shootauto invfunc_rcp120_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_rcp120_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.2, // damage
	6, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_806D, // shootsound
	1, // penetration
	1100, // initialrpm
	1100, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_special invfunc_rcp120_cloak = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_116, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_AUTOSWITCHUNSELECTABLE,
	HANDATTACKTYPE_RCP120CLOAK,
	30, // recoverytime60
	0, // soundnum (unused)
};

struct inventory_ammo invammo_rcp120 = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	120, // clip size
	invanim_rcp120_reload, // reload animation
	0, // flags
};

struct weapon invitem_rcp120 = {
	FILE_GRCP120, // hi model
	FILE_GRCP120LOD, // lo model
	invanim_rcp120_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_rcp120_rapidfire, &invfunc_rcp120_cloak }, // functions
	&invammo_rcp120, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	3, // muzzlez
	13, // posx
	-18.2, // posy
	-27.5, // posz
	1, // sway
	gunviscmds_rcp120, // gunviscmds
	invpartvisibility_rcp120, // part visibility
	L_GUN_022, // short name
	L_GUN_022, // name
	L_GUN_151, // manufacturer
	L_GUN_170, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN,
};

struct guncmd invanim_callisto_reload[] = {
	gunscript_playanimation(ANIM_GUN_CALLISTO_RELOAD, 0, 10000)
	gunscript_showpart(30, MODELPART_CALLISTO_ORB)
	gunscript_setsoundspeed(58, 933)
	gunscript_playsound(58, SFX_RELOAD_FARSIGHT)
	gunscript_hidepart(79, MODELPART_CALLISTO_ORB)
	gunscript_end
};

struct guncmd invanim_callisto_shoot[] = {
	gunscript_playanimation(ANIM_GUN_CALLISTO_SHOOT, 0, 10000)
	gunscript_waittime(8, 5)
	gunscript_waitforzreleased(8)
	gunscript_end
};

struct modelpartvisibility invpartvisibility_callisto[] = {
	{ MODELPART_CALLISTO_ORB, false },
	{ 255 },
};

struct gunviscmd gunviscmds_callisto[] = {
	gunviscmd_sethidden(MODELPART_CALLISTO_ORB)
	gunviscmd_end
};

struct weaponfunc_shootauto invfunc_callisto_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_callisto_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.2, // damage
	9, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	SFX_8063, // shootsound
	1, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootauto invfunc_callisto_highimpactshells = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_115, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_callisto_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	2.4, // damage
	9, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	SFX_8064, // shootsound
	5, // penetration
	300, // initialrpm
	300, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_callisto = {
	AMMOTYPE_SMG,
	CASING_NONE,
	32, // clip size
	invanim_callisto_reload, // reload animation
	0, // flags
};

struct weapon invitem_callisto = {
	FILE_GMAIANSMG, // hi model
	FILE_GMAYANSMGLOD, // lo model
	invanim_callisto_reload, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_callisto_rapidfire, &invfunc_callisto_highimpactshells }, // functions
	&invammo_callisto, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	3, // muzzlez
	17.5, // posx
	-22.7, // posy
	-25, // posz
	1, // sway
	gunviscmds_callisto, // gunviscmds
	invpartvisibility_callisto, // part visibility
	L_GUN_023, // short name
	L_GUN_023, // name
	L_GUN_153, // manufacturer
	L_GUN_171, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_dragon[] = {
	gunviscmd_sethidden(MODELPART_DRAGON_MAGAZINE1)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_dragon[] = {
	{ MODELPART_DRAGON_MAGAZINE1, false },
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ 255 },
};

struct guncmd invanim_dragon_shoot[] = {
	gunscript_playanimation(ANIM_GUN_DRAGON_SHOOT, 0, 10000)
	gunscript_end
};

struct guncmd invanim_dragon_reload[] = {
	gunscript_playanimation(ANIM_GUN_DRAGON_RELOAD, 0, 10000)
	gunscript_playsound(8, SFX_0053)
	gunscript_hidepart(17, MODELPART_DRAGON_MAGAZINE2)
	gunscript_showpart(17, MODELPART_DRAGON_MAGAZINE1)
	gunscript_waittime(41, 1)
	gunscript_playsound(41, SFX_05C5)
	gunscript_showpart(41, MODELPART_DRAGON_MAGAZINE2)
	gunscript_hidepart(41, MODELPART_DRAGON_MAGAZINE1)
	gunscript_end
};

struct weaponfunc_shootauto invfunc_dragon_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_dragon_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.1, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8049, // shootsound
	1, // penetration
	700, // initialrpm
	700, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_throw invfunc_dragon_selfdestruct = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_118, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_DISCARDWEAPON,
	MODEL_CHRDRAGON, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_dragon = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	invanim_dragon_reload, // reload animation
	0, // flags
};

struct weapon invitem_dragon = {
	FILE_GDYDRAGON, // hi model
	FILE_GDYDRAGONLOD, // lo model
	invanim_dragon_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_dragon_rapidfire, &invfunc_dragon_selfdestruct }, // functions
	&invammo_dragon, // pri ammo
	NULL, // sec ammo
	&invaimsettings_heavy,
	1, // muzzlez
	15, // posx
	-29.5, // posy
	-27, // posz
	1, // sway
	gunviscmds_dragon, // gunviscmds
	invpartvisibility_dragon, // part visibility
	L_GUN_017, // short name
	L_GUN_017, // name
	L_GUN_149, // manufacturer
	L_GUN_166, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_04000000,
};

struct gunviscmd gunviscmds_superdragon[] = {
	gunviscmd_sethidden(MODELPART_SUPERDRAGON_MAGAZINE1)
	gunviscmd_sethidden(MODELPART_SUPERDRAGON_MAGAZINE2)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_superdragon[] = {
	{ MODELPART_SUPERDRAGON_MAGAZINE1, false },
	{ MODELPART_SUPERDRAGON_MAGAZINE2, false },
	{ MODELPART_GUN_MUZZLEFLASH1,      false },
	{ 255 },
};

struct guncmd invanim_superdragon_reload[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_RELOAD_PRI, 0, 10000)
	gunscript_playsound(8, SFX_0053)
	gunscript_hidepart(16, MODELPART_SUPERDRAGON_MAGAZINE3)
	gunscript_showpart(16, MODELPART_SUPERDRAGON_MAGAZINE1)
	gunscript_playsound(41, SFX_05C5)
	gunscript_waittime(43, 1)
	gunscript_showpart(43, MODELPART_SUPERDRAGON_MAGAZINE3)
	gunscript_hidepart(43, MODELPART_SUPERDRAGON_MAGAZINE1)
	gunscript_end
};

struct guncmd invanim_superdragon_grenadereload[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_RELOAD_SEC, 0, 10000)
	gunscript_setsoundspeed(6, 700)
	gunscript_playsound(6, SFX_0053)
	gunscript_showpart(30, MODELPART_SUPERDRAGON_MAGAZINE2)
	gunscript_hidepart(30, MODELPART_SUPERDRAGON_MAGAZINE4)
	gunscript_hidepart(64, MODELPART_SUPERDRAGON_MAGAZINE2)
	gunscript_showpart(64, MODELPART_SUPERDRAGON_MAGAZINE4)
	gunscript_waittime(64, 1)
	gunscript_setsoundspeed(66, 850)
	gunscript_playsound(66, SFX_05C5)
	gunscript_end
};

struct guncmd invanim_superdragon_shoot[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_SHOOT_PRI, 0, 10000)
	gunscript_end
};

struct guncmd invanim_superdragon_shootgrenade[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_SHOOT_SEC, 0, 10000)
	gunscript_end
};

struct guncmd invanim_superdragon_pritosec[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_CHANGEFUNC, 0, 10000)
	gunscript_end
};

struct guncmd invanim_superdragon_sectopri[] = {
	gunscript_playanimation(ANIM_GUN_SUPERDRAGON_CHANGEFUNC, 65535, 55536)
	gunscript_end
};

struct weaponfunc_shootauto invfunc_superdragon_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_superdragon_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.2, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8049, // shootsound
	1, // penetration
	700, // initialrpm
	700, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootprojectile invfunc_superdragon_grenadelauncher = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_098, // name
	0, // unused
	1, // ammoindex
	&invnoisesettings_loudest,
	invanim_superdragon_shootgrenade, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_10000000 | FUNCFLAG_20000000,
	NULL,
	0,
	1.2, // damage
	6, // spread
	5, 15, -1, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_8073,
	1,
	MODEL_CHRDRAGGRENADE, // projectilemodelnum
	0, // unused
	1, // scale
	0, // speed
	0.08, // unk50
	30, // traveldist
	1200, // timer60
	0.1, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct inventory_ammo invammo_superdragon = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	invanim_superdragon_reload, // reload animation
	0, // flags
};

struct inventory_ammo invammo_superdragon_grenades = {
	AMMOTYPE_DEVASTATOR,
	CASING_NONE,
	6, // clip size
	invanim_superdragon_grenadereload, // reload animation
	0, // flags
};

struct weapon invitem_superdragon = {
	FILE_GDYSUPERDRAGON, // hi model
	FILE_GDYSUPERDRAGONLOD, // lo model
	invanim_superdragon_shoot, // equip animation
	NULL, // unequip animation
	invanim_superdragon_pritosec, // pritosec animation
	invanim_superdragon_sectopri, // sectopri animation
	{ &invfunc_superdragon_rapidfire, &invfunc_superdragon_grenadelauncher }, // functions
	&invammo_superdragon, // pri ammo
	&invammo_superdragon_grenades, // sec ammo
	&invaimsettings_heavy,
	1, // muzzlez
	15, // posx
	-29.5, // posy
	-27, // posz
	1, // sway
	gunviscmds_superdragon, // gunviscmds
	invpartvisibility_superdragon, // part visibility
	L_GUN_018, // short name
	L_GUN_018, // name
	L_GUN_149, // manufacturer
	L_GUN_167, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_ar34[] = {
	gunviscmd_sethidden(MODELPART_AR34_MAGAZINE2)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_ar34[] = {
	{ MODELPART_AR34_MAGAZINE2,   false },
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ 255 },
};

struct guncmd invanim_ar34_reload[] = {
	gunscript_playanimation(ANIM_GUN_AR34_RELOAD, 0, 10000)
	gunscript_setsoundspeed(16, 900)
	gunscript_playsound(16, SFX_0053)
	gunscript_hidepart(21, MODELPART_AR34_MAGAZINE1)
	gunscript_showpart(50, MODELPART_AR34_MAGAZINE2)
	gunscript_setsoundspeed(67, 1400)
	gunscript_playsound(67, SFX_8025)
	gunscript_hidepart(72, MODELPART_AR34_MAGAZINE2)
	gunscript_showpart(72, MODELPART_AR34_MAGAZINE1)
	gunscript_waittime(72, 1)
	gunscript_setsoundspeed(95, 900)
	gunscript_playsound(95, SFX_05C5)
	gunscript_end
};

struct invaimsettings invaimsettings_ar34 = {
	20,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_DEFAULT,
	5,
	INVAIMFLAG_AUTOAIM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weaponfunc_shootauto invfunc_ar34_burstfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_087, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	NULL, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.4, // damage
	8, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	5, // duration60
	SFX_8059, // shootsound
	1, // penetration
	750, // initialrpm
	750, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootauto invfunc_ar34_usescope = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_103, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	NULL, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.4, // damage
	8, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	5, // duration60
	SFX_8059, // shootsound
	1, // penetration
	750, // initialrpm
	750, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_ar34 = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	invanim_ar34_reload, // reload animation
	0, // flags
};

struct weapon invitem_ar34 = {
	FILE_GAR34, // hi model
	FILE_GAR34LOD, // lo model
	invanim_ar34_reload, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_ar34_burstfire, &invfunc_ar34_usescope }, // functions
	&invammo_ar34, // pri ammo
	NULL, // sec ammo
	&invaimsettings_ar34,
	1, // muzzlez
	11.5, // posx
	-25.7, // posy
	-30.5, // posz
	1, // sway
	gunviscmds_ar34, // gunviscmds
	invpartvisibility_ar34, // part visibility
	L_GUN_016, // short name
	L_GUN_016, // name
	L_GUN_000, // manufacturer
	L_GUN_165, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN | WEAPONFLAG_04000000,
};

struct gunviscmd gunviscmds_k7avenger[] = {
	gunviscmd_sethidden(MODELPART_K7AVENGER_MAGAZINE2)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_k7avenger[] = {
	{ MODELPART_K7AVENGER_MAGAZINE2, false },
	{ MODELPART_GUN_MUZZLEFLASH1,    false },
	{ 255 },
};

struct guncmd invanim_k7avenger_reload[] = {
	gunscript_playanimation(ANIM_GUN_K7AVENGER_RELOAD, 0, 10000)
	gunscript_setsoundspeed(15, 1200)
	gunscript_playsound(15, SFX_0053)
	gunscript_hidepart(22, MODELPART_K7AVENGER_MAGAZINE1)
	gunscript_showpart(48, MODELPART_K7AVENGER_MAGAZINE2)
	gunscript_waittime(68, 1)
	gunscript_hidepart(69, MODELPART_K7AVENGER_MAGAZINE2)
	gunscript_showpart(69, MODELPART_K7AVENGER_MAGAZINE1)
	gunscript_setsoundspeed(69, 1400)
	gunscript_playsound(69, SFX_8025)
	gunscript_playsound(91, SFX_05C5)
	gunscript_end
};

struct guncmd invanim_k7avenger_equip[] = {
	gunscript_playanimation(ANIM_GUN_K7AVENGER_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_k7avenger_unequip[] = {
	gunscript_playanimation(ANIM_GUN_K7AVENGER_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_unused_8007ce6c[] = {
	gunscript_playanimation(ANIM_00E8, 0, 10000)
	gunscript_end
};

struct invaimsettings invaimsettings_k7avenger = {
	20,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_DEFAULT,
	5,
	INVAIMFLAG_AUTOAIM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weaponfunc_shootauto invfunc_k7avenger_burstfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_087, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	NULL, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.5, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_805A, // shootsound
	1, // penetration
	950, // initialrpm
	950, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_shootauto invfunc_k7avenger_threatdetector = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_119, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	NULL, // fire animation
	FUNCFLAG_BURST3 | FUNCFLAG_THREATDETECTOR,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.5, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_805A, // shootsound
	1, // penetration
	950, // initialrpm
	950, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_k7avenger = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	25, // clip size
	invanim_k7avenger_reload, // reload animation
	0, // flags
};

struct weapon invitem_k7avenger = {
	FILE_GK7AVENGER, // hi model
	FILE_GAVENGERLOD, // lo model
	invanim_k7avenger_equip, // equip animation
	invanim_k7avenger_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_k7avenger_burstfire, &invfunc_k7avenger_threatdetector }, // functions
	&invammo_k7avenger, // pri ammo
	NULL, // sec ammo
	&invaimsettings_k7avenger,
	1, // muzzlez
	6.5, // posx
	-24, // posy
	-27, // posz
	1, // sway
	gunviscmds_k7avenger, // gunviscmds
	invpartvisibility_k7avenger, // part visibility
	L_GUN_019, // short name
	L_GUN_019, // name
	L_GUN_149, // manufacturer
	L_GUN_168, // description
#if VERSION >= VERSION_NTSC_1_0
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
#else
	// NTSC beta uses "an" in "Picked up an K7 Avenger"
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN,
#endif
};

struct gunviscmd gunviscmds_laptopgun[] = {
	gunviscmd_sethidden(MODELPART_LAPTOPGUN_MAGAZINE2)
	gunviscmd_sethidden(MODELPART_LAPTOPGUN_MAGAZINE1)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_laptopgun[] = {
	{ MODELPART_GUN_MUZZLEFLASH1,    false },
	{ MODELPART_LAPTOPGUN_MAGAZINE1, false },
	{ MODELPART_LAPTOPGUN_MAGAZINE2, false },
	{ 255 },
};

struct guncmd invanim_laptopgun_reload[] = {
	gunscript_playanimation(ANIM_GUN_LAPTOP_RELOAD, 0, 10000)
	gunscript_setsoundspeed(4, 1630)
	gunscript_playsound(4, SFX_RELOAD_04FB)
	gunscript_setsoundspeed(15, 1436)
	gunscript_playsound(15, SFX_04FA)
	gunscript_playsound(25, SFX_04F7)
	gunscript_showpart(40, MODELPART_LAPTOPGUN_MAGAZINE1)
	gunscript_playsound(42, SFX_0053)
	gunscript_showpart(48, MODELPART_LAPTOPGUN_MAGAZINE2)
	gunscript_hidepart(55, MODELPART_LAPTOPGUN_MAGAZINE1)
	gunscript_hidepart(63, MODELPART_LAPTOPGUN_MAGAZINE2)
	gunscript_showpart(63, MODELPART_LAPTOPGUN_MAGAZINE1)
	gunscript_playsound(63, SFX_04F8)
	gunscript_hidepart(85, MODELPART_LAPTOPGUN_MAGAZINE1)
	gunscript_waittime(85, 1)
	gunscript_playsound(85, SFX_05C5)
	gunscript_playsound(91, SFX_04F9)
	gunscript_end
};

struct guncmd invanim_laptopgun_shoot[] = {
	gunscript_playanimation(ANIM_GUN_LAPTOP_SHOOT, 0, 10000)
	gunscript_waittime(10, 5)
	gunscript_waitforzreleased(10)
	gunscript_end
};

struct guncmd invanim_laptopgun_equip[] = {
	gunscript_playanimation(ANIM_GUN_LAPTOP_EQUIP, 0, 10000)
	gunscript_setsoundspeed(15, 720)
	gunscript_playsound(15, SFX_04F7)
	gunscript_playsound(35, SFX_04F7)
	gunscript_playsound(58, SFX_04F8)
	gunscript_end
};

struct guncmd invanim_laptopgun_unequip[] = {
	gunscript_playanimation(ANIM_GUN_LAPTOP_UNEQUIP, 0, 10000)
	gunscript_playsound(1, SFX_04F7)
	gunscript_setsoundspeed(25, 720)
	gunscript_playsound(25, SFX_04F7)
	gunscript_playsound(50, SFX_04F8)
	gunscript_end
};

struct weaponfunc_shootauto invfunc_laptopgun_burstfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_087, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loud,
	invanim_laptopgun_shoot, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.15, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8044, // shootsound
	1, // penetration
	1000, // initialrpm
	1000, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weaponfunc_throw invfunc_laptopgun_deploy = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_117, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_STICKTOWALL | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_DISCARDWEAPON | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRAUTOGUN, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_laptopgun = {
	AMMOTYPE_SMG,
	CASING_NONE,
	50, // clip size
	invanim_laptopgun_reload, // reload animation
	0, // flags
};

struct weapon invitem_laptopgun = {
	FILE_GPCGUN, // hi model
	FILE_GPCGUNLOD, // lo model
	invanim_laptopgun_equip, // equip animation
	invanim_laptopgun_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_laptopgun_burstfire, &invfunc_laptopgun_deploy }, // functions
	&invammo_laptopgun, // pri ammo
	NULL, // sec ammo
	&invaimsettings_heavy,
	1.2, // muzzlez
	16, // posx
	-17.7, // posy
	-14.5, // posz
	1, // sway
	gunviscmds_laptopgun, // gunviscmds
	invpartvisibility_laptopgun, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_251, // short name
#else
	L_GUN_024, // short name
#endif
	L_GUN_024, // name
	L_GUN_150, // manufacturer
	L_GUN_172, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_04000000,
};

struct gunviscmd gunviscmds_shotgun[] = {
	gunviscmd_sethidden(MODELPART_SHOTGUN_CART)
	gunviscmd_end
};

struct modelpartvisibility invpartvisibility_shotgun[] = {
	{ MODELPART_SHOTGUN_CART,     false },
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ 255 },
};

struct guncmd invanim_shotgun_reload[] = {
	gunscript_playanimation(ANIM_GUN_SHOTGUN_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_SHOTGUN_CART)
	gunscript_playsound(67, SFX_01D8)
	gunscript_waittime(74, 1)
	gunscript_hidepart(75, MODELPART_SHOTGUN_CART)
	gunscript_repeatuntilfull(0x0054, 0x0000, 0x0032)
	gunscript_playsound(103, SFX_RELOAD_04FB)
	gunscript_end
};

struct guncmd invanim_shotgun_singleshot[] = {
	gunscript_playanimation(ANIM_GUN_SHOTGUN_SHOOT_SINGLE, 0, 10000)
	gunscript_waittime(9, 2)
	gunscript_playsound(34, SFX_RELOAD_04FB)
	gunscript_end
};

struct guncmd invanim_shotgun_doubleshot[] = {
	gunscript_playanimation(ANIM_GUN_SHOTGUN_SHOOT_DOUBLE, 0, 10000)
	gunscript_waittime(9, 2)
	gunscript_playsound(54, SFX_RELOAD_04FB)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_shotgun_single = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_089, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_shotgun_singleshot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	0.6, // damage
	30, // spread
	20, 28, 0, 0,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	4, // impactforce
	0, // duration60
	SFX_FIRE_SHOTGUN, // shootsound
	1, // penetration
};

struct weaponfunc_shootsingle invfunc_shotgun_double = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_105, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_shotgun_doubleshot, // fire animation
	FUNCFLAG_BURST2,
	&invrecoilsettings_default,
	0, // recoverytime60
	0.6, // damage
	16, // spread
	20, 28, 0, 0,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	4, // impactforce
	0, // duration60
	SFX_FIRE_SHOTGUN, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_shotgun = {
	AMMOTYPE_SHOTGUN,
	CASING_SHOTGUN,
	9, // clip size
	invanim_shotgun_reload, // reload animation
	AMMOFLAG_INCREMENTALRELOAD, // flags
};

struct weapon invitem_shotgun = {
	FILE_GSHOTGUN, // hi model
	FILE_GSHOTGUNLOD, // lo model
	invanim_shotgun_singleshot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_shotgun_single, &invfunc_shotgun_double }, // functions
	&invammo_shotgun, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12, // posx
	-16.7, // posy
	-21, // posz
	1, // sway
	gunviscmds_shotgun, // gunviscmds
	invpartvisibility_shotgun, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_253, // short name
#else
	L_GUN_025, // short name
#endif
	L_GUN_025, // name
	L_GUN_149, // manufacturer
	L_GUN_173, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

f32 vibrationstart_reaper[] = {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
f32 vibrationmax_reaper[] = {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};

struct modelpartvisibility invpartvisibility_reaper[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ MODELPART_GUN_MUZZLEFLASH2, false },
	{ MODELPART_GUN_MUZZLEFLASH3, false },
	{ MODELPART_REAPER_MAGAZINE2, false },
	{ 255 },
};

struct gunviscmd gunviscmds_reaper[] = {
	gunviscmd_sethidden(MODELPART_REAPER_MAGAZINE2)
	gunviscmd_end
};

struct guncmd invanim_reaper_shoot[] = {
	gunscript_playanimation(ANIM_GUN_REAPER_SHOOT, 0, 10000)
	gunscript_waittime(12, 5)
	gunscript_end
};

struct guncmd invanim_reaper_reload[] = {
	gunscript_playanimation(ANIM_GUN_REAPER_RELOAD, 0, 10000)
	gunscript_playsound(49, SFX_05C7)
	gunscript_hidepart(70, MODELPART_REAPER_MAGAZINE1)
	gunscript_showpart(90, MODELPART_REAPER_MAGAZINE2)
	gunscript_playsound(124, SFX_05C6)
	gunscript_showpart(134, MODELPART_REAPER_MAGAZINE1)
	gunscript_hidepart(134, MODELPART_REAPER_MAGAZINE2)
	gunscript_waittime(134, 1)
	gunscript_end
};

struct guncmd invanim_reaper_equip[] = {
	gunscript_playanimation(ANIM_GUN_REAPER_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_reaper_unequip[] = {
	gunscript_playanimation(ANIM_GUN_REAPER_UNEQUIP, 0, 10000)
	gunscript_end
};

struct weaponfunc_shootauto invfunc_reaper_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_088, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_reaper,
	invanim_reaper_shoot, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_reaper,
	0, // recoverytime60
	1.2, // damage
	56, // spread
	3, 9, 32, 28,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	4, // impactforce
	2, // duration60
	SFX_803F, // shootsound
	1, // penetration
	60, // initialrpm
	1800,  // maxrpm
	vibrationstart_reaper, // vibrationstart
	vibrationmax_reaper, // vibrationmax
	88, // turretaccel
	88, // turretdecel
};

struct weaponfunc_melee invfunc_reaper_grind = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_106, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	0.05, // damage
	80, // range
	105, // unused
	105, // unused
	105, // unused
	-0.5, // unused
	-21.2, // unused
	-0.5, // unused
	-21.2, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct inventory_ammo invammo_reaper = {
	AMMOTYPE_REAPER,
	CASING_REAPER,
	200, // clip size
	invanim_reaper_reload, // reload animation
	0, // flags
};

struct weapon invitem_reaper = {
	FILE_GSKMINIGUN, // hi model
	FILE_GSKMINIGUNLOD, // lo model
	invanim_reaper_equip, // equip animation
	invanim_reaper_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_reaper_shoot, &invfunc_reaper_grind }, // functions
	&invammo_reaper, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	4, // posx
	-21.2, // posy
	-30.5, // posz
	1, // sway
	gunviscmds_reaper, // gunviscmds
	invpartvisibility_reaper, // part visibility
	L_GUN_026, // short name
	L_GUN_026, // name
	L_GUN_152, // manufacturer
	L_GUN_174, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
};

struct modelpartvisibility invpartvisibility_rocketlauncher[] = {
	{ MODELPART_ROCKETLAUNCHER_ROCKET, false },
	{ 255 },
};

struct gunviscmd gunviscmds_rocketlauncher[] = {
	gunviscmd_sethidden(MODELPART_ROCKETLAUNCHER_ROCKET)
	gunviscmd_end
};

struct guncmd invanim_rocketlauncher_reload[] = {
	gunscript_playanimation(ANIM_GUN_ROCKETLAUNCHER_RELOAD, 0, 10000)
	gunscript_setsoundspeed(16, 900)
	gunscript_playsound(16, SFX_04F9)
	gunscript_playsound(40, SFX_0053)
	gunscript_showpart(24, MODELPART_ROCKETLAUNCHER_ROCKET)
	gunscript_waittime(76, 1)
	gunscript_hidepart(76, MODELPART_ROCKETLAUNCHER_ROCKET)
	gunscript_playsound(77, SFX_05D1)
	gunscript_setsoundspeed(106, 1000)
	gunscript_playsound(106, SFX_04F9)
	gunscript_playsound(123, SFX_0053)
	gunscript_playsound(135, SFX_04FA)
	gunscript_end
};

struct guncmd invanim_rockerlauncher_shoot[] = {
	gunscript_playanimation(ANIM_GUN_ROCKETLAUNCHER_SHOOT, 0, 10000)
	gunscript_end
};

struct invaimsettings invaimsettings_rocketlauncher = {
	0,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_ROCKETLAUNCHER,
	0,
	INVAIMFLAG_AUTOAIM,
};

struct weaponfunc_shootprojectile invfunc_rockerlauncher_shoot = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_091, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_rockerlauncher_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_PROJECTILE_POWERED,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_LAUNCH_ROCKET_8053,
	1,
	MODEL_CHRDYROCKETMIS, // projectilemodelnum
	0, // unused
	2.1, // scale
	60, // speed
	0, // unk50
	0, // traveldist
	-1, // timer60
	0.05, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct weaponfunc_shootprojectile invfunc_rocketlauncher_homing = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_092, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_rockerlauncher_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_PROJECTILE_POWERED | FUNCFLAG_HOMINGROCKET,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_LAUNCH_ROCKET_8053,
	1,
	MODEL_CHRDYROCKETMIS, // projectilemodelnum
	0, // unused
	2.1, // scale
	0, // speed
	0, // unk50
	5, // traveldist
	-1, // timer60
	0.05, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct inventory_ammo invammo_rocketlauncher = {
	AMMOTYPE_ROCKET,
	CASING_NONE,
	1, // clip size
	invanim_rocketlauncher_reload, // reload animation
	0, // flags
};

struct weapon invitem_rocketlauncher = {
	FILE_GDYROCKET, // hi model
	FILE_GROCKETLOD, // lo model
	invanim_rockerlauncher_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_rockerlauncher_shoot, &invfunc_rocketlauncher_homing }, // functions
	&invammo_rocketlauncher, // pri ammo
	NULL, // sec ammo
	&invaimsettings_rocketlauncher,
	1, // muzzlez
	24.5, // posx
	-25.2, // posy
	-30, // posz
	1, // sway
	gunviscmds_rocketlauncher, // gunviscmds
	invpartvisibility_rocketlauncher, // part visibility
	L_GUN_080, // short name
	L_GUN_027, // name
	L_GUN_149, // manufacturer
	L_GUN_175, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_AIMTRACK,
};

struct guncmd invanim_slayer_shoot[] = {
	gunscript_playanimation(ANIM_GUN_SLAYER_SHOOT, 0, 10000)
	gunscript_end
};

struct guncmd invanim_slayer_reload[] = {
	gunscript_playanimation(ANIM_GUN_SLAYER_RELOAD, 0, 10000)
	gunscript_setsoundspeed(52, 800)
	gunscript_playsound(52, SFX_0053)
	gunscript_playsound(79, SFX_EYESPYHIT)
	gunscript_setsoundspeed(126, 900)
	gunscript_playsound(126, SFX_RELOAD_04FB)
	gunscript_end
};

struct weaponfunc_shootprojectile invfunc_slayer_shoot = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_091, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_slayer_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_PROJECTILE_POWERED,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_8067,
	1,
	MODEL_CHRSKROCKETMIS, // projectilemodelnum
	0, // unused
	4.1, // scale
	10, // speed
	0, // unk50
	0, // traveldist
	-1, // timer60
	0.05, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct weaponfunc_shootprojectile invfunc_slayer_flybywire = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_093, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	NULL, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_FLYBYWIRE | FUNCFLAG_PROJECTILE_POWERED | FUNCFLAG_20000000,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_8067,
	1,
	MODEL_CHRSKROCKETMIS, // projectilemodelnum
	0, // unused
	4.1, // scale
	10, // speed
	0, // unk50
	0, // traveldist
	-1, // timer60
	0.05, // reflectangle
	SFX_SLAYER_WHIR, // soundnum
};

struct inventory_ammo invammo_slayer = {
	AMMOTYPE_ROCKET,
	CASING_NONE,
	1, // clip size
	invanim_slayer_reload, // reload animation
	0, // flags
};

struct weapon invitem_slayer = {
	FILE_GSKROCKET, // hi model
	FILE_GSKROCKETLOD, // lo model
	invanim_slayer_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_slayer_shoot, &invfunc_slayer_flybywire }, // functions
	&invammo_slayer, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	22.5, // posx
	-32, // posy
	-40.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_029, // short name
	L_GUN_029, // name
	L_GUN_152, // manufacturer
	L_GUN_177, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_02000000,
};

struct gunviscmd gunviscmds_devastator[] = {
	gunviscmd_sethidden(MODELPART_DEVASTATOR_MAGAZINE1)
	gunviscmd_end
};

struct guncmd invanim_devastator_shoot[] = {
	gunscript_playanimation(ANIM_GUN_DEVASTATOR_SHOOT, 0, 10000)
	gunscript_end
};

struct guncmd invanim_devastator_reload[] = {
	gunscript_playanimation(ANIM_GUN_DEVASTATOR_RELOAD, 0, 10000)
	gunscript_setsoundspeed(15, 600)
	gunscript_playsound(15, SFX_0053)
	gunscript_hidepart(22, MODELPART_DEVASTATOR_MAGAZINE2)
	gunscript_showpart(36, MODELPART_DEVASTATOR_MAGAZINE1)
	gunscript_setsoundspeed(47, 700)
	gunscript_playsound(47, SFX_05C5)
	gunscript_hidepart(47, MODELPART_DEVASTATOR_MAGAZINE1)
	gunscript_showpart(47, MODELPART_DEVASTATOR_MAGAZINE2)
	gunscript_waittime(47, 1)
	gunscript_playsound(66, SFX_05CC)
	gunscript_end
};

struct weaponfunc_shootprojectile invfunc_devastator_shoot = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_098, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_devastator_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_10000000 | FUNCFLAG_20000000,
	NULL,
	0,
	1, // damage
	6, // spread
	5, 58, -1, 0,
	5,
	8,
	0,
	4,
	4,
	SFX_8073,
	1,
	MODEL_CHRDEVGRENADE, // projectilemodelnum
	0, // unused
	1, // scale
	0, // speed
	0.08, // unk50
	40, // traveldist
	1200, // timer60
	0.3, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct weaponfunc_shootprojectile invfunc_devastator_wallhugger = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_099, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_devastator_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_STICKTOWALL | FUNCFLAG_10000000 | FUNCFLAG_20000000,
	NULL,
	0,
	1, // damage
	6, // spread
	5, 58, -1, 0,
	5,
	8,
	0,
	4,
	4,
	SFX_8073,
	1,
	MODEL_CHRDEVGRENADE, // projectilemodelnum
	0, // unused
	1, // scale
	0, // speed
	0.08, // unk50
	40, // traveldist
	360, // timer60
	0.3, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct inventory_ammo invammo_devastator = {
	AMMOTYPE_DEVASTATOR,
	CASING_NONE,
	8, // clip size
	invanim_devastator_reload, // reload animation
	AMMOFLAG_QTYAFFECTSPARTVIS, // flags
};

struct modelpartvisibility invpartvisibility_devastator[] = {
	{ MODELPART_DEVASTATOR_MAGAZINE1, false },
	{ 255 },
};

struct weapon invitem_devastator = {
	FILE_GDYDEVASTATOR, // hi model
	FILE_GDEVASTATORLOD, // lo model
	invanim_devastator_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_devastator_shoot, &invfunc_devastator_wallhugger }, // functions
	&invammo_devastator, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	19.5, // posx
	-25.5, // posy
	-29, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_devastator, // part visibility
	L_GUN_028, // short name
	L_GUN_028, // name
	L_GUN_149, // manufacturer
	L_GUN_176, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct weaponfunc invfunc_mine_threatdetector = {
	INVENTORYFUNCTYPE_NONE,
	L_GUN_119, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_THREATDETECTOR,
};

struct guncmd invanim_mine_equip[] = {
	gunscript_playanimation(ANIM_GUN_MINE_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_mine_unequip[] = {
	gunscript_playanimation(ANIM_GUN_MINE_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_mine_throw[] = {
	gunscript_playanimation(ANIM_GUN_MINE_THROW, 0, 10000)
	gunscript_waitforzreleased(10)
	gunscript_waittime(13, 2)
	gunscript_end
};

struct weaponfunc_throw invfunc_timedmine_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_120, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_mine_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRTIMEDMINE, // projectilemodelnum
	240, // activatetime60
	0, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_timedmine = {
	AMMOTYPE_TIMED_MINE,
	CASING_NONE,
	1, // clip size
	invanim_mine_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE, // flags
};

struct weapon invitem_timedmine = {
	FILE_GTIMEDMINE, // hi model
	FILE_GTIMEDMINE, // lo model
	invanim_mine_equip, // equip animation
	invanim_mine_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_timedmine_throw, &invfunc_mine_threatdetector }, // functions
	&invammo_timedmine, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	8, // posx
	-15, // posy
	-23, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_255, // short name
#else
	L_GUN_038, // short name
#endif
	L_GUN_038, // name
	L_GUN_000, // manufacturer
	L_GUN_185, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_remotemine[] = {
	gunviscmd_checkinlefthand(GUNVISOP_SETVISIBILITY, MODELPART_HAND_LEFT)
	gunviscmd_checkinlefthand(GUNVISOP_SETVISIBILITY, MODELPART_REMOTEMINE_DETONATOR)
	gunviscmd_checkinrighthand(GUNVISOP_SETVISIBILITY, MODELPART_HAND_RIGHT)
	gunviscmd_checkinrighthand(GUNVISOP_SETVISIBILITY, MODELPART_REMOTEMINE_MINE)
	gunviscmd_end
};

struct guncmd invanim_remotemine_equip[] = {
	gunscript_playanimation(ANIM_GUN_REMOTEMINE_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_remotemine_unequip[] = {
	gunscript_playanimation(ANIM_GUN_REMOTEMINE_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_remotemine_throw[] = {
	gunscript_playanimation(ANIM_GUN_REMOTEMINE_THROW, 0, 10000)
	gunscript_waitforzreleased(10)
	gunscript_waittime(11, 2)
	gunscript_end
};

struct weaponfunc_throw invfunc_remotemine_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_122, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_remotemine_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRREMOTEMINE, // projectilemodelnum
	240, // activatetime60
	0, // recoverytime60
	0, // damage
};

struct weaponfunc_special invfunc_remotemine_detonate = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_123, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_AUTOSWITCHUNSELECTABLE,
	HANDATTACKTYPE_DETONATE,
	30, // recoverytime60
	0, // soundnum (unused)
};

struct inventory_ammo invammo_remotemine = {
	AMMOTYPE_REMOTE_MINE,
	CASING_NONE,
	1, // clip size
	invanim_remotemine_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE, // flags
};

struct modelpartvisibility invpartvisibility_remotemine[] = {
	{ MODELPART_REMOTEMINE_DETONATOR, false },
	{ 255 },
};

struct weapon invitem_remotemine = {
	FILE_GREMOTEMINE, // hi model
	FILE_GREMOTEMINE, // lo model
	invanim_remotemine_equip, // equip animation
	invanim_remotemine_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_remotemine_throw, &invfunc_remotemine_detonate }, // functions
	&invammo_remotemine, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	4, // posx
	-15, // posy
	-23, // posz
	1, // sway
	gunviscmds_remotemine,
	invpartvisibility_remotemine, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_252, // short name
#else
	L_GUN_040, // short name
#endif
	L_GUN_040, // name
	L_GUN_000, // manufacturer
	L_GUN_187, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
};

struct weaponfunc_throw invfunc_proxymine_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_121, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_mine_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRPROXIMITYMINE, // projectilemodelnum
	240, // activatetime60
	0, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_proximitymine = {
	AMMOTYPE_PROXY_MINE,
	CASING_NONE,
	1, // clip size
	invanim_mine_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE, // flags
};

struct weapon invitem_proximitymine = {
	FILE_GPROXIMITYMINE, // hi model
	FILE_GPROXIMITYMINE, // lo model
	invanim_mine_equip, // equip animation
	invanim_mine_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_proxymine_throw, &invfunc_mine_threatdetector }, // functions
	&invammo_proximitymine, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	8, // posx
	-15, // posy
	-23, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_081, // short name
	L_GUN_039, // name
	L_GUN_000, // manufacturer
	L_GUN_186, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
};

struct guncmd invanim_ecmmine_equip[] = {
	gunscript_playanimation(ANIM_GUN_ECMMINE_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_ecmmine_unequip[] = {
	gunscript_playanimation(ANIM_GUN_ECMMINE_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_ecmmine_throw[] = {
	gunscript_playanimation(ANIM_GUN_ECMMINE_UNEQUIP, 0, 10000)
	gunscript_waittime(23, 2)
	gunscript_end
};

struct weaponfunc_throw invfunc_ecmmine_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_140, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_ecmmine_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRECMMINE, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_ecmmine = {
	AMMOTYPE_ECM_MINE,
	CASING_NONE,
	1, // clip size
	invanim_ecmmine_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE, // flags
};

struct weapon invitem_ecmmine = {
	FILE_GECMMINE, // hi model
	FILE_GECMMINE, // lo model
	invanim_ecmmine_equip, // equip animation
	invanim_ecmmine_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_ecmmine_throw, NULL }, // functions
	&invammo_ecmmine, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	14, // posx
	-25, // posy
	-31.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_JPN_FINAL
	L_GUN_256, // short name
#else
	L_GUN_041, // short name
#endif
	L_GUN_041, // name
	L_GUN_000, // manufacturer
	L_GUN_188, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN | WEAPONFLAG_UNDROPPABLE,
};

// Some unfinished Reaper-like gun. The name is "Timed Mine". The primary
// function is detonate (and it works). It can't shoot.
struct weapon invitem_unused_8007dd88 = {
	FILE_GSKMINIGUN, // hi model
	FILE_GSKMINIGUNLOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_remotemine_detonate, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_038, // short name
	L_GUN_038, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_00000040 | WEAPONFLAG_UNDROPPABLE,
};

struct gunviscmd gunviscmds_grenade[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_grenade_throw[] = {
	gunscript_playanimation(ANIM_GUN_GRENADE_THROW, 0, 10000)
	gunscript_popoutsackofpills(6)
	gunscript_playsound(6, SFX_05C1)
	gunscript_waitforzreleased(43)
	gunscript_waittime(49, 2)
	gunscript_end
};

struct guncmd invanim_grenade_equip[] = {
	gunscript_playanimation(ANIM_GUN_GRENADE_EQUIP, 0, 10000)
	gunscript_end
};

struct weaponfunc_throw invfunc_grenade_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_124, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_grenade_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH,
	MODEL_CHRGRENADE, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct weaponfunc_throw invfunc_grenade_pinball = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_125, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_grenade_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH,
	MODEL_CHRGRENADE, // projectilemodelnum
	90, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_grenade = {
	AMMOTYPE_GRENADE,
	CASING_NONE,
	1, // clip size
	invanim_grenade_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE | AMMOFLAG_QTYAFFECTSPARTVIS, // flags
};

struct weapon invitem_grenade = {
	FILE_GGRENADE, // hi model
	FILE_GGRENADELOD, // lo model
	invanim_grenade_equip, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_grenade_throw, &invfunc_grenade_pinball }, // functions
	&invammo_grenade, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	17, // posx
	-19.7, // posy
	-21, // posz
	1, // sway
	gunviscmds_grenade, // gunviscmds
	NULL, // part visibility
	L_GUN_036, // short name
	L_GUN_036, // name
	L_GUN_000, // manufacturer
	L_GUN_183, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct weaponfunc_throw invfunc_nbomb_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_134, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_grenade_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_MAKEDIZZY | FUNCFLAG_DISARM | FUNCFLAG_NOMUZZLEFLASH,
	MODEL_CHRNBOMB, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct weaponfunc_throw invfunc_nbomb_proxy = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_127, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_grenade_throw, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_MAKEDIZZY | FUNCFLAG_DISARM | FUNCFLAG_NOMUZZLEFLASH,
	MODEL_CHRNBOMB, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_nbomb = {
	AMMOTYPE_NBOMB,
	CASING_NONE,
	1, // clip size
	invanim_grenade_equip, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE | AMMOFLAG_QTYAFFECTSPARTVIS, // flags
};

struct weapon invitem_nbomb = {
	FILE_GNBOMB, // hi model
	FILE_GNBOMBLOD, // lo model
	invanim_grenade_equip, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_nbomb_throw, &invfunc_nbomb_proxy }, // functions
	&invammo_nbomb, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	17, // posx
	-19.7, // posy
	-21, // posz
	1, // sway
	gunviscmds_grenade, // gunviscmds
	NULL, // part visibility
	L_GUN_037, // short name
	L_GUN_037, // name
	L_GUN_000, // manufacturer
	L_GUN_184, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN,
};

struct gunviscmd gunviscmds_farsight[] = {
	gunviscmd_sethidden(0x0028)
	gunviscmd_end
};

struct guncmd invanim_farsight_reload[] = {
	gunscript_playanimation(ANIM_GUN_FARSIGHT_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_FARSIGHT_ORB)
	gunscript_playsound(72, SFX_RELOAD_FARSIGHT)
	gunscript_waittime(76, 1)
	gunscript_hidepart(82, MODELPART_FARSIGHT_ORB)
	gunscript_end
};

struct guncmd invanim_farsight_shoot[] = {
	gunscript_playanimation(ANIM_GUN_FARSIGHT_SHOOT, 0, 10000)
	gunscript_end
};

struct invaimsettings invaimsettings_farsight = {
	0,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_DEFAULT,
	6,
	INVAIMFLAG_MANUALZOOM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weaponfunc_shootsingle invfunc_farsight_shoot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_090, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_farsight_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	100, // damage
	0, // spread
	70, 40, 0, 0,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_813E, // shootsound
	5, // penetration
};

struct weaponfunc_shootsingle invfunc_farsight_targetlocator = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_111, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_louder,
	invanim_farsight_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	100, // damage
	0, // spread
	70, 40, 0, 0,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_813E, // shootsound
	5, // penetration
};

struct inventory_ammo invammo_farsight = {
	AMMOTYPE_FARSIGHT,
	CASING_NONE,
	8, // clip size
	invanim_farsight_reload, // reload animation
	0, // flags
};

struct modelpartvisibility invpartvisibility_farsight[] = {
	{ MODELPART_FARSIGHT_ORB, false },
	{ 255 },
};

struct weapon invitem_farsight = {
	FILE_GZ2020, // hi model
	FILE_GZ2020LOD, // lo model
	invanim_farsight_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_farsight_shoot, &invfunc_farsight_targetlocator }, // functions
	&invammo_farsight, // pri ammo
	NULL, // sec ammo
	&invaimsettings_farsight,
	6, // muzzlez
	21.5, // posx
	-25.2, // posy
	-32.5, // posz
	1, // sway
	gunviscmds_farsight, // gunviscmds
	invpartvisibility_farsight, // part visibility
	L_GUN_079, // short name
	L_GUN_031, // name
	L_GUN_000, // manufacturer
	L_GUN_178, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_crossbow[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_sethidden(0x0029)
	gunviscmd_end
};

struct guncmd invanim_crossbow_reload[] = {
	gunscript_playanimation(ANIM_GUN_CROSSBOW_RELOAD, 0, 10000)
	gunscript_hidepart(1, MODELPART_CROSSBOW_0028)
	gunscript_showpart(1, MODELPART_CROSSBOW_BOLT)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_hidepart(19, MODELPART_CROSSBOW_0028)
	gunscript_showpart(19, MODELPART_CROSSBOW_BOLT)
	gunscript_playsound(45, SFX_04FC)
	gunscript_waittime(50, 1)
	gunscript_hidepart(50, MODELPART_CROSSBOW_BOLT)
	gunscript_showpart(50, MODELPART_CROSSBOW_0028)
	gunscript_repeatuntilfull(0x0044, 0x0000, 0x0013)
	gunscript_end
};

struct guncmd invanim_crossbow_shoot[] = {
	gunscript_playanimation(ANIM_GUN_CROSSBOW_SHOOT, 0, 10000)
	gunscript_hidepart(20, MODELPART_CROSSBOW_0028)
	gunscript_playsound(37, SFX_04F8)
	gunscript_end
};

struct guncmd invanim_crossbow_unequip[] = {
	gunscript_playanimation(ANIM_GUN_CROSSBOW_UNEQUIP, 0, 10000)
	gunscript_hidepart(1, MODELPART_CROSSBOW_0028)
	gunscript_playsound(37, SFX_04F8)
	gunscript_end
};

struct guncmd invanim_crosbow_equip[] = {
	gunscript_playanimation(ANIM_GUN_CROSSBOW_EQUIP, 0, 10000)
	gunscript_playsound(32, SFX_04FC)
	gunscript_playsound(75, SFX_04F8)
	gunscript_end
};

struct weaponfunc_shootprojectile invfunc_crossbow_lethal = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_112, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_crossbow_shoot, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	NULL,
	0,
	100, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_8056,
	1,
	MODEL_CHRCROSSBOLT, // projectilemodelnum
	0, // unused
	2.1, // scale
	0, // speed
	0, // unk50
	60, // traveldist
	-1, // timer60
	0.05, // reflectangle
	-1, // soundnum
};

struct weaponfunc_shootprojectile invfunc_crossbow_shoot = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_107, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_crossbow_shoot, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_8056,
	1,
	MODEL_CHRCROSSBOLT, // projectilemodelnum
	0, // unused
	2.1, // scale
	0, // speed
	0, // unk50
	60, // traveldist
	-1, // timer60
	0.05, // reflectangle
	-1, // soundnum
};

struct inventory_ammo invammo_crossbow = {
	AMMOTYPE_CROSSBOW,
	CASING_NONE,
	5, // clip size
	invanim_crossbow_reload, // reload animation
	AMMOFLAG_INCREMENTALRELOAD, // flags
};

struct modelpartvisibility invpartvisibility_crossbow[] = {
	{ MODELPART_CROSSBOW_0028,   false },
	{ MODELPART_CROSSBOW_BOLT,   false },
	{ MODELPART_CROSSBOW_HANDLE, false },
	{ 255 },
};

struct weapon invitem_crossbow = {
	FILE_GCROSSBOW, // hi model
	FILE_GCROSSBOWLOD, // lo model
	invanim_crosbow_equip, // equip animation
	invanim_crossbow_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_crossbow_shoot, &invfunc_crossbow_lethal }, // functions
	&invammo_crossbow, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	11, // posx
	-15, // posy
	-21, // posz
	1, // sway
	gunviscmds_crossbow, // gunviscmds
	invpartvisibility_crossbow, // part visibility
	L_GUN_033, // short name
	L_GUN_033, // name
	L_GUN_000, // manufacturer
	L_GUN_180, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_tranquilizer[] = {
	gunviscmd_sethidden(0x0028)
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_tranquilizer_lethalinject[] = {
	gunscript_playanimation(ANIM_GUN_TRANQUILIZER_INJECT, 0, 10000)
	gunscript_waittime(18, 2)
	gunscript_end
};

struct guncmd invanim_tranquilizer_shoot[] = {
	gunscript_playanimation(ANIM_GUN_TRANQUILIZER_SHOOT, 0, 10000)
	gunscript_waittime(7, 5)
	gunscript_end
};

struct guncmd invanim_tranquilizer_reload[] = {
	gunscript_playanimation(ANIM_GUN_TRANQUILIZER_RELOAD, 0, 10000)
	gunscript_showpart(1, MODELPART_HAND_LEFT)
	gunscript_showpart(1, MODELPART_TRANQUILIZER_MAGAZINE1)
	gunscript_showpart(1, MODELPART_TRANQUILIZER_MAGAZINE3)
	gunscript_hidepart(1, MODELPART_TRANQUILIZER_MAGAZINE2)
	gunscript_popoutsackofpills(1)
	gunscript_setsoundspeed(3, 3500)
	gunscript_playsound(3, SFX_DOOR_8016)
	gunscript_waittime(39, 1)
	gunscript_hidepart(39, MODELPART_TRANQUILIZER_MAGAZINE3)
	gunscript_showpart(39, MODELPART_TRANQUILIZER_MAGAZINE2)
	gunscript_setsoundspeed(39, 3500)
	gunscript_playsound(39, SFX_HIT_MUD_8083)
	gunscript_setsoundspeed(63, 2070)
	gunscript_playsound(63, SFX_04F5)
	gunscript_setsoundspeed(72, 3000)
	gunscript_playsound(72, SFX_PRESS_SWITCH)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_tranquilizer_shoot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_107, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_tranquilizer_shoot, // fire animation
	FUNCFLAG_MAKEDIZZY,
	&invrecoilsettings_default,
	16, // recoverytime60
	0.25, // damage
	3, // spread
	3, 5, 5, 0,
	1, // recoildist
	0, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_DRUGSPY_FIREDART, // shootsound
	1, // penetration
};

struct weaponfunc_melee invfunc_tranquilizer_lethal = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_108, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_tranquilizer_lethalinject, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	100, // damage
	60, // range
	105, // unused
	105, // unused
	105, // unused
	10, // unused
	-16.2, // unused
	10, // unused
	-16.2, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct inventory_ammo invammo_tranquilizer = {
	AMMOTYPE_SEDATIVE,
	CASING_NONE,
	8, // clip size
	invanim_tranquilizer_reload, // reload animation
	0, // flags
};

struct modelpartvisibility invpartvisibility_tranquilizer[] = {
	{ MODELPART_TRANQUILIZER_MAGAZINE1, false },
	{ MODELPART_TRANQUILIZER_MAGAZINE2, false },
	{ MODELPART_TRANQUILIZER_MAGAZINE3, false },
	{ 255 },
};

struct weapon invitem_tranquilizer = {
	FILE_GDRUGGUN, // hi model
	FILE_GDRUGGUNLOD, // lo model
	invanim_tranquilizer_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_tranquilizer_shoot, &invfunc_tranquilizer_lethal }, // functions
	&invammo_tranquilizer, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	10, // posx
	-15.2, // posy
	-24, // posz
	1, // sway
	gunviscmds_tranquilizer, // gunviscmds
	invpartvisibility_tranquilizer, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_247, // short name
#else
	L_GUN_034, // short name
#endif
	L_GUN_034, // name
	L_GUN_000, // manufacturer
	L_GUN_181, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct inventory_ammo invammo_psychosisgun = {
	AMMOTYPE_PSYCHOSIS,
	CASING_NONE,
	8, // clip size
	invanim_tranquilizer_reload, // reload animation
	0, // flags
};

struct weaponfunc_shootsingle invfunc_psychosisgun_shoot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_131, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silenced,
	invanim_tranquilizer_shoot, // fire animation
	FUNCFLAG_MAKEDIZZY | FUNCFLAG_PSYCHOSIS,
	&invrecoilsettings_default,
	16, // recoverytime60
	0.5, // damage
	3, // spread
	3, 5, 5, 0,
	1, // recoildist
	0, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_DRUGSPY_FIREDART, // shootsound
	1, // penetration
};

struct weapon invitem_psychosisgun = {
	FILE_GDRUGGUN, // hi model
	FILE_GDRUGGUNLOD, // lo model
	invanim_tranquilizer_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_psychosisgun_shoot, NULL }, // functions
	&invammo_psychosisgun, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	10, // posx
	-16.2, // posy
	-28.5, // posz
	1, // sway
	gunviscmds_tranquilizer, // gunviscmds
	invpartvisibility_tranquilizer, // part visibility
	L_GUN_082, // short name
	L_GUN_049, // name
	L_GUN_000, // manufacturer
	L_GUN_210, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS,
};

struct gunviscmd gunviscmds_sniperrifle[] = {
	gunviscmd_sethidden(0x0029)
	gunviscmd_end
};

struct guncmd invanim_sniperrifle_equip[] = {
	gunscript_playanimation(ANIM_GUN_SNIPER_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_sniperrifle_reload[] = {
	gunscript_playanimation(ANIM_GUN_SNIPER_RELOAD, 0, 10000)
	gunscript_setsoundspeed(22, 850)
	gunscript_playsound(22, SFX_01D8)
	gunscript_showpart(42, MODELPART_SNIPERRIFLE_MAGAZINE2)
	gunscript_hidepart(42, MODELPART_SNIPERRIFLE_MAGAZINE1)
	gunscript_setsoundspeed(72, 850)
#if VERSION >= VERSION_NTSC_1_0
	gunscript_playsound(72, SFX_80F6)
#else
	gunscript_playsound(72, SFX_01D9)
#endif
	gunscript_hidepart(72, MODELPART_SNIPERRIFLE_MAGAZINE2)
	gunscript_showpart(72, MODELPART_SNIPERRIFLE_MAGAZINE1)
	gunscript_waittime(72, 1)
	gunscript_end
};

struct invaimsettings invaimsettings_sniperrifle = {
	0,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_DEFAULT,
	5,
	INVAIMFLAG_MANUALZOOM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weaponfunc_shootsingle invfunc_sniperrifle_singleshot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_sniper,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	&invrecoilsettings_default,
	16, // recoverytime60
	1.2, // damage
	0, // spread
	6, 10, 0, 0,
	8, // recoildist
	0, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_8058, // shootsound
	1, // penetration
};

struct weaponfunc_special invfunc_sniperrifle_crouch = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_130, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_AUTOSWITCHUNSELECTABLE,
	HANDATTACKTYPE_CROUCH,
	30, // recoverytime60
	0, // soundnum (unused)
};

struct inventory_ammo invammo_sniperrifle = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	8, // clip size
	invanim_sniperrifle_reload, // reload animation
	0, // flags
};

struct modelpartvisibility invpartvisibility_sniperrifle[] = {
	{ MODELPART_SNIPERRIFLE_MAGAZINE2, false },
	{ 255 },
};

struct weapon invitem_sniperrifle = {
	FILE_GSNIPERRIFLE, // hi model
	FILE_GSNIPERLOD, // lo model
	invanim_sniperrifle_equip, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_sniperrifle_singleshot, &invfunc_sniperrifle_crouch }, // functions
	&invammo_sniperrifle, // pri ammo
	NULL, // sec ammo
	&invaimsettings_sniperrifle,
	6, // muzzlez
	21, // posx
	-27.2, // posy
	-31.5, // posz
	1, // sway
	gunviscmds_sniperrifle, // gunviscmds
	invpartvisibility_sniperrifle, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_254, // short name
#else
	L_GUN_032, // short name
#endif
	L_GUN_032, // name
	L_GUN_000, // manufacturer
	L_GUN_179, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS,
};

struct guncmd invanim_laser_equip[] = {
	gunscript_playanimation(ANIM_GUN_LASER_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_laser_unequip[] = {
	gunscript_playanimation(ANIM_GUN_LASER_UNEQUIP, 0, 10000)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_laser_pulse = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_132, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	0, // spread
	6, 18, 24, -1,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	SFX_8043, // shootsound
	1, // penetration
};

struct weaponfunc_shootauto invfunc_laser_stream = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_133, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_laserstream,
	0, // recoverytime60
	0.1, // damage
	0, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	3, // duration60
	0, // shootsound
	1, // penetration
	3600, // initialrpm
	3600, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct weapon invitem_laser = {
	FILE_GLASER, // hi model
	FILE_GLASERLOD, // lo model
	invanim_laser_equip, // equip animation
	invanim_laser_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_laser_pulse, &invfunc_laser_stream }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	3, // muzzlez
	-12, // posx
	-12.7, // posy
	-21.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_047, // short name
	L_GUN_047, // name
	L_GUN_150, // manufacturer
	L_GUN_189, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_DONTCOUNTSHOTS | WEAPONFLAG_04000000,
};

struct modelpartvisibility invpartvisibility_classic[] = {
	{ MODELPART_GUN_MUZZLEFLASH1, false },
	{ 255 },
};

struct gunviscmd gunviscmds_classicpistol[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_pp9i_shoot[] = {
	gunscript_playanimation(ANIM_GUN_PP9I_SHOOT, 0, 10000)
	gunscript_waittime(10, 5)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_pp9i_shoot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_pp9i_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	16, // recoverytime60
	1, // damage
	1, // spread
	4, 8, 3, 0,
	3, // recoildist
	10, // recoilangle
	29.999998092651, // slidemax
	0, // impactforce
	0, // duration60
	SFX_8069, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_pp9i = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	7, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_pp9i = {
	FILE_GWPPK, // hi model
	FILE_GWPPK, // lo model
	invanim_pp9i_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_pp9i_shoot, NULL }, // functions
	&invammo_pp9i, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	10, // posx
	-14.8, // posy
	-19, // posz
	1, // sway
	gunviscmds_classicpistol, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_050, // short name
	L_GUN_050, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct guncmd invanim_cc13_shoot[] = {
	gunscript_playanimation(ANIM_GUN_CC13_SHOOT, 0, 10000)
	gunscript_waittime(10, 5)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_cc13_shoot = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_085, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_cc13_shoot, // fire animation
	0, // flags
	&invrecoilsettings_default,
	16, // recoverytime60
	1, // damage
	6, // spread
	4, 8, 3, 0,
	5, // recoildist
	10, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_806A, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_cc13 = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	8, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_cc13 = {
	FILE_GTT33, // hi model
	FILE_GTT33, // lo model
	invanim_cc13_shoot, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_cc13_shoot, NULL }, // functions
	&invammo_cc13, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	10, // posx
	-17, // posy
	-26.5, // posz
	1, // sway
	gunviscmds_classicpistol, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_051, // short name
	L_GUN_051, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_GANGSTA,
};

struct weaponfunc_shootauto invfunc_kl01313_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	0.6, // damage
	15, // spread
	0, 0, 0, 6,
	0, // recoildist
	0, // recoilangle
	0, // slidemax
	0, // impactforce
	11, // duration60
	SFX_806B, // shootsound
	1, // penetration
	450, // initialrpm
	450, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_kl01313 = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	20, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_kl01313 = {
	FILE_GSKORPION, // hi model
	FILE_GSKORPION, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_kl01313_shoot, NULL }, // functions
	&invammo_kl01313, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	8.5, // posx
	-13, // posy
	-29.5, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_052, // short name
	L_GUN_052, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000,
};

struct weaponfunc_shootauto invfunc_kf7special_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_087, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	10, // spread
	4, 12, 0, 6,
	4.5, // recoildist
	2, // recoilangle
	0, // slidemax
	3, // impactforce
	4, // duration60
	SFX_806C, // shootsound
	1, // penetration
	450, // initialrpm
	450, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_kf7special = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_kf7special = {
	FILE_GAK47, // hi model
	FILE_GAK47, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_kf7special_shoot, NULL }, // functions
	&invammo_kf7special, // pri ammo
	NULL, // sec ammo
	&invaimsettings_heavy,
	2, // muzzlez
	13, // posx
	-21, // posy
	-13.5, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_053, // short name
	L_GUN_053, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000,
};

struct weaponfunc_shootauto invfunc_zzt9mm_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	9, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_806D, // shootsound
	1, // penetration
	600, // initialrpm
	600, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_zzt9mm = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	32, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_zzt9mm = {
	FILE_GUZI, // hi model
	FILE_GUZI, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_zzt9mm_shoot, NULL }, // functions
	&invammo_zzt9mm, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	0.5, // muzzlez
	11, // posx
	-24.5, // posy
	-37, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_054, // short name
	L_GUN_054, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000,
};

struct weaponfunc_shootauto invfunc_dmc_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	7, // spread
	6, 18, 0, 6,
	4, // recoildist
	3, // recoilangle
	0, // slidemax
	0, // impactforce
	4, // duration60
	SFX_806E, // shootsound
	1, // penetration
	450, // initialrpm
	450, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_dmc = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	30, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_dmc = {
	FILE_GMP5K, // hi model
	FILE_GMP5K, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_dmc_shoot, NULL }, // functions
	&invammo_dmc, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	1, // muzzlez
	16.5, // posx
	-22.9, // posy
	-35, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_055, // short name
	L_GUN_055, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000,
};

struct weaponfunc_shootauto invfunc_ar53_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_087, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	FUNCFLAG_BURST3,
	&invrecoilsettings_default,
	0, // recoverytime60
	1.4, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	5, // duration60
	SFX_806F, // shootsound
	1, // penetration
	550, // initialrpm
	550, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_ar53 = {
	AMMOTYPE_RIFLE,
	CASING_RIFLE,
	30, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_ar53 = {
	FILE_GM16, // hi model
	FILE_GM16, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_ar53_shoot, NULL }, // functions
	&invammo_ar53, // pri ammo
	NULL, // sec ammo
	&invaimsettings_heavy,
	6, // muzzlez
	11, // posx
	-19.2, // posy
	-21.5, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_056, // short name
	L_GUN_056, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000 | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN,
};

struct weaponfunc_shootauto invfunc_rcp45_shoot = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1.8, // damage
	6, // spread
	4, 8, 0, 3,
	3.5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	2, // duration60
	SFX_8070, // shootsound
	1, // penetration
	600, // initialrpm
	600, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_rcp45 = {
	AMMOTYPE_SMG,
	CASING_STANDARD,
	80, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_rcp45 = {
	FILE_GFNP90, // hi model
	FILE_GFNP90, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_rcp45_shoot, NULL }, // functions
	&invammo_rcp45, // pri ammo
	NULL, // sec ammo
	&invaimsettings_smg,
	4, // muzzlez
	11, // posx
	-22.3, // posy
	-29, // posz
	1, // sway
	NULL, // gunviscmds
	invpartvisibility_classic, // part visibility
	L_GUN_057, // short name
	L_GUN_057, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00008000 | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN,
};

struct weaponfunc invfunc_briefcase_use = {
	INVENTORYFUNCTYPE_NONE,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	0, // flags
};

struct gunviscmd gunviscmds_unused_8006edcc[] = {
	{ 0, 0x001e, 0, 0x0002, 0x0001 },
	gunviscmd_end
};

struct weapon invitem_briefcase2 = {
	FILE_GFALCON2LOD, // hi model
	FILE_GFALCON2LOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_briefcase_use, &invfunc_briefcase_use }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_071, // short name
	L_GUN_071, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_00000800 | WEAPONFLAG_HIDEMENUMODEL | WEAPONFLAG_DONTCOUNTSHOTS | WEAPONFLAG_DETERMINER_S_THE | WEAPONFLAG_DETERMINER_F_THE,
};

struct weaponfunc_shootauto invfunc_choppergun_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_8059, // shootsound
	2, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_choppergun = {
	AMMOTYPE_SMG,
	CASING_RIFLE,
	27, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_choppergun = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_choppergun_rapidfire, NULL }, // functions
	&invammo_choppergun, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	16.5, // posx
	-19.5, // posy
	-24, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_000, // short name
	L_GUN_000, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_DUALWIELD,
};

struct weaponfunc_shootauto invfunc_watchlaser_rapidfire = {
	INVENTORYFUNCTYPE_SHOOT_AUTOMATIC,
	L_GUN_086, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	NULL, // fire animation
	0, // flags
	&invrecoilsettings_default,
	0, // recoverytime60
	1, // damage
	6, // spread
	6, 18, 2, 8,
	5, // recoildist
	2, // recoilangle
	0, // slidemax
	4, // impactforce
	4, // duration60
	SFX_803F, // shootsound
	1, // penetration
	900, // initialrpm
	900, // maxrpm
	NULL, // vibrationstart
	NULL, // vibrationmax
	0, // turretaccel
	0, // turretdecel
};

struct inventory_ammo invammo_watchlaser = {
	AMMOTYPE_SMG,
	CASING_RIFLE,
	27, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_watchlaser = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_watchlaser_rapidfire, NULL }, // functions
	&invammo_watchlaser, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	16.5, // posx
	-19.5, // posy
	-24, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_000, // short name
	L_GUN_000, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_DUALWIELD,
};

struct gunviscmd gunviscmds_knife[] = {
	gunviscmd_sethidden(MODELPART_HAND_LEFT)
	gunviscmd_end
};

struct guncmd invanim_combatknife_equip[] = {
	gunscript_playanimation(ANIM_GUN_KNIFE_EQUIP, 0, 10000)
	gunscript_waittime(24, 2)
	gunscript_end
};

struct guncmd invanim_combatknife_slash2[] = {
	gunscript_playanimation(ANIM_GUN_KNIFE_SLASH, 0, 10000)
	gunscript_waittime(24, 2)
	gunscript_end
};

struct guncmd invanim_combatknife_slash[] = {
	gunscript_random(50, invanim_combatknife_slash2)
	gunscript_include(0, invanim_combatknife_equip)
	gunscript_end
};

struct guncmd invanim_combatknife_pritosec[] = {
	gunscript_playanimation(ANIM_GUN_KNIFE_CHANGEFUNC, 0, 10000)
	gunscript_playsound(30, SFX_80A7)
	gunscript_playsound(40, SFX_80A6)
	gunscript_end
};

struct guncmd invanim_combatknife_sectopri[] = {
	gunscript_playanimation(ANIM_GUN_KNIFE_CHANGEFUNC, 65535, 55536)
	gunscript_playsound(10, SFX_80A8)
	gunscript_playsound(20, SFX_80A6)
	gunscript_end
};

struct guncmd invanim_combatknife_throw[] = {
	gunscript_playanimation(ANIM_GUN_KNIFE_THROW, 0, 10000)
	gunscript_waitforzreleased(12)
	gunscript_waittime(16, 2)
	gunscript_end
};

struct guncmd invanim_unused_8007f05c[] = {
	gunscript_playanimation(ANIM_041C, 0, 10000)
	gunscript_waittime(22, 1)
	gunscript_end
};

struct guncmd invanim_combatknife_reload[] = {
	gunscript_include(2, invanim_combatknife_sectopri)
	gunscript_include(0, invanim_combatknife_pritosec)
	gunscript_end
};

struct weaponfunc_melee invfunc_combatknife_slash = {
	INVENTORYFUNCTYPE_MELEE,
	L_GUN_109, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_combatknife_slash, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	2, // damage
	70, // range
	105, // unused
	105, // unused
	105, // unused
	11, // unused
	-25.5, // unused
	11, // unused
	-25.5, // unused
	-1, // unused
	-1, // unused
	2, // unused
	2, // unused
	0, // unused
};

struct weaponfunc_throw invfunc_combatknife_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_110, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	invanim_combatknife_throw, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRKNIFE, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	1, // damage
};

struct inventory_ammo invammo_combatknife = {
	AMMOTYPE_KNIFE,
	CASING_NONE,
	1, // clip size
	invanim_combatknife_reload, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE | AMMOFLAG_QTYAFFECTSPARTVIS, // flags
};

struct weapon invitem_combatknife = {
	FILE_GKNIFE, // hi model
	FILE_GKNIFELOD, // lo model
	invanim_combatknife_equip, // equip animation
	NULL, // unequip animation
	invanim_combatknife_pritosec, // pritosec animation
	invanim_combatknife_sectopri, // sectopri animation
	{ &invfunc_combatknife_slash, &invfunc_combatknife_throw }, // functions
	&invammo_combatknife, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	18.5, // posx
	-26.5, // posy
	-28, // posz
	1, // sway
	gunviscmds_knife, // gunviscmds
	NULL, // part visibility
	L_GUN_083, // short name
	L_GUN_035, // name
	L_GUN_000, // manufacturer
	L_GUN_182, // description
	WEAPONFLAG_THROWABLE | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_TRACKTIMEUSED | WEAPONFLAG_00000400 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_00008000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_04000000,
};

struct weaponfunc_throw invfunc_bug_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_CHRBUG, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct weaponfunc_throw invfunc_targetamplifier_throw = {
	INVENTORYFUNCTYPE_THROW,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_CALCULATETRAJECTORY,
	MODEL_TARGETAMP, // projectilemodelnum
	240, // activatetime60
	60, // recoverytime60
	0, // damage
};

struct inventory_ammo invammo_bug = {
	AMMOTYPE_BUG,
	CASING_NONE,
	1, // clip size
	NULL, // reload animation
	AMMOFLAG_EQUIPPEDISRESERVE, // flags
};

struct weapon invitem_commsrider = {
	FILE_PCHRBUG, // hi model
	FILE_PCHRBUG, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_bug_throw, NULL }, // functions
	&invammo_bug, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_222, // short name
	L_GUN_222, // name
	L_GUN_000, // manufacturer
	L_GUN_204, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_tracerbug = {
	FILE_PCHRBUG, // hi model
	FILE_PCHRBUG, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_bug_throw, NULL }, // functions
	&invammo_bug, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_223, // short name
	L_GUN_223, // name
	L_GUN_000, // manufacturer
	L_GUN_205, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_targetamplifier = {
	FILE_PTARGETAMP, // hi model
	FILE_PTARGETAMP, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_targetamplifier_throw, NULL }, // functions
	&invammo_bug, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_235, // short name
	L_GUN_224, // name
	L_GUN_000, // manufacturer
	L_GUN_206, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_device invfunc_nightvision_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_135, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_NIGHTVISION,
};

struct weapon invitem_nightvision = {
	FILE_PCHRNIGHTSIGHT, // hi model
	FILE_PCHRNIGHTSIGHT, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_nightvision_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_244, // short name
#else
	L_GUN_059, // short name
#endif
	L_GUN_059, // name
	L_GUN_000, // manufacturer
	L_GUN_191, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000080 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc invfunc_horizonscanner_primary = {
	INVENTORYFUNCTYPE_NONE,
	L_GUN_139, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	0, // flags
};

struct invaimsettings invaimsettings_horizonscanner = {
	0,
	3,
	8,
	15,
	0.9721,
	0.9767,
	SIGHTTRACKTYPE_NONE,
	5,
	INVAIMFLAG_MANUALZOOM | INVAIMFLAG_ACCURATESINGLESHOT,
};

struct weapon invitem_horizonscanner = {
	FILE_PBINOCULARS, // hi model
	FILE_PBINOCULARS, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_horizonscanner_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_horizonscanner,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_234, // short name
	L_GUN_076, // name
	L_GUN_000, // manufacturer
	L_GUN_196, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_device invfunc_cloak_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_116, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_CLOAKDEVICE,
};

struct inventory_ammo invammo_cloakingdevice = {
	AMMOTYPE_CLOAK,
	CASING_NONE,
	10, // clip size
	NULL, // reload animation
	AMMOFLAG_NORESERVE, // flags
};

struct weapon invitem_cloakingdevice = {
	FILE_PCHRCLOAKER, // hi model
	FILE_PCHRCLOAKER, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_cloak_primary, NULL }, // functions
	&invammo_cloakingdevice, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_073, // short name
	L_GUN_073, // name
	L_GUN_000, // manufacturer
	L_GUN_195, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_04000000,
};

struct weaponfunc_special invfunc_combatboost_boost = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_113, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	HANDATTACKTYPE_BOOST,
	30, // recoverytime60
	SFX_JO_BOOST_ACTIVATE, // soundnum (unused)
};

struct weaponfunc_special invfunc_combatboost_revert = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_114, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	HANDATTACKTYPE_REVERTBOOST,
	30, // recoverytime60
	SFX_JO_BOOST_ACTIVATE, // soundnum (unused)
};

struct inventory_ammo invammo_combatboost = {
	AMMOTYPE_BOOST,
	CASING_NONE,
	4, // clip size
	NULL, // reload animation
	AMMOFLAG_NORESERVE, // flags
};

struct weapon invitem_combatboost = {
	FILE_PCHRSPEEDPILL, // hi model
	FILE_PCHRSPEEDPILL, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_combatboost_boost, &invfunc_combatboost_revert }, // functions
	&invammo_combatboost, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_232, // short name
	L_GUN_074, // name
	L_GUN_000, // manufacturer
	L_GUN_190, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_04000000 | WEAPONFLAG_DETERMINER_S_SOME | WEAPONFLAG_DETERMINER_F_SOME,
};

struct weaponfunc_device invfunc_suicidepill_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_SUICIDEPILL,
};

struct weapon invitem_suicidepill = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_suicidepill_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_246, // short name
#else
	L_GUN_072, // short name
#endif
	L_GUN_072, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_device invfunc_irscanner_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_138, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_IRSCANNER,
};

struct weapon invitem_irscanner = {
	FILE_GIRSCANNER, // hi model
	FILE_GIRSCANNER, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_irscanner_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_069, // short name
	L_GUN_069, // name
	L_GUN_000, // manufacturer
	L_GUN_194, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_DUALWIELD | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc invfunc_disguise_primary = {
	INVENTORYFUNCTYPE_NONE,
	L_GUN_143, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	0, // flags
};

struct weapon invitem_disguise40 = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_disguise_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_250, // short name
#else
	L_GUN_043, // short name
#endif
	L_GUN_043, // name
	L_GUN_000, // manufacturer
	L_GUN_211, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_disguise41 = {
	0, // hi model
	0, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_disguise_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_250, // short name
#else
	L_GUN_043, // short name
#endif
	L_GUN_043, // name
	L_GUN_000, // manufacturer
	L_GUN_211, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weaponfunc_device invfunc_camspy_deploy = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_136, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_EYESPY,
};

struct weapon invitem_camspy = {
	FILE_CEYESPY, // hi model
	FILE_CEYESPY, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_camspy_deploy, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_060, // short name
	L_GUN_060, // name
	L_GUN_000, // manufacturer
	L_GUN_192, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALWIELD | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_device invfunc_rtracker_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_142, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_RTRACKER,
};

struct weapon invitem_rtracker = {
	FILE_PRUSSDAR, // hi model
	FILE_PRUSSDAR, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_rtracker_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_070, // short name
	L_GUN_070, // name
	L_GUN_000, // manufacturer
	L_GUN_198, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALWIELD | WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_device invfunc_xray_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_137, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_XRAYSCANNER,
};

struct weapon invitem_xrayscanner = {
	FILE_PXRAYSPECS, // hi model
	FILE_PXRAYSPECS, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_xray_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_233, // short name
	L_GUN_065, // name
	L_GUN_000, // manufacturer
	L_GUN_193, // description
#if VERSION >= VERSION_NTSC_1_0
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN | WEAPONFLAG_UNDROPPABLE,
#else
	// NTSC beta uses "a" in "Picked up a X-ray scanner"
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_00000040 | WEAPONFLAG_UNDROPPABLE,
#endif
};

struct guncmd invanim_datauplink_equip[] = {
	gunscript_playanimation(ANIM_GUN_UPLINK_EQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_datauplink_unequip[] = {
	gunscript_playanimation(ANIM_GUN_UPLINK_UNEQUIP, 0, 10000)
	gunscript_end
};

struct guncmd invanim_unused_8007f794[] = {
	gunscript_playanimation(ANIM_043B, 0, 10000)
	gunscript_end
};

struct weaponfunc_special invfunc_datauplink_primary = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_141, // name
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_AUTOSWITCHUNSELECTABLE,
	HANDATTACKTYPE_UPLINK,
	30, // recoverytime60
	0, // soundnum (unused)
};

struct weapon invitem_datauplink = {
	FILE_GCOMMSUPLINK, // hi model
	FILE_GCOMMSUPLINK, // lo model
	invanim_datauplink_equip, // equip animation
	invanim_datauplink_unequip, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_datauplink_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	8, // posx
	-16.2, // posy
	-17.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_243, // short name
#else
	L_GUN_075, // short name
#endif
	L_GUN_075, // name
	L_GUN_000, // manufacturer
	L_GUN_197, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_00000040 | WEAPONFLAG_00002000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_doordecoder = {
	FILE_PCHRDOORDECODER, // hi model
	FILE_PCHRDOORDECODER, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	35, // posx
	-26.7, // posy
	-41, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_245, // short name
#else
	L_GUN_063, // short name
#endif
	L_GUN_063, // name
	L_GUN_000, // manufacturer
	L_GUN_200, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_rocket = {
	FILE_GCYCLONE, // hi model
	FILE_GCYCLONELOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	&invammo_default, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_044, // short name
	L_GUN_044, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	0, // flags
};

struct weapon invitem_homingrocket = {
	FILE_GCYCLONE, // hi model
	FILE_GCYCLONELOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	&invammo_default, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_045, // short name
	L_GUN_045, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	0, // flags
};

struct weapon invitem_grenaderound = {
	FILE_GCYCLONE, // hi model
	FILE_GCYCLONELOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_devastator_shoot, &invfunc_devastator_wallhugger }, // functions
	&invammo_default, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_046, // short name
	L_GUN_046, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	0, // flags
};

struct weapon invitem_bolt = {
	FILE_GCYCLONE, // hi model
	FILE_GCYCLONELOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_crossbow_shoot, &invfunc_crossbow_lethal }, // functions
	&invammo_default, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	12.5, // posx
	-17, // posy
	-27.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_048, // short name
	L_GUN_048, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	0, // flags
};

struct weaponfunc_special invfunc_unused_8006f9a4 = {
	INVENTORYFUNCTYPE_SPECIAL,
	L_GUN_144, // "Place Explosives"
	0, // unused
	-1, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH | FUNCFLAG_AUTOSWITCHUNSELECTABLE,
	HANDATTACKTYPE_UPLINK,
	30, // recoverytime60
	0, // soundnum (unused)
};

struct weapon invitem_skedarbomb = {
	FILE_PSKEDARBOMB, // hi model
	FILE_PSKEDARBOMB, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_JPN_FINAL
	L_GUN_259, // short name
#else
	L_GUN_221, // short name
#endif
	L_GUN_221, // name
	L_GUN_000, // manufacturer
	L_GUN_203, // description
	WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_explosives = {
	FILE_PEXPLOSIVEBRICK, // hi model
	FILE_PEXPLOSIVEBRICK, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_064, // short name
	L_GUN_064, // name
	L_GUN_000, // manufacturer
	L_GUN_202, // description
	WEAPONFLAG_FIRETOACTIVATE,
};

struct weaponfunc_device invfunc_presidentscanner_primary = {
	INVENTORYFUNCTYPE_DEVICE,
	L_GUN_142, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_silent,
	NULL, // fire animation
	FUNCFLAG_NOMUZZLEFLASH,
	DEVICE_RTRACKER,
};

struct weapon invitem_presidentscanner = {
	FILE_PRUSSDAR, // hi model
	FILE_PRUSSDAR, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_presidentscanner_primary, NULL }, // functions
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	0, // posx
	-39.5, // posy
	-55.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_231, // short name
	L_GUN_219, // name
	L_GUN_000, // manufacturer
	L_GUN_199, // description
	WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALWIELD | WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_autosurgeon = {
	FILE_PAUTOSURGEON, // hi model
	FILE_PAUTOSURGEON, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_248, // short name
#else
	L_GUN_220, // short name
#endif
	L_GUN_220, // name
	L_GUN_000, // manufacturer
	L_GUN_201, // description
	WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_flightplans = {
	FILE_PSAFEITEM, // hi model
	FILE_PSAFEITEM, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_225, // short name
	L_GUN_225, // name
	L_GUN_000, // manufacturer
	L_GUN_207, // description
	WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_researchtape = {
	FILE_PRESEARCHTAPE, // hi model
	FILE_PRESEARCHTAPE, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_226, // short name
	L_GUN_226, // name
	L_GUN_000, // manufacturer
	L_GUN_208, // description
	WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_backupdisk = {
	FILE_PZIGGYCARD, // hi model
	FILE_PZIGGYCARD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_PAL_FINAL
	L_GUN_249, // short name
#else
	L_GUN_227, // short name
#endif
	L_GUN_227, // name
	L_GUN_000, // manufacturer
	L_GUN_209, // description
	WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_briefcase = {
	FILE_PKEYCARD, // hi model
	FILE_PKEYCARD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_229, // short name
	L_GUN_229, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_HIDEMENUMODEL | WEAPONFLAG_UNDROPPABLE,
};

struct weapon invitem_suitcase = {
	FILE_PCHRBRIEFCASE, // hi model
	FILE_PCHRBRIEFCASE, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
#if VERSION >= VERSION_JPN_FINAL
	L_GUN_258, // short name
#else
	L_GUN_067, // short name
#endif
	L_GUN_067, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_UNDROPPABLE | WEAPONFLAG_FIRETOACTIVATE,
};

struct weapon invitem_necklace = {
	FILE_PCHRCHAIN, // hi model
	FILE_PCHRCHAIN, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_230, // short name
	L_GUN_230, // name
	L_GUN_000, // manufacturer
	L_GUN_238, // description
	WEAPONFLAG_UNDROPPABLE,
};

#if VERSION >= VERSION_NTSC_1_0
struct weapon invitem_shieldtechitem = {
	FILE_PCHRSHIELD, // hi model
	FILE_PCHRSHIELD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_240, // short name
	L_GUN_240, // name
	L_GUN_000, // manufacturer
	L_GUN_241, // description
	WEAPONFLAG_UNDROPPABLE,
};
#endif

struct weapon invitem_keycard = {
	FILE_PKEYCARD, // hi model
	FILE_PKEYCARD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	NULL, // pri function
	NULL, // sec function
	NULL, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_228, // short name
	L_GUN_228, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_UNDROPPABLE,
};

struct weaponfunc_shootprojectile invfunc_rocketlauncher34_primary = {
	INVENTORYFUNCTYPE_SHOOT_PROJECTILE,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_loudest,
	invanim_rockerlauncher_shoot, // fire animation
	FUNCFLAG_NOAUTOAIM | FUNCFLAG_PROJECTILE_POWERED,
	NULL,
	0,
	1, // damage
	6, // spread
	3, 12, 15, 0,
	3,
	2,
	0,
	4,
	4,
	SFX_LAUNCH_ROCKET_8053,
	1,
	MODEL_CHRDYROCKETMIS, // projectilemodelnum
	0, // unused
	2.1, // scale
	20, // speed
	0, // unk50
	0, // traveldist
	-1, // timer60
	0.05, // reflectangle
	SFX_LAUNCH_ROCKET_8053, // soundnum
};

struct inventory_ammo invammo_rocketlauncher34 = {
	AMMOTYPE_ROCKET,
	CASING_NONE,
	1, // clip size
	invanim_rocketlauncher_reload, // reload animation
	0, // flags
};

struct weapon invitem_rocketlauncher_34 = {
	FILE_GDYROCKET, // hi model
	FILE_GROCKETLOD, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_rocketlauncher34_primary, NULL }, // functions
	&invammo_rocketlauncher34, // pri ammo
	NULL, // sec ammo
	&invaimsettings_rocketlauncher,
	1, // muzzlez
	24.5, // posx
	-25.2, // posy
	-30, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_080, // short name
	L_GUN_027, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_00002000 | WEAPONFLAG_00004000 | WEAPONFLAG_HASHANDS | WEAPONFLAG_AIMTRACK,
};

struct guncmd invanim_tester_shoot[] = {
	gunscript_playanimation(ANIM_GUN_TESTER_SHOOT, 0, 10000)
	gunscript_end
};

struct weaponfunc_shootsingle invfunc_tester_primary = {
	INVENTORYFUNCTYPE_SHOOT_SINGLE,
	L_GUN_000, // name
	0, // unused
	0, // ammoindex
	&invnoisesettings_default,
	invanim_tester_shoot, // fire animation
	0, // flags
	&invrecoilsettings_tester,
	16, // recoverytime60
	1, // damage
	6, // spread
	4, 8, 3, 0,
	0, // recoildist
	0, // recoilangle
	59.999996185303, // slidemax
	0, // impactforce
	0, // duration60
	SFX_804D, // shootsound
	1, // penetration
};

struct inventory_ammo invammo_tester = {
	AMMOTYPE_PISTOL,
	CASING_STANDARD,
	8, // clip size
	NULL, // reload animation
	0, // flags
};

struct weapon invitem_tester = {
	FILE_GTESTGUN, // hi model
	FILE_GTESTGUN, // lo model
	NULL, // equip animation
	NULL, // unequip animation
	NULL, // pritosec animation
	NULL, // sectopri animation
	{ &invfunc_tester_primary, NULL }, // functions
	&invammo_tester, // pri ammo
	NULL, // sec ammo
	&invaimsettings_default,
	1, // muzzlez
	15.5, // posx
	-20.2, // posy
	-33.5, // posz
	1, // sway
	NULL, // gunviscmds
	NULL, // part visibility
	L_GUN_058, // short name
	L_GUN_058, // name
	L_GUN_000, // manufacturer
	L_GUN_000, // description
	WEAPONFLAG_00000004 | WEAPONFLAG_ONEHANDED | WEAPONFLAG_AICANUSE | WEAPONFLAG_DUALFLIP | WEAPONFLAG_00000040 | WEAPONFLAG_00000400 | WEAPONFLAG_00002000 | WEAPONFLAG_00008000,
};

struct weapon *g_Weapons[] = {
	&invitem_nothing,
	&invitem_unarmed,
	&invitem_falcon2,
	&invitem_falcon2silencer,
	&invitem_falcon2scope,
	&invitem_magsec,
	&invitem_mauler,
	&invitem_phoenix,
	&invitem_dy357,
	&invitem_dy357lx,
	&invitem_cmp150,
	&invitem_cyclone,
	&invitem_callisto,
	&invitem_rcp120,
	&invitem_laptopgun,
	&invitem_dragon,
	&invitem_k7avenger,
	&invitem_ar34,
	&invitem_superdragon,
	&invitem_shotgun,
	&invitem_reaper,
	&invitem_sniperrifle,
	&invitem_farsight,
	&invitem_devastator,
	&invitem_rocketlauncher,
	&invitem_slayer,
	&invitem_combatknife,
	&invitem_crossbow,
	&invitem_tranquilizer,
	&invitem_laser,
	&invitem_grenade,
	&invitem_nbomb,
	&invitem_timedmine,
	&invitem_proximitymine,
	&invitem_remotemine,
	&invitem_combatboost,
	&invitem_pp9i,
	&invitem_cc13,
	&invitem_kl01313,
	&invitem_kf7special,
	&invitem_zzt9mm,
	&invitem_dmc,
	&invitem_ar53,
	&invitem_rcp45,
	&invitem_psychosisgun,
	&invitem_nightvision,
	&invitem_camspy,
	&invitem_xrayscanner,
	&invitem_irscanner,
	&invitem_cloakingdevice,
	&invitem_horizonscanner,
	&invitem_tester,
	&invitem_rocketlauncher_34,
	&invitem_ecmmine,
	&invitem_datauplink,
	&invitem_rtracker,
	&invitem_presidentscanner,
	&invitem_doordecoder,
	&invitem_autosurgeon,
	&invitem_explosives,
	&invitem_skedarbomb,
	&invitem_commsrider,
	&invitem_tracerbug,
	&invitem_targetamplifier,
	&invitem_disguise40,
	&invitem_disguise41,
	&invitem_flightplans,
	&invitem_researchtape,
	&invitem_backupdisk,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_keycard,
	&invitem_suitcase,
	&invitem_briefcase,
#if VERSION >= VERSION_NTSC_1_0
	&invitem_shieldtechitem,
#endif
	&invitem_necklace,
	&invitem_hammer,
	&invitem_hammer,
	&invitem_rocket,
	&invitem_homingrocket,
	&invitem_grenaderound,
	&invitem_bolt,
	&invitem_briefcase2,
	&invitem_rocket,
	&invitem_choppergun,
	&invitem_watchlaser,
	&invitem_hammer,
	&invitem_hammer,
	&invitem_suicidepill,
};
