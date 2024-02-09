#ifndef _IN_GAME_PLAYERMGR_H
#define _IN_GAME_PLAYERMGR_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void playermgrInit(void);
void playermgrReset(void);
void playermgrAllocatePlayers(s32 count);
void playermgrAllocatePlayer(s32 index);
void playermgrCalculateAiBuddyNums(void);
void setCurrentPlayerNum(s32 playernum);
void setCurrentCoopNum(s32 playernum);
void setCurrentAntiNum(s32 playernum);
s32 playermgrGetPlayerNumByProp(struct prop *prop);
void playermgrSetViewSize(s32 viewx, s32 viewy);
void playermgrSetViewPosition(s32 viewleft, s32 viewtop);
void playermgrSetFovY(f32 fovy);
void playermgrSetAspectRatio(f32 aspect);
s32 playermgrGetModelOfWeapon(s32 weapon);
void playermgrDeleteWeapon(s32 hand);
void playermgrCreateWeapon(s32 hand);
void playermgrShuffle(void);
s32 playermgrGetOrderOfPlayer(s32 playernum);
s32 playermgrGetPlayerAtOrder(s32 ordernum);

#endif
