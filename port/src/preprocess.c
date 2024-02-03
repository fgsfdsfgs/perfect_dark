#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <PR/ultratypes.h>
#include <PR/gbi.h>
#include <PR/libaudio.h>
#include "platform.h"
#include "data.h"
#include "bss.h"
#include "game/setuputils.h"
#include "game/texdecompress.h"
#include "preprocess.h"
#include "romdata.h"
#include "mod.h"
#include "system.h"

static inline f32 swapF32(f32 x) { *(u32 *)&x = PD_BE32(*(u32 *)&x); return x; }
static inline u32 swapU32(u32 x) { return PD_BE32(x); }
static inline s32 swapS32(s32 x) { return PD_BE32(x); }
static inline u16 swapU16(u16 x) { return PD_BE16(x); }
static inline s16 swapS16(s16 x) { return PD_BE16(x); }
static inline void *swapPtr(void **x) { return (void *)PD_BEPTR((uintptr_t)x); }
static inline struct coord swapCrd(struct coord crd) { crd.x = swapF32(crd.x); crd.y = swapF32(crd.y); crd.z = swapF32(crd.z); return crd; }
static inline u32 swapUnk(u32 x) { assert(0 && "unknown type"); return x; }

#define PD_SWAP_VAL(x) x = _Generic((x), \
	f32: swapF32, \
	u32: swapU32, \
	s32: swapS32, \
	u16: swapU16, \
	s16: swapS16, \
	struct coord: swapCrd, \
	default: swapUnk	\
)(x)

#define PD_SWAP_PTR(x) x = swapPtr((void *)(x))

#define PD_PTR_BASE(x, b) (void *)((u8 *)b + (u32)x)
#define PD_PTR_BASEOFS(x, b, d) (void *)((u8 *)b - d + (u32)x)

// HACK: to prevent double-swapping stuff, flag swapped offsets in a bitmap

static u8 swapmap[0x40000 >> 3];

static inline u32 alreadySwapped(const intptr_t addr) {
	const u32 mask = (1 << (addr & 7));
	const u32 old = swapmap[addr >> 3] & mask;
	if (!old) {
		swapmap[addr >> 3] |= mask;
	}
	return old;
}

static inline void preprocessALWaveTable(ALWaveTable *tbl, u8 *bankBase)
{
	if (alreadySwapped((u8 *)tbl - bankBase)) {
		return;
	}

	PD_SWAP_VAL(tbl->len);
	PD_SWAP_PTR(tbl->base);

	if (tbl->type == AL_ADPCM_WAVE) {
		if (tbl->waveInfo.adpcmWave.loop ) {
			PD_SWAP_PTR(tbl->waveInfo.adpcmWave.loop);
			ALADPCMloop *loop = PD_PTR_BASE(tbl->waveInfo.adpcmWave.loop, bankBase);
			PD_SWAP_VAL(loop->count);
			PD_SWAP_VAL(loop->start);
			PD_SWAP_VAL(loop->end);
			for (s32 i = 0; i < 16; ++i) {
				PD_SWAP_VAL(loop->state[i]);
			}
		}
		if (tbl->waveInfo.adpcmWave.book) {
			PD_SWAP_PTR(tbl->waveInfo.adpcmWave.book);
			ALADPCMBook *book = PD_PTR_BASE(tbl->waveInfo.adpcmWave.book, bankBase);
			PD_SWAP_VAL(book->npredictors);
			PD_SWAP_VAL(book->order);
			const s32 bookSize = book->order * book->npredictors * ADPCMVSIZE;
			for (s32 i = 0; i < bookSize && i < 128; ++i) {
				PD_SWAP_VAL(book->book[i]);
			}
		}
	} else if (tbl->type == AL_RAW16_WAVE) {
		if (tbl->waveInfo.rawWave.loop) {
			PD_SWAP_PTR(tbl->waveInfo.rawWave.loop);
			ALRawLoop *loop = PD_PTR_BASE(tbl->waveInfo.rawWave.loop, bankBase);
			PD_SWAP_VAL(loop->count);
			PD_SWAP_VAL(loop->start);
			PD_SWAP_VAL(loop->end);
		}
	}
}

static inline void preprocessALSound(ALSound *snd, u8 *bankBase)
{
	if (alreadySwapped((u8 *)snd - bankBase)) {
		return;
	}

	if (snd->envelope) {
		PD_SWAP_PTR(snd->envelope);
		if (!alreadySwapped((intptr_t)snd->envelope)) {
			ALEnvelope *env = PD_PTR_BASE(snd->envelope, bankBase);
			PD_SWAP_VAL(env->attackTime);
			PD_SWAP_VAL(env->releaseTime);
			PD_SWAP_VAL(env->decayTime);
		}
	}

	if (snd->keyMap) {
		PD_SWAP_PTR(snd->keyMap);
	}

	if (snd->wavetable) {
		PD_SWAP_PTR(snd->wavetable);
		preprocessALWaveTable(PD_PTR_BASE(snd->wavetable, bankBase), bankBase);
	}
}

static inline void preprocessALInstrument(ALInstrument *inst, u8 *bankBase)
{
	if (alreadySwapped((u8 *)inst - bankBase)) {
		return;
	}

	PD_SWAP_VAL(inst->bendRange);
	PD_SWAP_VAL(inst->soundCount);

	for (s16 i = 0; i < inst->soundCount; ++i) {
		PD_SWAP_PTR(inst->soundArray[i]);
		preprocessALSound(PD_PTR_BASE(inst->soundArray[i], bankBase), bankBase);
	}
}

static inline void preprocessALBank(ALBank *bank, u8 *bankBase)
{
	if (alreadySwapped((u8 *)bank - bankBase)) {
		return;
	}

	PD_SWAP_VAL(bank->sampleRate);
	PD_SWAP_VAL(bank->instCount);

	if (bank->percussion) {
		PD_SWAP_PTR(bank->percussion);
		preprocessALInstrument(PD_PTR_BASE(bank->percussion, bankBase), bankBase);
	}

	for (s16 i = 0; i < bank->instCount; ++i) {
		PD_SWAP_PTR(bank->instArray[i]);
		preprocessALInstrument(PD_PTR_BASE(bank->instArray[i], bankBase), bankBase);
	}
}

static inline void preprocessGfx(Gfx *gdl, u8 *base, uintptr_t ofs)
{
	while (gdl) {
		const s8 opcode = (s8)gdl->bytes[0];
		PD_SWAP_VAL(gdl->words.w0);
		PD_SWAP_VAL(gdl->words.w1);
		if (opcode == (s8)G_ENDDL) {
			break;
		}

		// mark all addresses in the DL as segmented
		switch (opcode) {
			case (s8)G_SETTIMG:
			case (s8)G_SETCIMG:
			case (s8)G_SETZIMG:
			case G_MOVEMEM:
			case G_MTX:
			case G_VTX:
			case G_COL:
			case G_DL:
				gdl->words.w1 |= 1;
				break;
			default:
				break;
		}

		++gdl;
	}
}

static inline void preprocessVtx(Vtx *vtx)
{
	PD_SWAP_VAL(vtx->x);
	PD_SWAP_VAL(vtx->y);
	PD_SWAP_VAL(vtx->z);
	PD_SWAP_VAL(vtx->s);
	PD_SWAP_VAL(vtx->t);
}

