#include <ultra64.h>
#include "constants.h"
#include "game/bondeyespy.h"
#include "game/chraction.h"
#include "game/chr.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/hudmsg.h"
#include "game/inv.h"
#include "game/lv.h"
#include "game/mplayer/ingame.h"
#include "game/playermgr.h"
#include "game/stagetable.h"
#include "game/lang.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "input.h"
#endif

u8 g_EyespyPickup = false;
u8 g_EyespyHit = EYESPYHIT_NONE;
u8 g_EyespyPrevHit = EYESPYHIT_NONE;
u8 var80070ecc = 0;
f32 g_EyespyMaxHeight = 160;
f32 g_EyespyMinHeight = 80;
u32 g_EyespyFallAccel = 100;
u32 g_EyespyMaxFallSpeed = 3000;

/**
 * Determines the eyespy's ground Y value by doing a collision check for a
 * position 50 units above the current position, or less if the eyespy is near
 * the top of its height range.
 */
f32 eyespyFindGround(RoomNum *floorroom)
{
	struct prop *prop = g_Vars.currentplayer->eyespy->prop;
	s32 inlift;
	struct prop *lift;
	struct coord pos;
	f32 yoffset = 50;
	f32 ground;

	if (g_EyespyMaxHeight - g_Vars.currentplayer->eyespy->height < 50) {
		yoffset = g_EyespyMaxHeight - g_Vars.currentplayer->eyespy->height;
	}

	if (yoffset < 0) {
		yoffset = 0;
	}

	pos.x = prop->pos.x;
	pos.y = prop->pos.y + yoffset;
	pos.z = prop->pos.z;

	ground = cdFindGroundInfoAtCyl(&pos, 26, prop->rooms, NULL, NULL, NULL, floorroom, &inlift, &lift);

	if (ground < -30000) {
		ground = -30000;
	}

	return ground;
}

s32 eyespyTryMoveUpwards(f32 yvel)
{
	s32 result;
	struct prop *prop = g_Vars.currentplayer->eyespy->prop;
	struct chrdata *chr = prop->chr;
	struct coord dstpos;
	RoomNum dstrooms[8];
	s32 types;
	u32 stack;
	f32 f0;
	u32 stack2;

	dstpos.x = prop->pos.x;
	dstpos.y = prop->pos.y + yvel;
	dstpos.z = prop->pos.z;

	types = g_Vars.bondcollisions ? CDTYPE_ALL : CDTYPE_BG;

	// Allow eyespy to go up steps 30cm or less
	if (chr->manground + 30 >= g_Vars.currentplayer->eyespy->oldground) {
		f0 = chr->manground - prop->pos.y + 30;
	} else {
		f0 = g_Vars.currentplayer->eyespy->oldground - prop->pos.y;
	}

	func0f065e74(&prop->pos, prop->rooms, &dstpos, dstrooms);
	chr0f021fa8(prop->chr, &dstpos, dstrooms);
	propSetPerimEnabled(prop, false);

	f0 -= 0.1f;

	result = cdTestVolume(&dstpos, 26, dstrooms, types, CHECKVERTICAL_YES, 15, f0);
	propSetPerimEnabled(prop, true);

	if (result == CDRESULT_NOCOLLISION) {
		prop->pos.y = dstpos.y;
		propDeregisterRooms(prop);
		roomsCopy(dstrooms, prop->rooms);
	}

	return result;
}

s32 eyespyCalculateNewPosition(struct coord *vel)
{
	bool result = true;
	struct prop *eyespyprop = g_Vars.currentplayer->eyespy->prop;
	struct chrdata *chr = eyespyprop->chr;
	struct coord dstpos;
	RoomNum dstrooms[8];
	RoomNum floorroom;
	RoomNum sp74[24];
	s32 types;
	s32 i;
	f32 ymin;
	f32 xdiff;
	f32 zdiff;
	u32 stack;
	f32 radius;
	f32 halfradius;
	struct prop *prop;
	s32 playernum;
	u32 stack2;

	eyespyFindGround(&floorroom);

	if (vel->x || vel->y || vel->z) {
		propSetPerimEnabled(eyespyprop, false);

		dstpos.x = vel->x + eyespyprop->pos.x;
		dstpos.y = vel->y + eyespyprop->pos.y;
		dstpos.z = vel->z + eyespyprop->pos.z;

		types = g_Vars.bondcollisions ? CDTYPE_ALL : CDTYPE_BG;

		// Allow eyespy to go up steps 30cm or less
		if (g_Vars.currentplayer->eyespy->oldground <= chr->manground + 30) {
			ymin = chr->manground - eyespyprop->pos.y + 30;
		} else {
			ymin = g_Vars.currentplayer->eyespy->oldground - eyespyprop->pos.y;
		}

		// This must be populating dstrooms at least
		func0f065dfc(&eyespyprop->pos, eyespyprop->rooms, &dstpos, dstrooms, sp74, 20);

		// Check if dstrooms contains the eyespy's old room.
		// If so, simplify dstrooms so it only contains that room.
		for (i = 0; dstrooms[i] != -1; i++) {
			if (dstrooms[i] == floorroom) {
				dstrooms[0] = floorroom;
				dstrooms[1] = -1;
				break;
			}
		}

		chr0f021fa8(eyespyprop->chr, &dstpos, dstrooms);

		// Check if the eyespy is moving 13cm or more along either the X or Z
		// axis in a single frame. If less, only do a collision check for the
		// dst position. If more, do a halfway check too?
		xdiff = dstpos.x - eyespyprop->pos.x;
		zdiff = dstpos.z - eyespyprop->pos.z;

		radius = 26;
		halfradius = radius * 0.5f;

		if (xdiff > halfradius || zdiff > halfradius || xdiff < -halfradius || zdiff < -halfradius) {
			result = cdExamCylMove06(&eyespyprop->pos, eyespyprop->rooms, &dstpos, dstrooms, radius, types, 1, 15, ymin);

			if (result == CDRESULT_NOCOLLISION) {
				result = cdExamCylMove02(&eyespyprop->pos, &dstpos, radius, dstrooms, types, true, 15, ymin);
			}
		} else {
			result = cdExamCylMove02(&eyespyprop->pos, &dstpos, radius, sp74, types, true, 15, ymin);
		}

		if (result == CDRESULT_COLLISION) {
			prop = cdGetObstacleProp();

			if (prop && prop->type == PROPTYPE_PLAYER) {
				playernum = g_Vars.currentplayernum;

				if (playernum == playermgrGetPlayerNumByProp(prop)) {
					g_EyespyPickup = true;
				}
			}
		}

		propSetPerimEnabled(eyespyprop, true);

		if (result == CDRESULT_NOCOLLISION) {
			// Apply the destination
			eyespyprop->pos.x = dstpos.x;
			eyespyprop->pos.y = dstpos.y;
			eyespyprop->pos.z = dstpos.z;

			propDeregisterRooms(eyespyprop);

			roomsCopy(dstrooms, eyespyprop->rooms);
		}
	}

	return result;
}

