#ifndef _IN_GAME_MPLAYER_INGAME_H
#define _IN_GAME_MPLAYER_INGAME_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

MenuItemHandlerResult mpStatsForPlayerDropdownHandler(s32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextInGameLimit(struct menuitem *item);
char *menutextPauseOrUnpause(s32 arg0);
char *menutextMatchTime(s32 arg0);
char *mpMenuTextWeaponDescription(struct menuitem *item);
char *mpMenuTitleStatsFor(struct menudialogdef *dialogdef);
char *mpMenuTextWeaponOfChoiceName(struct menuitem *item);
char *mpMenuTextAward1(struct menuitem *item);
char *mpMenuTextAward2(struct menuitem *item);
char *mpMenuTextPlacementWithSuffix(struct menuitem *item);
MenuItemHandlerResult mpPlacementMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult mpAwardsMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMpPauseSeparator(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult mpPlayerTitleMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextPlayerTitle(s32 arg0);
MenuItemHandlerResult mpConfirmPlayerNameHandler(s32 operation, struct menuitem *item, union handlerdata *data);
void mpPushPauseDialog(void);
void mpPushEndscreenDialog(u32 arg0, u32 playernum);
MenuItemHandlerResult menuhandlerMpEndGame(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler00178018(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMpInGameLimitLabel(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMpPause(s32 operation, struct menuitem *item, union handlerdata *data);

#endif