static u8 *preprocessModelGunDL(struct modelrodata_gundl *gundl, u8 *base, u32 ofs)
{
	PD_SWAP_PTR(gundl->baseaddr);
	PD_SWAP_PTR(gundl->vertices);
	PD_SWAP_VAL(gundl->numvertices);
	PD_SWAP_VAL(gundl->unk12);

	Vtx *vtx = PD_PTR_BASEOFS(gundl->vertices, base, ofs);
	for (s16 i = 0; i < gundl->numvertices; ++i, ++vtx) {
		preprocessVtx(vtx);
	}

	if (gundl->opagdl) {
		PD_SWAP_PTR(gundl->opagdl);
		preprocessGfx(PD_PTR_BASEOFS(gundl->opagdl, base, ofs), base, ofs);
		gundl->opagdl = SEGADDR(gundl->opagdl);
	}

	if (gundl->xlugdl) {
		PD_SWAP_PTR(gundl->xlugdl);
		preprocessGfx(PD_PTR_BASEOFS(gundl->xlugdl, base, ofs), base, ofs);
		gundl->xlugdl = SEGADDR(gundl->xlugdl);
	}

	u8 *lowptr = (u8 *)gundl->vertices;
	if (gundl->opagdl && (u8 *)gundl->opagdl < lowptr) {
		lowptr = (u8 *)gundl->opagdl;
	}
	if (gundl->xlugdl && (u8 *)gundl->xlugdl < lowptr) {
		lowptr = (u8 *)gundl->xlugdl;
	}

	return lowptr;
}

static u8 *preprocessModelDL(struct modelrodata_dl *dl, u8 *base, u32 ofs)
{
	PD_SWAP_PTR(dl->colours);
	PD_SWAP_PTR(dl->vertices);

	PD_SWAP_VAL(dl->numvertices);
	PD_SWAP_VAL(dl->numcolours);
	PD_SWAP_VAL(dl->mcount);
	PD_SWAP_VAL(dl->rwdataindex);

	Vtx *vtx = PD_PTR_BASEOFS(dl->vertices, base, ofs);
	for (s16 i = 0; i < dl->numvertices; ++i, ++vtx) {
		preprocessVtx(vtx);
	}

	if (dl->opagdl) {
		PD_SWAP_PTR(dl->opagdl);
		preprocessGfx(PD_PTR_BASEOFS(dl->opagdl, base, ofs), base, ofs);
		dl->opagdl = SEGADDR(dl->opagdl);
	}

	if (dl->xlugdl) {
		PD_SWAP_PTR(dl->xlugdl);
		preprocessGfx(PD_PTR_BASEOFS(dl->xlugdl, base, ofs), base, ofs);
		dl->xlugdl = SEGADDR(dl->xlugdl);
	}

	u8 *lowptr = (u8 *)dl->vertices;
	if (dl->colours && (u8 *)dl->colours < lowptr) {
		lowptr = (u8 *)dl->colours;
	}
	if (dl->opagdl && (u8 *)dl->opagdl < lowptr) {
		lowptr = (u8 *)dl->opagdl;
	}
	if (dl->xlugdl && (u8 *)dl->xlugdl < lowptr) {
		lowptr = (u8 *)dl->xlugdl;
	}

	return lowptr;
}

// returns the pointer to the start of the non-texture graphics data (verts, gdls, etc)
static u8 *preprocessModelNode(struct modelnode *node, u8 *base, u32 ofs)
{
	u8 *lowptr = NULL;
	u8 *tmp = NULL;

	while (node) {
		PD_SWAP_VAL(node->type);
		PD_SWAP_PTR(node->child);
		PD_SWAP_PTR(node->next);
		PD_SWAP_PTR(node->parent);
		PD_SWAP_PTR(node->rodata);

		if (node->rodata) {
			union modelrodata* ro = PD_PTR_BASEOFS(node->rodata, base, ofs);
			if (!lowptr || ((u8 *)node->rodata < lowptr)) {
				lowptr = (u8 *)node->rodata;
			}

			switch (node->type & 0xff) {
				case MODELNODETYPE_CHRINFO:
					PD_SWAP_VAL(ro->chrinfo.animpart);
					PD_SWAP_VAL(ro->chrinfo.mtxindex);
					PD_SWAP_VAL(ro->chrinfo.rwdataindex);
					PD_SWAP_VAL(ro->chrinfo.unk04);
					break;
				case MODELNODETYPE_POSITION:
					PD_SWAP_VAL(ro->position.drawdist);
					PD_SWAP_VAL(ro->position.pos);
					PD_SWAP_VAL(ro->position.part);
					PD_SWAP_VAL(ro->position.mtxindex0);
					PD_SWAP_VAL(ro->position.mtxindex1);
					PD_SWAP_VAL(ro->position.mtxindex2);
					break;
				case MODELNODETYPE_GUNDL:
					tmp = preprocessModelGunDL(&ro->gundl, base, ofs);
					if (tmp && (!lowptr || tmp < lowptr)) {
						lowptr = tmp;
					}
					break;
				case MODELNODETYPE_05:
					break;
				case MODELNODETYPE_DISTANCE:
					PD_SWAP_VAL(ro->distance.near);
					PD_SWAP_VAL(ro->distance.far);
					PD_SWAP_VAL(ro->distance.rwdataindex);
					PD_SWAP_PTR(ro->distance.target);
					// TODO: assuming target points to one of the nodes we'll swap by following node->next
					break;
				case MODELNODETYPE_REORDER:
					PD_SWAP_VAL(ro->reorder.unk00);
					PD_SWAP_VAL(ro->reorder.unk04);
					PD_SWAP_VAL(ro->reorder.unk08);
					PD_SWAP_VAL(ro->reorder.unk0c[0]);
					PD_SWAP_VAL(ro->reorder.unk0c[1]);
					PD_SWAP_VAL(ro->reorder.unk0c[2]);
					PD_SWAP_VAL(ro->reorder.rwdataindex);
					PD_SWAP_VAL(ro->reorder.side);
					PD_SWAP_PTR(ro->reorder.unk18);
					PD_SWAP_PTR(ro->reorder.unk1c);
					break;
				case MODELNODETYPE_BBOX:
					PD_SWAP_VAL(ro->bbox.xmin);
					PD_SWAP_VAL(ro->bbox.xmax);
					PD_SWAP_VAL(ro->bbox.ymin);
					PD_SWAP_VAL(ro->bbox.ymax);
					PD_SWAP_VAL(ro->bbox.zmin);
					PD_SWAP_VAL(ro->bbox.zmax);
					PD_SWAP_VAL(ro->bbox.hitpart);
					break;
				case MODELNODETYPE_0B:
					PD_SWAP_PTR(ro->type0b.baseaddr);
					PD_SWAP_VAL(ro->type0b.rwdataindex);
					PD_SWAP_VAL(ro->type0b.unk00);
					PD_SWAP_PTR(ro->type0b.unk3c);
					// TODO: do we need to swap the rest of the words?
					break;
				case MODELNODETYPE_CHRGUNFIRE:
					PD_SWAP_PTR(ro->chrgunfire.baseaddr);
					PD_SWAP_PTR(ro->chrgunfire.texture);
					PD_SWAP_VAL(ro->chrgunfire.rwdataindex);
					PD_SWAP_VAL(ro->chrgunfire.pos);
					PD_SWAP_VAL(ro->chrgunfire.dim);
					PD_SWAP_VAL(ro->chrgunfire.unk1c);
					// TODO: do we swap contents of *texture or is it part of mdl->textureconfigs[]?
					break;
				case MODELNODETYPE_0D:
					PD_SWAP_PTR(ro->type0d.baseaddr);
					PD_SWAP_PTR(ro->type0d.unk10);
					PD_SWAP_PTR(ro->type0d.unk14);
					// TODO: do we need to swap the rest of the words?
					break;
				case MODELNODETYPE_0E:
				case MODELNODETYPE_0F:
					break;
				case MODELNODETYPE_11:
					PD_SWAP_PTR(ro->type11.unk14);
					// TODO: do we need to swap the rest of the words?
					break;
				case MODELNODETYPE_TOGGLE:
					PD_SWAP_VAL(ro->toggle.rwdataindex);
					PD_SWAP_PTR(ro->toggle.target);
					// TODO: assuming target points to one of the nodes we'll swap by following node->next
					break;
				case MODELNODETYPE_POSITIONHELD:
					PD_SWAP_VAL(ro->positionheld.mtxindex);
					PD_SWAP_VAL(ro->positionheld.pos);
					break;
				case MODELNODETYPE_STARGUNFIRE:
					PD_SWAP_VAL(ro->stargunfire.unk00);
					PD_SWAP_PTR(ro->stargunfire.baseaddr);
					if (ro->stargunfire.gdl) {
						PD_SWAP_PTR(ro->stargunfire.gdl);
						preprocessGfx(PD_PTR_BASEOFS(ro->stargunfire.gdl, base, ofs), base, ofs);
						ro->stargunfire.gdl = SEGADDR(ro->stargunfire.gdl);
						if (!lowptr || (u8 *)ro->stargunfire.gdl < lowptr) {
							lowptr = (u8 *)ro->stargunfire.gdl;
						}
					}
					if (ro->stargunfire.vertices) {
						PD_SWAP_PTR(ro->stargunfire.vertices);
						Vtx *vtx = PD_PTR_BASEOFS(ro->stargunfire.vertices, base, ofs);
						for (s32 i = 0; i < 4 * ro->stargunfire.unk00; ++i, ++vtx) {
							preprocessVtx(vtx);
						}
						if (!lowptr || (u8 *)ro->stargunfire.vertices < lowptr) {
							lowptr = (u8 *)ro->stargunfire.vertices;
						}
					}
					break;
				case MODELNODETYPE_HEADSPOT:
					PD_SWAP_VAL(ro->headspot.rwdataindex);
					break;
				case MODELNODETYPE_DL:
					tmp = preprocessModelDL(&ro->dl, base, ofs);
					if (tmp && (!lowptr || tmp < lowptr)) {
						lowptr = tmp;
					}
					break;
				case 0x19:
					PD_SWAP_VAL(ro->type19.numvertices);
					for (s32 i = 0; i < ARRAYCOUNT(ro->type19.vertices); ++i) {
						PD_SWAP_VAL(ro->type19.vertices[i]);
					}
					break;
				default:
					sysLogPrintf(LOG_ERROR, "preprocessModelNode: node at %p: unknown node->type: 0x%02x", node, node->type & 0xff);
					break;
			}
		}

		if (node->child) {
			node = PD_PTR_BASEOFS(node->child, base, ofs);
		} else {
			while (node) {
				if (node->next) {
					node = PD_PTR_BASEOFS(node->next, base, ofs);
					break;
				}
				if (node->parent) {
					node = PD_PTR_BASEOFS(node->parent, base, ofs);
				} else {
					node = NULL;
				}
			}
		}
	}

	return (u8 *)UNSEGADDR(lowptr);
}

