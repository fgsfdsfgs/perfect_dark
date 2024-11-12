#ifndef _IN_SETUP_DECL
#define _IN_SETUP_DECL

#include "preprocess/common.h"

struct n64_stagesetup {
	u32 ptr_waypoints;
	u32 ptr_waygroups;
	u32 ptr_cover;
	u32 ptr_intro;
	u32 ptr_props;
	u32 ptr_paths;
	u32 ptr_ailists;
	u32 ptr_padfiledata;
};

struct n64_coord {
	f32 x;
	f32 y;
	f32 z;
};

struct n64_defaultobj {
	u16 extrascale;
	u8 hidden2;
	u8 type;
	s16 modelnum;
	s16 pad;
	u32 flags;
	u32 flags2;
	u32 flags3;
	u32 ptr_prop;
	u32 ptr_model;
	f32 realrot[3][3];
	u32 hidden;
	union {
		u32 ptr_geotilef;
		u32 ptr_geoblock;
		u32 ptr_geocyl;
		u32 ptr_unkgeo;
	};
	union {
		u32 ptr_projectile;
		u32 ptr_embedment;
	};
	s16 damage;
	s16 maxdamage;
	u8 shadecol[4];
	u8 nextcol[4];
	u16 floorcol;
	s8 geocount;
	s8 _pad_;
};

struct n64_doorobj {
	struct n64_defaultobj base;
	f32 maxfrac;
	f32 perimfrac;
	f32 accel;
	f32 decel;
	f32 maxspeed;
	u16 doorflags;
	u16 doortype;
	u32 keyflags;
	s32 autoclosetime;
	f32 frac;
	f32 fracspeed;
	s8 mode;
	s8 glasshits;
	s16 fadealpha;
	s16 xludist;
	s16 opadist;
	struct n64_coord startpos;
	f32 mtx98[3][3];
	u32 ptr_sibling;
	s32 lastopen60;
	s16 portalnum;
	s8 soundtype;
	s8 fadetime60;
	s32 lastcalc60;
	u8 laserfade;
	u8 unusedmaybe[3];
	u8 shadeinfo1[4];
	u8 shadeinfo2[4];
	u8 actual1;
	u8 actual2;
	u8 extra1;
	u8 extra2;
};

struct n64_doorscaleobj {
	u32 unk00;
	s32 scale;
};

struct n64_keyobj {
	struct n64_defaultobj base;
	u32 keyflags;
};

struct n64_alarmobj {
	struct n64_defaultobj base;
};

struct n64_cctvobj {
	struct n64_defaultobj base;

	s16 lookatpadnum;
	s16 toleft;
	//Mtxf camrotm;
	f32 m[4][4];
	f32 yzero;
	f32 yrot;
	f32 yleft;
	f32 yright;
	f32 yspeed;
	f32 ymaxspeed;
	s32 seebondtime60;
	f32 maxdist;
	f32 xzero;
};

struct n64_ammocrateobj {
	struct n64_defaultobj base;
	s32 ammotype;
};

struct n64_gset {
	u8 weaponnum;
	u8 unk0639;
	u8 unk063a;
	u8 weaponfunc;
};

struct n64_weaponobj {
	struct n64_defaultobj base;

	union {
		struct n64_gset gset;
		struct {
			u8 weaponnum;
			s8 unk5d;
			s8 unk5e;
			u8 gunfunc;
		};
	};

	s8 fadeouttimer60;
	s8 dualweaponnum;

	union {
		s16 timer240;
		s16 team;
	};

	u32 ptr_dualweapon;
};

struct n64_packedchr {
	s16 chrindex;
	s8 unk02;
	s8 typenum;
	u32 spawnflags;
	s16 chrnum;
	u16 padnum;
	u8 bodynum;
	s8 headnum;
	u16 ailistnum;
	u16 padpreset;
	u16 chrpreset;
	u16 hearscale;
	u16 viewdist;
	u32 flags;
	u32 flags2;
	u8 team;
	u8 squadron;
	s16 chair;
	u32 convtalk;
	u8 tude;
	u8 naturalanim;
	u8 yvisang;
	u8 teamscandist;
};

struct n64_tvscreen {
	u32 ptr_cmdlist;
	u16 offset;
	s16 pause60;
	u32 ptr_tconfig;
	f32 rot;
	f32 xscale;
	f32 xscalefrac;
	f32 xscaleinc;
	f32 xscaleold;
	f32 xscalenew;
	f32 yscale;
	f32 yscalefrac;
	f32 yscaleinc;
	f32 yscaleold;
	f32 yscalenew;
	f32 xmid;
	f32 xmidfrac;
	f32 xmidinc;
	f32 xmidold;
	f32 xmidnew;
	f32 ymid;
	f32 ymidfrac;
	f32 ymidinc;
	f32 ymidold;
	f32 ymidnew;
	u8 red;
	u8 redold;
	u8 rednew;
	u8 green;
	u8 greenold;
	u8 greennew;
	u8 blue;
	u8 blueold;
	u8 bluenew;
	u8 alpha;
	u8 alphaold;
	u8 alphanew;
	f32 colfrac;
	f32 colinc;
};

