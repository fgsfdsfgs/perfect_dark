#ifndef IN_GAME_PLAYERRESET_H
#define IN_GAME_PLAYERRESET_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void modelmgrReset(void);
void modelmgrSetLvResetting(bool value);
void modelmgrAllocateSlots(s32 numobjs, s32 numchrs);
bool modelmgrLoadProjectileModeldefs(s32 weaponnum);
void playerInitEyespy(void);
void playerReset(void);

void clearAntiPlayers(void);
void clearCoopPlayers(void);
#endif
