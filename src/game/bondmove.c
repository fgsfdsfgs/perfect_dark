#include <ultra64.h>
#include "constants.h"
#include "game/activemenu.h"
#include "game/bondbike.h"
#include "game/bondgrab.h"
#include "game/bondmove.h"
#include "game/bondwalk.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/footstep.h"
#include "game/game_006900.h"
#include "game/chr.h"
#include "game/prop.h"
#include "game/atan2f.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/bondcutscene.h"
#include "game/bondhead.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/lv.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/lib_17ce0.h"
#include "lib/vi.h"
#include "lib/collision.h"
#include "lib/joy.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "input.h"
#include "video.h"
#endif

void bmoveSetControlDef(u32 controldef)
{
	g_Vars.currentplayer->controldef = controldef;
}

void bmoveSetAutoMoveCentreEnabled(bool enabled)
{
	g_Vars.currentplayer->automovecentreenabled = enabled;
}

bool bmoveIsAutoMoveCentreEnabled(void)
{
	return g_Vars.currentplayer->automovecentreenabled;
}

void bmoveSetAutoAimY(bool enabled)
{
	g_Vars.currentplayer->autoyaimenabled = enabled;
}

bool bmoveIsAutoAimYEnabled(void)
{
	if (!g_Vars.normmplayerisrunning) {
		return g_Vars.currentplayer->autoyaimenabled;
	}

	if (g_MpSetup.options & MPOPTION_NOAUTOAIM) {
		return false;
	}

	return optionsGetAutoAim(g_Vars.currentplayerstats->mpindex);
}

bool bmoveIsAutoAimYEnabledForCurrentWeapon(void)
{
	struct weaponfunc *func = currentPlayerGetWeaponFunction(0);

	if (func) {
		if (func->flags & FUNCFLAG_NOAUTOAIM) {
			return false;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			return true;
		}
	}

	return bmoveIsAutoAimYEnabled();
}

bool bmoveIsInSightAimMode(void)
{
	return g_Vars.currentplayer->insightaimmode;
}

void bmoveUpdateAutoAimYProp(struct prop *prop, f32 autoaimy)
{
	if (g_Vars.currentplayer->autoyaimtime60 >= 0) {
		g_Vars.currentplayer->autoyaimtime60 -= g_Vars.lvupdate60;
	}

	if (prop != g_Vars.currentplayer->autoyaimprop) {
		if (g_Vars.currentplayer->autoyaimtime60 < 0) {
			g_Vars.currentplayer->autoyaimtime60 = TICKS(30);
			g_Vars.currentplayer->autoyaimprop = prop;
		} else {
			return;
		}
	}

	g_Vars.currentplayer->autoaimy = autoaimy;
}

void bmoveSetAutoAimX(bool enabled)
{
	g_Vars.currentplayer->autoxaimenabled = enabled;
}

bool bmoveIsAutoAimXEnabled(void)
{
	if (!g_Vars.normmplayerisrunning) {
		return g_Vars.currentplayer->autoxaimenabled;
	}

	if (g_MpSetup.options & MPOPTION_NOAUTOAIM) {
		return false;
	}

	return optionsGetAutoAim(g_Vars.currentplayerstats->mpindex);
}

bool bmoveIsAutoAimXEnabledForCurrentWeapon(void)
{
	struct weaponfunc *func = currentPlayerGetWeaponFunction(0);

	if (func) {
		if (func->flags & FUNCFLAG_NOAUTOAIM) {
			return false;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			return true;
		}
	}

	return bmoveIsAutoAimXEnabled();
}

void bmoveUpdateAutoAimXProp(struct prop *prop, f32 autoaimx)
{
	if (g_Vars.currentplayer->autoxaimtime60 >= 0) {
		g_Vars.currentplayer->autoxaimtime60 -= g_Vars.lvupdate60;
	}

	if (prop != g_Vars.currentplayer->autoxaimprop) {
		if (g_Vars.currentplayer->autoxaimtime60 < 0) {
			g_Vars.currentplayer->autoxaimtime60 = TICKS(30);
			g_Vars.currentplayer->autoxaimprop = prop;
		} else {
			return;
		}
	}

	g_Vars.currentplayer->autoaimx = autoaimx;
}

struct prop *bmoveGetHoverbike(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		return g_Vars.currentplayer->hoverbike;
	}

	return NULL;
}

struct prop *bmoveGetGrabbedProp(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		return g_Vars.currentplayer->grabbedprop;
	}

	return NULL;
}

void bmoveGrabProp(struct prop *prop)
{
	struct defaultobj *obj = prop->obj;

	if ((obj->hidden & OBJHFLAG_MOUNTED) == 0 && (obj->hidden & OBJHFLAG_GRABBED) == 0) {
		g_Vars.currentplayer->grabbedprop = prop;
		bgrabInit();
	}
}

void bmoveSetMode(u32 movemode)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabExit();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeExit();
	}

	if (movemode == MOVEMODE_BIKE) {
		bbikeInit();
	} else if (movemode == MOVEMODE_GRAB) {
		bgrabInit();
	} else if (movemode == MOVEMODE_CUTSCENE) {
		bcutsceneInit();
	} else if (movemode == MOVEMODE_WALK) {
		bwalkInit();
	}
}

void bmoveSetModeForAllPlayers(u32 movemode)
{
	u32 prevplayernum = g_Vars.currentplayernum;
	s32 i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);
		bmoveSetMode(movemode);
	}

	setCurrentPlayerNum(prevplayernum);
}

void bmoveHandleActivate(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeHandleActivate();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabHandleActivate();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkHandleActivate();
	}
}

void bmoveApplyMoveData(struct movedata *data)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeApplyMoveData(data);
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabApplyMoveData(data);
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkApplyMoveData(data);
	}
}

void bmoveUpdateSpeedTheta(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		// empty
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabUpdateSpeedTheta();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkUpdateSpeedTheta();
	}
}

f32 bmoveGetSpeedVertaLimit(f32 value)
{
	if (value > 0) {
		return (viGetFovY() * value * -0.7f) / 60.0f;
	}

	if (value < 0) {
		return (viGetFovY() * -value * 0.7f) / 60.0f;
	}

	return 0;
}

void bmoveUpdateSpeedVerta(f32 value)
{
	f32 mult = viGetFovY() / 60.0f;
	f32 limit = bmoveGetSpeedVertaLimit(value);

	if (value > 0) {
		if (g_Vars.currentplayer->speedverta > 0) {
			g_Vars.currentplayer->speedverta -= 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedverta -= 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedverta < limit) {
			g_Vars.currentplayer->speedverta = limit;
		}
	} else if (value < 0) {
		if (g_Vars.currentplayer->speedverta < 0) {
			g_Vars.currentplayer->speedverta += 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedverta += 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedverta > limit) {
			g_Vars.currentplayer->speedverta = limit;
		}
	} else {
		if (g_Vars.currentplayer->speedverta > limit) {
			g_Vars.currentplayer->speedverta -= 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedverta < limit) {
				g_Vars.currentplayer->speedverta = limit;
			}
		} else {
			g_Vars.currentplayer->speedverta += 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedverta > limit) {
				g_Vars.currentplayer->speedverta = limit;
			}
		}
	}
}

f32 bmoveGetSpeedThetaControlLimit(f32 value)
{
	if (value > 0) {
		return (viGetFovY() * value * -0.7f) / 60.0f;
	}

	if (value < 0) {
		return (viGetFovY() * -value * 0.7f) / 60.0f;
	}

	return 0;
}

void bmoveUpdateSpeedThetaControl(f32 value)
{
	f32 mult = viGetFovY() / 60.0f;
	f32 limit = bmoveGetSpeedThetaControlLimit(value);

	if (value > 0) {
		if (g_Vars.currentplayer->speedthetacontrol > 0) {
			g_Vars.currentplayer->speedthetacontrol -= 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedthetacontrol -= 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedthetacontrol < limit) {
			g_Vars.currentplayer->speedthetacontrol = limit;
		}
	} else if (value < 0) {
		if (g_Vars.currentplayer->speedthetacontrol < 0.0f) {
			g_Vars.currentplayer->speedthetacontrol += 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedthetacontrol += 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedthetacontrol > limit) {
			g_Vars.currentplayer->speedthetacontrol = limit;
		}
	} else {
		if (g_Vars.currentplayer->speedthetacontrol > limit) {
			g_Vars.currentplayer->speedthetacontrol -= 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedthetacontrol < limit) {
				g_Vars.currentplayer->speedthetacontrol = limit;
			}
		} else {
			g_Vars.currentplayer->speedthetacontrol += 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedthetacontrol > limit) {
				g_Vars.currentplayer->speedthetacontrol = limit;
			}
		}
	}
}

/**
 * Calculate the lookahead angle.
 *
 * The return value is the intended vertical angle to look at.
 * 90 = straight up
 * 0 = horizontal
 * -90 = straight down
 */