static inline void preprocessPadData(u8 *ptr, u32 size)
{
	// we'll try to avoid overwriting the next pad if possible in case the flags are wrong
	u8 *end = ptr + size;

	// for some insane reason pads are packed
	// header is always 1 word, the rest of the fields depends on the flags
	u32 *header = (u32 *) ptr;
	PD_SWAP_VAL(header[0]);

	const u32 flags = header[0] >> 14;

	ptr += 4;

	if ((flags & PADFLAG_INTPOS) || size <= 12) {
		// position is 3x s16
		s16 *sbuffer = (s16 *) ptr;
		PD_SWAP_VAL(sbuffer[0]);
		PD_SWAP_VAL(sbuffer[1]);
		PD_SWAP_VAL(sbuffer[2]);
		ptr += 8;
	} else if (size > 12) {
		// position is 3x f32
		f32 *fbuffer = (f32 *) ptr;
		PD_SWAP_VAL(fbuffer[0]);
		PD_SWAP_VAL(fbuffer[1]);
		PD_SWAP_VAL(fbuffer[2]);
		ptr += 12;
	}

	if (!(flags & (PADFLAG_UPALIGNTOX | PADFLAG_UPALIGNTOY | PADFLAG_UPALIGNTOZ)) && (end >= ptr + 12)) {
		// up vector, 3x f32
		f32 *fbuffer = (f32 *) ptr;
		PD_SWAP_VAL(fbuffer[0]);
		PD_SWAP_VAL(fbuffer[1]);
		PD_SWAP_VAL(fbuffer[2]);
		ptr += 12;
	}

	if (!(flags & (PADFLAG_LOOKALIGNTOX | PADFLAG_LOOKALIGNTOY | PADFLAG_LOOKALIGNTOZ)) && (end >= ptr + 12)) {
		// look vector, 3x f32
		f32 *fbuffer = (f32 *) ptr;
		PD_SWAP_VAL(fbuffer[0]);
		PD_SWAP_VAL(fbuffer[1]);
		PD_SWAP_VAL(fbuffer[2]);
		ptr += 12;
	}

	if ((flags & PADFLAG_HASBBOXDATA) && (end >= ptr + 24)) {
		// bbox, 3x f32 mins + 3x f32 maxs
		f32 *fbuffer = (f32 *) ptr;
		PD_SWAP_VAL(fbuffer[0]);
		PD_SWAP_VAL(fbuffer[1]);
		PD_SWAP_VAL(fbuffer[2]);
		PD_SWAP_VAL(fbuffer[3]);
		PD_SWAP_VAL(fbuffer[4]);
		PD_SWAP_VAL(fbuffer[5]);
	}
}

static inline void preprocessDefaultPropObj(struct defaultobj *obj)
{
	PD_SWAP_PTR(obj->projectile);
	PD_SWAP_PTR(obj->geoblock);
	PD_SWAP_PTR(obj->prop);
	PD_SWAP_PTR(obj->model);
	PD_SWAP_VAL(obj->flags);
	PD_SWAP_VAL(obj->flags2);
	PD_SWAP_VAL(obj->flags3);
	PD_SWAP_VAL(obj->hidden);
	PD_SWAP_VAL(obj->damage);
	PD_SWAP_VAL(obj->maxdamage);
	PD_SWAP_VAL(obj->extrascale);
	PD_SWAP_VAL(obj->floorcol);
	PD_SWAP_VAL(obj->modelnum);
	PD_SWAP_VAL(obj->pad);
	for (s32 i = 0; i < 3; ++i) {
		for (s32 j = 0; j < 3; ++j) {
			PD_SWAP_VAL(obj->realrot[i][j]);
		}
	}
}

static inline void preprocessDefaultPropObjHdr(struct defaultobj *obj)
{
	// don't touch the other fields
	PD_SWAP_VAL(obj->extrascale);
}

static inline void preprocessTvScreenPropObj(struct tvscreen *scr)
{
	PD_SWAP_PTR(scr->cmdlist);
	PD_SWAP_PTR(scr->tconfig);
	PD_SWAP_VAL(scr->offset);
	PD_SWAP_VAL(scr->pause60);
	PD_SWAP_VAL(scr->rot);
	PD_SWAP_VAL(scr->xscale);
	PD_SWAP_VAL(scr->xscalefrac);
	PD_SWAP_VAL(scr->xscaleinc);
	PD_SWAP_VAL(scr->xscaleold);
	PD_SWAP_VAL(scr->xscalenew);
	PD_SWAP_VAL(scr->yscale);
	PD_SWAP_VAL(scr->yscalefrac);
	PD_SWAP_VAL(scr->yscaleinc);
	PD_SWAP_VAL(scr->yscaleold);
	PD_SWAP_VAL(scr->yscalenew);
	PD_SWAP_VAL(scr->xmid);
	PD_SWAP_VAL(scr->xmidfrac);
	PD_SWAP_VAL(scr->xmidinc);
	PD_SWAP_VAL(scr->xmidold);
	PD_SWAP_VAL(scr->xmidnew);
	PD_SWAP_VAL(scr->ymid);
	PD_SWAP_VAL(scr->ymidfrac);
	PD_SWAP_VAL(scr->ymidinc);
	PD_SWAP_VAL(scr->ymidold);
	PD_SWAP_VAL(scr->ymidnew);
	PD_SWAP_VAL(scr->colfrac);
	PD_SWAP_VAL(scr->colinc);
}

static inline void preprocessHovPropObj(struct hov *hov)
{
	// don't think anything actually has to be swapped here
}

