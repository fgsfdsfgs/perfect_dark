#ifndef IN_GAME_UTILS_H
#define IN_GAME_UTILS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void func0f176d70(s32 arg0);
u32 align4(u32 arg0);
u32 align16(u32 arg0);
uintptr_t align32(uintptr_t arg0);
void utilsInit(void);
void func0f1770ac(struct coord *a, struct coord *b, struct coord *out);
bool func0f177164(struct coord *arg0, struct coord *arg1, u32 line, char *file);
bool func0f1773c8(struct coord *a, struct coord *b);
f32 coordsGetDistance(struct coord *a, struct coord *b);
f32 func0f1776cc(struct coord *a, struct coord *b, struct coord *c);
bool func0f17776c(struct coord *a, struct coord *b, f32 mult, struct coord *out);
void utilsReset(void);
s32 func0f177a54(u8 *arg0, s32 arg1, u8 *arg2, s32 arg3);
s32 func0f177bb4(u8 *arg0, s32 *arg1, s32 *arg2);
s32 func0f177c8c(u8 *arg0, s32 *arg1, s32 *roomnum);

#endif