f32 bmoveCalculateLookahead(void)
{
	f32 result = -4.0f;
	f32 sp160 = 400.0f;
	f32 ground = g_Vars.currentplayer->vv_ground;
	struct coord sp150;
	f32 ymax;
	f32 ymin;
	f32 radius;
	u32 stack2;
	f32 angles[5];
	bool populated[5];
	s32 numpopulated = 0;
	u16 flags = 0;
	u32 stack3;
	struct coord sp100;
	u32 stack;
	struct coord spf0;
	RoomNum spe0[8];
	s32 i;
	f32 angle;
	f32 value;
	u32 stack4;
	u32 stack5;
	u32 stack6;
	struct coord spbc;
	struct coord spb0;
	RoomNum spa0[8];
	RoomNum sp90[8];
	RoomNum sp80[8];
	s32 j;
	f32 sp78;
	s32 indextoremove;
	f32 angletoremove;

	if (g_Vars.currentplayer->inlift) {
		return result;
	}

	playerGetBbox(g_Vars.currentplayer->prop, &radius, &ymax, &ymin);

	sp100.x = g_Vars.currentplayer->bond2.unk00.x;
	sp100.y = g_Vars.currentplayer->bond2.unk00.y;
	sp100.z = g_Vars.currentplayer->bond2.unk00.z;

	spf0.x = g_Vars.currentplayer->prop->pos.x;
	spf0.y = g_Vars.currentplayer->prop->pos.y - 30;
	spf0.z = g_Vars.currentplayer->prop->pos.z;

	portal00018148(&g_Vars.currentplayer->prop->pos, &spf0,
			g_Vars.currentplayer->prop->rooms, spe0, NULL, 0);

	sp150.x = sp100.x * 400 + spf0.x;
	sp150.y = sp100.y * 400 + spf0.y;
	sp150.z = sp100.z * 400 + spf0.z;

	if (cdExamLos08(&spf0, spe0, &sp150,
				CDTYPE_BG | CDTYPE_CLOSEDDOORS,
				GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2 | GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT) == CDRESULT_COLLISION) {
		cdGetPos(&sp150, 455, "bondmove.c");
		flags = cdGetGeoFlags();

		sp160 = sqrtf((sp150.x - spf0.x) * (sp150.x - spf0.x)
				+ (sp150.y - spf0.y) * (sp150.y - spf0.y)
				+ (sp150.z - spf0.z) * (sp150.z - spf0.z));
	}

	if (sp160 > 60.0f || (flags & GEOFLAG_FLOOR1)) {
		for (i = 0; i < ARRAYCOUNT(populated); i++) {
			populated[i] = false;
			value = (i + 1) * sp160 * 0.2f;

			spbc.x = sp100.x * value + spf0.x;
			spbc.y = sp100.y * value + spf0.y;
			spbc.z = sp100.z * value + spf0.z;

			portal00018148(&spf0, &spbc, spe0, spa0, NULL, 0);

			spb0.x = spbc.x;
			spb0.y = spbc.y - 400;
			spb0.z = spbc.z;

			portal00018148(&spbc, &spb0, spa0, sp90, sp80, 7);

			if (
#if VERSION >= VERSION_NTSC_1_0
					cdFindFloorRoomYColourFlagsAtPos(&spbc, sp80, &sp78, NULL, NULL) > 0
#else
					cdFindFloorRoomYColourFlagsAtPos(&spbc, sp80, &sp78, NULL) > 0
#endif
					&& sp78 - ground < 200
					&& sp78 - ground > -200) {
				angle = atan2f(sp78 - g_Vars.currentplayer->vv_ground, value);
				angle = (angle * 360) / M_BADTAU + -4;

				if (angle >= 180) {
					angle -= 360;
				}

				if (angle >= -50 && angle <= 40) {
					populated[i] = true;
					angles[i] = angle;
					numpopulated++;
					ground = sp78;
				}
			}
		}

		for (i = 0; i < numpopulated - 1; i++) {
			indextoremove = -1;

			for (j = 0; j < ARRAYCOUNT(populated); j++) {
				if (populated[j]) {
					if (indextoremove < 0) {
						indextoremove = j;
						angletoremove = angles[j];
					} else if (i & 1) {
						if (angletoremove > angles[j]) {
							indextoremove = j;
							angletoremove = angles[j];
						}
					} else {
						if (angletoremove < angles[j]) {
							indextoremove = j;
							angletoremove = angles[j];
						}
					}
				}
			}

			if (indextoremove >= 0) {
				populated[indextoremove] = false;
			}
		}

		for (i = 0; i < ARRAYCOUNT(populated); i++) {
			if (populated[i]) {
				result = angles[i];
				break;
			}
		}
	}

	if (result > 0.0f) {
		result *= 0.86666667461395f;
	}

	return result;
}

void bmoveResetMoveData(struct movedata *data)
{
	data->canswivelgun = 0;
	data->canmanualaim = 0;
	data->triggeron = false;
	data->btapcount = 0;
	data->canlookahead = false;
	data->unk14 = 0;
	data->cannaturalturn = false;
	data->cannaturalpitch = false;
	data->digitalstepforward = false;
	data->digitalstepback = false;
	data->digitalstepleft = false;
	data->digitalstepright = false;
	data->weaponbackoffset = 0;
	data->weaponforwardoffset = 0;
	data->unk50 = 0;
	data->aiming = false;
	data->zooming = false;
	data->crouchdown = false;
	data->crouchup = false;
	data->rleanleft = false;
	data->rleanright = false;
	data->detonating = false;
	data->canautoaim = false;
	data->farsighttempautoseek = false;
	data->eyesshut = false;
	data->unk30 = 0;
	data->unk34 = 0;
	data->speedvertadown = 0;
	data->speedvertaup = 0;
	data->aimturnleftspeed = 0;
	data->aimturnrightspeed = 0;
	data->zoomoutfovpersec = 0;
	data->zoominfovpersec = 0;
	data->invertpitch = !optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex);
	data->disablelookahead = false;
	data->c1stickxsafe = 0;
	data->c1stickysafe = 0;
	data->c1stickxraw = 0;
	data->c1stickyraw = 0;
	data->analogturn = 0;
	data->analogpitch = 0;
	data->analogstrafe = 0;
	data->analogwalk = 0;
#ifndef PLATFORM_N64
	data->freelookdx = 0.0f;
	data->freelookdy = 0.0f;
#endif
	data->alt1tapcount = 0;
}

/**
 * Called with these arguments:
 * 0, 0, 0, 1 = tickmode 6
 * 0, 0, 0, 1 = eyespy
 * 0, 0, 0, 1 = teleportstate 3
 * 0, 0, 0, 1 = slayerrocket
 * 0, 0, 0, 1 = tickmode normal without control
 * 1, 1, ?, 0 = tickmode normal with control
 * 1, 1, ?, 0 = tickmodes 0 and 5
 * 0, 0, 0, 1 = tickmode mpswirl
 * 0, 0, 0, 1 = tickmode warp
 * 1, 1, 0, 1 = autowalk
 */
void bmoveProcessInput(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2)
{
	struct movedata movedata;
	s32 controlmode;
	s32 weaponnum;
	bool canmanualzoom;
	s32 result;
	u16 c1buttons;
	u16 c1buttonsthisframe;
	u16 c1allowedbuttons;
	u16 c1inhibitedbuttons;
	u32 aimonhist[20];
	u32 aimoffhist[20];
	s32 numsamples;
	f32 tmp;
	f32 fVar25;
	s8 shootpad;
	s8 aimpad;
	u16 aimallowedbuttons;
	u16 shootallowedbuttons;
	s8 c2stickx;
	u16 c2buttons;
	u16 c2buttonsthisframe;
	s32 i;
	s32 tmpc2sticky;
	u16 c2allowedbuttons;
	s32 tmpc2stickx;
	s32 c2sticky;
	u16 shootbuttons;
	u16 aimbuttons;
	u16 invbuttons;
	bool zoomout;
	bool zoomin;
	f32 increment;
	f32 savedverta;
	f32 noiseradius;
	f32 zoomfov;
	f32 eraserfov;
	struct coord spa0;
	f32 crosspos[2];
	f32 lookahead;
	s8 contpad1;
	s8 contpad2;
	s8 c1stickx;
	s8 c1sticky;
	u16 inhibitedbuttons;
	bool offbike;
	bool cancycleweapons;
	u32 stack;
	f32 increment2;
	f32 newverta;

	controlmode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);
	weaponnum = bgunGetWeaponNum(HAND_RIGHT);
	canmanualzoom = weaponHasAimFlag(weaponnum, INVAIMFLAG_MANUALZOOM);
	contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);

	c1stickx = allowc1x ? joyGetStickX(contpad1) : 0;
	c1sticky = allowc1y ? joyGetStickY(contpad1) : 0;

	c1buttons = allowc1buttons ? joyGetButtons(contpad1, 0xffff) : 0;
	c1buttonsthisframe = allowc1buttons ? joyGetButtonsPressedThisFrame(contpad1, 0xffff) : 0;

	c1allowedbuttons = 0xffff;

	if (g_Vars.currentplayer->joybutinhibit & 0xffff) {
		inhibitedbuttons = g_Vars.currentplayer->joybutinhibit & 0xffff;
		c1allowedbuttons = ~inhibitedbuttons;
		inhibitedbuttons = joyGetButtons(contpad1, 0xffff) & inhibitedbuttons;
		c1buttons &= ~inhibitedbuttons;
		c1buttonsthisframe &= ~inhibitedbuttons;
		g_Vars.currentplayer->joybutinhibit = (g_Vars.currentplayer->joybutinhibit & 0xffff0000) | inhibitedbuttons;
	}

	numsamples = joyGetNumSamples();
	bmoveResetMoveData(&movedata);

	if (c1stickx < -5) {
		movedata.c1stickxsafe = c1stickx + 5;
	} else if (c1stickx > 5) {
		movedata.c1stickxsafe = c1stickx - 5;
	} else {
		movedata.c1stickxsafe = 0;
	}

	if (c1sticky < -5) {
		movedata.c1stickysafe = c1sticky + 5;
	} else if (c1sticky > 5) {
		movedata.c1stickysafe = c1sticky - 5;
	} else {
		movedata.c1stickysafe = 0;
	}

	movedata.c1stickxraw = c1stickx;
	movedata.c1stickyraw = c1sticky;

	// These are zeroed further down conditionally on control style
	movedata.analogturn = movedata.c1stickxsafe;
	movedata.analogstrafe = movedata.c1stickxsafe;
	movedata.analogpitch = movedata.c1stickysafe;
	movedata.analogwalk = movedata.c1stickysafe;