bool eyespyCalculateNewPositionWithPush(struct coord *vel)
{
	s32 result = eyespyCalculateNewPosition(vel);
	struct prop *prop;

	if (result != CDRESULT_NOCOLLISION) {
		g_EyespyHit = EYESPYHIT_BG;

		prop = cdGetObstacleProp();

		if (prop && g_Vars.lvupdate240 > 0) {
			if (prop->type == PROPTYPE_DOOR) {
				struct doorobj *door = prop->door;

				g_EyespyHit = EYESPYHIT_DOOR;

				if (door->doorflags & DOORFLAG_DAMAGEONCONTACT) {
					f32 sp38[3];
					struct coord sp2c;
					struct coord sp20;

					cdGetEdge(&sp2c, &sp20, 286, "bondeyespy.c");

					// Nothing is actually done with these coordinates...
					// This code was likely copied from bondwalk then the bounce
					// feature removed
					sp38[0] = sp20.z - sp2c.z;
					sp38[1] = 0;
					sp38[2] = sp2c.x - sp20.x;

					if (sp38[0] || sp38[2]) {
						guNormalize(&sp38[0], &sp38[1], &sp38[2]);
					} else {
						sp38[2] = 1;
					}

					if (prop);

					g_EyespyHit = EYESPYHIT_DAMAGE;
				}
			}

			if (prop->type == PROPTYPE_CHR) {
				g_EyespyHit = EYESPYHIT_CHR;
			} else if (prop->type == PROPTYPE_PLAYER) {
				g_EyespyHit = EYESPYHIT_CHR;
			} else if (prop->type == PROPTYPE_OBJ) {
				g_EyespyHit = EYESPYHIT_OBJ;
			}
		}
	}

	return result;
}

s32 eyespy0f0cf890(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4)
{
	if (cd00024ea4()) {
		struct coord sp24;
		s32 someint;
		f32 somefloat = cd00024e98();
		sp24.x = arg0->x * somefloat * 0.25f;
		sp24.y = arg0->y * somefloat * 0.25f;
		sp24.z = arg0->z * somefloat * 0.25f;

		someint = eyespyCalculateNewPositionWithPush(&sp24);

		if (someint == 1) {
			return 1;
		}

		if (someint == 0) {
			cdGetEdge(arg3, arg4, 350, "bondeyespy.c");

			if (arg3->f[0] != arg1->f[0]
					|| arg3->f[1] != arg1->f[1]
					|| arg3->f[2] != arg1->f[2]
					|| arg4->f[0] != arg2->f[0]
					|| arg4->f[1] != arg2->f[1]
					|| arg4->f[2] != arg2->f[2]) {
				return 0;
			}
		}
	}

	return -1;
}

s32 eyespy0f0cf9f8(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	f32 tmp;
	struct coord sp30;
	struct coord sp24;
	f32 dist;

	if (arg1->f[0] != arg2->f[0] || arg1->f[2] != arg2->f[2]) {
		sp30.x = arg2->x - arg1->x;
		sp30.y = 0;
		sp30.z = arg2->z - arg1->z;

		dist = sqrtf(sp30.f[0] * sp30.f[0] + sp30.f[2] * sp30.f[2]);

		sp30.x *= 1.0f / dist;
		sp30.z *= 1.0f / dist;

		tmp = arg0->f[0] * sp30.f[0] + arg0->f[2] * sp30.f[2];

		sp24.x = sp30.x * tmp;
		sp24.y = 0;
		sp24.z = sp30.z * tmp;

		return eyespyCalculateNewPositionWithPush(&sp24);
	}

	return -1;
}

s32 eyespy0f0cfafc(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	struct coord sp34;
	struct coord sp28;
	f32 width = 26;
	struct prop *prop = g_Vars.currentplayer->eyespy->prop;
	f32 tmp;

	sp34.x = arg1->x - (prop->pos.x + arg0->f[0]);
	sp34.z = arg1->z - (prop->pos.z + arg0->f[2]);

	if (sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2] <= width * width) {
		if (arg1->f[0] != prop->pos.f[0] || arg1->f[2] != prop->pos.f[2]) {
			sp34.x = -(arg1->z - prop->pos.z);
			sp34.y = 0;
			sp34.z = arg1->x - prop->pos.x;

			tmp = sqrtf(sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2]);

			sp34.x = sp34.f[0] * (1.0f / tmp);
			sp34.z = sp34.f[2] * (1.0f / tmp);

			tmp = arg0->f[0] * sp34.f[0] + arg0->f[2] * sp34.f[2];

			sp34.x = sp34.x * tmp;
			sp34.z = sp34.z * tmp;

			sp28.x = sp34.x;
			sp28.y = 0;
			sp28.z = sp34.z;

			if (eyespyCalculateNewPositionWithPush(&sp28) == 1) {
				return true;
			}
		}
	} else {
		sp34.x = arg2->x - (prop->pos.x + arg0->f[0]);
		sp34.z = arg2->z - (prop->pos.z + arg0->f[2]);

		if (sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2] <= width * width) {
			if (arg2->f[0] != prop->pos.f[0] || arg2->f[2] != prop->pos.f[2]) {
				sp34.x = -(arg2->z - prop->pos.z);
				sp34.y = 0;
				sp34.z = arg2->x - prop->pos.x;

				tmp = sqrtf(sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2]);

				sp34.x = sp34.f[0] * (1.0f / tmp);
				sp34.z = sp34.f[2] * (1.0f / tmp);

				tmp = arg0->f[0] * sp34.f[0] + arg0->f[2] * sp34.f[2];

				sp34.x = sp34.x * tmp;
				sp34.z = sp34.z * tmp;

				sp28.x = sp34.x;
				sp28.y = 0;
				sp28.z = sp34.z;

				if (eyespyCalculateNewPositionWithPush(&sp28) == 1) {
					return true;
				}
			}
		}
	}

	return false;
}

