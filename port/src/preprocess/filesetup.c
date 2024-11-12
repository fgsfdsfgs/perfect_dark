#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "preprocess/common.h"
#include "preprocess/setup.h"

extern u32 chraiGetAilistLength(u8 *list);

static inline void convF32(f32 *dst, f32 src) { *(u32*)dst = PD_BE32(*(u32*)&src); }
static inline void convU32(u32 *dst, u32 src) { *dst = PD_BE32(src); }
static inline void convS32(s32 *dst, s32 src) { *dst = PD_BE32(src); }
static inline void convU16(u16 *dst, u32 src) { *dst = PD_BE16(src); }
static inline void convS16(s16 *dst, s32 src) { *dst = PD_BE16(src); }
static inline void cpyByte(u8 *dst, u8 src) { *dst = src; }
static inline void convCoord(struct coord* dst, struct n64_coord src) { convF32(&dst->x, src.x); convF32(&dst->y, src.y); convF32(&dst->z, src.z); }
static inline void convUnk(u32 x) { assert(0 && "unknown type"); }

#define PD_CONV_VAL(dst, src) _Generic((dst), \
	f32: convF32, \
	u32: convU32, \
	s32: convS32, \
	u16: convU16, \
	s16: convS16, \
	u8: cpyByte, \
	s8: cpyByte, \
	struct coord: convCoord, \
	default: convUnk	\
)(&dst, src)

#define PD_CONV_ARRAY(dst, src) { for (int i = 0; i < ARRAYCOUNT(dst); i++) PD_CONV_VAL(dst[i], src[i]); }
#define PD_CONV_ARRAY2D(dst, src) { \
	for (int i = 0; i < ARRAYCOUNT(dst); i++) \
		for (int j = 0; j < ARRAYCOUNT(dst); j++) PD_CONV_VAL(dst[i][j], src[i][j]); \
}

#define PD_CONV_PTR(dst, src, type) dst = (type)(uintptr_t)PD_BE32(src)

static inline u32 objSizeN64(struct n64_defaultobj *obj)
{
	switch (obj->type) {
	case OBJTYPE_CHR:                return sizeof(struct n64_packedchr) / sizeof(u32);
	case OBJTYPE_DOOR:               return sizeof(struct n64_doorobj) / sizeof(u32);
	case OBJTYPE_DOORSCALE:          return sizeof(struct n64_doorscaleobj) / sizeof(u32);
	case OBJTYPE_BASIC:              return sizeof(struct n64_defaultobj) / sizeof(u32);
	case OBJTYPE_DEBRIS:             return sizeof(struct n64_debrisobj) / sizeof(u32);
	case OBJTYPE_GLASS:              return sizeof(struct n64_glassobj) / sizeof(u32);
	case OBJTYPE_TINTEDGLASS:        return sizeof(struct n64_tintedglassobj) / sizeof(u32);
	case OBJTYPE_SAFE:               return sizeof(struct n64_safeobj) / sizeof(u32);
	case OBJTYPE_GASBOTTLE:          return sizeof(struct gasbottleobj) / sizeof(u32);
	case OBJTYPE_KEY:                return sizeof(struct n64_keyobj) / sizeof(u32);
	case OBJTYPE_ALARM:              return sizeof(struct n64_alarmobj) / sizeof(u32);
	case OBJTYPE_CCTV:               return sizeof(struct n64_cctvobj) / sizeof(u32);
	case OBJTYPE_AMMOCRATE:          return sizeof(struct n64_ammocrateobj) / sizeof(u32);
	case OBJTYPE_WEAPON:             return sizeof(struct n64_weaponobj) / sizeof(u32);
	case OBJTYPE_SINGLEMONITOR:      return sizeof(struct n64_singlemonitorobj) / sizeof(u32);
	case OBJTYPE_MULTIMONITOR:       return sizeof(struct n64_multimonitorobj) / sizeof(u32);
	case OBJTYPE_HANGINGMONITORS:    return sizeof(struct n64_hangingmonitorsobj) / sizeof(u32);
	case OBJTYPE_AUTOGUN:            return sizeof(struct n64_autogunobj) / sizeof(u32);
	case OBJTYPE_LINKGUNS:           return sizeof(struct linkgunsobj) / sizeof(u32);
	case OBJTYPE_HAT:                return sizeof(struct n64_hatobj) / sizeof(u32);
	case OBJTYPE_GRENADEPROB:        return sizeof(struct grenadeprobobj) / sizeof(u32);
	case OBJTYPE_LINKLIFTDOOR:       return sizeof(struct n64_linkliftdoorobj) / sizeof(u32);
	case OBJTYPE_SAFEITEM:           return sizeof(struct n64_safeitemobj) / sizeof(u32);
	case OBJTYPE_MULTIAMMOCRATE:     return sizeof(struct n64_multiammocrateobj) / sizeof(u32);
	case OBJTYPE_SHIELD:             return sizeof(struct n64_shieldobj) / sizeof(u32);
	case OBJTYPE_TAG:                return sizeof(struct n64_tag) / sizeof(u32);
	case OBJTYPE_RENAMEOBJ:          return sizeof(struct n64_textoverride) / sizeof(u32);
	case OBJTYPE_BEGINOBJECTIVE:     return sizeof(struct n64_objective) / sizeof(u32);
	case OBJTYPE_ENDOBJECTIVE:       return 1;
	case OBJECTIVETYPE_DESTROYOBJ:   return 2;
	case OBJECTIVETYPE_COMPFLAGS:    return 2;
	case OBJECTIVETYPE_FAILFLAGS:    return 2;
	case OBJECTIVETYPE_COLLECTOBJ:   return 2;
	case OBJECTIVETYPE_THROWOBJ:     return 2;
	case OBJECTIVETYPE_HOLOGRAPH:    return sizeof(struct n64_criteria_holograph) / sizeof(u32);
	case OBJECTIVETYPE_1F:           return 1;
	case OBJECTIVETYPE_ENTERROOM:    return sizeof(struct n64_criteria_roomentered) / sizeof(u32);
	case OBJECTIVETYPE_THROWINROOM:  return sizeof(struct n64_criteria_throwinroom) / sizeof(u32);
	case OBJTYPE_22:                 return 1;
	case OBJTYPE_BRIEFING:           return sizeof(struct n64_briefingobj) / sizeof(u32);
	case OBJTYPE_PADLOCKEDDOOR:      return sizeof(struct n64_padlockeddoorobj) / sizeof(u32);
	case OBJTYPE_TRUCK:              return sizeof(struct n64_truckobj) / sizeof(u32);
	case OBJTYPE_HELI:               return sizeof(struct n64_heliobj) / sizeof(u32);
	case OBJTYPE_TANK:               return 32;
	case OBJTYPE_CAMERAPOS:          return sizeof(struct n64_cameraposobj) / sizeof(u32);
	case OBJTYPE_LIFT:               return sizeof(struct n64_liftobj) / sizeof(u32);
	case OBJTYPE_CONDITIONALSCENERY: return sizeof(struct n64_linksceneryobj) / sizeof(u32);
	case OBJTYPE_BLOCKEDPATH:        return sizeof(struct n64_blockedpathobj) / sizeof(u32);
	case OBJTYPE_HOVERBIKE:          return sizeof(struct n64_hoverbikeobj) / sizeof(u32);
	case OBJTYPE_HOVERPROP:          return sizeof(struct n64_hoverpropobj) / sizeof(u32);
	case OBJTYPE_FAN:                return sizeof(struct n64_fanobj) / sizeof(u32);
	case OBJTYPE_HOVERCAR:           return sizeof(struct n64_hovercarobj) / sizeof(u32);
	case OBJTYPE_CHOPPER:            return sizeof(struct n64_chopperobj) / sizeof(u32);
	case OBJTYPE_PADEFFECT:          return sizeof(struct padeffectobj) / sizeof(u32);
	case OBJTYPE_MINE:               return sizeof(struct n64_weaponobj) / sizeof(u32);
	case OBJTYPE_ESCASTEP:           return sizeof(struct n64_escalatorobj) / sizeof(u32);
	}

	return 1;
}