#ifndef PLATFORM_N64
	if (g_Vars.currentplayernum == 0 && (allowc1x || allowc1y)) {
		inputMouseGetScaledDelta(&movedata.freelookdx, &movedata.freelookdy);
	}
#endif

	// Pausing
	if (g_Vars.currentplayer->isdead == false) {
		if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED && (c1buttonsthisframe & START_BUTTON)) {
			if (g_Vars.mplayerisrunning == false) {
				if (g_Vars.lvframenum > 15) {
					playerPause(MENUROOT_MAINMENU);
				}
			} else {
				mpPushPauseDialog();
			}
		}
	} else {
		if (g_Vars.mplayerisrunning) {
			if (PLAYERCOUNT() == 1) {
				if (mpIsPaused() && (c1buttonsthisframe & START_BUTTON) && g_MpSetup.paused != MPPAUSEMODE_GAMEOVER) {
					mpSetPaused(MPPAUSEMODE_UNPAUSED);
				}
			} else {
				if (mpIsPaused() && (c1buttonsthisframe & START_BUTTON)) {
					mpPushPauseDialog();
				}
			}
		}
	}

	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		if (g_Vars.currentplayer->isdead == false) {
			if (controlmode == CONTROLMODE_23 || controlmode == CONTROLMODE_24 || controlmode == CONTROLMODE_22 || controlmode == CONTROLMODE_21) {
				// 2.1: ctrl1 stick = walk/turn, z = fire, ctrl2 stick = look/strafe, z = aim
				// 2.2: ctrl1 stick = look,      z = fire, ctrl2 stick = walk/strafe, z = aim
				// 2.3: ctrl1 stick = walk/turn, z = aim,  ctrl2 stick = look/strafe, z = fire
				// 2.4: ctrl1 stick = look,      z = aim,  ctrl2 stick = walk/strafe, z = fire
				contpad2 = (s8) optionsGetContpadNum2(g_Vars.currentplayerstats->mpindex);
				c2stickx = (s8) joyGetStickX(contpad2);
				c2sticky = (joyGetStickY(contpad2) << 24) >> 24;
				c2buttons = joyGetButtons(contpad2, 0xffff);
				c2buttonsthisframe = joyGetButtonsPressedThisFrame(contpad2, 0xffff);

				tmpc2stickx = c2stickx;
				tmpc2sticky = c2sticky;

				c2allowedbuttons = 0xffff;

				if (g_Vars.currentplayer->joybutinhibit << 0 >> 16) {
					inhibitedbuttons = g_Vars.currentplayer->joybutinhibit >> 16;
					c2allowedbuttons = ~inhibitedbuttons;
					inhibitedbuttons = joyGetButtons(contpad2, 0xffff) & inhibitedbuttons;
					c2buttons &= ~inhibitedbuttons;
					c2buttonsthisframe &= ~inhibitedbuttons;
					g_Vars.currentplayer->joybutinhibit = (g_Vars.currentplayer->joybutinhibit & 0xffff) | (inhibitedbuttons << 16);
				}

				if (ignorec2) {
					c2stickx = 0;
					c2buttons = 0;
					tmpc2stickx = 0;
					tmpc2sticky = 0;
					c2buttonsthisframe = 0;
				}

				if (tmpc2stickx < -5) {
					tmpc2stickx += 5;
				} else if (tmpc2stickx > 5) {
					tmpc2stickx -= 5;
				} else {
					tmpc2stickx = 0;
				}

				if (tmpc2sticky < -5) {
					tmpc2sticky += 5;
				} else if (tmpc2sticky > 5) {
					tmpc2sticky -= 5;
				} else {
					tmpc2sticky = 0;
				}

				if (controlmode == CONTROLMODE_21 || controlmode == CONTROLMODE_23) {
					movedata.analogstrafe = tmpc2stickx;
					movedata.analogpitch = tmpc2sticky;
				} else {
					movedata.analogstrafe = tmpc2stickx;
					movedata.analogwalk = tmpc2sticky;
				}

				c2sticky = tmpc2sticky;

				if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
					movedata.digitalstepforward = false;
					movedata.digitalstepback = false;
					movedata.analogstrafe = 0;
					movedata.analogwalk = g_Vars.currentplayer->autocontrol_y;
					movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
					movedata.analogpitch = 0;
#ifndef PLATFORM_N64
					movedata.freelookdx = 0.0f;
					movedata.freelookdy = 0.0f;
#endif
				}

				if (controlmode == CONTROLMODE_21 || controlmode == CONTROLMODE_22) {
					aimpad = contpad2;
					shootpad = contpad1;
					aimallowedbuttons = c2allowedbuttons;
					shootallowedbuttons = c1allowedbuttons;
				} else {
					aimpad = contpad1;
					shootpad = contpad2;
					aimallowedbuttons = c1allowedbuttons;
					shootallowedbuttons = c2allowedbuttons;
				}

				if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
					for (i = 0; i < numsamples; i++) {
						aimonhist[i] = allowc1buttons && joyGetButtonsOnSample(i, aimpad, aimallowedbuttons & Z_TRIG);
						aimoffhist[i] = !aimonhist[i];
					}

					g_Vars.currentplayer->insightaimmode = aimonhist[numsamples - 1];
				}

				if (!lvIsPaused()) {
					// Handle aiming
					if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) != AIMCONTROL_HOLD) {
						for (i = 0; i < numsamples; i++) {
							if (allowc1buttons && joyGetButtonsPressedOnSample(i, aimpad, aimallowedbuttons & Z_TRIG)) {
								g_Vars.currentplayer->insightaimmode = !g_Vars.currentplayer->insightaimmode;
							}

							aimonhist[i] = g_Vars.currentplayer->insightaimmode;
							aimoffhist[i] = !aimonhist[i];
						}
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
						g_Vars.currentplayer->insightaimmode = true;
					}

					movedata.canswivelgun = !g_Vars.currentplayer->insightaimmode;
					movedata.canmanualaim = g_Vars.currentplayer->insightaimmode;
					movedata.canautoaim = !g_Vars.currentplayer->insightaimmode;
					movedata.digitalstepforward = false;
					movedata.digitalstepback = false;
					movedata.digitalstepleft = false;
					movedata.digitalstepright = false;
					movedata.canlookahead = !g_Vars.currentplayer->insightaimmode;
					movedata.unk14 = 1;
					movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;
					movedata.cannaturalpitch = !g_Vars.currentplayer->insightaimmode;

					// Handle turning while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw > 60) {
						movedata.speedvertadown = (movedata.c1stickyraw - 60) / 10.0f;

						if (movedata.speedvertadown > 1) {
							movedata.speedvertadown = 1;
						}
					} else {
						movedata.speedvertadown = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw < -60) {
						movedata.speedvertaup = (-60 - movedata.c1stickyraw) / 10.0f;

						if (movedata.speedvertaup > 1) {
							movedata.speedvertaup = 1;
						}
					} else {
						movedata.speedvertaup = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw < -60) {
						movedata.aimturnleftspeed = (-60 - movedata.c1stickxraw) / 10.0f;

						if (movedata.aimturnleftspeed > 1) {
							movedata.aimturnleftspeed = 1;
						}
					} else {
						movedata.aimturnleftspeed = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw > 60) {
						movedata.aimturnrightspeed = (movedata.c1stickxraw - 60) / 10.0f;

						if (movedata.aimturnrightspeed > 1) {
							movedata.aimturnrightspeed = 1;
						}
					} else {
						movedata.aimturnrightspeed = 0;
					}

					// Handle weapon switching
					if (allowc1buttons) {
						if (g_Vars.currentplayer->invdowntime < -2) {
							g_Vars.currentplayer->invdowntime += numsamples;

							if (g_Vars.currentplayer->invdowntime > -3) {
								g_Vars.currentplayer->invdowntime = 0;
							}
						} else {
							for (i = 0; i < numsamples; i++) {
								if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & A_BUTTON)
										|| joyGetButtonsOnSample(i, contpad2, c2allowedbuttons & A_BUTTON)) {
									if (g_Vars.currentplayer->invdowntime > -2) {
										if (joyGetButtonsPressedOnSample(i, shootpad, shootallowedbuttons & Z_TRIG)) {
											movedata.weaponbackoffset++;
											g_Vars.currentplayer->invdowntime = -1;
										}

										if (g_Vars.currentplayer->invdowntime > -1
												&& joyGetButtonsOnSample(i, shootpad, shootallowedbuttons & Z_TRIG) == 0) {
											if (g_Vars.currentplayer->invdowntime > TICKS(15)) {
												/* amOpen(); */
												g_Vars.currentplayer->invdowntime = -1;
											} else {
												g_Vars.currentplayer->invdowntime++;
											}
										}
									}
								} else {
									if (g_Vars.currentplayer->invdowntime > 0 &&
											(!allowc1buttons || joyGetButtonsOnSample(i, shootpad, shootallowedbuttons & Z_TRIG) == 0)) {
										movedata.weaponforwardoffset++;
									}

									g_Vars.currentplayer->invdowntime = 0;
								}
							}
						}
					}

					// Handle B button activation
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & B_BUTTON)
									|| joyGetButtonsOnSample(i, contpad2, c2allowedbuttons & B_BUTTON)) {
								if (g_Vars.currentplayer->usedowntime >= -1) {
									if (joyGetButtonsPressedOnSample(i, shootpad, shootallowedbuttons & Z_TRIG)
											&& g_Vars.currentplayer->usedowntime > -1
											&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, true, false, 0) != USETIMER_CONTINUE) {
										g_Vars.currentplayer->usedowntime = -3;
									}

									if (g_Vars.currentplayer->usedowntime > -1) {
										if (g_Vars.currentplayer->usedowntime > TICKS(25)) {
											result = bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);

											if (result == USETIMER_STOP) {
												g_Vars.currentplayer->usedowntime = -1;
											} else if (result == USETIMER_REPEAT) {
												g_Vars.currentplayer->usedowntime = -2;
											} else {
												g_Vars.currentplayer->usedowntime++;
											}
										} else {
											g_Vars.currentplayer->usedowntime++;
										}
									}
								} else if (g_Vars.currentplayer->usedowntime >= -2) {
									bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);
								}
							} else {
								// Released B - activate or reload
								if (g_Vars.currentplayer->usedowntime > 0) {
									movedata.btapcount++;
								}

								g_Vars.currentplayer->usedowntime = 0;
								bgun0f0a8c50();
							}
						}
					}

					// Handle manual zoom in and out (sniper, farsight and horizon scanner)
					if (canmanualzoom && g_Vars.currentplayer->insightaimmode) {
						if (c2sticky < 0) {
							movedata.zoomoutfovpersec = -c2sticky / 70.0f;

							if (movedata.zoomoutfovpersec > 1) {
								movedata.zoomoutfovpersec = 1;
							}

							movedata.zoomoutfovpersec = movedata.zoomoutfovpersec + movedata.zoomoutfovpersec;
						}

						if (c2sticky > 0) {
							movedata.zoominfovpersec = c2sticky / 70.0f;

							if (movedata.zoominfovpersec > 1) {
								movedata.zoominfovpersec = 1;
							}

							movedata.zoominfovpersec = movedata.zoominfovpersec + movedata.zoominfovpersec;
						}
					}

					// Handle crouch and uncrouch
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (!canmanualzoom && aimonhist[i]) {
								if (joyGetStickYOnSample(i, contpad2) > 30 && joyGetStickYOnSampleIndex(i, contpad2) <= 30) {
									if (movedata.crouchdown) {
										movedata.crouchdown--;
									} else {
										movedata.crouchup++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}

								if (joyGetStickYOnSample(i, contpad2) < -30 && joyGetStickYOnSampleIndex(i, contpad2) >= -30) {
									if (movedata.crouchup) {
										movedata.crouchup--;
									} else {
										movedata.crouchdown++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}
							}

							if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
								if (aimonhist[i]) {
									if (g_Vars.currentplayer->aimtaptime > -1) {
										g_Vars.currentplayer->aimtaptime++;
									}
								} else {
									if (g_Vars.currentplayer->aimtaptime > 0
											&& g_Vars.currentplayer->aimtaptime < TICKS(15)) {
										if (movedata.crouchdown) {
											movedata.crouchdown--;
										} else {
											movedata.crouchup++;
										}
									}

									g_Vars.currentplayer->aimtaptime = 0;
								}
							}
						}
					}

					// Handle shutting eyes in multiplayer
					if (bmoveGetCrouchPos() == CROUCHPOS_SQUAT
							&& g_Vars.currentplayer->crouchoffset == -90
							&& g_Vars.mplayerisrunning
							&& g_Vars.coopplayernum < 0) {
						movedata.eyesshut = g_Vars.currentplayer->insightaimmode
							&& !canmanualzoom
							&& joyGetStickY(contpad2) < -30;
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT) {
						if (g_Vars.currentplayer->insightaimmode) {
							movedata.unk14 = 0;
						}

						movedata.farsighttempautoseek = g_Vars.currentplayer->insightaimmode
							&& (c2stickx < -30 || c2stickx > 30);
					}

					movedata.rleanleft = false;
					movedata.rleanright = false;

					// Handle mine detonation
					if ((((c1buttons & A_BUTTON) && (c1buttonsthisframe & B_BUTTON))
								|| ((c1buttons & B_BUTTON) && (c1buttonsthisframe & A_BUTTON))
								|| ((c2buttons & A_BUTTON) && (c2buttonsthisframe & B_BUTTON))
								|| ((c2buttons & B_BUTTON) && (c2buttonsthisframe & A_BUTTON)))
							&& weaponnum == WEAPON_REMOTEMINE) {
						movedata.detonating = true;
						movedata.weaponbackoffset = 0;
						movedata.weaponforwardoffset = 0;
						movedata.btapcount = 0;
						g_Vars.currentplayer->invdowntime = -2;
						g_Vars.currentplayer->usedowntime = -2;
					}
				}

				movedata.aiming = g_Vars.currentplayer->insightaimmode;
				movedata.zooming = g_Vars.currentplayer->insightaimmode;

				if (g_Vars.currentplayer->waitforzrelease
						&& joyGetButtons(shootpad, shootallowedbuttons & Z_TRIG) == 0) {
					g_Vars.currentplayer->waitforzrelease = false;
				}

				if (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_FIRETOACTIVATE)) {
					if (allowc1buttons
							&& joyGetButtonsPressedThisFrame(shootpad, shootallowedbuttons & Z_TRIG)
							&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
						movedata.btapcount++;
					}
				} else {
					movedata.triggeron = g_Vars.currentplayer->waitforzrelease == false
						&& allowc1buttons
						&& joyGetButtons(shootpad, shootallowedbuttons & Z_TRIG)
						&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED
						&& (c1buttons & A_BUTTON) == 0
						&& (c2buttons & A_BUTTON) == 0;
				}

				movedata.disablelookahead = true;
			} else {
				// 1.x control style
				if (controlmode == CONTROLMODE_13 || controlmode == CONTROLMODE_14) {
					shootbuttons = A_BUTTON;
					aimbuttons = Z_TRIG;
					invbuttons = R_TRIG;
				} else {
					shootbuttons = Z_TRIG;
					aimbuttons = R_TRIG;
					invbuttons = A_BUTTON;
				}

				if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
					for (i = 0; i < numsamples; i++) {
						aimonhist[i] = allowc1buttons && joyGetButtonsOnSample(i, contpad1, aimbuttons & c1allowedbuttons);
						aimoffhist[i] = !aimonhist[i];
					}

					g_Vars.currentplayer->insightaimmode = aimonhist[numsamples - 1];
				}

				if (!lvIsPaused()) {
					// Handle aiming
					if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) != AIMCONTROL_HOLD) {
						for (i = 0; i < numsamples; i++) {
							if (allowc1buttons && joyGetButtonsPressedOnSample(i, contpad1, aimbuttons & c1allowedbuttons)) {
								g_Vars.currentplayer->insightaimmode = !g_Vars.currentplayer->insightaimmode;
							}

							aimonhist[i] = g_Vars.currentplayer->insightaimmode;
							aimoffhist[i] = !aimonhist[i];
						}
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
						g_Vars.currentplayer->insightaimmode = true;
					}

					movedata.canswivelgun = !g_Vars.currentplayer->insightaimmode;
					movedata.canmanualaim = g_Vars.currentplayer->insightaimmode;
					movedata.canautoaim = !g_Vars.currentplayer->insightaimmode;

					if (controlmode == CONTROLMODE_12 || controlmode == CONTROLMODE_14) {
						// Handle side stepping
						if (g_Vars.currentplayer->insightaimmode == false) {
							if (allowc1buttons) {
								movedata.digitalstepleft = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (L_CBUTTONS));
								movedata.digitalstepright = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (R_CBUTTONS));
							}
						} else {
							// This doesn't appear to be r-leaning.
							// R-leaning still works when these are commented.
							if (c1buttons & (L_CBUTTONS)) {
								movedata.unk30 = 1;
							}

							if (c1buttons & (R_CBUTTONS)) {
								movedata.unk34 = 1;
							}
						}

						movedata.digitalstepforward = !g_Vars.currentplayer->insightaimmode && (c1buttons & (U_CBUTTONS));
						movedata.digitalstepback = !g_Vars.currentplayer->insightaimmode && (c1buttons & (D_CBUTTONS));
						movedata.canlookahead = false;
						movedata.cannaturalpitch = !g_Vars.currentplayer->insightaimmode;
						movedata.speedvertadown = 0;
						movedata.speedvertaup = 0;
						movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;

