#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <PR/ultratypes.h>
#include <PR/gbi.h>

#include "data.h"
#include "bss.h"
#include "game/setuputils.h"
#include "game/texdecompress.h"
#include "mod.h"

#include "preprocess/common.h"

u8 *preprocessAnimations(u8* data, u32 size, u32* outSize)
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

	return NULL;
}

u8 *preprocessMpConfigs(u8* data, u32 size, u32* outSize)
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
		// convert MPWEAPON_ to take classic weapons and JPN weapons into account
		for (s32 j = 0; j < ARRAYCOUNT(cfg->setup.weapons); ++j) {
#if VERSION == VERSION_JPN_FINAL /* TODO: replace with runtime check */
			if (cfg->setup.weapons[j] >= 0x24) {
				// weapons after and including the shield need to be shifted
				cfg->setup.weapons[j] += (MPWEAPON_SHIELD - MPWEAPON_PP9I);
			}
			if (cfg->setup.weapons[j] >= 0x19) {
				// weapons after the combat knife also need to be shifted up in JPN
				cfg->setup.weapons[j]++;
			}
#else
			// in other versions we only care about the shield and above
			if (cfg->setup.weapons[j] >= 0x25) {
				cfg->setup.weapons[j] += (MPWEAPON_SHIELD - MPWEAPON_PP9I);
			}
#endif
		}
	}

	return NULL;
}

u8 *preprocessTexturesList(u8* data, u32 size, u32* outSize)
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

	return NULL;
}