static void convTvScreen(struct tvscreen* dstobj, struct n64_tvscreen* srcobj)
{
	PD_CONV_PTR(dstobj->cmdlist, srcobj->ptr_cmdlist, u32*);
	PD_CONV_VAL(dstobj->offset, srcobj->offset);
	PD_CONV_VAL(dstobj->pause60, srcobj->pause60);
	PD_CONV_PTR(dstobj->tconfig, srcobj->ptr_tconfig, struct textureconfig*);
	PD_CONV_VAL(dstobj->rot, srcobj->rot);
	PD_CONV_VAL(dstobj->xscale, srcobj->xscale);
	PD_CONV_VAL(dstobj->xscalefrac, srcobj->xscalefrac);
	PD_CONV_VAL(dstobj->xscaleinc, srcobj->xscaleinc);
	PD_CONV_VAL(dstobj->xscaleold, srcobj->xscaleold);
	PD_CONV_VAL(dstobj->xscalenew, srcobj->xscalenew);
	PD_CONV_VAL(dstobj->yscale, srcobj->yscale);
	PD_CONV_VAL(dstobj->yscalefrac, srcobj->yscalefrac);
	PD_CONV_VAL(dstobj->yscaleinc, srcobj->yscaleinc);
	PD_CONV_VAL(dstobj->yscaleold, srcobj->yscaleold);
	PD_CONV_VAL(dstobj->yscalenew, srcobj->yscalenew);
	PD_CONV_VAL(dstobj->xmid, srcobj->xmid);
	PD_CONV_VAL(dstobj->xmidfrac, srcobj->xmidfrac);
	PD_CONV_VAL(dstobj->xmidinc, srcobj->xmidinc);
	PD_CONV_VAL(dstobj->xmidold, srcobj->xmidold);
	PD_CONV_VAL(dstobj->xmidnew, srcobj->xmidnew);
	PD_CONV_VAL(dstobj->ymid, srcobj->ymid);
	PD_CONV_VAL(dstobj->ymidfrac, srcobj->ymidfrac);
	PD_CONV_VAL(dstobj->ymidinc, srcobj->ymidinc);
	PD_CONV_VAL(dstobj->ymidold, srcobj->ymidold);
	PD_CONV_VAL(dstobj->ymidnew, srcobj->ymidnew);
	PD_CONV_VAL(dstobj->red, srcobj->red);
	PD_CONV_VAL(dstobj->redold, srcobj->redold);
	PD_CONV_VAL(dstobj->rednew, srcobj->rednew);
	PD_CONV_VAL(dstobj->green, srcobj->green);
	PD_CONV_VAL(dstobj->greenold, srcobj->greenold);
	PD_CONV_VAL(dstobj->greennew, srcobj->greennew);
	PD_CONV_VAL(dstobj->blue, srcobj->blue);
	PD_CONV_VAL(dstobj->blueold, srcobj->blueold);
	PD_CONV_VAL(dstobj->bluenew, srcobj->bluenew);
	PD_CONV_VAL(dstobj->alpha, srcobj->alpha);
	PD_CONV_VAL(dstobj->alphaold, srcobj->alphaold);
	PD_CONV_VAL(dstobj->alphanew, srcobj->alphanew);
	PD_CONV_VAL(dstobj->colfrac, srcobj->colfrac);
	PD_CONV_VAL(dstobj->colinc, srcobj->colinc);
}

static void convertDefaultObjHdr(struct defaultobj* dstobj, struct n64_defaultobj* srcobj)
{
	PD_CONV_VAL(dstobj->extrascale, srcobj->extrascale);
	
	/* 'header' is like this inside a u32:
		u16 unk00_1,
		u8 unk00_2,
		u8 type,
	*/
	u16 *dst_unk00_1 = (u16*)&dstobj->extrascale;
	u8  *dst_unk00_2 = (u8*)(dst_unk00_1 + 1);

	u16 *src_unk00_1 = (u16*)&srcobj->extrascale;
	u8  *src_unk00_2 = (u8*)(src_unk00_1 + 1);

	*dst_unk00_1 = PD_BE16(*src_unk00_1);
	dst_unk00_2[0] = src_unk00_2[0];
	dst_unk00_2[1] = src_unk00_2[1];
}

static void convertDefaultObj(struct defaultobj* dstobj, struct n64_defaultobj* srcobj)
{
	PD_CONV_VAL(dstobj->extrascale, srcobj->extrascale);
	PD_CONV_VAL(dstobj->hidden2, srcobj->hidden2);
	PD_CONV_VAL(dstobj->type, srcobj->type);
	PD_CONV_VAL(dstobj->modelnum, srcobj->modelnum);
	PD_CONV_VAL(dstobj->pad, srcobj->pad);
	PD_CONV_VAL(dstobj->flags, srcobj->flags);
	PD_CONV_VAL(dstobj->flags2, srcobj->flags2);
	PD_CONV_VAL(dstobj->flags3, srcobj->flags3);
	PD_CONV_PTR(dstobj->prop, srcobj->ptr_prop, struct prop*);
	PD_CONV_PTR(dstobj->model, srcobj->ptr_model, struct model*);
	PD_CONV_ARRAY2D(dstobj->realrot, srcobj->realrot);
	PD_CONV_VAL(dstobj->hidden, srcobj->hidden);
	PD_CONV_PTR(dstobj->unkgeo, srcobj->ptr_unkgeo, struct geocyl*);
	PD_CONV_VAL(dstobj->damage, srcobj->damage);
	PD_CONV_VAL(dstobj->maxdamage, srcobj->maxdamage);
	PD_CONV_ARRAY(dstobj->shadecol, srcobj->shadecol);
	PD_CONV_ARRAY(dstobj->nextcol, srcobj->nextcol);
	PD_CONV_VAL(dstobj->floorcol, srcobj->floorcol);
	PD_CONV_VAL(dstobj->geocount, srcobj->geocount);
}

