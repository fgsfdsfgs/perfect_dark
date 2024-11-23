#ifndef _IN_GAME_SIGHT_H
#define _IN_GAME_SIGHT_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

bool sightIsPropFriendly(struct prop *prop);
bool sightCanTargetProp(struct prop *prop, s32 max);
bool sightIsReactiveToProp(struct prop *prop);
s32 sightFindFreeTargetIndex(s32 max);
void func0f0d7364(void);
void sightTick(bool sighton);
s32 sightCalculateBoxBound(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
Gfx *sightDrawTargetBox(Gfx *gdl, struct trackedprop *trackedprop, s32 textid, s32 time);
Gfx *sightDrawAimer(Gfx *gdl, s32 x, s32 y, s32 radius, s32 cornergap, u32 colour);
Gfx *sightDrawDelayedAimer(Gfx *gdl, s32 x, s32 y, s32 radius, s32 cornergap, u32 colour);
Gfx *sightDrawDefault(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawClassic(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawType2(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawSkedarTriangle(Gfx *gdl, s32 x, s32 y, s32 dir, u32 colour);
Gfx *sightDrawSkedar(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawZoom(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawMaian(Gfx *gdl, bool sighton, f32 crossx, f32 crossy);
Gfx *sightDrawTarget(Gfx *gdl, f32 crossx, f32 crossy);
bool sightHasTargetWhileAiming(s32 sight);
Gfx *sightDraw(Gfx *gdl, bool sighton, s32 sight);

#endif