s32 eyespy0f0cfdd0(struct coord *vel, struct coord *arg1, struct coord *arg2)
{
	bool result = eyespyCalculateNewPositionWithPush(vel);

	if (result != CDRESULT_NOCOLLISION) {
		cdGetEdge(arg1, arg2, 473, "bondeyespy.c");
	}

	return result;
}

void eyespyUpdateVertical(void)
{
	struct coord spac;
	struct coord spa0;
	struct prop *prop = g_Vars.currentplayer->eyespy->prop;
	struct coord dist;
	f32 newground;
	struct chrdata *chr = prop->chr;
	struct coord origpos;
	f32 accel;
	f32 maxfallspeed;
	u8 hit = EYESPYHIT_NONE;
	f32 newy;
	struct coord sp60;
	struct coord sp54;
	u32 stack;
	struct coord sp44;
	struct coord sp38;

	origpos.f[0] = prop->pos.x;
	origpos.f[1] = prop->pos.y;
	origpos.f[2] = prop->pos.z;

	dist.x = g_Vars.currentplayer->eyespy->vel.x;
	dist.y = 0;
	dist.z = g_Vars.currentplayer->eyespy->vel.z;

	if (eyespy0f0cfdd0(&dist, &spac, &spa0) == CDRESULT_COLLISION) {
		if (eyespy0f0cf890(&dist, &spac, &spa0, &sp60, &sp54)) {
			if (eyespy0f0cf9f8(&dist, &spac, &spa0) <= 0) {
				eyespy0f0cfafc(&dist, &spac, &spa0);
			}
		} else {
			eyespy0f0cf890(&dist, &sp60, &sp54, &sp44, &sp38);

			if (eyespy0f0cf9f8(&dist, &sp60, &sp54) <= 0
					&& eyespy0f0cf9f8(&dist, &spac, &spa0) <= 0
					&& eyespy0f0cfafc(&dist, &sp60, &sp54) <= 0) {
				eyespy0f0cfafc(&dist, &spac, &spa0);
			}
		}
	}

	// Handle gravity
	mainOverrideVariable("g", &g_EyespyFallAccel);
	mainOverrideVariable("l", &g_EyespyMaxFallSpeed);

	accel = g_EyespyFallAccel * 0.01; // always 1
	maxfallspeed = g_EyespyMaxFallSpeed * 0.01f; // always 30

	newy = chr->manground;
	newground = eyespyFindGround(NULL);

	if (newground != chr->manground) {
		if (newground < chr->manground) {
			// Descending a slope or ledge
			if (g_Vars.currentplayer->eyespy->gravity >= maxfallspeed) {
				g_Vars.currentplayer->eyespy->gravity = maxfallspeed;
			} else {
				g_Vars.currentplayer->eyespy->gravity += accel;
			}

			newy -= g_Vars.currentplayer->eyespy->gravity;

			if (newy < newground) {
				g_Vars.currentplayer->eyespy->gravity = 0;
				newy = newground;
			}
		} else if (newground > chr->manground) {
			// Ascending a slope or steps
			if (g_Vars.currentplayer->eyespy->gravity >= maxfallspeed) {
				g_Vars.currentplayer->eyespy->gravity = maxfallspeed;
			} else {
				g_Vars.currentplayer->eyespy->gravity += accel;
			}

			newy += g_Vars.currentplayer->eyespy->gravity;

			if (newy > newground) {
				g_Vars.currentplayer->eyespy->gravity = 0;
				newy = newground;
			}
		}

		if (chr->manground + g_Vars.currentplayer->eyespy->height < newground + g_EyespyMinHeight) {
			newy = (newground + g_EyespyMinHeight) - g_Vars.currentplayer->eyespy->height;
		}
	}

	if (eyespyTryMoveUpwards(newy - chr->manground)) {
		chr->manground = newy;
	} else {
		g_Vars.currentplayer->eyespy->gravity = 0;
	}

	g_Vars.currentplayer->eyespy->oldground = newground;
	chr->ground = chr->manground;
	chr->sumground = chr->manground * (PAL ? 8.4175090789795f : 9.999998f);

	// Handle rebound if hitting the min or max height
	if (g_Vars.currentplayer->eyespy->vel.y != 0) {
		f32 newheight = g_Vars.currentplayer->eyespy->vel.y + g_Vars.currentplayer->eyespy->height;
		bool rebound = false;

		if (newheight < g_EyespyMinHeight) {
			newheight = g_EyespyMinHeight;
			rebound = true;
		} else if (newheight > g_EyespyMaxHeight) {
			newheight = g_EyespyMaxHeight;
			rebound = true;
		}

		if (eyespyTryMoveUpwards(newheight - g_Vars.currentplayer->eyespy->height)) {
			g_Vars.currentplayer->eyespy->height = newheight;
		} else {
			rebound = true;
			hit = EYESPYHIT_BG;
		}

		if (rebound) {
			g_Vars.currentplayer->eyespy->vel.y = -g_Vars.currentplayer->eyespy->vel.y * 0.25f;
		}
	}

	if (g_EyespyHit == EYESPYHIT_NONE) {
		g_EyespyHit = hit;
	}

	chr0f0220ac(chr);

	dist.x = prop->pos.x - origpos.x;
	dist.y = prop->pos.y - origpos.y;
	dist.z = prop->pos.z - origpos.z;

	g_Vars.currentplayer->eyespy->speed =
		dist.f[0] * dist.f[0] +
		dist.f[1] * dist.f[1] +
		dist.f[2] * dist.f[2];
}

