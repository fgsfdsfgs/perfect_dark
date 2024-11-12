#ifndef _IN_GAME_TITLE_H
#define _IN_GAME_TITLE_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

#define TITLE_ALLOCSIZE 1024 * 378

extern u32 var800624d4;
extern u32 var800624d8;
extern u32 var800624dc;

void titleInit(void);

void titleReset(void);

Gfx *titleClear(Gfx *gdl);

char *mpPlayerGetWeaponOfChoiceName(u32 playernum, u32 slot);
void titleSetLight(Lights1 *light, u8 r, u8 g, u8 b, f32 luminosity, struct coord *dir);
void titleInitLegal(void);
void titleTickLegal(void);
void titleInitCheckControllers(void);
void titleExitCheckControllers(void);
void titleTickCheckControllers(void);
Gfx *titleRenderCheckControllers(Gfx *gdl);
Gfx *titleRenderLegal(Gfx *gdl);
void titleInitPdLogo(void);
void titleExitPdLogo(void);
void titleTickPdLogo(void);
Gfx *titleRenderPdLogoModel(Gfx *gdl, struct model *model, s32 arg2, f32 arg3, s32 arg4, f32 arg5, Mtxf *arg6, Vtx *vertices, Col *colours);
void titleSkipToPdTitle(void);
Gfx *titleRenderPdLogo(Gfx *gdl);
void titleInitRarePresents(void);
void titleExitRarePresents(void);
void titleTickRarePresents(void);
Gfx *titleRenderTypewriterText(Gfx *gdl, s32 *x, s32 *y, u16 textnum, s32 timer, s32 *colourcomponent);
Gfx *titleRenderRarePresents(Gfx *gdl);
void titleInitNintendoLogo(void);
void titleExitNintendoLogo(void);
void titleTickNintendoLogo(void);
Gfx *titleRenderNintendoLogo(Gfx *gdl);
void titleInitRareLogo(void);
void titleExitRareLogo(void);
void titleTickRareLogo(void);
f32 func0f019d0c(f32 arg0);
Gfx *titleRenderRareLogo(Gfx *gdl);
void titleInitSkip(void);
void titleInitNoController(void);
void titleTickNoController(void);
Gfx *titleRenderNoController(Gfx *gdl);
void titleSetNextMode(s32 mode);
void titleTick(void);
bool titleIsChangingMode(void);
bool titleIsKeepingMode(void);
void titleExit(void);
void titleInitFromAiCmd(u32 arg0);
bool func0f01ad5c(void);
void func0f01adb8(void);
void titleTickOld(void);
Gfx *titleRender(Gfx *gdl);
void titleExitLegal(void);
s32 getNumPlayers(void);
void setNumPlayers(s32 numplayers);
void titleExitNoController(void);

#endif
