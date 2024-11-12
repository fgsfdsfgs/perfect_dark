#include <ultra64.h>
#include "constants.h"
#include "game/bg.h"
#include "game/chraction.h"
#include "game/chr.h"
#include "game/vtxstore.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/mema.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

const char var7f1b5230[] = "VTXSTORE : vtxfixrefs -> Start - p1=%x, p2=%x\n";
const char var7f1b5260[] = "vtxfixrefs : Part=%x -- Mapping ptr %x -> %x\n";
const char var7f1b5290[] = "VTXSTORE : vtxfixrefs -> End - Done=%d\n";
const char var7f1b52b8[] = "vtxstorecheck : memaFree -> %u bytes at Ptr=%x(%x)\n";
const char var7f1b52ec[] = "vtxstorecheck : At block 1 %d -> Ref1=%x, Ref2=%x\n";
const char var7f1b5320[] = "vtxstorecheck : At block 2 %d -> Ref1=%x, Ref2=%x\n";
const char var7f1b5354[] = "vtx buffer low, need to delete objects\n";
const char var7f1b537c[] = "getfreevertices : %d of type %d -> ref1=%x, ref2=%x\n";
const char var7f1b53b4[] = "vtxstore: 1st mema alloc of %u bytes\n";

#if VERSION < VERSION_NTSC_1_0
const char var7f1af8ecnb[] = "vtxstore: Trying to free %d from mema (bgRooms)\n";
#endif

const char var7f1b53dc[] = "getfreevertices : Return ptr = %x\n";
const char var7f1b5400[] = "vtxstore: Out of mema (returning NULL)\n";
const char var7f1b5428[] = "vtxstore: GROSS! CorspeCount > MAX_CORPSES corpses! Freeing corpse %x\n";
const char var7f1b5470[] = "vtxstore:  CorpseCount %d, Trying to free %d\n";
const char var7f1b54a0[] = "vtxstore: Freeing corpse %x\n";
const char var7f1b54c0[] = "vtxstore: Out of vertices type %d wanted %d free %d (returning NULL)\n";
const char var7f1b5508[] = "vtxstore: freevertices type %d, list %x\n";
const char var7f1b5534[] = "freevertices: address not found in array %x\n";

struct vtxstoretype g_VtxstoreTypes[] = {
	{ 3000, 120, 3000, 80, 0, 0, 500,  20, 12, 0, 0, 0, 0 },
	{ 1500, 40,  500,  20, 0, 0, 500,  20, 12, 0, 0, 0, 0 },
	{ 6000, 120, 6000, 80, 0, 0, 1000, 20, 4,  0, 0, 0, 0 },
	{ 1500, 40,  500,  20, 0, 0, 500,  20, 4,  0, 0, 0, 0 },
};

/**
 * Search all props and their model data for something, and replace it with
 * something else.
 */
void vtxstoreFixRefs(void *find, void *replacement)
{
	u32 stack;
	struct prop *prop = g_Vars.activeprops;

	while (prop) {
		if (prop->type == PROPTYPE_OBJ) {
			struct defaultobj *obj = prop->obj;
			struct model *model = obj->model;
			struct modeldef *modeldef = model->definition;
			struct modelnode *node = modeldef->rootnode;
			struct modelrodata_dl *rodata;

			while (node) {
				switch (node->type & 0xff) {
				case MODELNODETYPE_DL:
					rodata = &node->rodata->dl;

					if (model->rwdatas[rodata->rwdataindex] == (uintptr_t) find) {
						model->rwdatas[rodata->rwdataindex] = (uintptr_t) replacement;
					}
					break;
				case MODELNODETYPE_DISTANCE:
					modelApplyDistanceRelations(obj->model, node);
					break;
				case MODELNODETYPE_TOGGLE:
					modelApplyToggleRelations(obj->model, node);
					break;
				case MODELNODETYPE_HEADSPOT:
					modelApplyHeadRelations(obj->model, node);
					break;
				}

				if (node->child) {
					node = node->child;
				} else {
					while (node) {
						if (node->next) {
							node = node->next;
							break;
						}

						node = node->parent;
					}
				}
			}
		}

		prop = prop->next;
	}
}