#ifndef PLATFORM_N64
						movedata.cannaturalpitch = movedata.cannaturalpitch || (movedata.freelookdy != 0.0f);
						movedata.cannaturalturn = movedata.cannaturalturn  || (movedata.freelookdx != 0.0f);
#endif

						if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
							movedata.digitalstepforward = (g_Vars.currentplayer->autocontrol_y > 0);
							movedata.digitalstepback = (g_Vars.currentplayer->autocontrol_y < 0);
							movedata.analogstrafe = 0;
							movedata.analogwalk = 0;
							movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
							movedata.analogpitch = 0;
#ifndef PLATFORM_N64
							movedata.freelookdx = 0.0f;
							movedata.freelookdy = 0.0f;
#endif
						}
					} else {
						// 1.1 or 1.3
						if (c1buttons & (L_CBUTTONS)) {
							movedata.unk30 = 1;
						}

						if (c1buttons & (R_CBUTTONS)) {
							movedata.unk34 = 1;
						}

						if (!g_Vars.currentplayer->insightaimmode && allowc1buttons) {
							movedata.digitalstepleft = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (L_CBUTTONS));
							movedata.digitalstepright = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (R_CBUTTONS));
						}

						movedata.digitalstepforward = false;
						movedata.digitalstepback = false;
						movedata.canlookahead = !g_Vars.currentplayer->insightaimmode;
						movedata.cannaturalpitch = false;

						// Looking up/down
						if (!g_Vars.currentplayer->insightaimmode && (c1buttons & (U_CBUTTONS))) {
							movedata.speedvertadown = 1;
						}

						if (!g_Vars.currentplayer->insightaimmode && (c1buttons & (D_CBUTTONS))) {
							movedata.speedvertaup = 1;
						}

						movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;
						movedata.unk14 = 0;

						if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
							movedata.analogstrafe = 0;
							movedata.analogwalk = g_Vars.currentplayer->autocontrol_y;
							movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
							movedata.analogpitch = 0;
#ifndef PLATFORM_N64
							movedata.freelookdx = 0.0f;
							movedata.freelookdy = 0.0f;