static void preprocessPropObj(struct defaultobj *obj)
{
	// TODO: help me jesus
	switch (obj->type) {
		case OBJTYPE_GRENADEPROB: {
			struct grenadeprobobj *grenadeprob = (struct grenadeprobobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(grenadeprob->chrnum);
			PD_SWAP_VAL(grenadeprob->probability);
			break;
		}
		case OBJTYPE_CHR: {
			struct packedchr *chr = (struct packedchr *)obj;
			PD_SWAP_VAL(chr->chrindex);
			PD_SWAP_VAL(chr->spawnflags);
			PD_SWAP_VAL(chr->chrnum);
			PD_SWAP_VAL(chr->padnum);
			PD_SWAP_VAL(chr->ailistnum);
			PD_SWAP_VAL(chr->padpreset);
			PD_SWAP_VAL(chr->chrpreset);
			PD_SWAP_VAL(chr->hearscale);
			PD_SWAP_VAL(chr->viewdist);
			PD_SWAP_VAL(chr->flags);
			PD_SWAP_VAL(chr->flags2);
			PD_SWAP_VAL(chr->chair);
			PD_SWAP_VAL(chr->convtalk);
			break;
		}
		case OBJTYPE_DOOR: {
			struct doorobj *door = (struct doorobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(door->sibling);
			PD_SWAP_VAL(door->maxfrac);
			PD_SWAP_VAL(door->perimfrac);
			PD_SWAP_VAL(door->accel);
			PD_SWAP_VAL(door->decel);
			PD_SWAP_VAL(door->maxspeed);
			PD_SWAP_VAL(door->frac);
			PD_SWAP_VAL(door->fracspeed);
			PD_SWAP_VAL(door->startpos);
			PD_SWAP_VAL(door->keyflags);
			PD_SWAP_VAL(door->autoclosetime);
			PD_SWAP_VAL(door->doorflags);
			PD_SWAP_VAL(door->doortype);
			PD_SWAP_VAL(door->fadealpha);
			PD_SWAP_VAL(door->xludist);
			PD_SWAP_VAL(door->opadist);
			break;
		}
		case OBJTYPE_DOORSCALE: {
			struct doorscaleobj *doorsc = (struct doorscaleobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(doorsc->scale);
			break;
		}
		case OBJTYPE_WEAPON: {
			struct weaponobj *wpn = (struct weaponobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(wpn->dualweapon);
			PD_SWAP_VAL(wpn->team);
			break;
		}
		case OBJTYPE_KEY: {
			struct keyobj *key = (struct keyobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(key->keyflags);
			break;
		}
		case OBJTYPE_CCTV: {
			struct cctvobj *cctv = (struct cctvobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(cctv->yzero);
			PD_SWAP_VAL(cctv->yrot);
			PD_SWAP_VAL(cctv->yleft);
			PD_SWAP_VAL(cctv->yright);
			PD_SWAP_VAL(cctv->yspeed);
			PD_SWAP_VAL(cctv->ymaxspeed);
			PD_SWAP_VAL(cctv->maxdist);
			PD_SWAP_VAL(cctv->xzero);
			PD_SWAP_VAL(cctv->seebondtime60);
			PD_SWAP_VAL(cctv->lookatpadnum);
			PD_SWAP_VAL(cctv->toleft);
			break;
		}
		case OBJTYPE_AUTOGUN: {
			struct autogunobj *agun = (struct autogunobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(agun->targetpad);
			PD_SWAP_VAL(agun->yzero);
			PD_SWAP_VAL(agun->ymaxleft);
			PD_SWAP_VAL(agun->ymaxright);
			PD_SWAP_VAL(agun->yrot);
			PD_SWAP_VAL(agun->yspeed);
			PD_SWAP_VAL(agun->xzero);
			PD_SWAP_VAL(agun->xrot);
			PD_SWAP_VAL(agun->xspeed);
			PD_SWAP_VAL(agun->maxspeed);
			PD_SWAP_VAL(agun->aimdist);
			PD_SWAP_VAL(agun->barrelspeed);
			PD_SWAP_VAL(agun->barrelrot);
			PD_SWAP_VAL(agun->allowsoundframe);
			PD_SWAP_PTR(agun->target);
			break;
		}
		case OBJTYPE_SINGLEMONITOR: {
			struct singlemonitorobj *mon = (struct singlemonitorobj *)obj;
			preprocessDefaultPropObj(obj);
			preprocessTvScreenPropObj((struct tvscreen *)&mon->screen);
			PD_SWAP_VAL(mon->owneroffset);
			break;
		}
		case OBJTYPE_MULTIMONITOR: {
			struct multimonitorobj *mmon = (struct multimonitorobj *)obj;
			preprocessDefaultPropObj(obj);
			preprocessTvScreenPropObj((struct tvscreen *)&mmon->screens[0]);
			preprocessTvScreenPropObj((struct tvscreen *)&mmon->screens[1]);
			preprocessTvScreenPropObj((struct tvscreen *)&mmon->screens[2]);
			preprocessTvScreenPropObj((struct tvscreen *)&mmon->screens[3]);
			break;
		}
		case OBJTYPE_SHIELD: {
			struct shieldobj *shld = (struct shieldobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(shld->initialamount);
			PD_SWAP_VAL(shld->amount);
			break;
		}
		case OBJTYPE_TINTEDGLASS: {
			struct tintedglassobj *tgls = (struct tintedglassobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(tgls->xludist);
			PD_SWAP_VAL(tgls->opadist);
			PD_SWAP_VAL(tgls->opacity);
			PD_SWAP_VAL(tgls->portalnum);
			PD_SWAP_VAL(tgls->unk64);
			break;
		}
		case OBJTYPE_LIFT: {
			struct liftobj *lift = (struct liftobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(lift->doors[0]);
			PD_SWAP_PTR(lift->doors[1]);
			PD_SWAP_PTR(lift->doors[2]);
			PD_SWAP_PTR(lift->doors[3]);
			PD_SWAP_VAL(lift->pads[0]);
			PD_SWAP_VAL(lift->pads[1]);
			PD_SWAP_VAL(lift->pads[2]);
			PD_SWAP_VAL(lift->pads[3]);
			PD_SWAP_VAL(lift->dist);
			PD_SWAP_VAL(lift->speed);
			PD_SWAP_VAL(lift->accel);
			PD_SWAP_VAL(lift->maxspeed);
			break;
		}
		case OBJTYPE_HOVERPROP: {
			struct hoverpropobj *hprop = (struct hoverpropobj *)obj;
			preprocessDefaultPropObj(obj);
			preprocessHovPropObj(&hprop->hov);
			break;
		}
		case OBJTYPE_HOVERBIKE: {
			struct hoverbikeobj *hbike = (struct hoverbikeobj *)obj;
			preprocessDefaultPropObj(obj);
			preprocessHovPropObj(&hbike->hov);
			break;
		}
		case OBJTYPE_FAN: {
			struct fanobj *fan = (struct fanobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(fan->yrot);
			PD_SWAP_VAL(fan->yrotprev);
			PD_SWAP_VAL(fan->ymaxspeed);
			PD_SWAP_VAL(fan->yspeed);
			PD_SWAP_VAL(fan->yaccel);
			break;
		}
		case OBJTYPE_GLASS: {
			struct glassobj *gls = (struct glassobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(gls->portalnum);
			break;
		}
		case OBJTYPE_AMMOCRATE: {
			struct ammocrateobj *ammo = (struct ammocrateobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_VAL(ammo->ammotype);
			break;
		}
		case OBJTYPE_MULTIAMMOCRATE: {
			struct multiammocrateobj *mammo = (struct multiammocrateobj *)obj;
			preprocessDefaultPropObj(obj);
			for (s32 i = 0; i < ARRAYCOUNT(mammo->slots); ++i) {
				PD_SWAP_VAL(mammo->slots[i].modelnum);
				PD_SWAP_VAL(mammo->slots[i].quantity);
			}
			break;
		}
		case OBJTYPE_TRUCK: {
			struct truckobj *truck = (struct truckobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(truck->ailist);
			break;
		}
		case OBJTYPE_HOVERCAR: {
			struct hovercarobj *car = (struct hovercarobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(car->ailist);
			break;
		}
		case OBJTYPE_CHOPPER: {
			struct chopperobj *chop = (struct chopperobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(chop->ailist);
			break;
		}
		case OBJTYPE_HELI: {
			struct heliobj *heli = (struct heliobj *)obj;
			preprocessDefaultPropObj(obj);
			PD_SWAP_PTR(heli->ailist);
			break;
		}
		case OBJTYPE_TAG: {
			struct tag *tag = (struct tag *)obj;
			PD_SWAP_PTR(tag->next);
			PD_SWAP_PTR(tag->obj);
			PD_SWAP_VAL(tag->cmdoffset);
			PD_SWAP_VAL(tag->tagnum);
			break;
		}
		case OBJTYPE_RENAMEOBJ: {
			struct textoverride *over = (struct textoverride *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(over->next);
			PD_SWAP_PTR(over->obj);
			PD_SWAP_VAL(over->objoffset);
			PD_SWAP_VAL(over->weapon);
			PD_SWAP_VAL(over->obtaintext);
			PD_SWAP_VAL(over->ownertext);
			PD_SWAP_VAL(over->inventorytext);
			PD_SWAP_VAL(over->inventory2text);
			PD_SWAP_VAL(over->pickuptext);
			break;
		}
		case OBJTYPE_BRIEFING: {
			struct briefingobj *brief = (struct briefingobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(brief->next);
			PD_SWAP_VAL(brief->type);
			PD_SWAP_VAL(brief->text);
			break;
		}
		case OBJTYPE_CAMERAPOS: {
			struct cameraposobj *campos = (struct cameraposobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(campos->x);
			PD_SWAP_VAL(campos->y);
			PD_SWAP_VAL(campos->z);
			PD_SWAP_VAL(campos->theta);
			PD_SWAP_VAL(campos->verta);
			PD_SWAP_VAL(campos->pad);
			break;
		}
		case OBJTYPE_BEGINOBJECTIVE: {
			struct objective *objective = (struct objective *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(objective->index);
			PD_SWAP_VAL(objective->text);
			PD_SWAP_VAL(objective->unk0c);
			break;
		}
		case OBJTYPE_ENDOBJECTIVE: {
			break;
		}
		case OBJTYPE_PADEFFECT: {
			struct padeffectobj *padeff = (struct padeffectobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(padeff->pad);
			PD_SWAP_VAL(padeff->effect);
			break;
		}
		case OBJTYPE_LINKGUNS: {
			struct linkgunsobj *linkg = (struct linkgunsobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_VAL(linkg->offset1);
			PD_SWAP_VAL(linkg->offset2);
			break;
		}
		case OBJTYPE_LINKLIFTDOOR: {
			struct linkliftdoorobj *linkd = (struct linkliftdoorobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(linkd->door);
			PD_SWAP_PTR(linkd->lift);
			PD_SWAP_PTR(linkd->next);
			PD_SWAP_VAL(linkd->stopnum);
			break;
		}
		case OBJTYPE_SAFEITEM: {
			struct safeitemobj *linki = (struct safeitemobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(linki->item);
			PD_SWAP_PTR(linki->safe);
			PD_SWAP_PTR(linki->door);
			PD_SWAP_PTR(linki->next);
			break;
		}
		case OBJTYPE_PADLOCKEDDOOR: {
			struct padlockeddoorobj *linkp = (struct padlockeddoorobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(linkp->door);
			PD_SWAP_PTR(linkp->lock);
			PD_SWAP_PTR(linkp->next);
			break;
		}
		case OBJTYPE_CONDITIONALSCENERY: {
			struct linksceneryobj *links = (struct linksceneryobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(links->trigger);
			PD_SWAP_PTR(links->unexp);
			PD_SWAP_PTR(links->exp);
			PD_SWAP_PTR(links->next);
			break;
		}
		case OBJTYPE_BLOCKEDPATH: {
			struct blockedpathobj *blkp = (struct blockedpathobj *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(blkp->blocker);
			PD_SWAP_PTR(blkp->next);
			PD_SWAP_VAL(blkp->waypoint1);
			PD_SWAP_VAL(blkp->waypoint2);
			break;
		}
		case OBJTYPE_MINE:
		case OBJTYPE_ESCASTEP:
		case OBJTYPE_HANGINGMONITORS:
		case OBJTYPE_BASIC:
		case OBJTYPE_ALARM:
		case OBJTYPE_DEBRIS:
		case OBJTYPE_GASBOTTLE:
		case OBJTYPE_29:
		case OBJTYPE_SAFE:
		case OBJTYPE_HAT: {
			preprocessDefaultPropObj(obj);
			break;
		}
		case OBJECTIVETYPE_ENTERROOM: {
			struct criteria_roomentered *obte = (struct criteria_roomentered *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(obte->next);
			PD_SWAP_VAL(obte->pad);
			PD_SWAP_VAL(obte->status);
			break;
		}
		case OBJECTIVETYPE_THROWINROOM: {
			struct criteria_throwinroom *obtt = (struct criteria_throwinroom *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(obtt->next);
			PD_SWAP_VAL(obtt->unk04);
			PD_SWAP_VAL(obtt->pad);
			PD_SWAP_VAL(obtt->status);
			break;
		}
		case OBJECTIVETYPE_HOLOGRAPH: {
			struct criteria_holograph *obth = (struct criteria_holograph *)obj;
			preprocessDefaultPropObjHdr(obj);
			PD_SWAP_PTR(obth->next);
			PD_SWAP_VAL(obth->obj);
			PD_SWAP_VAL(obth->status);
			break;
		}
		case OBJECTIVETYPE_DESTROYOBJ:
		case OBJECTIVETYPE_COMPFLAGS:
		case OBJECTIVETYPE_FAILFLAGS:
		case OBJECTIVETYPE_COLLECTOBJ:
		case OBJECTIVETYPE_THROWOBJ: {
			u32 *cmd = (u32 *)obj;
			PD_SWAP_VAL(cmd[1]);
			break;
		}
		default: {
			sysFatalError("Unknown object type in prop list: %02x\nat %p", obj->type, obj);
			break;
		}
	}
}

static void preprocessIntroScript(s32 *cmd)
{
	while (cmd[0] != PD_BE32(INTROCMD_END)) {
		PD_SWAP_VAL(cmd[0]);
		switch (cmd[0]) {
			case INTROCMD_HILL:
			case INTROCMD_OUTFIT:
			case INTROCMD_CREDITOFFSET:
			case INTROCMD_4:
				PD_SWAP_VAL(cmd[1]);
				cmd += 2;
				break;
			case INTROCMD_SPAWN:
			case INTROCMD_CASE:
			case INTROCMD_CASERESPAWN:
			case INTROCMD_WATCHTIME:
				PD_SWAP_VAL(cmd[1]);
				PD_SWAP_VAL(cmd[2]);
				cmd += 3;
				break;
			case INTROCMD_AMMO:
			case INTROCMD_WEAPON:
				PD_SWAP_VAL(cmd[1]);
				PD_SWAP_VAL(cmd[2]);
				PD_SWAP_VAL(cmd[3]);
				cmd += 4;
				break;
			case INTROCMD_3:
				PD_SWAP_VAL(cmd[1]);
				PD_SWAP_VAL(cmd[2]);
				PD_SWAP_VAL(cmd[3]);
				PD_SWAP_VAL(cmd[4]);
				PD_SWAP_VAL(cmd[5]);
				PD_SWAP_VAL(cmd[6]);
				PD_SWAP_VAL(cmd[7]);
				cmd += 8;
				break;
			case INTROCMD_6:
				PD_SWAP_VAL(cmd[1]);
				PD_SWAP_VAL(cmd[2]);
				PD_SWAP_VAL(cmd[3]);
				PD_SWAP_VAL(cmd[4]);
				PD_SWAP_VAL(cmd[5]);
				PD_SWAP_VAL(cmd[6]);
				PD_SWAP_VAL(cmd[7]);
				PD_SWAP_VAL(cmd[8]);
				PD_SWAP_VAL(cmd[9]);
				cmd += 10;
				break;
			default:
				cmd += 1;
				break;
		}
	}
	PD_SWAP_VAL(cmd[0]);
}

void preprocessAnimations(u8 *data, u32 size)
{
	// set the anim table pointers as well
	extern u8 *_animationsTableRomStart;
	extern u8 *_animationsTableRomEnd;

	// the animation table is at the end of the segment
	u32 *animtbl = (void *)(data + size - 0x38a0);
	_animationsTableRomStart = (u8 *)animtbl;
	_animationsTableRomEnd = data + size;

	PD_SWAP_VAL(*animtbl);
	const u32 count = *animtbl++;

	struct animtableentry *anim = (struct animtableentry *)animtbl;
	for (u32 i = 0; i < count; ++i, ++anim) {
		PD_SWAP_VAL(anim->numframes);
		PD_SWAP_VAL(anim->bytesperframe);
		PD_SWAP_VAL(anim->headerlen);
		PD_SWAP_VAL(anim->data);
		// if an external replacement exists, replace the table entry and mark the offset
		if (modAnimationLoadDescriptor(i, anim) > 0) {
			anim->data = 0xffffffff;
		}
	}
}

void preprocessMpConfigs(u8 *data, u32 size)
{
	const u32 count = size / sizeof(struct mpconfig);
	struct mpconfig *cfg = (struct mpconfig *)data;
	for (u32 i = 0; i < count; ++i, ++cfg) {
		PD_SWAP_VAL(cfg->setup.options);
		PD_SWAP_VAL(cfg->setup.teamscorelimit);
		PD_SWAP_VAL(cfg->setup.chrslots);
		// TODO: are these required or are they always 0?
		PD_SWAP_VAL(cfg->setup.fileguid.deviceserial);
		PD_SWAP_VAL(cfg->setup.fileguid.fileid);
		// convert MPWEAPON_ to take classic weapons into account, and combat knife in jpn-final
		for (s32 j = 0; j < ARRAYCOUNT(cfg->setup.weapons); ++j) {
#if VERSION == VERSION_JPN_FINAL
			// old MPWEAPON_COMBATKNIFE (0x00) and above are affected in jpn-final
			if (cfg->setup.weapons[j] == 0x00) {
				cfg->setup.weapons[j] = cfg->setup.weapons[j + 1];
			} else if (cfg->setup.weapons[j] >= MPWEAPON_COMBATKNIFE) {
				cfg->setup.weapons[j] += 0x01;
			}
#endif
			// old MPWEAPON_SHIELD (0x24/0x25) and above are affected
			if (cfg->setup.weapons[j] >= MPWEAPON_PP9I) {
				cfg->setup.weapons[j] += (MPWEAPON_SHIELD - MPWEAPON_PP9I);
			}
		}
	}
}

void preprocessFont(u8 *data, u32 size)
{
	struct font *fnt = (struct font *)data;

	for (s32 i = 0; i < ARRAYCOUNT(fnt->kerning); ++i) {
		PD_SWAP_VAL(fnt->kerning[i]);
	}

	s32 numchars = 94;

#if PAL
	// PAL has more characters in these fonts
	extern u8 *_fonthandelgothicsmSegmentRomStart;
	extern u8 *_fonthandelgothicxsSegmentRomStart;
	extern u8 *_fonthandelgothicmdSegmentRomStart;
	if (data == _fonthandelgothicsmSegmentRomStart
			|| data == _fonthandelgothicxsSegmentRomStart
			|| data == _fonthandelgothicmdSegmentRomStart) {
		numchars = 135;
	}
#endif

	for (s32 i = 0; i < numchars; ++i) {
		PD_SWAP_VAL(fnt->chars[i].kerningindex);
		PD_SWAP_PTR(fnt->chars[i].pixeldata);
	}
}

void preprocessJpnFont(u8 *data, u32 size)
{
	// ???
}

void preprocessALBankFile(u8 *data, u32 size)
{
	memset(swapmap, 0, sizeof(swapmap));

	ALBankFile *bankf = (ALBankFile *)data;
	PD_SWAP_VAL(bankf->revision);
	PD_SWAP_VAL(bankf->bankCount);

	for (s16 i = 0; i < bankf->bankCount; ++i) {
		PD_SWAP_PTR(bankf->bankArray[i]);
		preprocessALBank(PD_PTR_BASE(bankf->bankArray[i], data), data);
	}
}

void preprocessALCMidiHdr(u8 *data, u32 size)
{
	ALCMidiHdr *hdr = (ALCMidiHdr *)data;
	PD_SWAP_VAL(hdr->division);
	for (s32 i = 0; i < ARRAYCOUNT(hdr->trackOffset); ++i) {
		PD_SWAP_VAL(hdr->trackOffset[i]);
	}
}

void preprocessSequences(u8 *data, u32 size)
{
	struct seqtable *seq = (struct seqtable *)data;
	PD_SWAP_VAL(seq->count);

	for (s16 i = 0; i < seq->count; ++i) {
		PD_SWAP_VAL(seq->entries[i].binlen);
		PD_SWAP_VAL(seq->entries[i].ziplen);
		PD_SWAP_VAL(seq->entries[i].romaddr);
	}
}

void preprocessTexturesList(u8 *data, u32 size)
{
	struct texture *tex = (struct texture *)data;
	const u32 count = size / sizeof(*tex);
	for (u32 i = 0; i < count; ++i, ++tex) {
		// TODO: it sure looks like none of the fields except soundsurfacetype, surfacetype and dataoffset are set
		// just swap the last 3 bytes of the first word...
		const u32 dofs = (u32)tex->dataoffset << 8;
		tex->dataoffset = PD_BE32(dofs);
		// ...and the surface types in the first byte
		const u8 tmp = tex->soundsurfacetype;
		tex->soundsurfacetype = tex->surfacetype;
		tex->surfacetype = tmp;
	}
}

void preprocessTextureRGBA32Embedded(u32* dest, u32 size_bytes)
{
	for (uint32_t i = 0; i < size_bytes; i += 4, ++dest) {
		*dest = PD_BE32(*dest);
	}
}

void preprocessModel(u8 *base, u32 ofs)
{
	struct modeldef *mdl = (struct modeldef *)base;

	PD_SWAP_VAL(mdl->nummatrices);
	PD_SWAP_VAL(mdl->numparts);
	PD_SWAP_VAL(mdl->numtexconfigs);
	PD_SWAP_VAL(mdl->rwdatalen);
	PD_SWAP_VAL(mdl->scale);

	if (mdl->skel) {
		PD_SWAP_PTR(mdl->skel);
		if ((u32)mdl->skel >= 0x10000) {
			// skeleton is a pointer instead of an index
			// preprocessSkeleton((struct skeleton *)(base + (u32)mdl->skel)); // TODO
		}
	}

	if (mdl->parts) {
		PD_SWAP_PTR(mdl->parts);
		struct modelnode **parts = PD_PTR_BASEOFS(mdl->parts, base, ofs);
		for (s16 i = 0; i < mdl->numparts; ++i) {
			PD_SWAP_PTR(parts[i]);
		}
		s16 *partnums = (s16 *)&parts[mdl->numparts];
		for (s16 i = 0; i < mdl->numparts; ++i) {
			PD_SWAP_VAL(partnums[i]);
		}
	}

	u8 *texturesEnd = NULL;
	if (mdl->rootnode) {
		PD_SWAP_PTR(mdl->rootnode);
		texturesEnd = preprocessModelNode(PD_PTR_BASEOFS(mdl->rootnode, base, ofs), base, ofs);
	}

	if (mdl->texconfigs) {
		PD_SWAP_PTR(mdl->texconfigs);
		struct textureconfig *texconfigs = PD_PTR_BASEOFS(mdl->texconfigs, base, ofs);
		for (s16 i = 0; i < mdl->numtexconfigs; ++i) {
			PD_SWAP_VAL(texconfigs[i].texturenum);
			if ((texconfigs[i].texturenum & 0xf000000) == 0x5000000) {
				// embedded texture; we need to unswizzle this
				u8 *texdata = PD_PTR_BASEOFS(texconfigs[i].textureptr, base, ofs);
				// figure out the max possible size the texture can have, because sometimes the texconfig is wrong
				const u32 maxSize = (texturesEnd > texconfigs[i].textureptr) ? (texturesEnd - texconfigs[i].textureptr) : 0;
				// figure out the format and unswizzle
				const s32 format = texConfigToFormat(&texconfigs[i]);
				texSwizzleInternal(texdata, texconfigs[i].width, texconfigs[i].height, format, maxSize);

				if (format == TEXFORMAT_RGBA32) {
					// for some reason, RGBA32 embedded textures don't need to be byte-swapped,
					// so we byte-swap them here, which will be undone when the renderer imports it
					u32 size_bytes = texconfigs[i].width * texconfigs[i].height * 4;
					preprocessTextureRGBA32Embedded((u32*)texdata, size_bytes);
				}
			}
		}
	}
}

void preprocessLangFile(u8 *data, u32 size)
{
	// lang banks are just an offset table + text data right after
	// offsets are from the beginning of the bank
	// however we cannot byteswap them in advance because the length of the
	// offset table is unknown, so that's done in lang.c on demand instead
}

void preprocessPadsFile(u8 *data, u32 size)
{
	struct padsfileheader *hdr = (void *)data;

	PD_SWAP_VAL(hdr->numpads);
	PD_SWAP_VAL(hdr->numcovers);

	for (s32 i = 0; i < hdr->numpads; ++i) {
		if (hdr->padoffsets[i]) {
			PD_SWAP_VAL(hdr->padoffsets[i]);
			const u32 size = (i + 1 < hdr->numpads && hdr->padoffsets[i + 1]) ?
				(PD_BE16(hdr->padoffsets[i + 1]) - hdr->padoffsets[i]) : 0x40;
			preprocessPadData(PD_PTR_BASE(hdr->padoffsets[i], hdr), size);
		}
	}

	if (hdr->numcovers && hdr->coversoffset) {
		PD_SWAP_VAL(hdr->coversoffset);
		struct coverdefinition *def = PD_PTR_BASE(hdr->coversoffset, hdr);
		for (s32 i = 0; i < hdr->numcovers; ++i, ++def) {
			PD_SWAP_VAL(def->flags);
			PD_SWAP_VAL(def->look);
			PD_SWAP_VAL(def->pos);
		}
	}

	if (hdr->waypointsoffset) {
		PD_SWAP_VAL(hdr->waypointsoffset);
		struct waypoint *wp = PD_PTR_BASE(hdr->waypointsoffset, hdr);
		// the terminator is 0xFFFFFFFF, same for the s32 lists
		// TODO: the game checks for >= 0, so maybe something's terminated with a random negative number?
		while (wp->padnum != (s32)0xFFFFFFFF) {
			PD_SWAP_VAL(wp->padnum);
			PD_SWAP_VAL(wp->groupnum);
			PD_SWAP_VAL(wp->step);
			if (wp->neighbours) {
				PD_SWAP_PTR(wp->neighbours);
				s32 *nbr = PD_PTR_BASE(wp->neighbours, hdr);
				while (*nbr != (s32)0xFFFFFFFF) {
					PD_SWAP_VAL(*nbr);
					++nbr;
				}
			}
			++wp;
		}
	}

	if (hdr->waygroupsoffset) {
		PD_SWAP_VAL(hdr->waygroupsoffset);
		struct waygroup *wg = PD_PTR_BASE(hdr->waygroupsoffset, hdr);
		while (wg->neighbours) {
			PD_SWAP_VAL(wg->step);
			if (wg->neighbours) {
				PD_SWAP_PTR(wg->neighbours);
				s32 *nbr = PD_PTR_BASE(wg->neighbours, hdr);
				while (*nbr != (s32)0xFFFFFFFF) {
					PD_SWAP_VAL(*nbr);
					++nbr;
				}
			}
			if (wg->waypoints) {
				PD_SWAP_PTR(wg->waypoints);
				s32 *wgwp = PD_PTR_BASE(wg->waypoints, hdr);
				while (*wgwp != (s32)0xFFFFFFFF) {
					PD_SWAP_VAL(*wgwp);
					++wgwp;
				}
			}
			++wg;
		}
	}
}

void preprocessTilesFile(u8 *data, u32 size)
{
	u32 *roomTable = (u32 *)data;
	PD_SWAP_VAL(roomTable[0]);

	const u32 numRooms = roomTable[0];
	++roomTable;

	u32 *rooms = roomTable;
	// rooms[numRooms] is the end offset
	for (u32 i = 0; i <= numRooms; ++i) {
		PD_SWAP_VAL(rooms[i]);
	}

	struct geo *geo = (struct geo *)(data + rooms[0]);
	struct geo *end = (struct geo *)(data + rooms[numRooms]);

	while (geo < end) {
		PD_SWAP_VAL(geo->flags);
		switch (geo->type) {
			case GEOTYPE_TILE_I: {
				struct geotilei *tile = (struct geotilei *)geo;
				PD_SWAP_VAL(tile->floorcol);
				PD_SWAP_VAL(tile->floortype);
				for (u32 i = 0; i < geo->numvertices; ++i) {
					PD_SWAP_VAL(tile->vertices[i][0]);
					PD_SWAP_VAL(tile->vertices[i][1]);
					PD_SWAP_VAL(tile->vertices[i][2]);
				}
				geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 6 + 0x18e);
				break;
			}
			case GEOTYPE_TILE_F: {
				struct geotilef *tile = (struct geotilef *)geo;
				PD_SWAP_VAL(tile->floorcol);
				PD_SWAP_VAL(tile->floortype);
				for (u32 i = 0; i < geo->numvertices; ++i) {
					PD_SWAP_VAL(tile->vertices[i].x);
					PD_SWAP_VAL(tile->vertices[i].y);
					PD_SWAP_VAL(tile->vertices[i].z);
				}
				geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 12 + 0x310);
				break;
			}
			case GEOTYPE_BLOCK: {
				struct geoblock *blk = (struct geoblock *)geo;
				PD_SWAP_VAL(blk->ymin);
				PD_SWAP_VAL(blk->ymax);
				for (u32 i = 0; i < geo->numvertices; ++i) {
					PD_SWAP_VAL(blk->vertices[i][0]);
					PD_SWAP_VAL(blk->vertices[i][1]);
				}
				geo = (struct geo *)((u8 *)geo + sizeof(struct geoblock));
				break;
			}
			case GEOTYPE_CYL: {
				struct geocyl *cyl = (struct geocyl *)geo;
				PD_SWAP_VAL(cyl->x);
				PD_SWAP_VAL(cyl->z);
				PD_SWAP_VAL(cyl->radius);
				PD_SWAP_VAL(cyl->ymin);
				PD_SWAP_VAL(cyl->ymax);
				geo = (struct geo *)((u8 *)geo + sizeof(struct geocyl));
				break;
			}
			default:
				break;
		}
	}
}

void preprocessSetupFile(u8 *data, u32 size)
{
	struct stagesetup *set = (void *)data;

	if (set->intro) {
		// intro commands are made up of a command id + up to 3 args, which can have any value, even 0x0c (end marker)
		// so we have to swap each type by hand here as well
		PD_SWAP_PTR(set->intro);
		preprocessIntroScript(PD_PTR_BASE(set->intro, set));
	}

	if (set->props) {
		PD_SWAP_PTR(set->props);
		// commands are casted to non-trivial structs elsewhere, so we'll have to parse the fucking list
		// and swap each type by hand
		struct defaultobj *obj = PD_PTR_BASE(set->props, set);
		while (obj->type != OBJTYPE_END) {
			preprocessPropObj(obj);
			obj = (struct defaultobj *)((u32 *)obj + setupGetCmdLength((u32 *)obj));
		}
	}

	if (set->paths) {
		PD_SWAP_PTR(set->paths);
		struct path *p = PD_PTR_BASE(set->paths, set);
		while ((u8 *)p < data + size && p->pads) {
			PD_SWAP_PTR(p->pads);
			s32 *pp = PD_PTR_BASE(p->pads, set);
			while ((u8 *)pp < data + size) {
				PD_SWAP_VAL(*pp);
				if (*pp < 0) {
					break;
				}
				++pp;
			}
			++p;
		}
	}

	if (set->ailists) {
		PD_SWAP_PTR(set->ailists);
		struct ailist *p = PD_PTR_BASE(set->ailists, set);
		while ((u8 *)p < data + size && p->list) {
			PD_SWAP_VAL(p->id);
			PD_SWAP_PTR(p->list);
			// the actual command lists are made up of variable length instructions,
			// but they don't do any wonky word casting, so they should probably work as is
			++p;
		}
	}

	// the rest of the pointers are set after loading
}

void preprocessModelFile(u8 *data, u32 size)
{
	preprocessModel(data, 0x5000000);
}

void preprocessGunFile(u8 *data, u32 size)
{
	preprocessModel(data, 0x5000000);
}

void preprocessBgSection1Header(u8 *data, u32 size)
{
	u32 *header = (u32 *)data;
	PD_SWAP_VAL(header[0]); // inflatedsize
	PD_SWAP_VAL(header[1]); // section1size
	PD_SWAP_VAL(header[2]); // primcompsize
}

void preprocessBgSection1(u8 *data, u32 ofs) {
	struct {
		u32 unk00;
		u32 roomsofs;
		u32 portalsofs;
		u32 commandsofs;
		u32 lightsofs;
		u32 stanofs;
	} *header = (void *)data;

	PD_SWAP_VAL(header->unk00);
	PD_SWAP_VAL(header->roomsofs);
	PD_SWAP_VAL(header->portalsofs);

	struct bgroom *rm = PD_PTR_BASEOFS(header->roomsofs, data, ofs);
	for (++rm; rm->unk00; ++rm) {
		PD_SWAP_VAL(rm->unk00);
		PD_SWAP_VAL(rm->pos);
	}

	struct bgportal *portals = PD_PTR_BASEOFS(header->portalsofs, data, ofs);
	s32 numportals = 0;
	for (struct bgportal *prt = portals; prt->verticesoffset; ++prt) {
		++numportals;
	}

	s32 maxbatchnum = 0;
	for (s32 i = 0; i < numportals; ++i) {
		PD_SWAP_VAL(portals[i].verticesoffset);
		PD_SWAP_VAL(portals[i].roomnum1);
		PD_SWAP_VAL(portals[i].roomnum2);
		if (portals[i].verticesoffset > maxbatchnum) {
			maxbatchnum = portals[i].verticesoffset;
		}
	}

	uintptr_t pvoffset = sizeof(portals[0]) * (numportals + 1);
	for (s32 i = 1; i <= maxbatchnum; i++) {
		struct portalvertices *pverts = PD_PTR_BASE(pvoffset, portals);
		for (u32 j = 0; j < pverts->count; ++j) {
			PD_SWAP_VAL(pverts->vertices[j]);
		}
		pvoffset += 4 + pverts->count * 12;
	}

	if (header->commandsofs) {
		PD_SWAP_VAL(header->commandsofs);
		struct bgcmd *cmd = PD_PTR_BASEOFS(header->commandsofs, data, ofs);
		for (; cmd->type != 0x00; cmd++) {
			PD_SWAP_VAL(cmd->param);
		}
	}

	if (header->lightsofs) {
		PD_SWAP_VAL(header->lightsofs);
		// data swapping done later in preprocessBgLights
	}

	if (header->stanofs) {
		PD_SWAP_VAL(header->stanofs);
		// unused, so won't swap the contents
	}
}

void preprocessBgSection2Header(u8 *data, u32 size)
{
	u16 *header = (u16 *)data;
	PD_SWAP_VAL(header[0]); // inflatedsize
	PD_SWAP_VAL(header[1]); // section2compsize
}

void preprocessBgSection2(u8 *data, u32 size)
{
	// section2 is a texture id list
	u16 *section2 = (u16 *)data;
	for (s32 i = 0; i < size; ++i) {
		PD_SWAP_VAL(section2[i]);
	}
}

void preprocessBgSection3Header(u8 *data, u32 size)
{
	u16 *header = (u16 *)data;
	PD_SWAP_VAL(header[0]);
	PD_SWAP_VAL(header[1]);
}

void preprocessBgSection3(u8 *data, u32 size)
{
	// room bounding boxes, 6x s16 per room
	s16 *bbox = (s16 *)data;
	for (s32 i = 1; i < g_Vars.roomcount; ++i) {
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
	}

	// gfxdatalen list
	u16 *gfxdatalen = (u16 *)bbox;
	for (s32 i = 1; i < g_Vars.roomcount; ++i) {
		PD_SWAP_VAL(*gfxdatalen); ++gfxdatalen;
	}
}

void preprocessBgRoom(u8 *data, u32 ofs) {
	struct roomgfxdata *rgfx = (void *)data;

	PD_SWAP_VAL(rgfx->numvertices);
	PD_SWAP_VAL(rgfx->numcolours);
	PD_SWAP_VAL(rgfx->numlights);
	PD_SWAP_VAL(rgfx->lightsindex);
	PD_SWAP_PTR(rgfx->vertices);
	PD_SWAP_PTR(rgfx->opablocks);
	PD_SWAP_PTR(rgfx->xlublocks);
	PD_SWAP_PTR(rgfx->colours);

	// numvertices is actually 0 in the data because fuck you
	const s32 numverts = ((uintptr_t) rgfx->colours - (uintptr_t) rgfx->vertices) / sizeof(Vtx);
	Vtx *vertices = NULL;
	if (rgfx->vertices) {
		vertices = PD_PTR_BASEOFS(rgfx->vertices, data, ofs);
		Vtx *vtx = vertices;
		for (s32 i = 0; i < numverts; ++i, ++vtx) {
			preprocessVtx(vtx);
		}
	}

	// here's hoping that xlublocks and opablocks aren't completely separate trees

	struct roomblock *blk = rgfx->blocks;
	struct roomblock *end = (void *)vertices;
	struct coord *tmp;

	for (; blk + 1 <= end; blk++) {
		PD_SWAP_PTR(blk->next);
		switch (blk->type) {
			case ROOMBLOCKTYPE_LEAF:
				if (blk->gdl) {
					PD_SWAP_PTR(blk->gdl);
					preprocessGfx(PD_PTR_BASEOFS(blk->gdl, data, ofs), data, ofs);
				}
				PD_SWAP_PTR(blk->vertices);
				PD_SWAP_PTR(blk->colours);
				break;
			case ROOMBLOCKTYPE_PARENT:
				PD_SWAP_PTR(blk->child);
				PD_SWAP_PTR(blk->unk0c);
				tmp = PD_PTR_BASEOFS(blk->unk0c, data, ofs);
				for (s32 i = 0; i < 2; ++i) {
					PD_SWAP_VAL(tmp[i].x);
					PD_SWAP_VAL(tmp[i].y);
					PD_SWAP_VAL(tmp[i].z);
				}
				if ((u8 *)tmp < (u8 *)end) {
					end = (void *)tmp;
				}
				break;
			default:
				break;
		}
	}
}

void preprocessBgLights(u8 *data, u32 ofs)
{
	struct light *lbase = (void *)data;
	if (!lbase) {
		return;
	}

	for (s32 i = 0; i < g_Vars.roomcount; ++i) {
		if (g_Rooms[i].numlights) {
			struct light *lit = &lbase[g_Rooms[i].lightindex];
			for (s32 j = 0; j < g_Rooms[i].numlights; ++j, ++lit) {
				PD_SWAP_VAL(lit->colour);
				PD_SWAP_VAL(lit->roomnum);
				for (s32 k = 0; k < ARRAYCOUNT(lit->bbox); ++k) {
					PD_SWAP_VAL(lit->bbox[k].x);
					PD_SWAP_VAL(lit->bbox[k].y);
					PD_SWAP_VAL(lit->bbox[k].z);
				}
			}
		}
	}
}