struct n64_singlemonitorobj {
	struct n64_defaultobj base;
	struct n64_tvscreen screen;
	s16 owneroffset;
	s8 ownerpart;
	u8 imagenum;
};

struct n64_multimonitorobj {
	struct n64_defaultobj base;
	struct n64_tvscreen screens[4];
	u8 imagenums[4];
};

struct n64_hangingmonitorsobj {
	struct n64_defaultobj base;
};

struct n64_autogunobj {
	struct n64_defaultobj base;
	s16 targetpad;
	s8 firing;
	u8 firecount;
	f32 yzero;
	f32 ymaxleft;
	f32 ymaxright;
	f32 yrot;
	f32 yspeed;
	f32 xzero;
	f32 xrot;
	f32 xspeed;
	f32 maxspeed;
	f32 aimdist;
	f32 barrelspeed;
	f32 barrelrot;
	s32 lastseebond60;
	s32 lastaimbond60;
	s32 allowsoundframe;
	u32 ptr_beam;
	f32 shotbondsum;
	u32 ptr_target;
	u8 targetteam;
	u8 ammoquantity;
	s16 nextchrtest;
};

struct n64_linkgunsobj {
	u32 unk00;
	s16 offset1;
	s16 offset2;
};

struct n64_debrisobj {
	struct n64_defaultobj base;
};

struct n64_hatobj {
	struct n64_defaultobj base;
};

struct n64_grenadeprobobj { // objtype 0x12
	u32 unk00;
	s16 chrnum;
	u16 probability;
};

struct n64_linkliftdoorobj {
	u32 unk00;
	u32 ptr_door;
	u32 ptr_lift;
	u32 ptr_next;
	s32 stopnum;
};

struct n64_multiammocrateslot {
	u16 modelnum;
	u16 quantity;
};

struct n64_multiammocrateobj {
	struct n64_defaultobj base;
	struct n64_multiammocrateslot slots[19];
};

struct n64_shieldobj {
	struct n64_defaultobj base;
	f32 initialamount;
	f32 amount;
	u32 unk64;
};

struct n64_tag {
	u32 identifier;
	u16 tagnum;
	s16 cmdoffset;
	u32 ptr_next;
	u32 ptr_obj;
};

struct n64_objective {
	u32 unk00;
	s32 index;
	u32 text;
	u16 unk0c;
	u8 flags;
	s8 difficulties;
};

struct n64_stdobjective {
	u32 cmd;
};

struct n64_objectivecmd {
	u8 cmd0[4];
	u32 cmd1;
};

struct n64_briefingobj {
	u32 unk00;
	u32 type;
	u32 text;
	u32 ptr_next;
};

struct n64_padlockeddoorobj {
	u32 unk00;
	u32 ptr_door;
	u32 ptr_lock;
	u32 ptr_next;
};

struct n64_truckobj {
	struct n64_defaultobj base;
	u32 ptr_ailist;
	u16 aioffset;
	s16 aireturnlist;
	f32 speed;
	f32 wheelxrot;
	f32 wheelyrot;
	f32 speedaim;
	f32 speedtime60;
	f32 turnrot60;
	f32 roty;
	u32 ptr_path;
	s32 nextstep;
};

struct n64_tankobj {
	u8 _pad_[128];
};

struct n64_heliobj {
	struct n64_defaultobj base;
	u32 ptr_ailist;
	u16 aioffset;
	s16 aireturnlist;
	f32 rotoryrot;
	f32 rotoryspeed;
	f32 rotoryspeedaim;
	f32 rotoryspeedtime;
	f32 speed;
	f32 speedaim;
	f32 speedtime60;
	f32 yrot;
	u32 ptr_path;
	s32 nextstep;
};

struct n64_glassobj {
	struct n64_defaultobj base;
	s16 portalnum;
	s16 _pad_;
};

struct n64_safeobj {
	struct n64_defaultobj base;
};

struct n64_safeitemobj {
	u32 unk00;
	u32 ptr_item;
	u32 ptr_safe;
	u32 ptr_door;
	u32 ptr_next;
};

struct n64_cameraposobj {
	s32 type;
	f32 x;
	f32 y;
	f32 z;
	f32 theta;
	f32 verta;
	s32 pad;
};

struct n64_tintedglassobj {
	struct n64_defaultobj base;
	s16 xludist;
	s16 opadist;
	s16 opacity;
	s16 portalnum;
	f32 unk64;
};