#endif
						}
					}

					// Handle looking up/down while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw > 60) {
						movedata.speedvertadown = (movedata.c1stickyraw - 60) / 10.0f;

						if (movedata.speedvertadown > 1) {
							movedata.speedvertadown = 1;
						}
					} else if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw < -60) {
						movedata.speedvertaup = (-60 - movedata.c1stickyraw) / 10.0f;

						if (movedata.speedvertaup > 1) {
							movedata.speedvertaup = 1;
						}
					}

					// Handle looking left/right while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw < -60) {
						movedata.aimturnleftspeed = (-60 - movedata.c1stickxraw) / 10.0f;

						if (movedata.aimturnleftspeed > 1) {
							movedata.aimturnleftspeed = 1;
						}
					} else if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw > 60) {
						movedata.aimturnrightspeed = (movedata.c1stickxraw - 60) / 10.0f;

						if (movedata.aimturnrightspeed > 1) {
							movedata.aimturnrightspeed = 1;
						}
					}

					// Handle A button
					if (allowc1buttons) {
						if (g_Vars.currentplayer->invdowntime < -2) {
							g_Vars.currentplayer->invdowntime += numsamples;

							if (g_Vars.currentplayer->invdowntime > -3) {
								g_Vars.currentplayer->invdowntime = 0;
							}
						} else {
							for (i = 0; i < numsamples; i++) {
								if (joyGetButtonsOnSample(i, contpad1, invbuttons & c1allowedbuttons)) {
									if (g_Vars.currentplayer->invdowntime > -2) {
										if (joyGetButtonsPressedOnSample(i, contpad1, shootbuttons & c1allowedbuttons)) {
											movedata.weaponbackoffset++;
											g_Vars.currentplayer->invdowntime = -1;
										}

										if (g_Vars.currentplayer->invdowntime >= 0 && joyGetButtonsOnSample(i, contpad1, shootbuttons & c1allowedbuttons) == 0) {
											// Holding A and haven't pressed Z
											if (g_Vars.currentplayer->invdowntime > TICKS(15)) {
												/* amOpen(); */
												g_Vars.currentplayer->invdowntime = -1;
											} else {
												g_Vars.currentplayer->invdowntime++;
											}
										}
									}
								} else {
									// Wasn't holding A on this sample
									if (g_Vars.currentplayer->invdowntime > 0 &&
											(!allowc1buttons || joyGetButtonsOnSample(i, contpad1, shootbuttons & c1allowedbuttons) == 0)) {
										// But was on previous sample, so cycle weapon
										movedata.weaponforwardoffset++;
									}

									g_Vars.currentplayer->invdowntime = 0;
								}
							}
						}
					}

					// Handle B button
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & B_BUTTON)) {
								if (g_Vars.currentplayer->usedowntime >= -1) {
									if (joyGetButtonsPressedOnSample(i, contpad1, shootbuttons & c1allowedbuttons)
											&& g_Vars.currentplayer->usedowntime >= 0
											&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, true, false, 0) != USETIMER_CONTINUE) {
										g_Vars.currentplayer->usedowntime = -3;
									}

									if (g_Vars.currentplayer->usedowntime >= 0) {
										if (g_Vars.currentplayer->usedowntime > TICKS(25)) {
											s32 result = bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);

											if (result == USETIMER_STOP) {
												g_Vars.currentplayer->usedowntime = -1;
											} else if (result == USETIMER_REPEAT) {
												g_Vars.currentplayer->usedowntime = -2;
											} else {
												g_Vars.currentplayer->usedowntime++;
											}
										} else {
											g_Vars.currentplayer->usedowntime++;
										}
									}
								} else {
									if (g_Vars.currentplayer->usedowntime >= -2) {
										bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);
									}
								}
							} else {
								// Released B
								if (g_Vars.currentplayer->usedowntime > 0) {
									movedata.btapcount++;
								}

								g_Vars.currentplayer->usedowntime = 0;
								bgun0f0a8c50();
							}
						}
					}


					// handle L button
					for (i = 0; i< numsamples; i++) {
						if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & L_TRIG)) {
							if (g_Vars.currentplayer->altdowntime >= -1) {
								if (joyGetButtonsPressedOnSample(i, contpad1, shootbuttons & c1allowedbuttons)
										&& g_Vars.currentplayer->altdowntime >= 0
										&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, true, false, true) != USETIMER_CONTINUE) {
									g_Vars.currentplayer->altdowntime = -3;
								}

								if (g_Vars.currentplayer->altdowntime != -4) {
									if (g_Vars.currentplayer->altdowntime <= 0) {
										g_Vars.currentplayer->altdowntime++;
									}
								}
							} else  {
								if (g_Vars.currentplayer->altdowntime == -2) {
									bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, false, false, true);
									g_Vars.currentplayer->altdowntime = -4;
								}
							}
						} else {
							// Released L
							if (g_Vars.currentplayer->altdowntime != 0) {
								s32 result = bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, (g_Vars.currentplayer->altdowntime == -3 ? true: false), false, true);

								if (result == USETIMER_STOP) {
									g_Vars.currentplayer->altdowntime = -1;
								} else if (result == USETIMER_REPEAT) {
									g_Vars.currentplayer->altdowntime = -2;
								}
							}
							g_Vars.currentplayer->altdowntime = 0;
						}
					}

					// Handle ALT1 / MI Reload Hack
					for (i = 0; i < numsamples; i++) {
						if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & X_BUTTON)) {
							movedata.alt1tapcount++;
						}
					}


					// Handle radial menu (D-Down)
					for (i = 0; i < numsamples; i++) {
						if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & D_JPAD)) {
							if (g_Vars.currentplayer->amdowntime < -2) {
								g_Vars.currentplayer->amdowntime += numsamples;

								if (g_Vars.currentplayer->amdowntime > -3) {
									g_Vars.currentplayer->amdowntime = 0;
								}
							} else {
								if (g_Vars.currentplayer->amdowntime >= 0) {
									if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & D_JPAD)) {
										amOpen();
										g_Vars.currentplayer->amdowntime = -1;
									} else {
										g_Vars.currentplayer->amdowntime++;
									}
								}
							}
						} else if (g_Vars.currentplayer) {
							g_Vars.currentplayer->amdowntime = 0;
						}
					}

					// Handle manual zoom in and out (sniper, farsight and horizon scanner)
					if (canmanualzoom && g_Vars.currentplayer->insightaimmode) {
						increment = 1;
						zoomout = c1buttons & (D_CBUTTONS);
						zoomin = c1buttons & (U_CBUTTONS);

						// @bug? Should this be HAND_RIGHT?
						if (bgunGetWeaponNum(HAND_LEFT) == WEAPON_FARSIGHT) {
							increment = 0.5f;
						}

						if (zoomout) {
							movedata.zoomoutfovpersec = increment;
						}

						if (zoomin) {
							movedata.zoominfovpersec = increment;
						}
					}

					// Handle crouch and uncrouch
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (!canmanualzoom && aimonhist[i]) {
								if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & (U_CBUTTONS))) {
									if (movedata.crouchdown) {
										movedata.crouchdown--;
									} else {
										movedata.crouchup++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}

								if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & (D_CBUTTONS))) {
									if (movedata.crouchup) {
										movedata.crouchup--;
									} else {
										movedata.crouchdown++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}
							}

							if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
								if (aimonhist[i]) {
									if (g_Vars.currentplayer->aimtaptime >= 0) {
										g_Vars.currentplayer->aimtaptime++;
									}
								} else {
									// Released aim
									if (g_Vars.currentplayer->aimtaptime > 0 && g_Vars.currentplayer->aimtaptime < TICKS(15)) {
										// Was only a tap, so uncrouch
										if (movedata.crouchdown) {
											movedata.crouchdown--;
										} else {
											movedata.crouchup++;
										}
									}

									g_Vars.currentplayer->aimtaptime = 0;
								}
							}
						}
					}

					// Handle shutting eyes in multiplayer
					if (bmoveGetCrouchPos() == CROUCHPOS_SQUAT
							&& g_Vars.currentplayer->crouchoffset == -90
							&& g_Vars.mplayerisrunning
							&& g_Vars.coopplayernum <= -1) {
						movedata.eyesshut = g_Vars.currentplayer->insightaimmode
							&& !canmanualzoom
							&& joyGetButtons(contpad1, c1allowedbuttons & (D_CBUTTONS));
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT) {
						movedata.farsighttempautoseek = g_Vars.currentplayer->insightaimmode && (c1buttons & (L_CBUTTONS | R_CBUTTONS));
					} else {
						movedata.rleanleft = g_Vars.currentplayer->insightaimmode && (c1buttons & (L_CBUTTONS));
						movedata.rleanright = g_Vars.currentplayer->insightaimmode && (c1buttons & (R_CBUTTONS));
					}

					// Handle mine detonation
					if ((((c1buttons & invbuttons) && (c1buttonsthisframe & B_BUTTON))
							|| ((c1buttons & B_BUTTON) && (c1buttonsthisframe & invbuttons)))
							&& weaponnum == WEAPON_REMOTEMINE) {
						movedata.detonating = true;
						movedata.weaponbackoffset = 0;
						movedata.weaponforwardoffset = 0;
						movedata.btapcount = 0;
						g_Vars.currentplayer->invdowntime = -2;
						g_Vars.currentplayer->usedowntime = -2;
					}
				}

				movedata.aiming = g_Vars.currentplayer->insightaimmode;
				movedata.zooming = g_Vars.currentplayer->insightaimmode;

				if (g_Vars.currentplayer->waitforzrelease
						&& (c1buttons & shootbuttons) == 0) {
					g_Vars.currentplayer->waitforzrelease = false;
				}

				if (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_FIRETOACTIVATE)) {
					if ((c1buttonsthisframe & shootbuttons)
							&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
						movedata.btapcount++;
					}
				} else {
					movedata.triggeron = g_Vars.currentplayer->waitforzrelease == false
						&& (c1buttons & shootbuttons)
						&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED
						&& (c1buttons & invbuttons) == 0;
				}

				if (controlmode == CONTROLMODE_12 || controlmode == CONTROLMODE_14) {
					movedata.disablelookahead = true;
				}
			} // end 1.x
		}
	}

	g_Vars.currentplayer->bondactivateorreload = 0;

	if (movedata.alt1tapcount) {
		g_Vars.currentplayer->bondactivateorreload = g_Vars.currentplayer->bondactivateorreload | JO_ACTION_RELOAD;
	}
	if (movedata.btapcount) {
		g_Vars.currentplayer->activatetimelast = g_Vars.currentplayer->activatetimethis;
		g_Vars.currentplayer->activatetimethis = g_Vars.lvframe60;
		g_Vars.currentplayer->bondactivateorreload = g_Vars.currentplayer->bondactivateorreload | JO_ACTION_ACTIVATE;

		bmoveHandleActivate();
	}

	if (!movedata.invertpitch) {
		savedverta = movedata.speedvertadown;
		movedata.analogpitch = -movedata.analogpitch;
		movedata.c1stickyraw = -movedata.c1stickyraw;
		movedata.speedvertadown = movedata.speedvertaup;
		movedata.speedvertaup = savedverta;
	}

	bgunTickGameplay(movedata.triggeron);

	if (g_Vars.bondvisible && (bgunIsFiring(HAND_RIGHT) || bgunIsFiring(HAND_LEFT))) {
		noiseradius = 0;

		if (bgunIsFiring(HAND_RIGHT) && bgunGetNoiseRadius(HAND_RIGHT) > noiseradius) {
			noiseradius = bgunGetNoiseRadius(HAND_RIGHT);
		}

		if (bgunIsFiring(HAND_LEFT) && bgunGetNoiseRadius(HAND_LEFT) > noiseradius) {
			noiseradius = bgunGetNoiseRadius(HAND_LEFT);
		}

		chrsCheckForNoise(noiseradius);
	}

	bgunSetSightVisible(GUNSIGHTREASON_NOTAIMING, movedata.aiming);

	if (movedata.zoomoutfovpersec > 0) {
		currentPlayerZoomOut(movedata.zoomoutfovpersec);
	}

	if (movedata.zoominfovpersec > 0) {
		currentPlayerZoomIn(movedata.zoominfovpersec);
	}

	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED && !g_MainIsEndscreen) {
#ifdef PLATFORM_N64
		zoomfov = 60;
#else
		zoomfov = videoGetPlayerFovY();
#endif

		// FarSight in secondary function
		if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT
				&& g_Vars.currentplayer->insightaimmode
				&& (movedata.farsighttempautoseek || g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY)
				&& g_Vars.currentplayer->autoeraserdist > 0) {
			eraserfov = cam0f0b49b8(500.0f / g_Vars.currentplayer->autoeraserdist);

#ifdef PLATFORM_N64
			if (eraserfov > 60) {
				eraserfov = 60;
			}
#else
			if (eraserfov > videoGetPlayerFovY()) {
				eraserfov = videoGetPlayerFovY();
			}
#endif

			if (eraserfov < 2) {
				eraserfov = 2;
			}

			g_Vars.currentplayer->gunzoomfovs[1] = eraserfov;

			mtx4TransformVec(camGetWorldToScreenMtxf(), &g_Vars.currentplayer->autoerasertarget->pos, &spa0);

			cam0f0b4eb8(&spa0, crosspos, eraserfov, g_Vars.currentplayer->c_perspaspect);

			if (crosspos[0] < (camGetScreenLeft() + camGetScreenWidth() * 0.5f) - 20.0f) {
				movedata.aimturnleftspeed = 0.25f;
			} else if (crosspos[0] > camGetScreenLeft() + camGetScreenWidth() * 0.5f + 20.0f) {
				movedata.aimturnrightspeed = 0.25f;
			}

			if (crosspos[1] < (camGetScreenTop() + camGetScreenHeight() * 0.5f) - 20.0f) {
				movedata.speedvertaup = 0.25f;
			} else if (crosspos[1] > camGetScreenTop() + camGetScreenHeight() * 0.5f + 20.0f) {
				movedata.speedvertadown = 0.25f;
			}
		}

		if (movedata.zooming) {
			zoomfov = currentPlayerGetGunZoomFov();
		}

		if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_AR34
				&& g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY) {
			zoomfov = currentPlayerGetGunZoomFov();
		}

