#include <ultra64.h>
#include "constants.h"
#include "game/playerreset.h"
#include "bss.h"
#include "data.h"
#include "types.h"

#if VERSION < VERSION_NTSC_1_0
u32 var8009e6b0[4];
#endif

struct g_vars g_Vars;

void varsInit(void)
{
	g_Vars.diffframe60f = 1;
	g_Vars.lvframe60 = 0;
	g_Vars.lvframenum = 0;
	g_Vars.lvmpbotlevel = 0;
	g_Vars.lockscreen = 0;
	g_Vars.diffframe60 = 1;
	g_Vars.prevframestart240 = -1;
	g_Vars.thisframestart240 = 0;
	g_Vars.diffframe240 = 4;
	g_Vars.diffframe60freal = 1;
	g_Vars.prevframestartt = 0;
	g_Vars.thisframestartt = 0;
	g_Vars.diffframet = 0;
	g_Vars.lostframetime60t = 0;
	g_Vars.lostframetime240t = 0;
	g_Vars.lvupdate240rem = 2;
#ifdef PLATFORM_N64
	g_Vars.mininc60 = 1;
#else
	g_Vars.mininc60 = g_TickRateDiv;
#endif
	g_Vars.roomportalrecursionlimit = 254;
	g_Vars.diffframe240f = 4;
	g_Vars.diffframe240freal = 4;
	g_Vars.bondplayernum = 0;
	g_Vars.coopplayernum = -1;
	g_Vars.antiplayernum = -1;
	g_Vars.bond = NULL;
	clearCoopPlayers();
	clearAntiPlayers();
	g_Vars.coopradaron = true;
	g_Vars.coopfriendlyfire = true;
	g_Vars.antiradaron = true;
	g_Vars.pendingantiplayernum = 1;
	g_Vars.modifiedfiles = 0;
	g_Vars.numaibuddies = 0;
	g_Vars.numhumanbuddies = 0;
	g_Vars.bondvisible = true;
	g_Vars.bondcollisions = true;
	g_Vars.tickmode = TICKMODE_GE_FADEIN;
	g_Vars.roomcount = 350;
	g_Vars.mplayerisrunning = 0;
	g_Vars.normmplayerisrunning = 0;
	g_Vars.perfectbuddynum = 0;
	g_Vars.enableslopes = false;
	g_Vars.restartlevel = 0;
	g_Vars.useperimshoot = false;
	g_Vars.mpsetupmenu = 0;
	g_Vars.waitingtojoin[0] = false;
	g_Vars.waitingtojoin[1] = false;
	g_Vars.waitingtojoin[2] = false;
	g_Vars.waitingtojoin[3] = false;
	g_Vars.unk000498 = 0;
	g_Vars.usingadvsetup = false;
	g_Vars.unk0004a0 = 1;
	g_Vars.mpquickteamnumsims = 1;
	g_Vars.mpsimdifficulty = BOTDIFF_NORMAL;
	g_Vars.mpplayerteams[0] = 0;
	g_Vars.mpplayerteams[1] = 1;
	g_Vars.mpplayerteams[2] = 2;
	g_Vars.mpplayerteams[3] = 3;
	g_Vars.mpquickteam = MPQUICKTEAM_NONE;
	g_Vars.dontplaynrg = false;
	g_Vars.in_cutscene = false;
	g_Vars.autocutnum = -1;
	g_Vars.autocutplaying = false;
	g_Vars.autocutgroupcur = -1;
	g_Vars.autocutgroupleft = 0;
	g_Vars.autocutgroupskip = false;
	g_Vars.fourmeg2player = false;
	g_Vars.cutsceneskip60ths = 0;
}
