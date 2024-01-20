#ifndef IN_GAME_MENU_H
#define IN_GAME_MENU_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern const struct menucolourpalette g_MenuColours[];
extern const struct menucolourpalette g_MenuWave1Colours[];
extern const struct menucolourpalette g_MenuWave2Colours[];

void menuTick(void);

void menuStop(void);

void menuPlaySound(s32 menusound);
bool menuIsSoloMissionOrMp(void);
bool currentPlayerIsMenuOpenInSoloOrMp(void);
bool func0f0f0c68(void);
void menuSetBanner(s32 bannernum, bool allplayers);

#if VERSION >= VERSION_NTSC_1_0
Gfx *menuRenderBanner(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, bool big, s32 msgnum, s32 arg7, s32 arg8);
#else
Gfx *menuRenderBanner(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, bool big, s32 msgnum);
#endif

struct menudfc *func0f0f1338(struct menuitem *item);
void func0f0f139c(struct menuitem *item, f32 arg1);
void func0f0f13ec(struct menuitem *item);
void func0f0f1418(void);
void func0f0f1494(void);
char *menuResolveText(uintptr_t thing, void *dialogoritem);
char *menuResolveParam2Text(struct menuitem *item);
char *menuResolveDialogTitle(struct menudialogdef *dialogdef);
void menuGetItemBlocksRequired(struct menuitem *item, s32 *arg1);
void menuCalculateItemSize(struct menuitem *item, s16 *width, s16 *height, struct menudialog *dialog);
void func0f0f1d6c(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu);
void dialog0f0f1ef4(struct menudialog *dialog);
void dialogCalculateContentSize(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu);
s32 dialogFindItem(struct menudialog *dialog, struct menuitem *item, s32 *rowindex, s32 *colindex);
bool menuIsScrollableUnscrollable(struct menuitem *item);
bool menuIsItemDisabled(struct menuitem *item, struct menudialog *dialog);
bool menuIsItemFocusable(struct menuitem *item, struct menudialog *dialog, s32 arg2);
struct menuitem *dialogFindItemAtColY(s32 targety, s32 colindex, struct menudialogdef *dialogdef, s32 *rowindexptr, struct menudialog *dialog);
struct menuitem *dialogFindFirstItem(struct menudialog *dialog);
struct menuitem *dialogFindFirstItemRight(struct menudialog *dialog);
void dialogChangeItemFocusVertically(struct menudialog *dialog, s32 updown);
s32 dialogChangeItemFocusHorizontally(struct menudialog *dialog, s32 leftright);
s32 dialogChangeItemFocus(struct menudialog *dialog, s32 leftright, s32 updown);
void menuOpenDialog(struct menudialogdef *dialogdef, struct menudialog *arg1, struct menu *menu);
void menuPushDialog(struct menudialogdef *dialogdef);

#if VERSION >= VERSION_NTSC_1_0
bool func0f0f3220(s32 arg0);
#else
void func0f0f3220(s32 arg0);
#endif

void menuCloseDialog(void);
void menuUpdateCurFrame(void);
void menuPopDialog(void);
void func0f0f3704(struct menudialogdef *dialogdef);
void menuConfigureModel(struct menumodel *menumodel, f32 x, f32 y, f32 z, f32 rotx, f32 roty, f32 rotz, f32 scale, u8 flags);
void menuUnsetModel(struct menumodel *menumodel);
Gfx *menuRenderModel(Gfx *gdl, struct menumodel *menumodel, s32 modeltype);
void menuGetTeamTitlebarColours(u32 *top, u32 *middle, u32 *bottom);
Gfx *menuApplyScissor(Gfx *gdl);
Gfx *dialogRender(Gfx *gdl, struct menudialog *dialog, struct menu *menu, bool lightweight);
void menuGetContPads(s8 *contpadnum1, s8 *contpadnum2);
void func0f0f7594(s32 arg0, s32 *vdir, s32 *hdir);

#if VERSION >= VERSION_JPN_FINAL
void menuFindAvailableSize(s32 *xmin, s32 *ymin, s32 *xmax, s32 *ymax, struct menudialog *dialog);
#else
void menuFindAvailableSize(s32 *xmin, s32 *ymin, s32 *xmax, s32 *ymax);
#endif

void dialogCalculatePosition(struct menudialog *dialog);
void menuClose(void);
void func0f0f8120(void);
void func0f0f820c(struct menudialogdef *dialogdef, s32 root);
void menuSetBackground(s32 bg);
void func0f0f8300(void);
void menuPushRootDialog(struct menudialogdef *dialogdef, s32 arg1);
void func0f0f85e0(struct menudialogdef *dialogdef, s32 root);
Gfx *menuRenderDialog(Gfx *gdl, struct menudialog *dialog, struct menu *menu, bool lightweight);
Gfx *menuRenderDialogs(Gfx *gdl);
void menuResetModel(struct menumodel *menumodel, u32 allocationlen, bool allocate);
void menuReset(void);
void menuSwipe(s32 direction);
void dialogTick(struct menudialog *dialog, struct menuinputs *inputs, u32 tickflags);
void dialogInitItems(struct menudialog *dialog);
void handleMenuClose(void);
void menuProcessInput(void);
Gfx *menuRenderBackgroundLayer1(Gfx *gdl, u8 bg, f32 frac);
Gfx *menuRenderBackgroundLayer2(Gfx *gdl, u8 bg, f32 frac);
Gfx *menuRender(Gfx *gdl);
u32 menuChooseMusic(void);
u32 menuGetRoot(void);
void menuPushPakDialogForPlayer(struct menudialogdef *dialogdef, s32 playernum, s32 arg2);
char *menuTextSaveDeviceName(struct menuitem *item);
s32 menuPakNumToPlayerNum(s32 paknum);
bool menuIsReadyForPakError(s32 device, s32 arg1);
void menuPushPakErrorDialog(s32 arg0, s32 arg1);
void func0f0fd494(struct coord *pos);
void func0f0fd548(s32 arg0);
struct menudialog *menuIsDialogOpen(struct menudialogdef *dialogdef);
struct chrdata *currentPlayerGetCommandingAibot(void);

MenuItemHandlerResult menuhandler000fcc34(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult amPickTargetMenuList(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerRepairPak(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerRetrySavePak(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerWarnRepairPak(s32 operation, struct menuitem *item, union handlerdata *data);

extern const char var7f1b2658[];
extern const char var7f1b265c[];
extern const char var7f1b2660[];
extern const char var7f1b2664[];

#endif