#ifdef PLATFORM_N64
		if (zoomfov <= 0) {
			zoomfov = 60;
		}
#else
		if (zoomfov <= 0) {
			zoomfov = videoGetPlayerFovY();
		}
#endif

		playerTweenFovY(zoomfov);
		playerUpdateZoom();
	}

	bmoveApplyMoveData(&movedata);

	// Speed boost
	// After 3 seconds of holding forward at max speed, apply boost multiplier.
	// The multiplier starts at 1 and reaches 1.25 after about 0.1 seconds.
	if (g_Vars.currentplayer->speedmaxtime60 >= TICKS(180)) {
		if (g_Vars.currentplayer->speedboost < 1.25f) {
			g_Vars.currentplayer->speedboost += 0.01f * g_Vars.lvupdate60freal;
		}

		if (g_Vars.currentplayer->speedboost > 1.25f) {
#if PIRACYCHECKS
			piracyRestore();
#endif
			g_Vars.currentplayer->speedboost = 1.25f;
		}
	} else {
		if (g_Vars.currentplayer->speedboost > 1) {
			g_Vars.currentplayer->speedboost -= 0.01f * g_Vars.lvupdate60freal;
		}

		if (g_Vars.currentplayer->speedboost < 1) {
			g_Vars.currentplayer->speedboost = 1;
		}
	}

	// Look ahead
	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		lookahead = -4;

		offbike = g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK
			|| g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB;

		if (g_Vars.currentplayer->lookaheadcentreenabled) {
			if (g_Vars.lvframenum != g_Vars.currentplayer->lookaheadframe
					&& g_Vars.currentplayernum == (g_Vars.lvframenum & 3)) {
				g_Vars.currentplayer->cachedlookahead = bmoveCalculateLookahead();
			}

			lookahead = g_Vars.currentplayer->cachedlookahead;
		}

		if (g_Vars.currentplayer->movecentrerelease
				&& movedata.analogwalk < 40 && movedata.analogwalk > -40) {
			g_Vars.currentplayer->movecentrerelease = false;
		}

		if (offbike) {
			if (movedata.speedvertadown > 0 || movedata.speedvertaup > 0) {
				g_Vars.currentplayer->docentreupdown = false;
				g_Vars.currentplayer->prevupdown = true;
				g_Vars.currentplayer->automovecentre = false;
			} else {
				if (movedata.disablelookahead) {
					g_Vars.currentplayer->automovecentre = false;
				} else if (g_Vars.currentplayer->automovecentreenabled) {
					if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
						g_Vars.currentplayer->automovecentre = true;
					}

					if (g_Vars.currentplayer->automovecentre
							&& (g_Vars.currentplayer->vv_verta > lookahead + 5.0f || g_Vars.currentplayer->vv_verta < lookahead + -10.0f)
							&& g_Vars.currentplayer->movecentrerelease == false) {
						g_Vars.currentplayer->docentreupdown = true;
					}
				} else if (g_Vars.currentplayer->fastmovecentreenabled
						&& movedata.canlookahead
						&& (movedata.analogwalk > 60 || movedata.analogwalk < -60)
						&& (g_Vars.currentplayer->vv_verta > lookahead + 5.0f || g_Vars.currentplayer->vv_verta < lookahead + -10.0f)
						&& g_Vars.currentplayer->movecentrerelease == false) {
					g_Vars.currentplayer->docentreupdown = true;
				}

				g_Vars.currentplayer->prevupdown = false;
			}
		}

#if VERSION >= VERSION_NTSC_1_0
		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
			g_Vars.currentplayer->docentreupdown = false;
		}
