#ifndef _IN_GAME_MPLAYER_SCENARIOS_H
#define _IN_GAME_MPLAYER_SCENARIOS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_MpScenarioMenuDialog;
extern struct menudialogdef g_MpQuickTeamScenarioMenuDialog;

struct mpscenariooverview {
	char *name;
	char *shortname;
	u8 requirefeature;
	u8 teamonly;
};

extern struct mpscenariooverview g_MpScenarioOverviews[6];

MenuItemHandlerResult menuhandlerMpOpenOptions(s32 operation, struct menuitem *item, union handlerdata *data);
void scenarioReadSave(struct savebuffer *buffer, u8 version);
void scenarioWriteSave(struct savebuffer *buffer);
void scenarioInit(void);
s32 scenarioNumProps(void);
void scenarioInitProps(void);
void scenarioTick(void);
void scenarioTickChr(struct chrdata *chr);
Gfx *scenarioRadarExtra(Gfx *gdl);
bool scenarioRadarChr(Gfx **gdl, struct prop *prop);
f32 scenarioChooseSpawnLocation(f32 chrradius, struct coord *pos, RoomNum *rooms, struct prop *prop);
s32 scenarioGetMaxTeams(void);
void scenarioHighlightRoom(RoomNum room, s32 *arg1, s32 *arg2, s32 *arg3);

#endif
