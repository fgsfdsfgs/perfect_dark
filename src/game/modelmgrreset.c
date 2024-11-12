#include <ultra64.h>
#include "constants.h"
#include "constants.h"
#include "game/bondmove.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/playerreset.h"
#include "game/chr.h"
#include "game/body.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/inv.h"
#include "game/stagetable.h"
#include "game/mplayer/scenarios.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "bss.h"
#include "lib/collision.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "data.h"
#include "types.h"

#define NUMTYPE1() (IS4MB() ? 0 : 35)
#define NUMTYPE2() (IS4MB() ? 24 : 25)
#define NUMTYPE3() (IS4MB() ? 0 : 20)
#define NUMSPARE() (IS4MB() ? 40 : 60)

void modelmgrReset(void)
{
	g_MaxAnims = 0;
	g_MaxModels = 0;
}

void modelmgrSetLvResetting(bool value)
{
	g_ModelIsLvResetting = value;
}

/**
 * Allocate slots for model instances and animations.
 *
 * Each model instance can require rwdata: a block of bytes where it can save
 * information specific to its own instance.
 *
 * The rwdata slots are split into 3 types. Types 1 and 2 use different slot
 * sizes, while type 3 has no allocated slots at all. When models are
 * instantiated they require a certain amount of rwdata, and are bound to type 1
 * if possible, otherwise type 2 if possible. If neither of those are possible
 * then it allocates rwdata on the fly and puts it in type 3.
 *
 * The function allocates more than what is specified in the arguments.
 * If the expansion pak is being used, 60 spare model slots are allocated
 * (eg. for thrown weapons), and a further 20 model and 20 anim slots are
 * allocated for animated objects.
 */
void modelmgrAllocateSlots(s32 numobjs, s32 numchrs)
{
	s32 rwdata2sizetotal;
	s32 rwdata1sizetotal;
	s32 rwdata3sizetotal;
	s32 bindingssize;
	u8 *ptr;
	s32 i;
	s32 totalsize;
	s32 modelssize;
	s32 animssize;
	s32 rwdata1sizeeach = 0x10;
	s32 rwdata2sizeeach = 0xd0;
	s32 rwdata3sizeeach = 0;
	s32 maxanimatedobjs = 20;
	s32 numspare;

	g_ModelNumObjs = numobjs;
	g_ModelNumChrs = numchrs;

	numspare = NUMSPARE();
	g_MaxModels = numobjs + numspare + numchrs + maxanimatedobjs;
	g_MaxAnims = numchrs + maxanimatedobjs;

	i = NUMTYPE2();
	bindingssize = (NUMTYPE1() + i + NUMTYPE3()) * sizeof(struct modelrwdatabinding);

	rwdata1sizetotal = NUMTYPE1() * rwdata1sizeeach;
	rwdata2sizetotal = NUMTYPE2() * rwdata2sizeeach;
	rwdata3sizetotal = NUMTYPE3() * rwdata3sizeeach;

	modelssize = ALIGN16(g_MaxModels * sizeof(struct model));
	animssize = ALIGN16(g_MaxAnims * sizeof(struct anim));

	totalsize = ALIGN16(bindingssize + rwdata1sizetotal + rwdata2sizetotal + rwdata3sizetotal + modelssize + animssize);

	g_ModelRwdataBindings[0] = NULL;
	g_ModelRwdataBindings[1] = NULL;
	g_ModelRwdataBindings[2] = NULL;

	ptr = mempAlloc(totalsize, MEMPOOL_STAGE);

	if (NUMTYPE1()) {
		g_ModelRwdataBindings[0] = (struct modelrwdatabinding *) ptr;
		ptr += NUMTYPE1() * sizeof(struct modelrwdatabinding);
	}

	if (NUMTYPE2()) {
		g_ModelRwdataBindings[1] = (struct modelrwdatabinding *) ptr;
		ptr += NUMTYPE2() * sizeof(struct modelrwdatabinding);
	}

	if (NUMTYPE3()) {
		g_ModelRwdataBindings[2] = (struct modelrwdatabinding *) ptr;
		ptr += NUMTYPE3() * sizeof(struct modelrwdatabinding);
	}

	g_ModelSlots = (struct model *) ptr;
	ptr += modelssize;
	g_AnimSlots = (struct anim *) ptr;
	ptr += animssize;

	for (i = 0; i < NUMTYPE1(); i++) {
		g_ModelRwdataBindings[0][i].rwdata = ptr;
		g_ModelRwdataBindings[0][i].model = NULL;

		ptr += rwdata1sizeeach;
	}

	for (i = 0; i < NUMTYPE2(); i++) {
		g_ModelRwdataBindings[1][i].rwdata = ptr;
		g_ModelRwdataBindings[1][i].model = NULL;

		ptr += rwdata2sizeeach;
	}

	for (i = 0; i < NUMTYPE3(); i++) {
		g_ModelRwdataBindings[2][i].rwdata = NULL;
		g_ModelRwdataBindings[2][i].model = NULL;

		ptr += rwdata3sizeeach;
	}

	for (i = 0; i < g_MaxModels; i++) {
		g_ModelSlots[i].definition = NULL;
		g_ModelSlots[i].rwdatas = NULL;
	}

	for (i = 0; i < g_MaxAnims; i++) {
		g_AnimSlots[i].animnum = -1;
	}

	g_ModelMostType1 = 0;
	g_ModelMostType2 = 0;
	g_ModelMostType3 = 0;
	g_ModelMostModels = 0;
	g_ModelMostAnims = 0;
}

bool modelmgrLoadProjectileModeldefs(s32 weaponnum)
{
	bool result = false;
	struct weapon *weapon = g_Weapons[weaponnum];
	s32 i;

	for (i = 0; i != 2; i++) {
		if (weapon->functions[i]) {
			struct weaponfunc *genericfunc = weapon->functions[i];

			if (genericfunc->type == INVENTORYFUNCTYPE_SHOOT_PROJECTILE) {
				struct weaponfunc_shootprojectile *func = (struct weaponfunc_shootprojectile *)genericfunc;

				if (func->projectilemodelnum >= 0) {
					result |= setupLoadModeldef(func->projectilemodelnum);
				}
			} else if (genericfunc->type == INVENTORYFUNCTYPE_THROW) {
				struct weaponfunc_throw *func = (struct weaponfunc_throw *)genericfunc;

				if (func->projectilemodelnum >= 0) {
					result |= setupLoadModeldef(func->projectilemodelnum);
				}
			}
		}
	}

	return result;
}