#endif

		if (g_Vars.currentplayer->docentreupdown) {
			if (offbike) {
				// Determine direction for lookahead increment
				increment2 = (g_Vars.currentplayer->speedverta * g_Vars.currentplayer->speedverta * 0.5f) / 0.05f;

				if (g_Vars.currentplayer->vv_verta > lookahead + increment2) {
					bmoveUpdateSpeedVerta(1);
				} else if (g_Vars.currentplayer->vv_verta < lookahead - increment2) {
					bmoveUpdateSpeedVerta(-1);
				} else {
					bmoveUpdateSpeedVerta(0);
				}

				// Calculate new verta
				newverta = g_Vars.currentplayer->vv_verta + (g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal + g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal);

				if (g_Vars.currentplayer->vv_verta > lookahead && newverta > lookahead) {
					g_Vars.currentplayer->vv_verta = newverta;
				} else if (g_Vars.currentplayer->vv_verta < lookahead && newverta < lookahead) {
					g_Vars.currentplayer->vv_verta = newverta;
				} else {
					g_Vars.currentplayer->vv_verta = lookahead;
					g_Vars.currentplayer->speedverta = 0;

					if (g_Vars.currentplayer->prevupdown == false) {
						g_Vars.currentplayer->docentreupdown = false;
					}
				}
			}
		} else {
			if (movedata.cannaturalpitch) {
				tmp = viGetFovY() / 60.0f;
				fVar25 = movedata.analogpitch / 70.0f;

				if (fVar25 > 1) {
					fVar25 = 1;
				} else if (fVar25 < -1) {
					fVar25 = -1;
				}

				if (fVar25 >= 0) {
					fVar25 *= fVar25;
				} else {
					fVar25 *= -fVar25;
				}

#ifndef PLATFORM_N64
				if (movedata.freelookdy) {
					fVar25 += movedata.freelookdy;
				}
#endif

				g_Vars.currentplayer->speedverta = -fVar25 * tmp;
			} else if (movedata.speedvertadown > 0) {
				bmoveUpdateSpeedVerta(movedata.speedvertadown);

				if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
					g_Vars.currentplayer->movecentrerelease = true;
				}
			} else if (movedata.speedvertaup > 0) {
				bmoveUpdateSpeedVerta(-movedata.speedvertaup);

				if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
					g_Vars.currentplayer->movecentrerelease = true;
				}
			} else {
				bmoveUpdateSpeedVerta(0);
			}

			g_Vars.currentplayer->vv_verta += g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal * 3.5f;
		}
	}

	if (movedata.cannaturalturn) {
		tmp = viGetFovY() / 60.0f;
		fVar25 = movedata.analogturn / 70.0f;

		if (fVar25 > 1) {
			fVar25 = 1;
		} else if (fVar25 < -1) {
			fVar25 = -1;
		}

		if (fVar25 >= 0) {
			fVar25 *= fVar25;
		} else {
			fVar25 *= -fVar25;
		}

#ifndef PLATFORM_N64
		if (movedata.freelookdx) {
			fVar25 += movedata.freelookdx;
		}
#endif

		g_Vars.currentplayer->speedthetacontrol = fVar25 * tmp;
	} else if (movedata.aimturnleftspeed > 0) {
		bmoveUpdateSpeedThetaControl(movedata.aimturnleftspeed);
	} else if (movedata.aimturnrightspeed > 0) {
		bmoveUpdateSpeedThetaControl(-movedata.aimturnrightspeed);
	} else {
		bmoveUpdateSpeedThetaControl(0);
	}

	g_Vars.currentplayer->speedtheta = g_Vars.currentplayer->speedthetacontrol;
	bmoveUpdateSpeedTheta();

	if (movedata.detonating) {
		g_Vars.currentplayer->hands[HAND_RIGHT].mode = HANDMODE_NONE;
		g_Vars.currentplayer->hands[HAND_RIGHT].modenext = HANDMODE_NONE;
		playerActivateRemoteMineDetonator(g_Vars.currentplayernum);
	}

	cancycleweapons = true;

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		cancycleweapons = false;
	}

	if (g_Vars.lvframenum < 10) {
		cancycleweapons = false;
	}

	if (cancycleweapons) {
		while (movedata.weaponbackoffset-- > 0) {
			bgunCycleBack();
		}

		while (movedata.weaponforwardoffset-- > 0) {
			bgunCycleForward();
		}
	}

	if (g_Vars.currentplayer->unk1c64) {
		g_Vars.currentplayer->unk1c64 = 0;
	} else if (movedata.canswivelgun) {
		f32 x;
		f32 y;

		bgunSetAimType(0);

		if (
				(
				 movedata.canautoaim
				 && (bmoveIsAutoAimXEnabledForCurrentWeapon() || bmoveIsAutoAimYEnabledForCurrentWeapon())
				 && g_Vars.currentplayer->autoxaimprop
				 && g_Vars.currentplayer->autoyaimprop
				 && weaponHasAimFlag(weaponnum, INVAIMFLAG_AUTOAIM)
				)
				|| (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_CMP150 && g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY)) {
			// Auto aim - move crosshair towards target
			s32 followlockon = false;

			if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_CMP150
					&& g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY) {
				followlockon = true;
			}

			if (g_Vars.currentplayer->autoaimdamp > (PAL ? 0.955f : 0.963f)) {
				g_Vars.currentplayer->autoaimdamp -= (PAL ? 0.00037999986670911f : 0.00031999943894334f) * g_Vars.lvupdate60freal;
			}

			if (g_Vars.currentplayer->autoaimdamp < (PAL ? 0.955f : 0.963f)) {
				g_Vars.currentplayer->autoaimdamp = (PAL ? 0.955f : 0.963f);
			}

			x = g_Vars.currentplayer->autoaimx;
			y = g_Vars.currentplayer->autoaimy;

			if (followlockon) {
				bgunSwivel(x, y, PAL ? 0.899f : 0.915f, PAL ? 0.899f : 0.915f);
			} else {
				bgunSwivelWithDamp(x, y, g_Vars.currentplayer->autoaimdamp);
			}
		} else {
			// This code moves the crosshair as the player turns and makes
			// it return to the centre when not affected by anything else.
			if (g_Vars.currentplayer->autoaimdamp < (PAL ? 0.974f : 0.979f)) {
				g_Vars.currentplayer->autoaimdamp += (PAL ? 0.00037999986670911f : 0.00031999943894334f) * g_Vars.lvupdate60freal;
			}

			if (g_Vars.currentplayer->autoaimdamp > (PAL ? 0.974f : 0.979f)) {
				g_Vars.currentplayer->autoaimdamp = (PAL ? 0.974f : 0.979f);
			}

#ifdef PLATFORM_N64
			x = g_Vars.currentplayer->speedtheta * 0.3f + g_Vars.currentplayer->gunextraaimx;
			y = -g_Vars.currentplayer->speedverta * 0.1f + g_Vars.currentplayer->gunextraaimy;
#else
			f32 xscale, yscale;
			if (movedata.freelookdx || movedata.freelookdy) {
				xscale = 320.f / (f32)videoGetWidth();
				yscale = 240.f / (f32)videoGetHeight();
			} else {
				xscale = 1.f;
				yscale = 1.f;
			}
			x = g_Vars.currentplayer->speedtheta * 0.3f * xscale + g_Vars.currentplayer->gunextraaimx;
			y = -g_Vars.currentplayer->speedverta * 0.1f * yscale + g_Vars.currentplayer->gunextraaimy;
#endif

			bgunSwivelWithDamp(x, y, PAL ? 0.955f : 0.963f);
		}
	} else if (movedata.canmanualaim) {
		// Adjust crosshair's position on screen
		// when holding aim and moving stick
		bgunSetAimType(0);
		bgunSwivelWithoutDamp((movedata.c1stickxraw * 0.65f) / 80.0f, (movedata.c1stickyraw * 0.65f) / 80.0f);
	}
}

void bmoveFindEnteredRoomsByPos(struct player *player, struct coord *mid, RoomNum *rooms)
{
	struct coord bbmin;
	struct coord bbmax;
	f32 eyeheight = g_Vars.players[playermgrGetPlayerNumByProp(player->prop)]->vv_eyeheight;
	f32 headheight = g_Vars.players[playermgrGetPlayerNumByProp(player->prop)]->vv_headheight;

	bbmin.x = mid->x - 50;
	bbmin.y = mid->y - player->crouchheight - eyeheight - 10;
	bbmin.z = mid->z - 50;

	bbmax.x = mid->x + 50;
	bbmax.y = mid->y - player->crouchheight - eyeheight + headheight + 10;
	bbmax.z = mid->z + 50;

	bgFindEnteredRooms(&bbmin, &bbmax, rooms, 7, false);
}

void bmoveFindEnteredRooms(struct player *player, RoomNum *rooms)
{
	bmoveFindEnteredRoomsByPos(player, &player->prop->pos, rooms);
}

void bmoveUpdateRooms(struct player *player)
{
	propDeregisterRooms(player->prop);
	bmoveFindEnteredRooms(player, player->prop->rooms);
	propRegisterRooms(player->prop);
}

void bmove0f0cb904(struct coord *arg0)
{
	if (arg0->f[0] || arg0->f[2]) {
		f32 hypotenuse = sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2]);
		s32 i;

		if (hypotenuse > 1.5f) {
			arg0->x *= 1.5f / hypotenuse;
			arg0->z *= 1.5f / hypotenuse;
			hypotenuse = 1.5f;
		}

		for (i = 0; i < 3; i++) {
			if (hypotenuse > 0.0001f) {
				if (arg0->f[i] != 0) {
					if (arg0->f[i] > 0) {
						arg0->f[i] -= (1.0f / 30.0f) * g_Vars.lvupdate60freal * arg0->f[i] / hypotenuse;

						if (arg0->f[i] < 0) {
							arg0->f[i] = 0;
						}
					} else if (arg0->f[i] < 0) {
						arg0->f[i] -= (1.0f / 30.0f) * g_Vars.lvupdate60freal * arg0->f[i] / hypotenuse;

						if (arg0->f[i] > 0) {
							arg0->f[i] = 0;
						}
					}
				}
			} else {
				arg0->f[i] = 0;
			}
		}
	}
}

void bmove0f0cba88(f32 *a, f32 *b, struct coord *c, f32 mult1, f32 mult2)
{
	if (c->x != 0 || c->z != 0) {
		bmove0f0cb904(c);
		*a = c->z * mult2 + -c->x * mult1;
		*b = -c->x * mult2 - c->z * mult1;
	} else {
		*a = 0;
		*b = 0;
	}
}

void bmoveUpdateMoveInitSpeed(struct coord *newpos)
{
	if (g_Vars.currentplayer->moveinitspeed.x != 0) {
		if (g_Vars.currentplayer->moveinitspeed.x < 0.001f && g_Vars.currentplayer->moveinitspeed.x > -0.001f) {
			g_Vars.currentplayer->moveinitspeed.x = 0;
		} else {
			g_Vars.currentplayer->moveinitspeed.x *= 0.9f;
			newpos->x += g_Vars.currentplayer->moveinitspeed.x * g_Vars.lvupdate60freal;
		}
	}

	if (g_Vars.currentplayer->moveinitspeed.z != 0) {
		if (g_Vars.currentplayer->moveinitspeed.z < 0.001f && g_Vars.currentplayer->moveinitspeed.z > -0.001f) {
			g_Vars.currentplayer->moveinitspeed.z = 0;
		} else {
			g_Vars.currentplayer->moveinitspeed.z *= 0.9f;
			newpos->z += g_Vars.currentplayer->moveinitspeed.z * g_Vars.lvupdate60freal;
		}
	}
}