static u32 convertProps(u8* dst, u8* src)
{
	u8* start = dst;
	struct n64_defaultobj* cmd = (struct n64_defaultobj*)src;

	int i = 0;
	u8 type = cmd->type;

	while (type != OBJTYPE_END) {
		//sysLogPrintf(LOG_NOTE, "#%03d obj 0x%02x (%s)", ++i, type, objName(cmd));
		switch (type) {
			case OBJTYPE_DOOR:
			{
				struct n64_doorobj* srcobj = (struct n64_doorobj*)cmd;
				struct doorobj* dstobj = (struct doorobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->maxfrac, srcobj->maxfrac);
				PD_CONV_VAL(dstobj->perimfrac, srcobj->perimfrac);
				PD_CONV_VAL(dstobj->accel, srcobj->accel);
				PD_CONV_VAL(dstobj->decel, srcobj->decel);
				PD_CONV_VAL(dstobj->maxspeed, srcobj->maxspeed);
				PD_CONV_VAL(dstobj->doorflags, srcobj->doorflags);
				PD_CONV_VAL(dstobj->doortype, srcobj->doortype);
				PD_CONV_VAL(dstobj->keyflags, srcobj->keyflags);
				PD_CONV_VAL(dstobj->autoclosetime, srcobj->autoclosetime);
				PD_CONV_VAL(dstobj->frac, srcobj->frac);
				PD_CONV_VAL(dstobj->fracspeed, srcobj->fracspeed);
				PD_CONV_VAL(dstobj->mode, srcobj->mode);
				PD_CONV_VAL(dstobj->glasshits, srcobj->glasshits);
				PD_CONV_VAL(dstobj->fadealpha, srcobj->fadealpha);
				PD_CONV_VAL(dstobj->xludist, srcobj->xludist);
				PD_CONV_VAL(dstobj->opadist, srcobj->opadist);
				PD_CONV_VAL(dstobj->startpos, srcobj->startpos);
				PD_CONV_ARRAY2D(dstobj->mtx98, srcobj->mtx98);
				PD_CONV_PTR(dstobj->sibling, srcobj->ptr_sibling, struct doorobj*);
				PD_CONV_VAL(dstobj->lastopen60, srcobj->lastopen60);
				PD_CONV_VAL(dstobj->portalnum, srcobj->portalnum);
				PD_CONV_VAL(dstobj->soundtype, srcobj->soundtype);
				PD_CONV_VAL(dstobj->fadetime60, srcobj->fadetime60);
				PD_CONV_VAL(dstobj->lastcalc60, srcobj->lastcalc60);
				PD_CONV_VAL(dstobj->laserfade, srcobj->laserfade);
				PD_CONV_ARRAY(dstobj->unusedmaybe, srcobj->unusedmaybe);
				PD_CONV_ARRAY(dstobj->shadeinfo1, srcobj->shadeinfo1);
				PD_CONV_ARRAY(dstobj->shadeinfo2, srcobj->shadeinfo2);

				PD_CONV_VAL(dstobj->actual1, srcobj->actual1);
				PD_CONV_VAL(dstobj->actual2, srcobj->actual2);
				PD_CONV_VAL(dstobj->extra1, srcobj->extra1);
				PD_CONV_VAL(dstobj->extra2, srcobj->extra2);

				dst += sizeof(struct doorobj);
				break;
			}
			case OBJTYPE_DOORSCALE:
			{
				struct n64_doorscaleobj* srcobj = (struct n64_doorscaleobj*)cmd;
				struct doorscaleobj* dstobj = (struct doorscaleobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->scale, srcobj->scale);

				dst += sizeof(struct doorscaleobj);
				break;
			}
			case OBJTYPE_BASIC:
			case OBJTYPE_HANGINGMONITORS:
			case OBJTYPE_DEBRIS:
			case OBJTYPE_HAT:
			case OBJTYPE_ALARM:
			{
				struct n64_defaultobj* srcobj = (struct n64_defaultobj*)cmd;
				struct defaultobj* dstobj = (struct defaultobj*)dst;

				convertDefaultObj(dstobj, cmd);

				dst += sizeof(struct defaultobj);
				break;
			}
			case OBJTYPE_KEY:
			{
				struct n64_keyobj* srcobj = (struct n64_keyobj*)cmd;
				struct keyobj* dstobj = (struct keyobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);
				PD_CONV_VAL(dstobj->keyflags, srcobj->keyflags);

				dst += sizeof(struct keyobj);
				src += sizeof(struct n64_keyobj);
				break;
			}
			case OBJTYPE_CCTV:
			{
				struct n64_cctvobj* srcobj = (struct n64_cctvobj*)cmd;
				struct cctvobj* dstobj = (struct cctvobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->lookatpadnum, srcobj->lookatpadnum);
				PD_CONV_VAL(dstobj->toleft, srcobj->toleft);
				PD_CONV_VAL(dstobj->yzero, srcobj->yzero);
				PD_CONV_VAL(dstobj->yrot, srcobj->yrot);
				PD_CONV_VAL(dstobj->yleft, srcobj->yleft);
				PD_CONV_VAL(dstobj->yright, srcobj->yright);
				PD_CONV_VAL(dstobj->yspeed, srcobj->yspeed);
				PD_CONV_VAL(dstobj->ymaxspeed, srcobj->ymaxspeed);
				PD_CONV_VAL(dstobj->seebondtime60, srcobj->seebondtime60);
				PD_CONV_VAL(dstobj->maxdist, srcobj->maxdist);
				PD_CONV_VAL(dstobj->xzero, srcobj->xzero);

				dst += sizeof(struct cctvobj);
				break;
			}
			case OBJTYPE_AMMOCRATE:
			{
				struct n64_ammocrateobj* srcobj = (struct n64_ammocrateobj*)cmd;
				struct ammocrateobj* dstobj = (struct ammocrateobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);
				PD_CONV_VAL(dstobj->ammotype, srcobj->ammotype);

				dst += sizeof(struct ammocrateobj);
				break;
			}
			case OBJTYPE_WEAPON:
			case OBJTYPE_MINE:
			{
				struct n64_weaponobj* srcobj = (struct n64_weaponobj*)cmd;
				struct weaponobj* dstobj = (struct weaponobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->weaponnum, srcobj->weaponnum);
				PD_CONV_VAL(dstobj->unk5d, srcobj->unk5d);
				PD_CONV_VAL(dstobj->unk5e, srcobj->unk5e);
				PD_CONV_VAL(dstobj->gunfunc, srcobj->gunfunc);

				PD_CONV_VAL(dstobj->fadeouttimer60, srcobj->fadeouttimer60);
				PD_CONV_VAL(dstobj->dualweaponnum, srcobj->dualweaponnum);

				PD_CONV_VAL(dstobj->team, srcobj->team);

				PD_CONV_PTR(dstobj->dualweapon, srcobj->ptr_dualweapon, struct weaponobj*);

				dst += sizeof(struct weaponobj);
				break;
			}
			case OBJTYPE_CHR:
			{
				struct n64_packedchr* srcobj = (struct n64_packedchr*)cmd;
				struct packedchr* dstobj = (struct packedchr*)dst;

				PD_CONV_VAL(dstobj->chrindex, srcobj->chrindex);
				PD_CONV_VAL(dstobj->unk02, srcobj->unk02);
				PD_CONV_VAL(dstobj->typenum, srcobj->typenum);
				PD_CONV_VAL(dstobj->spawnflags, srcobj->spawnflags);
				PD_CONV_VAL(dstobj->chrnum, srcobj->chrnum);
				PD_CONV_VAL(dstobj->padnum, srcobj->padnum);
				PD_CONV_VAL(dstobj->bodynum, srcobj->bodynum);
				PD_CONV_VAL(dstobj->headnum, srcobj->headnum);
				PD_CONV_VAL(dstobj->ailistnum, srcobj->ailistnum);
				PD_CONV_VAL(dstobj->padpreset, srcobj->padpreset);
				PD_CONV_VAL(dstobj->chrpreset, srcobj->chrpreset);
				PD_CONV_VAL(dstobj->hearscale, srcobj->hearscale);
				PD_CONV_VAL(dstobj->viewdist, srcobj->viewdist);
				PD_CONV_VAL(dstobj->flags, srcobj->flags);
				PD_CONV_VAL(dstobj->flags2, srcobj->flags2);
				PD_CONV_VAL(dstobj->team, srcobj->team);
				PD_CONV_VAL(dstobj->squadron, srcobj->squadron);
				PD_CONV_VAL(dstobj->chair, srcobj->chair);
				PD_CONV_VAL(dstobj->convtalk, srcobj->convtalk);
				PD_CONV_VAL(dstobj->tude, srcobj->tude);
				PD_CONV_VAL(dstobj->naturalanim, srcobj->naturalanim);
				PD_CONV_VAL(dstobj->yvisang, srcobj->yvisang);
				PD_CONV_VAL(dstobj->teamscandist, srcobj->teamscandist);

				dst += sizeof(struct packedchr);
				break;
			}
			case OBJTYPE_SINGLEMONITOR:
			{
				struct n64_singlemonitorobj* srcobj = (struct n64_singlemonitorobj*)cmd;
				struct singlemonitorobj* dstobj = (struct singlemonitorobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);
				convTvScreen(&dstobj->screen, &srcobj->screen);

				PD_CONV_VAL(dstobj->owneroffset, srcobj->owneroffset);
				PD_CONV_VAL(dstobj->ownerpart, srcobj->ownerpart);
				PD_CONV_VAL(dstobj->imagenum, srcobj->imagenum);

				dst += sizeof(struct singlemonitorobj);
				break;
			}
			case OBJTYPE_MULTIMONITOR:
			{
				struct n64_multimonitorobj* srcobj = (struct n64_multimonitorobj*)cmd;
				struct multimonitorobj* dstobj = (struct multimonitorobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				for (size_t i = 0; i < 4; i++)
					convTvScreen(&dstobj->screens[i], &srcobj->screens[i]);

				PD_CONV_ARRAY(dstobj->imagenums, srcobj->imagenums);
				dst += sizeof(struct multimonitorobj);
				break;
			}
			case OBJTYPE_AUTOGUN:
			{
				struct n64_autogunobj* srcobj = (struct n64_autogunobj*)cmd;
				struct autogunobj* dstobj = (struct autogunobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->targetpad, srcobj->targetpad);
				PD_CONV_VAL(dstobj->firing, srcobj->firing);
				PD_CONV_VAL(dstobj->firecount, srcobj->firecount);
				PD_CONV_VAL(dstobj->yzero, srcobj->yzero);
				PD_CONV_VAL(dstobj->ymaxleft, srcobj->ymaxleft);
				PD_CONV_VAL(dstobj->ymaxright, srcobj->ymaxright);
				PD_CONV_VAL(dstobj->yrot, srcobj->yrot);
				PD_CONV_VAL(dstobj->yspeed, srcobj->yspeed);
				PD_CONV_VAL(dstobj->xzero, srcobj->xzero);
				PD_CONV_VAL(dstobj->xrot, srcobj->xrot);
				PD_CONV_VAL(dstobj->xspeed, srcobj->xspeed);
				PD_CONV_VAL(dstobj->maxspeed, srcobj->maxspeed);
				PD_CONV_VAL(dstobj->aimdist, srcobj->aimdist);
				PD_CONV_VAL(dstobj->barrelspeed, srcobj->barrelspeed);
				PD_CONV_VAL(dstobj->barrelrot, srcobj->barrelrot);
				PD_CONV_VAL(dstobj->lastseebond60, srcobj->lastseebond60);
				PD_CONV_VAL(dstobj->lastaimbond60, srcobj->lastaimbond60);
				PD_CONV_VAL(dstobj->allowsoundframe, srcobj->allowsoundframe);
				PD_CONV_PTR(dstobj->beam, srcobj->ptr_beam, struct beam*);
				PD_CONV_VAL(dstobj->shotbondsum, srcobj->shotbondsum);
				PD_CONV_PTR(dstobj->target, srcobj->ptr_target, struct prop*);
				PD_CONV_VAL(dstobj->targetteam, srcobj->targetteam);
				PD_CONV_VAL(dstobj->ammoquantity, srcobj->ammoquantity);
				PD_CONV_VAL(dstobj->nextchrtest, srcobj->nextchrtest);

				dst += sizeof(struct autogunobj);
				break;
			}
			case OBJTYPE_LINKGUNS:
			{
				struct n64_linkgunsobj* srcobj = (struct n64_linkgunsobj*)cmd;
				struct linkgunsobj* dstobj = (struct linkgunsobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->offset1, srcobj->offset1);
				PD_CONV_VAL(dstobj->offset2, srcobj->offset2);

				dst += sizeof(struct linkgunsobj);
				break;
			}
			case OBJTYPE_GRENADEPROB:
			{
				struct n64_grenadeprobobj* srcobj = (struct n64_grenadeprobobj*)cmd;
				struct grenadeprobobj* dstobj = (struct grenadeprobobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->chrnum, srcobj->chrnum);
				PD_CONV_VAL(dstobj->probability, srcobj->probability);

				dst += sizeof(struct grenadeprobobj);
				break;
			}
			case OBJTYPE_LINKLIFTDOOR:
			{
				struct n64_linkliftdoorobj* srcobj = (struct n64_linkliftdoorobj*)cmd;
				struct linkliftdoorobj* dstobj = (struct linkliftdoorobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_PTR(dstobj->door, srcobj->ptr_door, struct prop*);
				PD_CONV_PTR(dstobj->lift, srcobj->ptr_lift, struct prop*);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct linkliftdoorobj*);
				PD_CONV_VAL(dstobj->stopnum, srcobj->stopnum);

				dst += sizeof(struct linkliftdoorobj);
				break;
			}
			case OBJTYPE_MULTIAMMOCRATE:
			{
				struct n64_multiammocrateobj* srcobj = (struct n64_multiammocrateobj*)cmd;
				struct multiammocrateobj* dstobj = (struct multiammocrateobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				for (size_t i = 0; i < 19; i++) {
					PD_CONV_VAL(dstobj->slots[i].modelnum, srcobj->slots[i].modelnum);
					PD_CONV_VAL(dstobj->slots[i].quantity, srcobj->slots[i].quantity);
				}

				dst += sizeof(struct multiammocrateobj);
				break;
			}
			case OBJTYPE_SHIELD:
			{
				struct n64_shieldobj* srcobj = (struct n64_shieldobj*)cmd;
				struct shieldobj* dstobj = (struct shieldobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->initialamount, srcobj->initialamount);
				PD_CONV_VAL(dstobj->amount, srcobj->amount);
				PD_CONV_VAL(dstobj->unk64, srcobj->unk64);

				dst += sizeof(struct shieldobj);
				break;
			}
			case OBJTYPE_TAG:
			{
				struct n64_tag* srcobj = (struct n64_tag*)cmd;
				struct tag* dstobj = (struct tag*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->tagnum, srcobj->tagnum);
				PD_CONV_VAL(dstobj->cmdoffset, srcobj->cmdoffset);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct tag*);
				PD_CONV_PTR(dstobj->obj, srcobj->ptr_obj, struct defaultobj*);

				dst += sizeof(struct tag);
				break;
			}
			case OBJTYPE_BEGINOBJECTIVE:
			{
				struct n64_objective* srcobj = (struct n64_objective*)cmd;
				struct objective* dstobj = (struct objective*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->index, srcobj->index);
				PD_CONV_VAL(dstobj->text, srcobj->text);
				PD_CONV_VAL(dstobj->unk0c, srcobj->unk0c);
				PD_CONV_VAL(dstobj->flags, srcobj->flags);
				PD_CONV_VAL(dstobj->difficulties, srcobj->difficulties);

				dst += sizeof(struct objective);
				break;
			}
			case OBJTYPE_ENDOBJECTIVE:
			case OBJECTIVETYPE_1F:
			case OBJTYPE_22:
			case OBJTYPE_GASBOTTLE:
			case OBJTYPE_29:
			case OBJTYPE_SAFE:
			{
				struct n64_stdobjective* srcobj = (struct n64_stdobjective*)cmd;
				struct n64_stdobjective* dstobj = (struct n64_stdobjective*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);

				dst += sizeof(struct n64_stdobjective);
				break;
			}
			case OBJECTIVETYPE_DESTROYOBJ:
			case OBJECTIVETYPE_COMPFLAGS:
			case OBJECTIVETYPE_FAILFLAGS:
			case OBJECTIVETYPE_COLLECTOBJ:
			case OBJECTIVETYPE_THROWOBJ:
			{
				struct n64_objectivecmd* srcobj = (struct n64_objectivecmd*)cmd;
				struct n64_objectivecmd* dstobj = (struct n64_objectivecmd*)dst;

				PD_CONV_ARRAY(dstobj->cmd0, srcobj->cmd0);
				PD_CONV_VAL(dstobj->cmd1, srcobj->cmd1);

				dst += sizeof(struct n64_objectivecmd);
				break;
			}
			case OBJECTIVETYPE_HOLOGRAPH:
			{
				struct n64_criteria_holograph* srcobj = (struct n64_criteria_holograph*)cmd;
				struct criteria_holograph* dstobj = (struct criteria_holograph*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->obj, srcobj->obj);
				PD_CONV_VAL(dstobj->status, srcobj->status);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct criteria_holograph*);

				dst += sizeof(struct criteria_holograph);
				break;
			}
			case OBJECTIVETYPE_ENTERROOM:
			{
				struct n64_criteria_roomentered* srcobj = (struct n64_criteria_roomentered*)cmd;
				struct criteria_roomentered* dstobj = (struct criteria_roomentered*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->pad, srcobj->pad);
				PD_CONV_VAL(dstobj->status, srcobj->status);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct criteria_roomentered*);

				dst += sizeof(struct criteria_roomentered);
				break;
			}
			case OBJECTIVETYPE_THROWINROOM:
			{
				struct n64_criteria_throwinroom* srcobj = (struct n64_criteria_throwinroom*)cmd;
				struct criteria_throwinroom* dstobj = (struct criteria_throwinroom*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->pad, srcobj->pad);
				PD_CONV_VAL(dstobj->status, srcobj->status);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct criteria_throwinroom*);

				dst += sizeof(struct criteria_throwinroom);
				break;
			}
			case OBJTYPE_BRIEFING:
			{
				struct n64_briefingobj* srcobj = (struct n64_briefingobj*)cmd;
				struct briefingobj* dstobj = (struct briefingobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->type, srcobj->type);
				PD_CONV_VAL(dstobj->text, srcobj->text);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct briefingobj*);

				dst += sizeof(struct briefingobj);
				break;
			}
			case OBJTYPE_RENAMEOBJ:
			{
				struct n64_textoverride* srcobj = (struct n64_textoverride*)cmd;
				struct textoverride* dstobj = (struct textoverride*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->objoffset, srcobj->objoffset);
				PD_CONV_VAL(dstobj->weapon, srcobj->weapon);
				PD_CONV_VAL(dstobj->obtaintext, srcobj->obtaintext);
				PD_CONV_VAL(dstobj->ownertext, srcobj->ownertext);
				PD_CONV_VAL(dstobj->inventorytext, srcobj->inventorytext);
				PD_CONV_VAL(dstobj->inventory2text, srcobj->inventory2text);
				PD_CONV_VAL(dstobj->pickuptext, srcobj->pickuptext);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct textoverride*);
				PD_CONV_PTR(dstobj->obj, srcobj->ptr_obj, struct defaultobj*);

				dst += sizeof(struct textoverride);
				break;
			}
			case OBJTYPE_PADLOCKEDDOOR:
			{
				struct n64_padlockeddoorobj* srcobj = (struct n64_padlockeddoorobj*)cmd;
				struct padlockeddoorobj* dstobj = (struct padlockeddoorobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_PTR(dstobj->door, srcobj->ptr_door, struct doorobj*);
				PD_CONV_PTR(dstobj->lock, srcobj->ptr_lock, struct defaultobj*);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct padlockeddoorobj*);

				dst += sizeof(struct padlockeddoorobj);
				break;
			}
			case OBJTYPE_TRUCK:
			{
				struct n64_truckobj* srcobj = (struct n64_truckobj*)cmd;
				struct truckobj* dstobj = (struct truckobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_PTR(dstobj->ailist, srcobj->ptr_ailist, u8*);
				PD_CONV_VAL(dstobj->aioffset, srcobj->aioffset);
				PD_CONV_VAL(dstobj->aireturnlist, srcobj->aireturnlist);
				PD_CONV_VAL(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->wheelxrot, srcobj->wheelxrot);
				PD_CONV_VAL(dstobj->wheelyrot, srcobj->wheelyrot);
				PD_CONV_VAL(dstobj->speedaim, srcobj->speedaim);
				PD_CONV_VAL(dstobj->speedtime60, srcobj->speedtime60);
				PD_CONV_VAL(dstobj->turnrot60, srcobj->turnrot60);
				PD_CONV_VAL(dstobj->roty, srcobj->roty);
				PD_CONV_PTR(dstobj->path, srcobj->ptr_path, struct path*);
				PD_CONV_VAL(dstobj->nextstep, srcobj->nextstep);

				dst += sizeof(struct truckobj);
				break;
			}
			case OBJTYPE_HELI:
			{
				struct n64_heliobj* srcobj = (struct n64_heliobj*)cmd;
				struct heliobj* dstobj = (struct heliobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_PTR(dstobj->ailist, srcobj->ptr_ailist, u8*);
				PD_CONV_VAL(dstobj->aioffset, srcobj->aioffset);
				PD_CONV_VAL(dstobj->aireturnlist, srcobj->aireturnlist);
				PD_CONV_VAL(dstobj->rotoryrot, srcobj->rotoryrot);
				PD_CONV_VAL(dstobj->rotoryspeed, srcobj->rotoryspeed);
				PD_CONV_VAL(dstobj->rotoryspeedaim, srcobj->rotoryspeedaim);
				PD_CONV_VAL(dstobj->rotoryspeedtime, srcobj->rotoryspeedtime);
				PD_CONV_VAL(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->speedaim, srcobj->speedaim);
				PD_CONV_VAL(dstobj->speedtime60, srcobj->speedtime60);
				PD_CONV_VAL(dstobj->yrot, srcobj->yrot);
				PD_CONV_PTR(dstobj->path, srcobj->ptr_path, struct path*);
				PD_CONV_VAL(dstobj->nextstep, srcobj->nextstep);

				dst += sizeof(struct heliobj);
				break;
			}
			case OBJTYPE_GLASS:
			{
				struct n64_glassobj* srcobj = (struct n64_glassobj*)cmd;
				struct glassobj* dstobj = (struct glassobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);
				PD_CONV_VAL(dstobj->portalnum, srcobj->portalnum);

				dst += sizeof(struct glassobj);
				break;
			}
			case OBJTYPE_SAFEITEM:
			{
				struct n64_safeitemobj* srcobj = (struct n64_safeitemobj*)cmd;
				struct safeitemobj* dstobj = (struct safeitemobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_PTR(dstobj->item, srcobj->ptr_item, struct defaultobj*);
				PD_CONV_PTR(dstobj->safe, srcobj->ptr_safe, struct safeobj*);
				PD_CONV_PTR(dstobj->door, srcobj->ptr_door, struct doorobj*);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct safeitemobj*);

				dst += sizeof(struct safeitemobj);
				break;
			}
			case OBJTYPE_TANK:
			{
				dst += sizeof(struct n64_tankobj);
				break;
			}
			case OBJTYPE_CAMERAPOS:
			{
				struct n64_cameraposobj* srcobj = (struct n64_cameraposobj*)cmd;
				struct cameraposobj* dstobj = (struct cameraposobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->x, srcobj->x);
				PD_CONV_VAL(dstobj->y, srcobj->y);
				PD_CONV_VAL(dstobj->z, srcobj->z);
				PD_CONV_VAL(dstobj->theta, srcobj->theta);
				PD_CONV_VAL(dstobj->verta, srcobj->verta);
				PD_CONV_VAL(dstobj->pad, srcobj->pad);

				dst += sizeof(struct cameraposobj);
				break;
			}
			case OBJTYPE_TINTEDGLASS:
			{
				struct n64_tintedglassobj* srcobj = (struct n64_tintedglassobj*)cmd;
				struct tintedglassobj* dstobj = (struct tintedglassobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->xludist, srcobj->xludist);
				PD_CONV_VAL(dstobj->opadist, srcobj->opadist);
				PD_CONV_VAL(dstobj->opacity, srcobj->opacity);
				PD_CONV_VAL(dstobj->portalnum, srcobj->portalnum);
				PD_CONV_VAL(dstobj->unk64, srcobj->unk64);

				dst += sizeof(struct tintedglassobj);
				break;
			}
			case OBJTYPE_LIFT:
			{
				struct n64_liftobj* srcobj = (struct n64_liftobj*)cmd;
				struct liftobj* dstobj = (struct liftobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_ARRAY(dstobj->pads, srcobj->pads);
				for (int k = 0; k < 4; k++)
					PD_CONV_PTR(dstobj->doors[k], srcobj->ptr_doors[k], struct doorobj*);
				PD_CONV_VAL(dstobj->dist, srcobj->dist);
				PD_CONV_VAL(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->accel, srcobj->accel);
				PD_CONV_VAL(dstobj->maxspeed, srcobj->maxspeed);
				PD_CONV_VAL(dstobj->soundtype, srcobj->soundtype);
				PD_CONV_VAL(dstobj->levelcur, srcobj->levelcur);
				PD_CONV_VAL(dstobj->levelaim, srcobj->levelaim);
				PD_CONV_VAL(dstobj->prevpos, srcobj->prevpos);

				dst += sizeof(struct liftobj);
				break;
			}
			case OBJTYPE_CONDITIONALSCENERY:
			{
				struct n64_linksceneryobj* srcobj = (struct n64_linksceneryobj*)cmd;
				struct linksceneryobj* dstobj = (struct linksceneryobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_PTR(dstobj->trigger, srcobj->ptr_trigger, struct defaultobj*);
				PD_CONV_PTR(dstobj->unexp, srcobj->ptr_unexp, struct defaultobj*);
				PD_CONV_PTR(dstobj->exp, srcobj->ptr_exp, struct defaultobj*);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct linksceneryobj*);

				dst += sizeof(struct linksceneryobj);
				break;
			}
			case OBJTYPE_BLOCKEDPATH:
			{
				struct n64_blockedpathobj* srcobj = (struct n64_blockedpathobj*)cmd;
				struct blockedpathobj* dstobj = (struct blockedpathobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_PTR(dstobj->blocker, srcobj->ptr_blocker, struct defaultobj*);
				PD_CONV_VAL(dstobj->waypoint1, srcobj->waypoint1);
				PD_CONV_VAL(dstobj->waypoint2, srcobj->waypoint2);
				PD_CONV_PTR(dstobj->next, srcobj->ptr_next, struct blockedpathobj*);

				dst += sizeof(struct blockedpathobj);
				break;
			}
			case OBJTYPE_HOVERBIKE: {
				struct n64_hoverbikeobj* srcobj = (struct n64_hoverbikeobj*)cmd;
				struct hoverbikeobj* dstobj = (struct hoverbikeobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->hov.type, srcobj->hov.type);
				PD_CONV_VAL(dstobj->hov.flags, srcobj->hov.flags);
				PD_CONV_VAL(dstobj->hov.bobycur, srcobj->hov.bobycur);
				PD_CONV_VAL(dstobj->hov.bobytarget, srcobj->hov.bobytarget);
				PD_CONV_VAL(dstobj->hov.bobyspeed, srcobj->hov.bobyspeed);
				PD_CONV_VAL(dstobj->hov.yrot, srcobj->hov.yrot);
				PD_CONV_VAL(dstobj->hov.bobpitchcur, srcobj->hov.bobpitchcur);
				PD_CONV_VAL(dstobj->hov.bobpitchtarget, srcobj->hov.bobpitchtarget);
				PD_CONV_VAL(dstobj->hov.bobpitchspeed, srcobj->hov.bobpitchspeed);
				PD_CONV_VAL(dstobj->hov.bobrollcur, srcobj->hov.bobrollcur);
				PD_CONV_VAL(dstobj->hov.bobrolltarget, srcobj->hov.bobrolltarget);
				PD_CONV_VAL(dstobj->hov.bobrollspeed, srcobj->hov.bobrollspeed);
				PD_CONV_VAL(dstobj->hov.groundpitch, srcobj->hov.groundpitch);
				PD_CONV_VAL(dstobj->hov.y, srcobj->hov.y);
				PD_CONV_VAL(dstobj->hov.ground, srcobj->hov.ground);
				PD_CONV_VAL(dstobj->hov.prevframe60, srcobj->hov.prevframe60);
				PD_CONV_VAL(dstobj->hov.prevgroundframe60, srcobj->hov.prevgroundframe60);

				PD_CONV_ARRAY(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->exreal, srcobj->exreal);
				PD_CONV_VAL(dstobj->ezreal, srcobj->ezreal);
				PD_CONV_VAL(dstobj->ezreal2, srcobj->ezreal2);
				PD_CONV_VAL(dstobj->leanspeed, srcobj->leanspeed);
				PD_CONV_VAL(dstobj->leandiff, srcobj->leandiff);
				PD_CONV_VAL(dstobj->maxspeedtime240, srcobj->maxspeedtime240);
				PD_CONV_ARRAY(dstobj->prevpos, srcobj->prevpos);
				PD_CONV_ARRAY(dstobj->rels, srcobj->rels);
				PD_CONV_ARRAY(dstobj->speedabs, srcobj->speedabs);
				PD_CONV_ARRAY(dstobj->speedrel, srcobj->speedrel);

				dst += sizeof(struct hoverbikeobj);
				break;
			}
			case OBJTYPE_HOVERPROP:
			{
				struct n64_hoverpropobj* srcobj = (struct n64_hoverpropobj*)cmd;
				struct hoverpropobj* dstobj = (struct hoverpropobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->hov.type, srcobj->hov.type);
				PD_CONV_VAL(dstobj->hov.flags, srcobj->hov.flags);
				PD_CONV_VAL(dstobj->hov.bobycur, srcobj->hov.bobycur);
				PD_CONV_VAL(dstobj->hov.bobytarget, srcobj->hov.bobytarget);
				PD_CONV_VAL(dstobj->hov.bobyspeed, srcobj->hov.bobyspeed);
				PD_CONV_VAL(dstobj->hov.yrot, srcobj->hov.yrot);
				PD_CONV_VAL(dstobj->hov.bobpitchcur, srcobj->hov.bobpitchcur);
				PD_CONV_VAL(dstobj->hov.bobpitchtarget, srcobj->hov.bobpitchtarget);
				PD_CONV_VAL(dstobj->hov.bobpitchspeed, srcobj->hov.bobpitchspeed);
				PD_CONV_VAL(dstobj->hov.bobrollcur, srcobj->hov.bobrollcur);
				PD_CONV_VAL(dstobj->hov.bobrolltarget, srcobj->hov.bobrolltarget);
				PD_CONV_VAL(dstobj->hov.bobrollspeed, srcobj->hov.bobrollspeed);
				PD_CONV_VAL(dstobj->hov.groundpitch, srcobj->hov.groundpitch);
				PD_CONV_VAL(dstobj->hov.y, srcobj->hov.y);
				PD_CONV_VAL(dstobj->hov.ground, srcobj->hov.ground);
				PD_CONV_VAL(dstobj->hov.prevframe60, srcobj->hov.prevframe60);
				PD_CONV_VAL(dstobj->hov.prevgroundframe60, srcobj->hov.prevgroundframe60);

				dst += sizeof(struct hoverpropobj);
				break;
			}
			case OBJTYPE_FAN:
			{
				struct n64_fanobj* srcobj = (struct n64_fanobj*)cmd;
				struct fanobj* dstobj = (struct fanobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->yrot, srcobj->yrot);
				PD_CONV_VAL(dstobj->yrotprev, srcobj->yrotprev);
				PD_CONV_VAL(dstobj->ymaxspeed, srcobj->ymaxspeed);
				PD_CONV_VAL(dstobj->yspeed, srcobj->yspeed);
				PD_CONV_VAL(dstobj->yaccel, srcobj->yaccel);
				PD_CONV_VAL(dstobj->on, srcobj->on);

				dst += sizeof(struct fanobj);
				break;
			}
			case OBJTYPE_HOVERCAR:
			{
				struct n64_hovercarobj* srcobj = (struct n64_hovercarobj*)cmd;
				struct hovercarobj* dstobj = (struct hovercarobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_PTR(dstobj->ailist, srcobj->ptr_ailist, u8*);
				PD_CONV_VAL(dstobj->aioffset, srcobj->aioffset);
				PD_CONV_VAL(dstobj->aireturnlist, srcobj->aireturnlist);
				PD_CONV_VAL(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->speedaim, srcobj->speedaim);
				PD_CONV_VAL(dstobj->speedtime60, srcobj->speedtime60);
				PD_CONV_VAL(dstobj->turnyspeed60, srcobj->turnyspeed60);
				PD_CONV_VAL(dstobj->turnxspeed60, srcobj->turnxspeed60);
				PD_CONV_VAL(dstobj->turnrot60, srcobj->turnrot60);
				PD_CONV_VAL(dstobj->roty, srcobj->roty);
				PD_CONV_VAL(dstobj->rotx, srcobj->rotx);
				PD_CONV_VAL(dstobj->rotz, srcobj->rotz);
				PD_CONV_PTR(dstobj->path, srcobj->ptr_path, struct path*);
				PD_CONV_VAL(dstobj->nextstep, srcobj->nextstep);
				PD_CONV_VAL(dstobj->status, srcobj->status);
				PD_CONV_VAL(dstobj->dead, srcobj->dead);
				PD_CONV_VAL(dstobj->deadtimer60, srcobj->deadtimer60);
				PD_CONV_VAL(dstobj->sparkstimer60, srcobj->sparkstimer60);

				dst += sizeof(struct hovercarobj);
				break;
			}
			case OBJTYPE_PADEFFECT:
			{
				struct padeffectobj* srcobj = (struct padeffectobj*)cmd;
				struct padeffectobj* dstobj = (struct padeffectobj*)dst;

				convertDefaultObjHdr((struct defaultobj*)dstobj, cmd);
				PD_CONV_VAL(dstobj->effect, srcobj->effect);
				PD_CONV_VAL(dstobj->pad, srcobj->pad);

				dst += sizeof(struct padeffectobj);
				break;
			}
			case OBJTYPE_CHOPPER:
			{
				struct n64_chopperobj* srcobj = (struct n64_chopperobj*)cmd;
				struct chopperobj* dstobj = (struct chopperobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_PTR(dstobj->ailist, srcobj->ptr_ailist, u8*);
				PD_CONV_VAL(dstobj->aioffset, srcobj->aioffset);
				PD_CONV_VAL(dstobj->aireturnlist, srcobj->aireturnlist);
				PD_CONV_VAL(dstobj->speed, srcobj->speed);
				PD_CONV_VAL(dstobj->speedaim, srcobj->speedaim);
				PD_CONV_VAL(dstobj->speedtime60, srcobj->speedtime60);
				PD_CONV_VAL(dstobj->turnyspeed60, srcobj->turnyspeed60);
				PD_CONV_VAL(dstobj->turnxspeed60, srcobj->turnxspeed60);
				PD_CONV_VAL(dstobj->turnrot60, srcobj->turnrot60);
				PD_CONV_VAL(dstobj->roty, srcobj->roty);
				PD_CONV_VAL(dstobj->rotx, srcobj->rotx);
				PD_CONV_VAL(dstobj->rotz, srcobj->rotz);
				PD_CONV_PTR(dstobj->path, srcobj->ptr_path, struct path*);
				PD_CONV_VAL(dstobj->nextstep, srcobj->nextstep);
				PD_CONV_VAL(dstobj->weaponsarmed, srcobj->weaponsarmed);
				PD_CONV_VAL(dstobj->ontarget, srcobj->ontarget);
				PD_CONV_VAL(dstobj->target, srcobj->target);
				PD_CONV_VAL(dstobj->attackmode, srcobj->attackmode);
				PD_CONV_VAL(dstobj->cw, srcobj->cw);
				PD_CONV_VAL(dstobj->vx, srcobj->vx);
				PD_CONV_VAL(dstobj->vy, srcobj->vy);
				PD_CONV_VAL(dstobj->vz, srcobj->vz);
				PD_CONV_VAL(dstobj->power, srcobj->power);
				PD_CONV_VAL(dstobj->otx, srcobj->otx);
				PD_CONV_VAL(dstobj->oty, srcobj->oty);
				PD_CONV_VAL(dstobj->otz, srcobj->otz);
				PD_CONV_VAL(dstobj->bob, srcobj->bob);
				PD_CONV_VAL(dstobj->bobstrength, srcobj->bobstrength);
				PD_CONV_VAL(dstobj->targetvisible, srcobj->targetvisible);
				PD_CONV_VAL(dstobj->timer60, srcobj->timer60);
				PD_CONV_VAL(dstobj->patroltimer60, srcobj->patroltimer60);
				PD_CONV_VAL(dstobj->gunturnyspeed60, srcobj->gunturnyspeed60);
				PD_CONV_VAL(dstobj->gunturnxspeed60, srcobj->gunturnxspeed60);
				PD_CONV_VAL(dstobj->gunroty, srcobj->gunroty);
				PD_CONV_VAL(dstobj->gunrotx, srcobj->gunrotx);
				PD_CONV_VAL(dstobj->barrelrotspeed, srcobj->barrelrotspeed);
				PD_CONV_VAL(dstobj->barrelrot, srcobj->barrelrot);
				PD_CONV_PTR(dstobj->fireslotthing, srcobj->ptr_fireslotthing, struct fireslotthing*);
				PD_CONV_VAL(dstobj->dead, srcobj->dead);

				dst += sizeof(struct chopperobj);
				break;
			}
			case OBJTYPE_ESCASTEP:
			{
				struct n64_escalatorobj* srcobj = (struct n64_escalatorobj*)cmd;
				struct escalatorobj* dstobj = (struct escalatorobj*)dst;

				convertDefaultObj(&dstobj->base, cmd);

				PD_CONV_VAL(dstobj->frame, srcobj->frame);
				PD_CONV_VAL(dstobj->prevpos, srcobj->prevpos);

				dst += sizeof(struct escalatorobj);
				break;
			}
		}

		cmd = (struct n64_defaultobj*)((u32*)cmd + objSizeN64(cmd));
		type = cmd->type;
	}

	*(u32*)(dst) = PD_BE32(OBJTYPE_END);
	dst += sizeof(u32);

	return (u32)(dst - start);
}

static uintptr_t convertIntro(u8 *dst, u8 *src)
{
	u8 cmd_size[] = {
		3, // INTROCMD_SPAWN
		4, // INTROCMD_WEAPON
		4, // INTROCMD_AMMO
		8, // INTROCMD_3
		2, // INTROCMD_4
		2, // INTROCMD_OUTFIT
		10, // INTROCMD_6
		3, // INTROCMD_WATCHTIME
		2, // INTROCMD_CREDITOFFSET
		3, // INTROCMD_CASE
		3, // INTROCMD_CASERESPAWN
		2, // INTROCMD_HILL
		1, // the INTROCMD_END constant itself
	};

	s32* dstintro = (s32*)dst;
	s32* srcintro = (s32*)src;

	while (true) {
		s32 cmd = dstintro[0] = PD_BE32(srcintro[0]);
		u8 size = cmd_size[*dstintro];
		for (int i = 1; i < size; i++) {
			dstintro[i] = PD_BE32(srcintro[i]);
		}

		dstintro += size;
		srcintro += size;

		if (cmd == INTROCMD_END) break;
	}

	return (u32)((u8*)dstintro - dst);
}

static u32 convertPaths(u8 *dst, u8 *src)
{
	struct n64_path* srcpath = (struct n64_path*)src;
	struct path* dstpath = (struct path*)dst;

	while (true) {
		PD_CONV_PTR(dstpath->pads, srcpath->ptr_pads, s32*);
		PD_CONV_VAL(dstpath->id, srcpath->id);
		PD_CONV_VAL(dstpath->flags, srcpath->flags);
		PD_CONV_VAL(dstpath->len, srcpath->len);

		if (srcpath->ptr_pads == 0) break;

		dstpath++;
		srcpath++;
	}

	// extra pointer for the end marker
	return (u32)((u8*)dstpath - dst + sizeof(uintptr_t));
}

static u32 convertPads(struct path *dstpaths, u8 *dst, u8 *src, u32 dstpos)
{
	u32 start = dstpos;
	s32 *dstpads = (s32 *)&dst[dstpos];
	while (dstpaths->pads) {
		s32 *pads = (s32 *)&src[(uintptr_t)dstpaths->pads];
		dstpaths->pads = (s32 *)(uintptr_t)dstpos;

		while (*pads) {
			s32 p = PD_BE32(*pads++);
			*dstpads++ = p;
			dstpos += sizeof(s32);
			if (p == -1) break;
		}
		
		dstpaths++;
	}
	
	return dstpos - start;
}

static u32 convertAiLists(u8* dst, u8* src)
{
	struct n64_ailist* srcailist = (struct n64_ailist *)src;
	struct ailist* dstailist = (struct ailist *)dst;

	while (true) {
		// at this point these fields are already converted, so we just assign
		dstailist->list = (u8 *)(uintptr_t)srcailist->ptr_list;
		dstailist->id = srcailist->id;


		if (srcailist->ptr_list == 0) break;

		dstailist++;
		srcailist++;
	}

	return (u32)((u8 *)dstailist - dst + sizeof(uintptr_t));
}

static u32 convertLists(u8 *dst, u8 *src, u32 dstpos, u32 src_ofs)
{
	ptrReset();
	struct n64_ailist *src_ailists = (struct n64_ailist*)&src[src_ofs];

	// count the lists and pre-convert the fields
	int nlists = 0;
	for (struct n64_ailist *tmp = src_ailists; tmp->ptr_list; tmp++, nlists++) {
		tmp->ptr_list = PD_BE32(tmp->ptr_list);
		tmp->id = PD_BE32(tmp->id);
	}

	for (struct n64_ailist *ailist = src_ailists; ailist->ptr_list; ailist++) {
		// update the src list pointers here
		uintptr_t src_ptr_list = (uintptr_t) ailist->ptr_list;
		ailist->ptr_list = dstpos;

		// multiple ailists can point to the same list, so we need to check if it was already copied
		struct ptrmarker *marker = ptrFind(src_ptr_list);
		if (marker) {
			ailist->ptr_list = marker->ptr_host;
			continue;
		}

		ptrAdd(src_ptr_list, dstpos);

		u8 *list = &src[src_ptr_list];
		u32 listsize = chraiGetAilistLength(list);
		memcpy(dst + dstpos, list, listsize);

		dstpos += PD_ALIGN(listsize, 4);
	}
	
	return dstpos;
}

static u32 convertSetup(u8 *dst, u8 *src, u32 srclen)
{
	struct n64_stagesetup *src_header = (struct n64_stagesetup*)src;
	struct stagesetup *dst_header = (struct stagesetup*)dst;

	src_header->ptr_props = PD_BE32(src_header->ptr_props);
	src_header->ptr_intro = PD_BE32(src_header->ptr_intro);
	src_header->ptr_ailists = PD_BE32(src_header->ptr_ailists);
	src_header->ptr_paths = PD_BE32(src_header->ptr_paths);

	u32 srcpos = sizeof(*src_header);
	u32 dstpos = sizeof(*dst_header);

	srcpos = src_header->ptr_props;
	dst_header->props = (u32 *)(uintptr_t)dstpos;
	dstpos += convertProps(&dst[dstpos], &src[srcpos]);

	srcpos = src_header->ptr_intro;
	dst_header->intro = (s32 *)(uintptr_t)dstpos;
	dstpos += convertIntro(&dst[dstpos], &src[srcpos]);
	
	// write the lists bytecodes before the ailists entries
	dstpos = convertLists(dst, src, dstpos, src_header->ptr_ailists);

	srcpos = src_header->ptr_ailists;
	dst_header->ailists = (struct ailist *)(uintptr_t)dstpos;
	dstpos += convertAiLists(&dst[dstpos], &src[srcpos]);

	srcpos = src_header->ptr_paths;
	dst_header->paths = (struct path *)(uintptr_t)dstpos;
	dstpos += convertPaths(&dst[dstpos], &src[srcpos]);

	dstpos += convertPads((struct path*)&dst[(uintptr_t)dst_header->paths], dst, src, dstpos);

	return dstpos;
}

u8 *preprocessSetupFile(u8 *data, u32 size, u32 *outSize) {
	u32 newSizeEstimated = romdataFileGetEstimatedSize(size, LOADTYPE_SETUP);
	u8 *dst = sysMemZeroAlloc(newSizeEstimated);

	u32 newSize = convertSetup(dst, data, size);

	if (newSize > newSizeEstimated) {
		sysFatalError("overflow when trying to preprocess a model file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
