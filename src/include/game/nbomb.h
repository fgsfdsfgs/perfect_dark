#ifndef IN_GAME_NBOMB_H
#define IN_GAME_NBOMB_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

Gfx *func0f008558(Gfx *gdl, s32 depth);
void nbombReset(struct nbomb *nbomb);
s32 nbombCalculateAlpha(struct nbomb *nbomb);
Gfx *nbombCreateGdl(void);
Gfx *nbombRender(Gfx *gdl, struct nbomb *nbomb, Gfx *subgdl);
void func0f0099a4(void);
void nbombInflictDamage(struct nbomb *nbomb);
void nbombTick(struct nbomb *nbomb);
void nbombsTick(void);
Gfx *nbombsRender(Gfx *gdl);
void nbombCreateStorm(struct prop *prop, struct prop *ownerprop);
f32 gasGetDoorFrac(s32 tagnum);
Gfx *nbombRenderOverlay(Gfx *gdl);
Gfx *gasRender(Gfx *gdl);

#endif
