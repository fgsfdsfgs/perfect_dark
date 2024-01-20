#include <ultra64.h>
#include "constants.h"
#include "game/inv.h"
#include "game/bondgun.h"
#include "game/stagetable.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "game/player.h"
#endif

extern u32 g_BgunGunMemBaseSize4Mb2P;

void bgunReset(void)
{
	s32 i;

	struct hand hand = {
		0,
		0,
		0,
		0,
		0,
		0,
		1, // gunon
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1, // unk06ac
		0,
		0,
		0,
		0,
		1, // unk06c0
		0,
		0,
		0,
		0,
		1, // unk06d4
		0,
		0,
		0,
		0,
		1, // unk06e8
		0,
		0,
		0,
		0,
		0,
		0,
		-1, // unk0704
		0,
		1, // unk070c
		0,
		0,
		0,
		0,
		0,
		0,
		(PAL ? -16.750415802002f : -19.999996185303f), // unk0728
		0,
		(PAL ? 16.750415802002f : 19.999996185303f),  // unk0730
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		-1, // unk0770
		0,
		0,
		-1, // unk077c
		0,
		0,
		-1, // unk0788
		0,
		0,
		-1, // unk0794
		0,
		1, // unk079c
		0,
		0,
		1, // unk07a8
		0,
		0,
		1, // unk07b4
		0,
		0,
		1, // unk07c0
		0,
		0,
		0,
		1, // unk07d0
		1, // unk07d4
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		{0, 0, 1000}, // unk07f8
		0,
		0,
		0,
		0,
		{-1}, // beam
	};

	if (IS4MB() && PLAYERCOUNT() == 2) {
		i = ALIGN16(g_BgunGunMemBaseSize4Mb2P);
	} else {
		i = ALIGN16(bgunCalculateGunMemCapacity());
	}

	g_Vars.currentplayer->gunctrl.gunmem = mempAlloc(i, MEMPOOL_STAGE);
	g_Vars.currentplayer->gunctrl.invmem = mempAlloc(i, MEMPOOL_STAGE);
	g_Vars.currentplayer->gunctrl.handfilenum = 0;
	g_Vars.currentplayer->gunctrl.handmemloadptr = 0;
	g_Vars.currentplayer->gunctrl.handmemloadremaining = 0;
	g_Vars.currentplayer->gunctrl.masterloadstate = 0;
	g_Vars.currentplayer->gunctrl.gunloadstate = 0;
	g_Vars.currentplayer->gunctrl.gunmemtype = 0;
	g_Vars.currentplayer->gunctrl.gunmemnew = -1;
	g_Vars.currentplayer->gunctrl.gunmemowner = GUNMEMOWNER_CHRBODY;
	g_Vars.currentplayer->gunctrl.gunlocktimer = 0;

	g_Vars.currentplayer->gunctrl.loadall = true;
	g_Vars.currentplayer->gunctrl.dualwielding = false;
	g_Vars.currentplayer->gunctrl.throwing = false;

	g_Vars.currentplayer->gunctrl.switchtoweaponnum = -1;
	g_Vars.currentplayer->gunctrl.fnfader = 0;

	g_Vars.currentplayer->gunctrl.invertgunfunc = false;

	g_Vars.currentplayer->hands[0] = hand;
	g_Vars.currentplayer->hands[1] = hand;

	g_Vars.currentplayer->hands[0].unk0d0f_02 = false;
	g_Vars.currentplayer->hands[1].unk0d0f_02 = false;

	g_Vars.currentplayer->hands[1].audiohandle = NULL;
	g_Vars.currentplayer->hands[0].audiohandle = NULL;

	g_Vars.currentplayer->gunctrl.curgunstr = 0;

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->hands[1].gunroundsspent); i++) {
		g_Vars.currentplayer->hands[1].gunroundsspent[i] = 0;
		g_Vars.currentplayer->hands[0].gunroundsspent[i] = 0;
	}

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->ammoheldarr); i++) {
		g_Vars.currentplayer->ammoheldarr[i] = 0;
	}

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayerstats->shotcount); i++) {
		g_Vars.currentplayerstats->shotcount[i] = 0;
	}

	g_Vars.currentplayerstats->killcount = 0;
	g_Vars.currentplayerstats->ggkillcount = 0;
	g_Vars.currentplayer->deathcount = 0;
	g_Vars.currentplayer->gunposamplitude = 1;
	g_Vars.currentplayer->gunxamplitude = 1;
	g_Vars.currentplayer->doautoselect = false;
	g_Vars.currentplayer->playertriggeron = false;
	g_Vars.currentplayer->playertriggerprev = false;
	g_Vars.currentplayer->playertrigtime240 = 0;
	g_Vars.currentplayer->curguntofire = 0;

	g_Vars.currentplayer->gunshadecol[0] = 0xff;
	g_Vars.currentplayer->gunshadecol[1] = 0xff;
	g_Vars.currentplayer->gunshadecol[2] = 0xff;
	g_Vars.currentplayer->gunshadecol[3] = 0;
	g_Vars.currentplayer->resetshadecol = 1;
	g_Vars.currentplayer->aimtype = 0;
	g_Vars.currentplayer->crosspos[0] = 0;
	g_Vars.currentplayer->crosspos[1] = 0;
	g_Vars.currentplayer->crosspossum[0] = 0;
	g_Vars.currentplayer->crosspossum[1] = 0;
	g_Vars.currentplayer->guncrossdamp = 0.9f;
	g_Vars.currentplayer->crosspos2[0] = 0;
	g_Vars.currentplayer->crosspos2[1] = 0;
	g_Vars.currentplayer->crosssum2[0] = 0;
	g_Vars.currentplayer->crosssum2[1] = 0;
	g_Vars.currentplayer->gunaimdamp = 0.9f;
	g_Vars.currentplayer->aimangle.x = 0;
	g_Vars.currentplayer->aimangle.y = -M_PI;
	g_Vars.currentplayer->aimangle.z = 0;
	g_Vars.currentplayer->copiedgoldeneye = false;
	g_Vars.currentplayer->magnetattracttime = -1;
	g_Vars.currentplayer->gunsync = 0;
	g_Vars.currentplayer->syncchange = 0;
	g_Vars.currentplayer->synccount = 0;
	g_Vars.currentplayer->syncoffset = 0;
	g_Vars.currentplayer->cyclesum = 0;
	g_Vars.currentplayer->gunampsum = 0;

	bgunCalculateBlend(HAND_RIGHT);
	bgunCalculateBlend(HAND_RIGHT);
	bgunCalculateBlend(HAND_RIGHT);
	bgunCalculateBlend(HAND_LEFT);
	bgunCalculateBlend(HAND_LEFT);
	bgunCalculateBlend(HAND_LEFT);

	g_Vars.currentplayer->gunammooff = 0;
	g_Vars.currentplayer->gunsightoff = GUNSIGHTREASON_NOTAIMING;
	g_Vars.currentplayer->gunzoomfovs[0] = ADJUST_ZOOM_FOV(15);
	g_Vars.currentplayer->gunzoomfovs[1] = ADJUST_ZOOM_FOV(60);
	g_Vars.currentplayer->gunzoomfovs[2] = ADJUST_ZOOM_FOV(30);

	if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_AIRBASE) {
		g_Weapons[WEAPON_EYESPY]->name = L_GUN_061; // "DrugSpy"
		g_Weapons[WEAPON_EYESPY]->shortname = L_GUN_061; // "DrugSpy"
		g_Weapons[WEAPON_EYESPY]->flags &= ~(WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN);
	} else if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_CHICAGO
			|| (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR)) {
		g_Weapons[WEAPON_EYESPY]->name = L_GUN_062; // "BombSpy"
		g_Weapons[WEAPON_EYESPY]->shortname = L_GUN_062; // "BombSpy"
		g_Weapons[WEAPON_EYESPY]->flags &= ~(WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN);
	} else {
		// This is setting the "an" determiner for the camspy which seems
		// unusual at first. The theory is that during development it was called
		// the eyespy, then the pickup message was changed to "your camspy"
		// which made the determiner unused, hence no need to update it.
		g_Weapons[WEAPON_EYESPY]->name = L_GUN_060; // "CamSpy"
		g_Weapons[WEAPON_EYESPY]->shortname = L_GUN_060; // "CamSpy"
		g_Weapons[WEAPON_EYESPY]->flags |= (WEAPONFLAG_DETERMINER_S_AN | WEAPONFLAG_DETERMINER_F_AN);
	}

	bgunInitHandAnims();
}