bool eyespyTryLaunch(void)
{
	struct coord playerpos;
	struct coord testfrompos;
	struct chrdata *chr = g_Vars.currentplayer->eyespy->prop->chr;
	bool launched;
	bool insafe;
	char text[48];

	insafe = stageGetIndex(g_Vars.stagenum) == STAGEINDEX_G5BUILDING
		&& g_Vars.currentplayer->prop->rooms[0] == 0x53;

	playerpos.x = g_Vars.currentplayer->prop->pos.x;
	playerpos.y = g_Vars.currentplayer->prop->pos.y;
	playerpos.z = g_Vars.currentplayer->prop->pos.z;

	chr->soundtimer = TICKS(10);

	g_Vars.currentplayer->eyespy->held = false;
	g_Vars.currentplayer->eyespy->up.x = 0;
	g_Vars.currentplayer->eyespy->up.y = 1;
	g_Vars.currentplayer->eyespy->up.z = 0;
	g_Vars.currentplayer->eyespy->startuptimer60 = 0;

	if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_G5BUILDING
			|| stageGetIndex(g_Vars.stagenum) == STAGEINDEX_CITRAINING) {
		g_EyespyMinHeight = 30;
	} else {
		g_EyespyMinHeight = 80;
	}

	g_Vars.currentplayer->eyespy->oldground = g_Vars.currentplayer->vv_ground;
	chr->ground = g_Vars.currentplayer->eyespy->oldground;
	chr->manground = g_Vars.currentplayer->eyespy->oldground;

	g_Vars.currentplayer->eyespy->height = g_EyespyMinHeight;
	g_Vars.currentplayer->eyespy->theta = g_Vars.currentplayer->vv_theta + 180;

	if (g_Vars.currentplayer->eyespy->theta >= 360) {
		g_Vars.currentplayer->eyespy->theta -= 360;
	}

	g_Vars.currentplayer->eyespy->costheta = cosf(g_Vars.currentplayer->eyespy->theta * 0.017453292384744f);
	g_Vars.currentplayer->eyespy->sintheta = sinf(g_Vars.currentplayer->eyespy->theta * 0.017453292384744f);

	chr->prevpos.x = g_Vars.currentplayer->eyespy->prop->pos.x = playerpos.f[0] + g_Vars.currentplayer->eyespy->sintheta * 100;
	chr->prevpos.y = g_Vars.currentplayer->eyespy->prop->pos.y = g_Vars.currentplayer->eyespy->oldground + g_Vars.currentplayer->eyespy->height;
	chr->prevpos.z = g_Vars.currentplayer->eyespy->prop->pos.z = playerpos.f[2] + g_Vars.currentplayer->eyespy->costheta * -100;

	chr->fallspeed.x = 0;
	chr->fallspeed.y = 0;
	chr->fallspeed.z = 0;

	testfrompos.x = playerpos.x;
	testfrompos.y = g_Vars.currentplayer->eyespy->oldground + g_Vars.currentplayer->eyespy->height;
	testfrompos.z = playerpos.z;

	g_Vars.currentplayer->eyespy->look.x = g_Vars.currentplayer->eyespy->sintheta;
	g_Vars.currentplayer->eyespy->look.y = 0;
	g_Vars.currentplayer->eyespy->look.z = -g_Vars.currentplayer->eyespy->costheta;
	g_Vars.currentplayer->eyespy->verta = 0;
	g_Vars.currentplayer->eyespy->cosverta = 1;
	g_Vars.currentplayer->eyespy->sinverta = 0;
	g_Vars.currentplayer->eyespy->vel.x = 0;
	g_Vars.currentplayer->eyespy->vel.y = 0;
	g_Vars.currentplayer->eyespy->vel.z = 0;
	g_Vars.currentplayer->eyespy->bobdir = 1;
	g_Vars.currentplayer->eyespy->bobtimer = 0;
	g_Vars.currentplayer->eyespy->bobactive = true;
	g_Vars.currentplayer->eyespy->speed = 0;
	g_Vars.currentplayer->eyespy->gravity = 0;
	g_Vars.currentplayer->eyespy->active = true;
	g_Vars.currentplayer->eyespy->camerabuttonheld = false;
	g_Vars.currentplayer->eyespy->buttonheld = g_Vars.currentplayer->eyespy->camerabuttonheld;
	g_Vars.currentplayer->eyespy->camerashuttertime = 0;
	g_Vars.currentplayer->eyespy->opendoor = false;
	g_Vars.currentplayer->eyespy->velf[0] = 0;
	g_Vars.currentplayer->eyespy->velf[1] = 0;
	g_Vars.currentplayer->eyespy->vels[0] = 0;
	g_Vars.currentplayer->eyespy->vels[1] = 0;
	g_Vars.currentplayer->eyespy->pitch = 0;

	playerSetPerimEnabled(g_Vars.currentplayer->prop, false);

	if (insafe || !cdExamLos08(&testfrompos, g_Vars.currentplayer->prop->rooms,
				&g_Vars.currentplayer->eyespy->prop->pos,
				CDTYPE_ALL,
				GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2 | GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT)) {
		// Launch failed due to not enough physical space, or we're in the G5 safe
		g_Vars.currentplayer->eyespy->deployed = false;

		chr->prevpos.x = g_Vars.currentplayer->eyespy->prop->pos.x = playerpos.f[0];
		chr->prevpos.y = g_Vars.currentplayer->eyespy->prop->pos.y = g_Vars.currentplayer->eyespy->oldground + g_Vars.currentplayer->eyespy->height;
		chr->prevpos.z = g_Vars.currentplayer->eyespy->prop->pos.z = playerpos.f[2];

		propSetPerimEnabled(g_Vars.currentplayer->eyespy->prop, false);

		// "Not enough room to launch "
		sprintf(text, "%s%s", langGet(L_MISC_218), bgunGetName(WEAPON_EYESPY));
		hudmsgCreate(text, HUDMSGTYPE_DEFAULT);
		launched = false;
	} else {
		// Launch successful
		g_Vars.currentplayer->eyespy->deployed = true;

		sndStart(var80095200, SFX_DETONATE, 0, -1, -1, -1, -1, -1);

		launched = true;

		chr->chrflags &= ~CHRCFLAG_HIDDEN;
		chr->chrflags &= ~CHRCFLAG_INVINCIBLE;

#if VERSION >= VERSION_NTSC_1_0
		psCreate(NULL, g_Vars.currentplayer->eyespy->prop, SFX_EYESPY_RUNNING, -1,
				-1, PSFLAG_REPEATING, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
#else
		psCreate(NULL, g_Vars.currentplayer->eyespy->prop, SFX_EYESPY_RUNNING, -1,
				-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
#endif
	}

	playerSetPerimEnabled(g_Vars.currentplayer->prop, true);
	propDeregisterRooms(g_Vars.currentplayer->eyespy->prop);
	func0f065e74(&g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms,
			&g_Vars.currentplayer->eyespy->prop->pos, g_Vars.currentplayer->eyespy->prop->rooms);

	chr0f0220ac(chr);

	return launched;
}

void eyespyProcessInput(bool allowbuttons)
{
	struct chrdata *chr = g_Vars.currentplayer->eyespy->prop->chr;
	f32 spe0 = PAL ? 0.952f : 0.96f;
	f32 f;
	s32 i;
	f32 spd4;
	f32 spd0;
	f32 spcc;
	f32 spc8;
	f32 spc4;
	s8 contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
	s8 c1stickx = joyGetStickX(contpad1);
	s8 c2stickx;
	s8 c1sticky = joyGetStickY(contpad1);
	s8 c2sticky;
	u32 c1buttons = allowbuttons ? joyGetButtons(contpad1, 0xffffffff) : 0;
	u32 c1buttonsthisframe = allowbuttons ? joyGetButtonsPressedThisFrame(contpad1, 0xffffffff): 0;
	u32 c2buttons;
	u32 c2buttonsthisframe;
	bool domovecentre = true;
	s32 controlmode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);

	bool aimpressed;
	bool shootpressed;
	bool exitpressed;
	bool activatepressed;
	f32 forwardspeed = 0.0f;
	f32 pitchspeed = 0.0f;
	f32 sidespeed = 0.0f;
	f32 angle;
	f32 ascendspeed = 0.0f;
	f32 prevverta = g_Vars.currentplayer->eyespy->verta;
	RoomNum prevrooms[8];
	s32 contpad2;
	u32 stack[5];
	f32 tmp;
	u32 umask, dmask, lmask, rmask;

	bool pausepressed;
	if (controlmode == CONTROLMODE_PC) {
		umask = U_CBUTTONS;
		dmask = D_CBUTTONS;
		lmask = L_CBUTTONS;
		rmask = R_CBUTTONS;
	} else {
		umask = U_JPAD | U_CBUTTONS;
		dmask = D_JPAD | D_CBUTTONS;
		lmask = L_JPAD | L_CBUTTONS;
		rmask = R_JPAD | R_CBUTTONS;
	}

	if (controlmode >= CONTROLMODE_21 && controlmode < CONTROLMODE_PC) {
		contpad2 = (s8) optionsGetContpadNum2(g_Vars.currentplayerstats->mpindex);
		c2stickx = joyGetStickX(contpad2);
		c2sticky = joyGetStickY(contpad2);

		c2buttons = allowbuttons ? joyGetButtons(contpad2, 0xffffffff) : 0;
		c2buttonsthisframe = allowbuttons ? joyGetButtonsPressedThisFrame(contpad2, 0xffffffff) : 0;
	} else {
#ifndef PLATFORM_N64
		if (controlmode == CONTROLMODE_PC) {
			c2stickx = joyGetRStickX(contpad1);
			c2sticky = joyGetRStickY(contpad1);
		} else
#else
		{
			c2stickx = c1stickx;
			c2sticky = c1sticky;
		}
#endif
		c2buttons = c1buttons;
		c2buttonsthisframe = c1buttonsthisframe;
	}

	pausepressed = c1buttonsthisframe & START_BUTTON;
	if (controlmode == CONTROLMODE_13 || controlmode == CONTROLMODE_14) {
		aimpressed = c1buttons & Z_TRIG;
		shootpressed = c1buttons & A_BUTTON;
		exitpressed = c1buttonsthisframe & R_TRIG;
		activatepressed = c1buttonsthisframe & B_BUTTON;
	} else if (controlmode <= CONTROLMODE_14 || controlmode == CONTROLMODE_PC) {
		aimpressed = c1buttons & (R_TRIG);
		shootpressed = c1buttons & Z_TRIG;
#ifndef PLATFORM_N64
		if (controlmode == CONTROLMODE_PC) {
			exitpressed = c1buttonsthisframe & (BUTTON_WPNBACK | BUTTON_RADIAL);
			activatepressed = c1buttonsthisframe & (BUTTON_CANCEL_USE | BUTTON_ACCEPT_USE);
		} else
#endif
		{
			exitpressed = (c1buttonsthisframe | c2buttonsthisframe) & A_BUTTON;
			activatepressed = (c1buttonsthisframe | c2buttonsthisframe) & B_BUTTON;
		}
	} else {
		if (controlmode >= CONTROLMODE_23) {
			aimpressed = c1buttons & Z_TRIG;
			shootpressed = c2buttons & Z_TRIG;
		} else {
			aimpressed = c2buttons & Z_TRIG;
			shootpressed = c1buttons & Z_TRIG;
		}

		exitpressed = (c1buttonsthisframe | c2buttonsthisframe) & A_BUTTON;
		activatepressed = (c1buttonsthisframe | c2buttonsthisframe) & B_BUTTON;
	}

	// Apply safe zone for c1stickx
	if (c1stickx > -10 && c1stickx < 10) {
		c1stickx = 0;
	}

	if (c1stickx <= -10) {
		c1stickx += 10;
	}

	if (c1stickx >= 10) {
		c1stickx -= 10;
	}

	// Apply safe zone for c2stickx
	if (c2stickx > -10 && c2stickx < 10) {
		c2stickx = 0;
	}

	if (c2stickx <= -10) {
		c2stickx += 10;
	}

	if (c2stickx >= 10) {
		c2stickx -= 10;
	}

	// Apply safe zone for c1sticky
	if (c1sticky > -20 && c1sticky < 20) {
		c1sticky = 0;
	}

	if (c1sticky <= -20) {
		c1sticky += 20;
	}

	if (c1sticky >= 20) {
		c1sticky -= 20;
	}

	// Apply safe zone for c2sticky
	if (c2sticky > -20 && c2sticky < 20) {
		c2sticky = 0;
	}

	if (c2sticky <= -20) {
		c2sticky += 20;
	}

	if (c2sticky >= 20) {
		c2sticky -= 20;
	}

	c1sticky *= 0.3f;
	c2sticky *= 0.3f;

	if (controlmode == CONTROLMODE_11 || controlmode == CONTROLMODE_13) {
		if (aimpressed) {
			domovecentre = false;
			pitchspeed = c1sticky;
		} else {
			forwardspeed = c1sticky;
		}

		ascendspeed = (c1buttons & (U_CBUTTONS | U_JPAD) ? 1 : 0) - (c1buttons & (D_CBUTTONS | D_JPAD) ? 1 : 0);
		sidespeed = (c1buttons & (R_CBUTTONS | R_JPAD) ? 1 : 0) - (c1buttons & (L_CBUTTONS | L_JPAD) ? 1 : 0);
	} else if (controlmode <= CONTROLMODE_14 || controlmode == CONTROLMODE_PC) {
		if (aimpressed) {
			domovecentre = false;
			pitchspeed = c1sticky;
		} else {
			ascendspeed = c1sticky * 0.25f;
			forwardspeed = (c1buttons & umask ? 24.0f : 0) - (c1buttons & dmask ? 24.0f : 0);
#ifndef PLATFORM_N64
			if (controlmode == CONTROLMODE_PC) {
				forwardspeed += c2sticky;
			}
#endif
		}

		sidespeed = (c1buttons & rmask ? 1 : 0) - (c1buttons & lmask ? 1 : 0);
#ifndef PLATFORM_N64
		if (!sidespeed && controlmode == CONTROLMODE_PC) sidespeed = c2stickx * 0.0125f;
#endif
	} else if (controlmode == CONTROLMODE_21 || controlmode == CONTROLMODE_23) {
		forwardspeed = c1sticky;

		if (aimpressed) {
			pitchspeed = c2sticky;
		} else {
			ascendspeed = c2sticky * 0.25f;
		}

		sidespeed = c2stickx * 0.0125f;
	} else {
		if (aimpressed) {
			pitchspeed = c1sticky;
		} else {
			ascendspeed = c1sticky * 0.25f;
		}

		forwardspeed = c2sticky;
		sidespeed = c2stickx * 0.0125f;
	}

	g_EyespyPickup = false;

#if VERSION >= VERSION_PAL_BETA
	for (f = 1; f < g_Vars.lvupdate60; f++) {
		spe0 *= PAL ? 0.952f : 0.96f;
	}
#else
	for (f = 1; f < g_Vars.lvupdate60freal; f++) {
		spe0 *= 0.96f;
	}
#endif

	if (g_Vars.currentplayer->eyespy->startuptimer60 < TICKS(50)) {
		g_Vars.currentplayer->eyespy->startuptimer60 += g_Vars.lvupdate60;
	} else {
		g_Vars.currentplayer->eyespy->startuptimer60 = TICKS(50);
	}

	chr->prevpos.x = g_Vars.currentplayer->eyespy->prop->pos.x;
	chr->prevpos.y = g_Vars.currentplayer->eyespy->prop->pos.y;
	chr->prevpos.z = g_Vars.currentplayer->eyespy->prop->pos.z;

	roomsCopy(g_Vars.currentplayer->eyespy->prop->rooms, prevrooms);

	if (!invHasSingleWeaponIncAllGuns(WEAPON_EYESPY)) {
		g_Vars.currentplayer->eyespy->deployed = false;
		g_Vars.currentplayer->eyespy->held = true;
		g_Vars.currentplayer->eyespy->active = false;

		chr->chrflags |= CHRCFLAG_HIDDEN;

		chrClearReferences(g_Vars.currentplayer->eyespy->prop - g_Vars.props);
	}

	if (g_Vars.currentplayer->eyespy->active && g_PlayersWithControl[g_Vars.currentplayernum]) {
		g_Vars.currentplayer->joybutinhibit = 0xffffffff;

#if VERSION < VERSION_NTSC_1_0
		if (g_Vars.currentplayer->isdead == false
				&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED
				&& (c1buttons & START_BUTTON)) {
			if (!g_Vars.mplayerisrunning) {
				playerStartPause(MENUROOT_MAINMENU);
			} else {
				mpPushPauseDialog();
			}
		}
#endif

#ifndef PLATFORM_N64
		if (g_Vars.currentplayernum == 0) {
			f32 mdx, mdy;
			inputMouseGetScaledDelta(&mdx, &mdy);
			if (mdx || mdy) {
				if (g_Vars.currentplayerstats && !optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex)) {
					mdy = -mdy;
				}
				g_Vars.currentplayer->eyespy->theta += mdx * 1.5f;
				// hold aim to move up and down, release to look up and down
				if (aimpressed) {
					ascendspeed -= mdy;
				} else {
					g_Vars.currentplayer->eyespy->verta -= mdy * 1.5f;
				}
			}
		}
#endif

		// Update theta
		g_Vars.currentplayer->eyespy->theta += c1stickx * 0.0625f * g_Vars.lvupdate60freal;

		while (g_Vars.currentplayer->eyespy->theta < 0.0f) {
			g_Vars.currentplayer->eyespy->theta += 360.0f;
		}

		while (g_Vars.currentplayer->eyespy->theta >= 360.0f) {
			g_Vars.currentplayer->eyespy->theta -= 360.0f;
		}

		g_Vars.currentplayer->eyespy->costheta = cosf(g_Vars.currentplayer->eyespy->theta * 0.017453292384744f);
		g_Vars.currentplayer->eyespy->sintheta = sinf(g_Vars.currentplayer->eyespy->theta * 0.017453292384744f);

		// Update verta
#ifndef PLATFORM_N64
		// respect the invert pitch setting
		if (optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex)) {
			pitchspeed = -pitchspeed;
		}