void vtxstoreTick(void)
{
	s32 i;
	s32 j;

	if (g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].val2 < g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].val1 >> 2) {
		for (i = 0; i < g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].numallocated - 1; i++) {
			if (g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[i].unk0e > 0) {
				for (j = i + 1; j < g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].numallocated; j++) {
					if (g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].unk0e > 0
							&& g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[i].node == g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].node
							&& g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[i].level == g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].level) {
						s32 size = ALIGN16(g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].count * 0x0c);
						vtxstoreFixRefs(g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].unk00, g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[i].unk00);
						g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[i].unk0e += g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].unk0e;
						memaFree(g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].unk00, size);
						g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].unk0e = 0;
						g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].val2 += g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].unk24[j].count;
					}
				}
			}
		}
	}

	if (g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].val2 < g_VtxstoreTypes[VTXSTORETYPE_OBJVTX].val1 >> 2) {
		func0f091030();
	}
}

void *vtxstoreAllocate(s32 count, s32 index, struct modelnode *node, s32 level)
{
	s32 i;
	s32 numchrs;
	s32 tally;
	s32 rand;
	u32 size;
	struct chrdata *chrs[6];

#if VERSION >= VERSION_NTSC_1_0
	if (IS4MB()) {
		return NULL;
	}
#endif

	if (count <= g_VtxstoreTypes[index].val2) {
		for (i = 0; i < g_VtxstoreTypes[index].numallocated; i++) {
			if (g_VtxstoreTypes[index].unk24[i].unk0e == 0) {
				size = ALIGN16(count * 0xc);
				g_VtxstoreTypes[index].unk24[i].unk00 = memaAlloc(size);

#if VERSION < VERSION_NTSC_1_0
				if (!g_VtxstoreTypes[index].unk24[i].unk00) {
					bgGarbageCollectRooms(size, false);
					g_VtxstoreTypes[index].unk24[i].unk00 = memaAlloc(size);
				}
#endif

				if (g_VtxstoreTypes[index].unk24[i].unk00) {
					g_VtxstoreTypes[index].unk24[i].count = count;
					g_VtxstoreTypes[index].unk24[i].unk0e = 1;
					g_VtxstoreTypes[index].unk24[i].node = node;
					g_VtxstoreTypes[index].unk24[i].level = level;
					g_VtxstoreTypes[index].val2 -= count;

					return g_VtxstoreTypes[index].unk24[i].unk00;
				}

				return NULL;
			}
		}
	}

	// Build an array of all corpses. If the array becomes full then enable
	// reaping on a random corpse and replace its entry in the array.
	// So at the end, we'll have an array of up to six unreapable corpses and
	// all other corpses will be flagged for reaping.
	numchrs = chrsGetNumSlots();
	tally = 0;

	for (i = 0; i < numchrs; i++) {
		struct chrdata *chr = &g_ChrSlots[i];

		if (chr->model
				&& chr->prop
				&& (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) == 0
				&& chr->actiontype == ACT_DEAD
				&& chr->act_dead.fadewheninvis == false) {
			if (tally < 6) {
				chrs[tally] = chr;
				tally++;
			} else {
				rand = random() % tally;
				chrFadeCorpseWhenOffScreen(chrs[rand]);
				chrs[rand] = chr;
			}
		}
	}

	// Enable reaping on half the remaining corpses.
	// I'm reusing the rand and i variables here in order to get a match.
	// The original code likely didn't reuse them.
	rand = tally >> 1;

	while (rand) {
		i = random() % tally;

		if (chrs[i]) {
			chrFadeCorpseWhenOffScreen(chrs[i]);
			chrs[i] = NULL;
			rand--;
		}
	}

	return NULL;
}

void vtxstoreFree(s32 type, void *arg1)
{
	s32 i;

	for (i = 0; i < g_VtxstoreTypes[type].numallocated; i++) {
		if (g_VtxstoreTypes[type].unk24[i].unk0e > 0 && arg1 == g_VtxstoreTypes[type].unk24[i].unk00) {
			g_VtxstoreTypes[type].unk24[i].unk0e--;

			if (g_VtxstoreTypes[type].unk24[i].unk0e) {
				return;
			}

			memaFree(g_VtxstoreTypes[type].unk24[i].unk00, ALIGN16(g_VtxstoreTypes[type].unk24[i].count * 0xc));

			g_VtxstoreTypes[type].val2 += g_VtxstoreTypes[type].unk24[i].count;
			return;
		}
	}
}