void bmoveTick(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2)
{
	struct chrdata *chr;
	u8 foot;
	s32 sound;
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;
	f32 distance;

	bmoveProcessInput(allowc1x, allowc1y, allowc1buttons, ignorec2);

	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeTick();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabTick();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_CUTSCENE) {
		bcutsceneTick();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkTick();
	}

	// Update footstep sounds
	if ((g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK || g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB)
			&& (g_Vars.currentplayer->speedforwards || g_Vars.currentplayer->speedsideways)
			&& (!g_Vars.normmplayerisrunning || PLAYERCOUNT() == 1)) {
		chr = g_Vars.currentplayer->prop->chr;

		if (g_Vars.currentplayer->cameramode == CAMERAMODE_DEFAULT
				&& g_Vars.currentplayer->bdeltapos.y >= -6.0f) {
			xdiff = g_Vars.currentplayer->bondprevpos.x - g_Vars.currentplayer->prop->pos.x;
			ydiff = g_Vars.currentplayer->bondprevpos.y - g_Vars.currentplayer->prop->pos.y;
			zdiff = g_Vars.currentplayer->bondprevpos.z - g_Vars.currentplayer->prop->pos.z;

			foot = 0;
			distance = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

			g_Vars.currentplayer->footstepdist += distance;

			if (g_Vars.currentplayer->footstepdist >= 150.0f) {
				foot = 1;
				g_Vars.currentplayer->footstepdist = 0;
			}

			if (foot) {
				if (g_Vars.currentplayer->foot) {
					chr->footstep = 1;
				} else {
					chr->footstep = 2;
				}

				g_Vars.currentplayer->foot = 1 - g_Vars.currentplayer->foot;

				chr->floortype = g_Vars.currentplayer->floortype;

				sound = footstepChooseSound(chr, distance > 10);

				if (sound != -1) {
					snd00010718(0, 0, AL_VOL_FULL, AL_PAN_CENTER, sound, 1, 1, -1, true);
				}
			}
		}
	}
}

void bmoveUpdateVerta(void)
{
	while (g_Vars.currentplayer->vv_verta < -180) {
		g_Vars.currentplayer->vv_verta += 360;
	}

	while (g_Vars.currentplayer->vv_verta >= 180) {
		g_Vars.currentplayer->vv_verta -= 360;
	}

	if (g_Vars.currentplayer->vv_verta > 90) {
		g_Vars.currentplayer->vv_verta = 90;
	} else if (g_Vars.currentplayer->vv_verta < -90) {
		g_Vars.currentplayer->vv_verta = -90;
	}

	g_Vars.currentplayer->vv_costheta = cosf(BADDEG2RAD(g_Vars.currentplayer->vv_theta));
	g_Vars.currentplayer->vv_sintheta = sinf(BADDEG2RAD(g_Vars.currentplayer->vv_theta));

	g_Vars.currentplayer->vv_verta360 = g_Vars.currentplayer->vv_verta;

	if (g_Vars.currentplayer->vv_verta360 < 0) {
		g_Vars.currentplayer->vv_verta360 += 360;
	}

	g_Vars.currentplayer->vv_cosverta = cosf(BADDEG2RAD(g_Vars.currentplayer->vv_verta360));
	g_Vars.currentplayer->vv_sinverta = sinf(BADDEG2RAD(g_Vars.currentplayer->vv_verta360));

	g_Vars.currentplayer->bond2.unk00.x = -g_Vars.currentplayer->vv_sintheta;
	g_Vars.currentplayer->bond2.unk00.y = 0;
	g_Vars.currentplayer->bond2.unk00.z = g_Vars.currentplayer->vv_costheta;

	if (g_Vars.currentplayer->prop) {
		struct chrdata *chr = g_Vars.currentplayer->prop->chr;

		if (chr && chr->model) {
			chrSetLookAngle(chr, BADDEG2RAD(360 - g_Vars.currentplayer->vv_theta));
		}
	}
}

void bmove0f0cc19c(struct coord *arg)
{
	f32 min;
	f32 mult;

	g_Vars.currentplayer->bond2.unk10.x = arg->x;
	g_Vars.currentplayer->bond2.unk10.y = arg->y;
	g_Vars.currentplayer->bond2.unk10.z = arg->z;

	if (g_Vars.currentplayer->isdead && g_Vars.currentplayer->bondleandown > 0) {
		g_Vars.currentplayer->bondleandown -= 0.25f;

		if (g_Vars.currentplayer->bondleandown < 0) {
			g_Vars.currentplayer->bondleandown = 0;
		}
	}

	if (g_Vars.currentplayer->vv_verta < 0) {
		g_Vars.currentplayer->bond2.unk10.y += -(1.0f - g_Vars.currentplayer->vv_cosverta) * g_Vars.currentplayer->bondleandown;
	}

	if (cheatIsActive(CHEAT_SMALLJO)) {
		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
			mult = g_Vars.currentplayer->bondentert * 0.6f + 0.4f;
		} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK && g_Vars.currentplayer->walkinitmove) {
			mult = (1.0f - g_Vars.currentplayer->walkinitt) * 0.6f + 0.4f;
			g_Vars.currentplayer->bond2.unk10.y += (g_Vars.currentplayer->crouchoffsetreal - g_Vars.currentplayer->crouchoffsetrealsmall) * g_Vars.currentplayer->walkinitt;
		} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
			mult = 0.4f;
			g_Vars.currentplayer->bond2.unk10.y += (g_Vars.currentplayer->crouchoffsetreal - g_Vars.currentplayer->crouchoffsetrealsmall);
		} else {
			mult = 0.4f;
		}

		g_Vars.currentplayer->bond2.unk10.y = (g_Vars.currentplayer->bond2.unk10.y - g_Vars.currentplayer->vv_manground) * mult;

#if VERSION < VERSION_NTSC_1_0
		if (g_Vars.currentplayer->bond2.unk10.y < 30) {
			g_Vars.currentplayer->bond2.unk10.y = 30;
		}
#endif

		g_Vars.currentplayer->bond2.unk10.y += g_Vars.currentplayer->vv_manground;
	}

#if VERSION >= VERSION_NTSC_1_0
	min = g_Vars.currentplayer->vv_ground + 10;

	if (g_Vars.currentplayer->bond2.unk10.y < min) {
		g_Vars.currentplayer->bond2.unk10.y = min;
	}
#endif
}

void bmoveUpdateHead(f32 arg0, f32 arg1, f32 arg2, Mtxf *arg3, f32 arg4)
{
	f32 sp244 = 0;
	Mtxf sp180;
	Mtxf sp116;
	f32 sp100[4];
	f32 sp84[4];
	f32 sp68[4];

	if (g_Vars.currentplayer->isdead == false) {
		bheadAdjustAnimation(arg0);

		if (arg0 != 0) {
			sp244 = arg1 / arg0;
		} else if (arg1 == 0) {
			arg0 = 0;
		}
	} else {
		if (g_Vars.currentplayer->startnewbonddie) {
			bheadStartDeathAnimation(g_DeathAnimations[random() % g_NumDeathAnimations], random() % 2, 0, 1);
			g_Vars.currentplayer->startnewbonddie = false;
		}

		bheadSetSpeed(0.5);
		arg2 = 0;
	}

	bheadUpdate(sp244, arg2);
	mtx4LoadXRotation(BADDEG2RAD(360 - g_Vars.currentplayer->vv_verta360), &sp180);

	if (optionsGetHeadRoll(g_Vars.currentplayerstats->mpindex)) {
		mtx00016d58(&sp116,
				0, 0, 0,
				-g_Vars.currentplayer->headlook.x, -g_Vars.currentplayer->headlook.y, -g_Vars.currentplayer->headlook.z,
				g_Vars.currentplayer->headup.x, g_Vars.currentplayer->headup.y, g_Vars.currentplayer->headup.z);
		mtx4MultMtx4InPlace(&sp116, &sp180);
	}

	mtx4LoadYRotation(BADDEG2RAD(360 - g_Vars.currentplayer->vv_theta), &sp116);
	mtx4MultMtx4InPlace(&sp116, &sp180);

	if (arg3) {
		quaternion0f097044(&sp180, sp100);
		quaternion0f097044(arg3, sp84);
		quaternion0f0976c0(sp100, sp84);
		quaternionSlerp(sp100, sp84, arg4, sp68);
		quaternionToMtx(sp68, &sp180);
	}

	g_Vars.currentplayer->bond2.unk1c.x = sp180.m[2][0];
	g_Vars.currentplayer->bond2.unk1c.y = sp180.m[2][1];
	g_Vars.currentplayer->bond2.unk1c.z = sp180.m[2][2];
	g_Vars.currentplayer->bond2.unk28.x = sp180.m[1][0];
	g_Vars.currentplayer->bond2.unk28.y = sp180.m[1][1];
	g_Vars.currentplayer->bond2.unk28.z = sp180.m[1][2];
}

void bmove0f0cc654(f32 arg0, f32 arg1, f32 arg2)
{
	bmoveUpdateHead(arg0, arg1, arg2, NULL, 0);
}

s32 bmoveGetCrouchPos(void)
{
	return (g_Vars.currentplayer->crouchpos < g_Vars.currentplayer->autocrouchpos)
		? g_Vars.currentplayer->crouchpos
		: g_Vars.currentplayer->autocrouchpos;
}

s32 bmoveGetCrouchPosByPlayer(s32 playernum)
{
	return (g_Vars.players[playernum]->crouchpos < g_Vars.players[playernum]->autocrouchpos)
		? g_Vars.players[playernum]->crouchpos
		: g_Vars.players[playernum]->autocrouchpos;
}