struct n64_liftobj {
	struct n64_defaultobj base;
	s16 pads[4];
	u32 ptr_doors[4];
	f32 dist;
	f32 speed;
	f32 accel;
	f32 maxspeed;
	s8 soundtype;
	s8 levelcur;
	s8 levelaim;
	s8 _pad_;
	struct n64_coord prevpos;
};

struct n64_linksceneryobj {
	u32 unk00;
	u32 ptr_trigger;
	u32 ptr_unexp;
	u32 ptr_exp;
	u32 ptr_next;
};

struct n64_blockedpathobj {
	u32 unk00;
	u32 ptr_blocker;
	s16 waypoint1;
	s16 waypoint2;
	u32 ptr_next;
};

struct n64_hov {
	u8 type;
	u8 flags;
	s16 _pad_;
	f32 bobycur;
	f32 bobytarget;
	f32 bobyspeed;
	f32 yrot;
	f32 bobpitchcur;
	f32 bobpitchtarget;
	f32 bobpitchspeed;
	f32 bobrollcur;
	f32 bobrolltarget;
	f32 bobrollspeed;
	f32 groundpitch;
	f32 y;
	f32 ground;
	s32 prevframe60;
	s32 prevgroundframe60;
};

struct n64_hoverbikeobj {
	struct n64_defaultobj base;
	struct n64_hov hov;
	f32 speed[2];
	f32 prevpos[2];
	f32 w;
	f32 rels[2];
	f32 exreal;
	f32 ezreal;
	f32 ezreal2;
	f32 leanspeed;
	f32 leandiff;
	s32 maxspeedtime240;
	f32 speedabs[2];
	f32 speedrel[2];
};

struct n64_hoverpropobj {
	struct n64_defaultobj base;
	struct n64_hov hov;
};

struct n64_fanobj {
	struct n64_defaultobj base;
	f32 yrot;
	f32 yrotprev;
	f32 ymaxspeed;
	f32 yspeed;
	f32 yaccel;
	s8 on;
	u8 _pad_[3];
};

struct n64_hovercarobj {
	struct n64_defaultobj base;
	u32 ptr_ailist;
	u16 aioffset;
	s16 aireturnlist;
	f32 speed;
	f32 speedaim;
	f32 speedtime60;
	f32 turnyspeed60;
	f32 turnxspeed60;
	f32 turnrot60;
	f32 roty;
	f32 rotx;
	f32 rotz;
	u32 ptr_path;
	s32 nextstep;
	s16 status;
	s16 dead;
	s16 deadtimer60;
	s16 sparkstimer60;
};

struct n64_chopperobj {
	struct n64_defaultobj base;
	u32 ptr_ailist;
	u16 aioffset;
	s16 aireturnlist;
	union {
		struct {
			f32 speed;
			f32 speedaim;
			f32 speedtime60;
		};
		struct n64_coord fall;
	};
	f32 turnyspeed60;
	f32 turnxspeed60;
	f32 turnrot60;
	f32 roty;
	f32 rotx;
	f32 rotz;
	u32 ptr_path;
	s32 nextstep;//
	s16 weaponsarmed;
	s16 ontarget;
	s16 target;
	u8 attackmode;
	u8 cw;
	f32 vx;
	f32 vy;
	f32 vz;
	f32 power;
	f32 otx;
	f32 oty;
	f32 otz;
	f32 bob;
	f32 bobstrength;
	s32 targetvisible;
	s32 timer60;
	s32 patroltimer60;
	f32 gunturnyspeed60;
	f32 gunturnxspeed60;
	f32 gunroty;
	f32 gunrotx;
	f32 barrelrotspeed;
	f32 barrelrot;
	u32 ptr_fireslotthing;
	s32 dead;
};

struct n64_escalatorobj {
	struct n64_defaultobj base;
	s32 frame;
	struct n64_coord prevpos;
};

struct n64_textoverride {
	u32 unk00;
	s32 objoffset;
	s32 weapon;
	u32 obtaintext;
	u32 ownertext;
	u32 inventorytext;
	u32 inventory2text;
	u32 pickuptext;
	u32 ptr_next;
	u32 ptr_obj;
};

struct n64_criteria_roomentered {
	u32 unk00;
	u32 pad;
	u32 status;
	u32 ptr_next;
};

struct n64_criteria_throwinroom {
	u32 unk00;
	u32 unk04;
	u32 pad;
	u32 status;
	u32 ptr_next;
};

struct n64_criteria_holograph {
	u32 unk00;
	u32 obj;
	u32 status;
	u32 ptr_next;
};

struct n64_ailist {
	u32 ptr_list;
	s32 id;
};

struct n64_path {
	u32 ptr_pads;
	u8 id;
	u8 flags;
	u16 len;
};

#endif