#endif
		g_Vars.currentplayer->eyespy->verta -= pitchspeed * 0.0625f * g_Vars.lvupdate60freal;

		if (prevverta != g_Vars.currentplayer->eyespy->verta) {
#ifndef PLATFORM_N64 // limit eyespy range to 75 degrees
			while (g_Vars.currentplayer->eyespy->verta > 90.0f) {
				g_Vars.currentplayer->eyespy->verta -= 360.0f;
			}

			if (g_Vars.currentplayer->eyespy->verta < -75.0f) {
				g_Vars.currentplayer->eyespy->verta = -75.0f;
			}

			if (g_Vars.currentplayer->eyespy->verta > 75.0f) {
				g_Vars.currentplayer->eyespy->verta = 75.0f;
			}
#endif
			while (g_Vars.currentplayer->eyespy->verta < 0.0f) {
				g_Vars.currentplayer->eyespy->verta += 360.0f;
			}

			while (g_Vars.currentplayer->eyespy->verta >= 360.0f) {
				g_Vars.currentplayer->eyespy->verta -= 360.0f;
			}

			g_Vars.currentplayer->eyespy->cosverta = cosf(g_Vars.currentplayer->eyespy->verta * 0.017453292384744f);
			g_Vars.currentplayer->eyespy->sinverta = sinf(g_Vars.currentplayer->eyespy->verta * 0.017453292384744f);
		}

		spc4 = 0.0f;
		spc8 = 0.0f;
		spcc = 0.0f;
		spd0 = 0.0f;
		spd4 = 0.0f;

		// Make eyespy look horizontally
		if (domovecentre) {
			if (g_Vars.currentplayer->eyespy->verta > 0.0f && forwardspeed != 0) {
				if (controlmode != CONTROLMODE_PC) {
					if (g_Vars.currentplayer->eyespy->verta < 180.0f) {
						tmp = g_Vars.currentplayer->eyespy->verta;

						for (i = 0; i < g_Vars.lvupdate60; i++) {
							tmp *= 0.04f;
							g_Vars.currentplayer->eyespy->verta -= tmp;
						}
					} else {
						tmp = 360.0f - g_Vars.currentplayer->eyespy->verta;

						for (i = 0; i < g_Vars.lvupdate60; i++) {
							tmp *= 0.04f;
							g_Vars.currentplayer->eyespy->verta += tmp;
						}
					}
				}

				g_Vars.currentplayer->eyespy->cosverta = cosf(g_Vars.currentplayer->eyespy->verta * 0.017453292384744f);
				g_Vars.currentplayer->eyespy->sinverta = sinf(g_Vars.currentplayer->eyespy->verta * 0.017453292384744f);
			}

			spcc += forwardspeed * g_Vars.currentplayer->eyespy->sintheta * 0.15f * g_Vars.lvupdate60freal;
			spc8 += -forwardspeed * g_Vars.currentplayer->eyespy->costheta * 0.15f * g_Vars.lvupdate60freal;
		}

		if (sidespeed != 0) {
			spd4 += sidespeed * 5 * g_Vars.currentplayer->eyespy->costheta * 0.15f * g_Vars.lvupdate60freal;
			spd0 += sidespeed * 5 * g_Vars.currentplayer->eyespy->sintheta * 0.15f * g_Vars.lvupdate60freal;
		}

		if (ascendspeed != 0) {
			spc4 += ascendspeed * 3 * 0.15f * g_Vars.lvupdate60freal;

			g_Vars.currentplayer->eyespy->bobdir = (ascendspeed < 0.0f) ? -1 : 1;
			g_Vars.currentplayer->eyespy->bobtimer = 0;
			g_Vars.currentplayer->eyespy->bobactive = false;
		}

		g_Vars.currentplayer->eyespy->velf[0] += spcc;
		g_Vars.currentplayer->eyespy->velf[1] += spc8;
		g_Vars.currentplayer->eyespy->vels[0] += spd4;
		g_Vars.currentplayer->eyespy->vels[1] += spd0;

		spcc = g_Vars.currentplayer->eyespy->velf[0] * g_Vars.currentplayer->eyespy->velf[0]
			+ g_Vars.currentplayer->eyespy->velf[1] * g_Vars.currentplayer->eyespy->velf[1];

		if (spcc > 90.25f * g_Vars.lvupdate60freal * g_Vars.lvupdate60freal) {
			spcc = 9.5f * g_Vars.lvupdate60freal / sqrtf(spcc);

			g_Vars.currentplayer->eyespy->velf[0] *= spcc;
			g_Vars.currentplayer->eyespy->velf[1] *= spcc;
		}

		spd4 = g_Vars.currentplayer->eyespy->vels[0] * g_Vars.currentplayer->eyespy->vels[0]
			+ g_Vars.currentplayer->eyespy->vels[1] * g_Vars.currentplayer->eyespy->vels[1];

		if (spd4 > 225.0f * g_Vars.lvupdate60freal * g_Vars.lvupdate60freal) {
			spd4 = 15.0f * g_Vars.lvupdate60freal / sqrtf(spd4);

			g_Vars.currentplayer->eyespy->vels[0] *= spd4;
			g_Vars.currentplayer->eyespy->vels[1] *= spd4;
		}

		g_Vars.currentplayer->eyespy->vel.y += spc4;

		if (g_Vars.currentplayer->eyespy->vel.y < -(5 * g_Vars.lvupdate60freal)) {
			g_Vars.currentplayer->eyespy->vel.y = -(5 * g_Vars.lvupdate60freal);
		}

		if (g_Vars.currentplayer->eyespy->vel.y > 5 * g_Vars.lvupdate60freal) {
			g_Vars.currentplayer->eyespy->vel.y = 5 * g_Vars.lvupdate60freal;
		}
	} else {
		g_Vars.currentplayer->eyespy->bobactive = true;
	}

	// Update bob
	if (spc4 == 0.0f) {
		if (g_Vars.currentplayer->eyespy->bobactive || ABS(g_Vars.currentplayer->eyespy->vel.y) < 0.1f) {
			g_Vars.currentplayer->eyespy->bobactive = true;
			g_Vars.currentplayer->eyespy->bobtimer += g_Vars.lvupdate60;
#ifdef PLATFORM_N64
			g_Vars.currentplayer->eyespy->vel.y += 0.025f * g_Vars.currentplayer->eyespy->bobdir;
#else
			// HACK: how do I scale this properly?
			const f32 scale = (g_Vars.lvupdate60freal <= 1.1f) ? 0.0055f : 0.0125f;
			g_Vars.currentplayer->eyespy->vel.y += scale * g_Vars.lvupdate60freal * g_Vars.currentplayer->eyespy->bobdir;
#endif

			if (g_Vars.currentplayer->eyespy->bobtimer > TICKS(120)) {
				g_Vars.currentplayer->eyespy->bobtimer = 0;
				g_Vars.currentplayer->eyespy->bobdir = -g_Vars.currentplayer->eyespy->bobdir;
			}
		}
	}

	g_Vars.currentplayer->eyespy->vel.y *= spe0;

	g_Vars.currentplayer->eyespy->vels[0] *= spe0;
	g_Vars.currentplayer->eyespy->vels[1] *= spe0;
	g_Vars.currentplayer->eyespy->velf[0] *= spe0;
	g_Vars.currentplayer->eyespy->velf[1] *= spe0;

	g_Vars.currentplayer->eyespy->vel.x = g_Vars.currentplayer->eyespy->vels[0] + g_Vars.currentplayer->eyespy->velf[0];
	g_Vars.currentplayer->eyespy->vel.z = g_Vars.currentplayer->eyespy->vels[1] + g_Vars.currentplayer->eyespy->velf[1];

	g_Vars.currentplayer->eyespy->look.y = g_Vars.currentplayer->eyespy->sinverta;
	g_Vars.currentplayer->eyespy->look.x = g_Vars.currentplayer->eyespy->cosverta * g_Vars.currentplayer->eyespy->sintheta;
	g_Vars.currentplayer->eyespy->look.z = -g_Vars.currentplayer->eyespy->cosverta * g_Vars.currentplayer->eyespy->costheta;

	g_Vars.currentplayer->eyespy->up.y = g_Vars.currentplayer->eyespy->cosverta;
	g_Vars.currentplayer->eyespy->up.x = -g_Vars.currentplayer->eyespy->sinverta * g_Vars.currentplayer->eyespy->sintheta;
	g_Vars.currentplayer->eyespy->up.z = g_Vars.currentplayer->eyespy->sinverta * g_Vars.currentplayer->eyespy->costheta;

	g_EyespyPrevHit = g_EyespyHit;
	g_EyespyHit = EYESPYHIT_NONE;
	var80070ecc = 0;

	eyespyUpdateVertical();

	// Consider playing the tap sound when the eyespy is driven into a wall or object
	if (g_Vars.currentplayer->eyespy->active
			&& g_EyespyHit != g_EyespyPrevHit
			&& g_EyespyHit > 0
			&& chr->soundtimer > TICKS(10)) {
		chr->soundtimer = 0;

		switch (g_EyespyHit) {
		case EYESPYHIT_BG:
			snd00010718(NULL, 0, 16000, AL_PAN_CENTER, SFX_EYESPYHIT, 1, 1, -1, 1);
			break;
		case EYESPYHIT_OBJ:
			snd00010718(NULL, 0, 16000, AL_PAN_CENTER, SFX_EYESPYHIT, 1, 1, -1, 1);
			break;
		case EYESPYHIT_DOOR:
			snd00010718(NULL, 0, 16000, AL_PAN_CENTER, SFX_EYESPYHIT, 1, 1, -1, 1);
			break;
		case EYESPYHIT_CHR:
			snd00010718(NULL, 0, 16000, AL_PAN_CENTER, SFX_EYESPYHIT, 1, 1, -1, 1);
			break;
		case EYESPYHIT_DAMAGE:
			sndStart(var80095200, SFX_PICKUP_LASER, NULL, -1, -1, -1, -1, -1);
			break;
		default:
			break;
		}
	}

	// Apply speed and angle
	if (g_Vars.currentplayer->eyespy->speed > 1600.0f) {
		g_Vars.currentplayer->eyespy->speed = 1600.0f;
	}

	tmp = g_Vars.currentplayer->eyespy->speed / 1600.0f;
	psSetVolume(g_Vars.currentplayer->eyespy->prop, tmp * 400.0f);

	angle = 180 - g_Vars.currentplayer->eyespy->theta;

	if (angle < 0) {
		angle += 360.0f;
	}

	angle = angle / 360.0f * M_BADTAU;

	chrSetLookAngle(chr, angle);

	if (g_Vars.currentplayer->eyespy->startuptimer60 < TICKS(50)) {
		return;
	}

	// handle pause menu
	if (!g_Vars.currentplayer->pausemode) {
		if (g_Vars.currentplayer->eyespy->active && !g_Vars.currentplayer->isdead) {
			if (!( g_Vars.currentplayer->pausemode & (PAUSEMODE_PAUSED | PAUSEMODE_PAUSING) ) && (pausepressed)) {
				if (g_Vars.mplayerisrunning == false) {
					if (g_Vars.lvframenum > 15) {
						playerStartPause(MENUROOT_MAINMENU);
					}
				} else {
					mpPushPauseDialog();
				}
			}

		}
		// Handle trigger
		if (g_Vars.currentplayer->eyespy->camerashuttertime <= 0 && shootpressed && g_Vars.currentplayer->eyespy->active) {
			if (g_Vars.currentplayer->eyespy->camerabuttonheld == false) {
				g_Vars.currentplayer->eyespy->camerabuttonheld = true;
				g_Vars.currentplayer->eyespy->camerashuttertime = TICKS(24);
			}
		} else {
			g_Vars.currentplayer->eyespy->camerabuttonheld = false;
		}

		// Handle exit
		if (exitpressed && g_Vars.currentplayer->eyespy->active) {
			g_Vars.currentplayer->eyespy->active = false;
			g_Vars.currentplayer->devicesactive &= ~DEVICE_EYESPY;
		}

		// Handle B button activation
		// but only if not in a pause-like state
		if (activatepressed && g_Vars.currentplayer->eyespy->active) {
			if (g_Vars.currentplayer->eyespy->buttonheld == false) {
				g_Vars.currentplayer->eyespy->buttonheld = true;
				g_Vars.currentplayer->eyespy->opendoor = true;
			} else {
				g_Vars.currentplayer->eyespy->opendoor = false;
			}
		} else {
			g_Vars.currentplayer->eyespy->opendoor = false;
			g_Vars.currentplayer->eyespy->buttonheld = false;
		}
	}

	g_Vars.currentplayer->eyespy->hit = g_EyespyHit;

	// Check if the eyespy is inactive and coasting into the player's pickup range
	if (g_Vars.currentplayer->eyespy->active == false && !g_EyespyPickup) {
		s32 cdresult;

		f32 xdiff = g_Vars.currentplayer->eyespy->prop->pos.x - g_Vars.currentplayer->prop->pos.x;
		f32 zdiff = g_Vars.currentplayer->eyespy->prop->pos.z - g_Vars.currentplayer->prop->pos.z;

		g_EyespyPickup = true;

		if (xdiff * xdiff + zdiff * zdiff > 100 * 100) {
			g_EyespyPickup = false;
		}

		cdresult = cdTestLos05(&g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms,
				&g_Vars.currentplayer->eyespy->prop->pos, g_Vars.currentplayer->eyespy->prop->rooms,
				CDTYPE_DOORS | CDTYPE_BG,
				GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT);

		if (cdresult == CDRESULT_COLLISION) {
			// Something's in the way
			g_EyespyPickup = false;
		}
	}

	// Sanity check - can't pick up the eyespy while it's active
	if (g_Vars.currentplayer->eyespy->active) {
		g_EyespyPickup = false;
	}

	// Handle pickup
	if (g_EyespyPickup) {
		g_Vars.currentplayer->eyespy->deployed = false;
		g_Vars.currentplayer->eyespy->held = true;
		g_Vars.currentplayer->eyespy->active = false;

		chr->chrflags |= CHRCFLAG_HIDDEN;
		chr->chrflags |= CHRCFLAG_INVINCIBLE;

		weaponPlayPickupSound(WEAPON_EYESPY);
		currentPlayerQueuePickupWeaponHudmsg(WEAPON_EYESPY, false);
		psStopSound(g_Vars.currentplayer->eyespy->prop, PSTYPE_GENERAL, 0xffff);
		chrClearReferences(g_Vars.currentplayer->eyespy->prop - g_Vars.props);
	}

	coordTriggerProxies(&chr->prop->pos, true);
}
