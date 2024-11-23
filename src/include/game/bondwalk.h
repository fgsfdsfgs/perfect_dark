#ifndef IN_GAME_BONDWALK_H
#define IN_GAME_BONDWALK_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void bwalkInit(void);
void bwalkSetSwayTarget(s32 value);
void bwalkSetSwayTargetf(f32 value);
void bwalkAdjustCrouchPos(s32 value);
void bwalk0f0c3b38(struct coord *param_1, struct defaultobj *obj);
s32 bwalkTryMoveUpwards(f32 amount);
bool bwalkCanMoveUpwards(f32 amount);
bool bwalkCalculateNewPosition(struct coord *vel, f32 rotateamount, bool apply, f32 extrawidth, s32 arg4);
bool bwalkCalculateNewPositionWithPush(struct coord *delta, f32 rotateamount, bool apply, f32 extrawidth, s32 types);
s32 bwalk0f0c4764(struct coord *delta, struct coord *arg1, struct coord *arg2, s32 types);
s32 bwalk0f0c47d0(struct coord *a, struct coord *b, struct coord *c, struct coord *d, struct coord *e, s32 types);
s32 bwalk0f0c494c(struct coord *a, struct coord *b, struct coord *c, s32 types);
s32 bwalk0f0c4a5c(struct coord *a, struct coord *b, struct coord *c, s32 types);
void bwalk0f0c4d98(void);
void bwalkUpdateSpeedSideways(f32 targetspeed, f32 accelspeed, s32 mult);
void bwalkUpdateSpeedForwards(f32 targetspeed, f32 accelspeed);
void bwalkUpdateVertical(void);
void bwalkApplyCrouchSpeed(void);
bool bwalkCanUncrouch(void);
void bwalkUpdateCrouchOffsetReal(void);
void bwalkUpdateCrouchOffset(void);
void bwalkUpdateTheta(void);
void bwalk0f0c63bc(struct coord *arg0, u32 arg1, s32 types);
void bwalkUpdatePrevPos(void);
void bwalkHandleActivate(void);
void bwalkApplyMoveData(struct movedata *data);
void bwalkUpdateSpeedTheta(void);
void bwalk0f0c69b8(void);
void bwalkTick(void);

#endif
