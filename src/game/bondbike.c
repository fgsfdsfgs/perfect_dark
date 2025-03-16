#include <ultra64.h>
#include "constants.h"
#include "game/bondbike.h"
#include "game/bondmove.h"
#include "game/camera.h"
#include "game/chraction.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/propsnd.h"
#include "game/objectives.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/tex.h"
#include "game/player.h"
#include "game/bondhead.h"
#include "game/lv.h"
#include "game/objectives.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "lib/joy.h"
#include "data.h"
#include "types.h"

void bbikeInit(void)
{
	struct hoverbikeobj *hoverbike = (struct hoverbikeobj *)g_Vars.currentplayer->hoverbike->obj;
	Mtxf matrix;

	g_Vars.currentplayer->bondmovemode = MOVEMODE_BIKE;
	g_Vars.currentplayer->bondvehiclemode = 0;
	g_Vars.currentplayer->guncloseroffset = 0;
	g_Vars.currentplayer->gunextraaimx = 0;
	g_Vars.currentplayer->gunextraaimy = 0;

	bbikeUpdateVehicleOffset();

	g_Vars.currentplayer->bondentert = 0;
	g_Vars.currentplayer->bondentert2 = 1;
	g_Vars.currentplayer->bondenterpos.x = g_Vars.currentplayer->prop->pos.x;
	g_Vars.currentplayer->bondenterpos.y = g_Vars.currentplayer->prop->pos.y;
	g_Vars.currentplayer->bondenterpos.z = g_Vars.currentplayer->prop->pos.z;

	mtx3ToMtx4(hoverbike->base.realrot, &matrix);
	mtx4SetTranslation(&hoverbike->base.prop->pos, &matrix);
	mtx4TransformVec(&matrix, &g_Vars.currentplayer->bondvehicleoffset, &g_Vars.currentplayer->bondenteraim);
	mtx00016b58(&g_Vars.currentplayer->bondentermtx,
			0, 0, 0,
			-g_Vars.currentplayer->bond2.unk1c.x, -g_Vars.currentplayer->bond2.unk1c.y, -g_Vars.currentplayer->bond2.unk1c.z,
			g_Vars.currentplayer->bond2.unk28.x, g_Vars.currentplayer->bond2.unk28.y, g_Vars.currentplayer->bond2.unk28.z);

	g_Vars.currentplayer->speedtheta = 0;
	g_Vars.currentplayer->speedthetacontrol = 0;
	g_Vars.currentplayer->speedforwards = 0;
	g_Vars.currentplayer->speedsideways = 0;

	if (hoverbike->base.hidden & OBJHFLAG_PROJECTILE) {
		struct projectile *projectile = hoverbike->base.projectile;
		hoverbike->speed[0] = projectile->speed.x;
		hoverbike->speed[1] = projectile->speed.z;
		hoverbike->w = projectile->unk0dc;
	}

	objFreeEmbedmentOrProjectile(g_Vars.currentplayer->hoverbike);

	hoverbike->base.hidden |= OBJHFLAG_MOUNTED;
}

void bbikeExit(void)
{
	struct defaultobj *obj = g_Vars.currentplayer->hoverbike->obj;
	struct hoverbikeobj *bikeobj = (struct hoverbikeobj *)g_Vars.currentplayer->hoverbike->obj;
	struct coord speed;
	f32 rotation;

	obj->hidden &= ~OBJHFLAG_MOUNTED;

	speed.x = bikeobj->speed[0];
	speed.y = 0;
	speed.z = bikeobj->speed[1];
	rotation = bikeobj->w;

	objApplyMomentum(obj, &speed, rotation, false, false);
	psStopSound(g_Vars.currentplayer->hoverbike, PSTYPE_GENERAL, 0xffff);
	psStopSound(g_Vars.currentplayer->prop, PSTYPE_GENERAL, 0xffff);
	psCreate(NULL, g_Vars.currentplayer->hoverbike, SFX_BIKE_PULSE, -1,
			-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);

	obj->flags |= OBJFLAG_HOVERBIKE_MOVINGWHILEEMPTY;
}

void bbikeUpdateVehicleOffset(void)
{
	struct defaultobj *hoverbike = g_Vars.currentplayer->hoverbike->obj;

	g_Vars.currentplayer->bondvehicleoffset.x = 0.0f / hoverbike->model->scale;
	g_Vars.currentplayer->bondvehicleoffset.y = 80.0f / hoverbike->model->scale;
	g_Vars.currentplayer->bondvehicleoffset.z = -50.0f / hoverbike->model->scale;
}

void bbikeTryDismountAngle(f32 relativeangle, f32 distance)
{
	u32 stack;
	struct hoverbikeobj *bike;
	f32 angle;
	struct coord pos;
	RoomNum rooms[8];
	s32 result;
	f32 ymax;
	f32 ymin;
	f32 radius;

	if (g_Vars.currentplayer->walkinitmove == 0) {
		bike = (struct hoverbikeobj *)g_Vars.currentplayer->hoverbike->obj;
		angle = hoverpropGetTurnAngle(&bike->base);

		playerGetBbox(g_Vars.currentplayer->prop, &radius, &ymax, &ymin);

		distance += radius + 10;

		angle += relativeangle;

		if (angle >= M_BADTAU) {
			angle -= M_BADTAU;
		}

		pos.x = g_Vars.currentplayer->hoverbike->pos.x + sinf(angle) * distance;
		pos.y = g_Vars.currentplayer->hoverbike->pos.y;
		pos.z = g_Vars.currentplayer->hoverbike->pos.z + cosf(angle) * distance;

		propSetPerimEnabled(g_Vars.currentplayer->hoverbike, false);
		propSetPerimEnabled(g_Vars.currentplayer->prop, false);

		func0f065e74(&g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms, &pos, rooms);
		bmoveFindEnteredRoomsByPos(g_Vars.currentplayer, &pos, rooms);

		result = cdTestCylMove02(&g_Vars.currentplayer->prop->pos, g_Vars.currentplayer->prop->rooms,
				&pos, rooms, CDTYPE_ALL, true,
				ymax - g_Vars.currentplayer->prop->pos.y,
				ymin - g_Vars.currentplayer->prop->pos.y);

		propSetPerimEnabled(g_Vars.currentplayer->hoverbike, true);

		if (result == CDRESULT_NOCOLLISION) {
			result = cdTestVolume(&pos, radius, rooms, CDTYPE_ALL, CHECKVERTICAL_YES,
					ymax - g_Vars.currentplayer->prop->pos.y,
					ymin - g_Vars.currentplayer->prop->pos.y);
		}

		if (result == CDRESULT_NOCOLLISION) {
			g_Vars.currentplayer->walkinitmove = true;

			g_Vars.currentplayer->walkinitpos.x = pos.x;
			g_Vars.currentplayer->walkinitpos.y = pos.y;
			g_Vars.currentplayer->walkinitpos.z = pos.z;

			g_Vars.currentplayer->moveinitspeed.x = bike->speed[0];
			g_Vars.currentplayer->moveinitspeed.y = 0;
			g_Vars.currentplayer->moveinitspeed.z = bike->speed[1];
		}

		propSetPerimEnabled(g_Vars.currentplayer->prop, true);
	}
}

void bbikeHandleActivate(void)
{
	if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_RUNNING
			&& g_Vars.lvframe60 - g_Vars.currentplayer->activatetimelast < TICKS(25)) {
		struct hoverbikeobj *bike = (struct hoverbikeobj *)g_Vars.currentplayer->hoverbike->obj;
		struct modelrodata_bbox *bbox = objFindBboxRodata(&bike->base);

		f32 sidedist = bbox->xmax * bike->base.model->scale;
		f32 frontdist = bbox->zmax * bike->base.model->scale;
		f32 diagdist = sqrtf(sidedist * sidedist + frontdist * frontdist);

		g_Vars.currentplayer->walkinitmove = false;

		bbikeTryDismountAngle(1.5705462694168f, sidedist);  // 90  - left
		bbikeTryDismountAngle(4.7116389274597f, sidedist);  // 270 - right
		bbikeTryDismountAngle(0.7852731347084f, diagdist);  // 45  - front left
		bbikeTryDismountAngle(5.4969120025635f, diagdist);  // 315 - front right
		bbikeTryDismountAngle(2.3558194637299f, diagdist);  // 135 - back left
		bbikeTryDismountAngle(3.9263656139374f, diagdist);  // 225 - back right
		bbikeTryDismountAngle(0, frontdist);                // 0   - front
		bbikeTryDismountAngle(3.1410925388336f, frontdist); // 180 - back

		if (g_Vars.currentplayer->walkinitmove) {
			bmoveSetMode(MOVEMODE_WALK);
		}

		g_Vars.currentplayer->bondactivateorreload = 0;
	}
}

void bbikeApplyMoveData(struct movedata *data)
{
	struct hoverbikeobj *bike = (struct hoverbikeobj *)g_Vars.currentplayer->hoverbike->obj;
	s8 contnum = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
	f32 value1;
	f32 tmp;
	s32 contmode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);

	if ((contmode == CONTROLMODE_12
				|| contmode == CONTROLMODE_14
				|| contmode == CONTROLMODE_13
				|| contmode == CONTROLMODE_11
				|| contmode == CONTROLMODE_PC)
			&& !g_Vars.currentplayer->pausemode) {
		u32 lmask, rmask;
		if (contmode == CONTROLMODE_PC) {
			lmask = L_CBUTTONS;
			rmask = R_CBUTTONS;
		} else {
			lmask = L_JPAD | L_CBUTTONS;
			rmask = R_JPAD | R_CBUTTONS;
		}
		if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
			data->digitalstepleft = joyCountButtonsOnSpecificSamples(0, contnum, lmask);
			data->digitalstepright = joyCountButtonsOnSpecificSamples(0, contnum, rmask);
		}
	}

	// Forward/back
	if (data->digitalstepforward) {
		value1 = 1.0f - g_Vars.currentplayer->speedforwards;

		if (value1 > 0.1f * g_Vars.lvupdate60freal) {
			value1 = 0.1f * g_Vars.lvupdate60freal;
		}

		g_Vars.currentplayer->speedforwards += value1;
	} else if (data->digitalstepback) {
		value1 = -1.0f - g_Vars.currentplayer->speedforwards;

		if (value1 < -0.1f * g_Vars.lvupdate60freal) {
			value1 = -0.1f * g_Vars.lvupdate60freal;
		}

		g_Vars.currentplayer->speedforwards += value1;
	} else if (data->canlookahead) {
		g_Vars.currentplayer->speedforwards = data->analogwalk / 70.0f;

		if (g_Vars.currentplayer->speedforwards > 1.0f) {
			g_Vars.currentplayer->speedforwards = 1.0f;
		} else if (g_Vars.currentplayer->speedforwards < -1.0f) {
			g_Vars.currentplayer->speedforwards = -1.0f;
		}
	} else {
		g_Vars.currentplayer->speedforwards = 0;
	}

	// Sideways
	if (data->digitalstepleft) {
		f32 value2 = -1.0f - g_Vars.currentplayer->speedsideways;
		f32 tmp = data->digitalstepleft * PALUPF(-0.1f);

		if (value2 < tmp) {
			value2 = tmp;
		}

		g_Vars.currentplayer->speedsideways += value2;
	} else if (data->digitalstepright) {
		f32 value2 = 1.0f - g_Vars.currentplayer->speedsideways;
		f32 tmp = data->digitalstepright * PALUPF(0.1f);

		if (value2 > tmp) {
			value2 = tmp;
		}

		g_Vars.currentplayer->speedsideways += value2;
	} else if (data->unk14) {
		g_Vars.currentplayer->speedsideways = data->analogstrafe / 70.0f;

		if (g_Vars.currentplayer->speedsideways > 1.0f) {
			g_Vars.currentplayer->speedsideways = 1.0f;
		}

		if (g_Vars.currentplayer->speedsideways < -1.0f) {
			g_Vars.currentplayer->speedsideways = -1.0f;
		}
	} else {
		g_Vars.currentplayer->speedsideways = 0;
	}

	{
		f32 sp3c;
		struct coord sp30;
		f32 sp28[2];

		sp3c = -bike->exreal;

		if (bike->hov.bobpitchcur < M_PI) {
			sp3c += -bike->hov.bobpitchcur * 0.8f;
		} else {
			sp3c += (M_BADTAU - bike->hov.bobpitchcur) * 0.8f;
		}

		if (sp3c < 0) {
			sp3c += M_BADTAU;
		} else if (sp3c >= M_BADTAU) {
			sp3c -= M_BADTAU;
		}

		sp30.f[0] = 0;
		sp30.f[1] = -sinf(sp3c);
		sp30.f[2] = cosf(sp3c);

		cam0f0b4d04(&sp30, sp28);

		g_Vars.currentplayer->gunextraaimy = -((sp28[1] - camGetScreenTop()) * 2.0f / camGetScreenHeight() - 1.0f) * 0.75f;
	}
}

void bbike0f0d2b40(struct defaultobj *bike, struct coord *arg1, f32 arg2, struct defaultobj *obstacle)
{
	struct coord sp9c;
	struct coord sp90;
	struct coord sp84 = {0, 0, 0};
	struct coord sp78;
	struct coord sp6c;
	struct coord sp60;
	struct coord sp54;

	cdGetEdge(&sp78, &sp6c, 333, "bondbike.c");

	sp60.x = bike->prop->pos.x;
	sp60.y = bike->prop->pos.y;
	sp60.z = bike->prop->pos.z;

	sp54.x = obstacle->prop->pos.x - bike->prop->pos.x;
	sp54.y = obstacle->prop->pos.y - bike->prop->pos.y;
	sp54.z = obstacle->prop->pos.z - bike->prop->pos.z;

	func0f02e3dc(&sp78, &sp6c, &sp60, &sp54, &sp9c);

	sp90.x = arg1->f[0];
	sp90.y = 0;
	sp90.z = arg1->f[2];

	if (sp90.f[0] || sp90.f[2]) {
		guNormalize(&sp90.x, &sp90.y, &sp90.z);
	} else {
		sp90.z = 1;
	}

	if (arg1->x) {
		sp84.x += arg1->x / g_Vars.lvupdate60freal;
	}

	if (arg1->z) {
		sp84.z += arg1->z / g_Vars.lvupdate60freal;
	}

	func0f082e84(obstacle, &sp9c, &sp90, &sp84, false);

	if (arg2) {
		f32 xdiff = sp9c.x - bike->prop->pos.x;
		f32 zdiff = sp9c.z - bike->prop->pos.z;
		f32 rotation = 0;
		struct coord speed = {0, 0, 0};

		f32 tmp = 1 / sqrtf(xdiff * xdiff + zdiff * zdiff);

		xdiff *= tmp;
		zdiff *= tmp;

		arg2 = arg2 / g_Vars.lvupdate60freal;

		speed.x += -zdiff * arg2 * 40;
		speed.z += xdiff * arg2 * 40;

		rotation += arg2 * 0.1f;

		objApplyMomentum(obstacle, &speed, rotation, true, true);
	}
}

s32 bbikeCalculateNewPosition(struct coord *vel, f32 angledelta)
{
	s32 result = CDRESULT_NOCOLLISION;
	struct coord dstpos;
	RoomNum dstrooms[8];
	struct hov hov;
	bool hasvel = false;
	struct hoverbikeobj *bike = (struct hoverbikeobj *) g_Vars.currentplayer->hoverbike->obj;
	RoomNum spa8[20];
	f32 xdiff;
	f32 zdiff;
	f32 ymax;
	f32 ymin;
	f32 radius;
	f32 halfradius;

	dstpos.x = g_Vars.currentplayer->hoverbike->pos.x;
	dstpos.y = g_Vars.currentplayer->hoverbike->pos.y;
	dstpos.z = g_Vars.currentplayer->hoverbike->pos.z;

	if (vel->x || vel->y || vel->z) {
		propSetPerimEnabled(g_Vars.currentplayer->prop, false);
		propSetPerimEnabled(g_Vars.currentplayer->hoverbike, false);

		dstpos.x += vel->x;
		dstpos.z += vel->z;

		objGetBbox(g_Vars.currentplayer->hoverbike, &radius, &ymax, &ymin);
		func0f065dfc(&g_Vars.currentplayer->hoverbike->pos,
				g_Vars.currentplayer->hoverbike->rooms,
				&dstpos, dstrooms, spa8, 20);

		setup0f09233c(&bike->base, &dstpos, bike->base.realrot, dstrooms);

		hasvel = true;

		hov = bike->hov;

		hovUpdateGround(&bike->base, &hov, &dstpos, dstrooms, bike->base.realrot);

		dstpos.y += hov.ground - bike->hov.ground;

		halfradius = radius * 0.5f;
		xdiff = dstpos.x - g_Vars.currentplayer->hoverbike->pos.x;
		zdiff = dstpos.z - g_Vars.currentplayer->hoverbike->pos.z;

		if (xdiff > halfradius || zdiff > halfradius || xdiff < -halfradius || zdiff < -halfradius) {
			result = cdExamCylMove06(&g_Vars.currentplayer->hoverbike->pos,
					g_Vars.currentplayer->hoverbike->rooms,
					&dstpos, dstrooms, radius, CDTYPE_ALL, 1,
					ymax - g_Vars.currentplayer->hoverbike->pos.y,
					ymin - g_Vars.currentplayer->hoverbike->pos.y);

			if (result == CDRESULT_NOCOLLISION) {
				result = cdExamCylMove02(&g_Vars.currentplayer->hoverbike->pos,
						&dstpos, radius, dstrooms, CDTYPE_ALL, true,
						ymax - g_Vars.currentplayer->hoverbike->pos.y,
						ymin - g_Vars.currentplayer->hoverbike->pos.y);
			}
		} else {
			result = cdExamCylMove02(&g_Vars.currentplayer->hoverbike->pos,
					&dstpos, radius, spa8, CDTYPE_ALL, true,
					ymax - g_Vars.currentplayer->hoverbike->pos.y,
					ymin - g_Vars.currentplayer->hoverbike->pos.y);
		}

		propSetPerimEnabled(g_Vars.currentplayer->prop, true);
		propSetPerimEnabled(g_Vars.currentplayer->hoverbike, true);
	}

	if (angledelta) {
		u32 stack[2];
		f32 newangle = hoverpropGetTurnAngle(&bike->base) - angledelta;
		Mtxf sp44;

		if (newangle >= M_BADTAU) {
			newangle -= M_BADTAU;
		} else if (newangle < 0.0f) {
			newangle += M_BADTAU;
		}

		hoverpropSetTurnAngle(&bike->base, newangle);

		mtx4LoadYRotation(newangle, &sp44);
		mtx00015f04(bike->base.model->scale, &sp44);
		mtx4ToMtx3(&sp44, bike->base.realrot);
	}

	if (result == CDRESULT_NOCOLLISION && hasvel) {
		g_Vars.currentplayer->hoverbike->pos.x = dstpos.x;
		g_Vars.currentplayer->hoverbike->pos.z = dstpos.z;

		propDeregisterRooms(g_Vars.currentplayer->hoverbike);
		roomsCopy(dstrooms, g_Vars.currentplayer->hoverbike->rooms);

		bike->hov = hov;
	}

	return result;
}

s32 bbikeCalculateNewPositionWithPush(struct coord *arg0, f32 arg1)
{
	s32 result = bbikeCalculateNewPosition(arg0, arg1);

	if (result != CDRESULT_NOCOLLISION) {
		struct prop *obstacle = cdGetObstacleProp();

		if (obstacle && g_Vars.lvupdate240 > 0) {
			if (obstacle->type == PROPTYPE_CHR) {
				// empty
			} else if (obstacle->type == PROPTYPE_PLAYER) {
				// empty
			} else if (obstacle->type == PROPTYPE_OBJ) {
				struct defaultobj *obj = obstacle->obj;

				if ((obj->hidden & OBJHFLAG_MOUNTED) == 0
						&& (obj->hidden & OBJHFLAG_GRABBED) == 0
						&& (obj->flags3 & OBJFLAG3_PUSHABLE)) {
					bool pass = true;
					struct defaultobj *bike = g_Vars.currentplayer->hoverbike->obj;

					if ((obj->hidden & OBJHFLAG_PROJECTILE)
							&& (obj->projectile->flags & PROJECTILEFLAG_00001000)) {
						pass = false;
					}

					if (pass) {
						bbike0f0d2b40(bike, arg0, arg1, obj);

						if ((obj->hidden & OBJHFLAG_PROJECTILE)
								&& (obj->projectile->flags & PROJECTILEFLAG_SLIDING)) {
							s32 somevalue;
							bool embedded = false;
							somevalue = projectileTick(obj, &embedded);

							if (obj->hidden & OBJHFLAG_PROJECTILE) {
								obj->projectile->flags |= PROJECTILEFLAG_00001000;

								if (somevalue) {
									obj->projectile->flags |= PROJECTILEFLAG_00002000;
								} else {
									obj->projectile->flags &= ~PROJECTILEFLAG_00002000;
								}
							}

							if (somevalue) {
								result = bbikeCalculateNewPosition(arg0, arg1);
							}
						}
					}
				}
			}
		}
	}

	return result;
}

void bbikeUpdateVertical(struct coord *pos)
{
	struct defaultobj *bike = g_Vars.currentplayer->hoverbike->obj;
	f32 angle;
	RoomNum newrooms[8];
	bool newinlift;
	struct prop *lift = NULL;
	f32 ground;

	angle = hoverpropGetTurnAngle(bike);

	func0f065e74(&bike->prop->pos, bike->prop->rooms, pos, newrooms);

#if VERSION < VERSION_NTSC_1_0
	{
		s32 i;

		for (i = 0; newrooms[i] != -1; i++) {
			if (g_Vars.currentplayer->floorroom == newrooms[i]) {
				newrooms[0] = g_Vars.currentplayer->floorroom;
				newrooms[1] = -1;
				break;
			}
		}
	}
#endif

	bmoveFindEnteredRoomsByPos(g_Vars.currentplayer, pos, newrooms);
	propDeregisterRooms(g_Vars.currentplayer->prop);
	roomsCopy(newrooms, g_Vars.currentplayer->prop->rooms);

	g_Vars.currentplayer->vv_theta = (M_BADTAU - angle) * 360.0f / M_BADTAU;

	g_Vars.currentplayer->prop->pos.x = pos->x;
	g_Vars.currentplayer->prop->pos.y = pos->y;
	g_Vars.currentplayer->prop->pos.z = pos->z;

	ground = cdFindGroundInfoAtCyl(&g_Vars.currentplayer->prop->pos,
			g_Vars.currentplayer->bond2.radius,
			g_Vars.currentplayer->prop->rooms,
			&g_Vars.currentplayer->floorcol,
			&g_Vars.currentplayer->floortype,
			&g_Vars.currentplayer->floorflags,
			&g_Vars.currentplayer->floorroom,
			&newinlift, &lift);

	if (ground < -30000) {
		ground = -30000;
	}

	if (g_Vars.currentplayer->inlift && newinlift) {
		f32 diff = ground - g_Vars.currentplayer->vv_ground;

		// If getting on the bike
		if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_OFF) {
			g_Vars.currentplayer->bondenterpos.y += diff;
		}
	} else {
		lift = NULL;
	}

	g_Vars.currentplayer->inlift = newinlift;

	if (newinlift) {
		g_Vars.currentplayer->liftground = ground;
	}

	g_Vars.currentplayer->lift = lift;
	g_Vars.currentplayer->vv_ground = ground;
	g_Vars.currentplayer->vv_height = g_Vars.currentplayer->headpos.y / g_Vars.currentplayer->standheight * g_Vars.currentplayer->vv_eyeheight;
	g_Vars.currentplayer->vv_manground = g_Vars.currentplayer->prop->pos.y - g_Vars.currentplayer->vv_height;

	if (g_Vars.currentplayer->floorflags & GEOFLAG_DIE) {
		playerDie(true);
	}

	bmoveUpdateVerta();
}

s32 bbike0f0d363c(f32 arg0)
{
	struct coord coord = {0, 0, 0};

	return bbikeCalculateNewPositionWithPush(&coord, arg0);
}

s32 bbike0f0d3680(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	s32 result = bbikeCalculateNewPositionWithPush(arg0, 0);

	if (!result) {
#if VERSION >= VERSION_NTSC_1_0
		cdGetEdge(arg1, arg2, 659, "bondbike.c");
#else
		cdGetEdge(arg1, arg2, 656, "bondbike.c");
#endif
	}

	return result;
}

s32 bbike0f0d36d4(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4)
{
	if (cd00024ea4()) {
		struct coord sp24;
		f32 somefloat = cd00024e98();
		s32 someint;

		sp24.x = arg0->x * somefloat * 0.25f;
		sp24.y = arg0->y * somefloat * 0.25f;
		sp24.z = arg0->z * somefloat * 0.25f;

		someint = bbikeCalculateNewPositionWithPush(&sp24, 0);

		if (someint == 1) {
			return 1;
		}

		if (someint == 0) {
#if VERSION >= VERSION_NTSC_1_0
			cdGetEdge(arg3, arg4, 685, "bondbike.c");
#else
			cdGetEdge(arg3, arg4, 682, "bondbike.c");
#endif

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

s32 bbike0f0d3840(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	s32 result;

	if (arg1->f[0] != arg2->f[0] || arg1->f[2] != arg2->f[2]) {
		struct coord sp30;
		struct coord sp24;
		f32 tmp;

		sp30.x = arg2->x - arg1->x;
		sp30.z = arg2->z - arg1->z;

		tmp = sqrtf(sp30.f[0] * sp30.f[0] + sp30.f[2] * sp30.f[2]);

		sp30.x *= 1.0f / tmp;
		sp30.z *= 1.0f / tmp;

		tmp = arg0->f[0] * sp30.f[0] + arg0->f[2] * sp30.f[2];

		sp24.x = sp30.x * tmp;
		sp24.y = 0;
		sp24.z = sp30.z * tmp;

		result = bbikeCalculateNewPositionWithPush(&sp24, 0);
	} else {
		result = -1;
	}

	return result;
}

s32 bbike0f0d3940(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	struct coord sp34;
	struct coord sp28;
	f32 ymax;
	f32 ymin;
	f32 tmp;
	f32 radius;

	objGetBbox(g_Vars.currentplayer->hoverbike, &radius, &ymax, &ymin);

	sp34.x = arg1->x - (g_Vars.currentplayer->hoverbike->pos.x + arg0->f[0]);
	sp34.z = arg1->z - (g_Vars.currentplayer->hoverbike->pos.z + arg0->f[2]);

	if (sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2] <= radius * radius) {
		if (arg1->f[0] != g_Vars.currentplayer->hoverbike->pos.f[0] || arg1->f[2] != g_Vars.currentplayer->hoverbike->pos.f[2]) {
			sp34.x = -(arg1->z - g_Vars.currentplayer->hoverbike->pos.z);
			sp34.y = 0;
			sp34.z = arg1->x - g_Vars.currentplayer->hoverbike->pos.x;

			tmp = sqrtf(sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2]);

			sp34.x = sp34.f[0] * (1.0f / tmp);
			sp34.z = sp34.f[2] * (1.0f / tmp);

			tmp = arg0->f[0] * sp34.f[0] + arg0->f[2] * sp34.f[2];

			sp34.x = sp34.x * tmp;
			sp34.z = sp34.z * tmp;

			sp28.x = sp34.x;
			sp28.y = 0;
			sp28.z = sp34.z;

			if (bbikeCalculateNewPositionWithPush(&sp28, 0) == CDRESULT_NOCOLLISION) {
				return true;
			}
		}
	} else {
		sp34.x = arg2->x - (g_Vars.currentplayer->hoverbike->pos.x + arg0->f[0]);
		sp34.z = arg2->z - (g_Vars.currentplayer->hoverbike->pos.z + arg0->f[2]);

		if (sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2] <= radius * radius) {
			if (arg2->f[0] != g_Vars.currentplayer->hoverbike->pos.f[0] || arg2->f[2] != g_Vars.currentplayer->hoverbike->pos.f[2]) {
				sp34.x = -(arg2->z - g_Vars.currentplayer->hoverbike->pos.z);
				sp34.y = 0;
				sp34.z = arg2->x - g_Vars.currentplayer->hoverbike->pos.x;

				tmp = sqrtf(sp34.f[0] * sp34.f[0] + sp34.f[2] * sp34.f[2]);

				sp34.x = sp34.f[0] * (1.0f / tmp);
				sp34.z = sp34.f[2] * (1.0f / tmp);

				tmp = arg0->f[0] * sp34.f[0] + arg0->f[2] * sp34.f[2];

				sp34.x = sp34.x * tmp;
				sp34.z = sp34.z * tmp;

				sp28.x = sp34.x;
				sp28.y = 0;
				sp28.z = sp34.z;

				if (bbikeCalculateNewPositionWithPush(&sp28, 0) == CDRESULT_NOCOLLISION) {
					return true;
				}
			}
		}
	}

	return false;
}

void bbike0f0d3c60(struct coord *arg0)
{
	struct coord sp64;
	struct coord sp58;
	struct coord sp4c;
	struct coord sp40;
	s32 value;
	struct coord sp30;
	struct coord sp24;

	if (bbike0f0d3680(arg0, &sp64, &sp58) == 0) {
		value = bbike0f0d36d4(arg0, &sp64, &sp58, &sp4c, &sp40);

		if (value > 0 || value < 0) {
			if (bbike0f0d3840(arg0, &sp64, &sp58) <= 0
					&& bbike0f0d3940(arg0, &sp64, &sp58) <= 0) {
				// empty
			}
		} else if (value == 0) {
			bbike0f0d36d4(arg0, &sp4c, &sp40, &sp30, &sp24);

			if (bbike0f0d3840(arg0, &sp4c, &sp40) <= 0
					&& bbike0f0d3840(arg0, &sp64, &sp58) <= 0
					&& bbike0f0d3940(arg0, &sp4c, &sp40) <= 0
					&& bbike0f0d3940(arg0, &sp64, &sp58) <= 0) {
				if (&arg0);
			}
		}
	}
}

void bbikeTick(void)
{
	struct defaultobj *obj = g_Vars.currentplayer->hoverbike->obj;
	struct hoverbikeobj *bike = (struct hoverbikeobj *) g_Vars.currentplayer->hoverbike->obj;
	struct coord sp20c;
	struct prop *prop;
	s32 i;
	f32 sp200;
	u32 stack;
	f32 sp1f8;
	f32 sp1f4;
	struct coord sp1e8;
	Mtxf sp1a8;
#if VERSION >= VERSION_NTSC_1_0
	s32 j;
#endif
	Mtxf sp164;
	Mtxf sp124;
	Mtxf spe4;
	f32 spd4[4];
	f32 spc4[4];
	f32 spb4[4];
	f32 spa4[4];
	f32 sp94[4];
	f32 sp84[4];
	struct coord pos;
	f32 breathing;
	f32 sp70;
	f32 sqdist;

	static f32 var80070f04 = 0;

	if (g_Vars.lvupdate240 > 0) {
		g_Vars.currentplayer->bondprevpos.x = g_Vars.currentplayer->prop->pos.x;
		g_Vars.currentplayer->bondprevpos.y = g_Vars.currentplayer->prop->pos.y;
		g_Vars.currentplayer->bondprevpos.z = g_Vars.currentplayer->prop->pos.z;

		g_Vars.currentplayer->bondbreathing -= (0.75f * g_Vars.lvupdate60freal) / 2700.0f;

		if (g_Vars.currentplayer->bondbreathing < 0.0f) {
			g_Vars.currentplayer->bondbreathing = 0.0f;
		}

		if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_OFF) {
			hoverbikeUpdateMovement(bike,
					g_Vars.currentplayer->speedforwards,
					g_Vars.currentplayer->speedsideways,
					g_Vars.currentplayer->speedtheta);
		} else {
			if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_ENGINESTART) {
				g_Vars.currentplayer->bondvehiclemode = VEHICLEMODE_RUNNING;
			}

			hoverbikeUpdateMovement(bike,
					g_Vars.currentplayer->speedforwards,
					g_Vars.currentplayer->speedsideways,
					g_Vars.currentplayer->speedtheta);
		}

		if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_RUNNING) {
			sqdist = g_Vars.currentplayer->speedforwards * g_Vars.currentplayer->speedforwards
				+ g_Vars.currentplayer->speedsideways * g_Vars.currentplayer->speedsideways;

			if (sqdist > 1.0f) {
				sqdist = 1.0f;
			}

			for (i = 0; i < g_Vars.lvupdate240; i++) {
				var80070f04 += (sqdist - var80070f04) * (PAL ? 0.003f : 0.0025f);
			}

			sp200 = 1.0f - (var80070f04 + var80070f04);

			if (var80070f04 + var80070f04 > 1.0f) {
				sp200 = 0.0f;
			}

			sp200 *= 300.0f;

			psSetPitch(g_Vars.currentplayer->hoverbike, var80070f04, -1);
			psSetVolume(g_Vars.currentplayer->hoverbike, var80070f04 * 300.0f);
			psSetVolume(g_Vars.currentplayer->prop, sp200);
		}

		if (1);

		bbike0f0d363c(bike->w * g_Vars.lvupdate60freal);

		sp20c.x = bike->speed[0] * g_Vars.lvupdate60freal;
		sp20c.y = 0.0f;
		sp20c.z = bike->speed[1] * g_Vars.lvupdate60freal;

		bike->prevpos[0] = bike->base.prop->pos.x;
		bike->prevpos[1] = bike->base.prop->pos.z;

		bbike0f0d3c60(&sp20c);

		sp1f8 = (bike->base.prop->pos.x - bike->prevpos[0]) / g_Vars.lvupdate60freal;
		sp1f4 = (bike->base.prop->pos.z - bike->prevpos[1]) / g_Vars.lvupdate60freal;

		if (sp1f8 != 0.0f || sp1f4 != 0.0f) {
			f32 tmp = sp1f8 * sp1f8 + sp1f4 * sp1f4;

			if (tmp > 0.0f) {
				tmp = sqrtf((bike->speed[0] * bike->speed[0] + bike->speed[1] * bike->speed[1]) / tmp);

				if (tmp < 1.0f) {
					sp1f8 *= tmp;
					sp1f4 *= tmp;
				}
			}
		}

		bike->speed[0] = sp1f8;
		bike->speed[1] = sp1f4;

		hovTick(obj, &bike->hov);
		func0f069c70(obj, true, true);
		mtx3ToMtx4(obj->realrot, &sp1a8);
		mtx4SetTranslation(&obj->prop->pos, &sp1a8);
		mtx4TransformVec(&sp1a8, &g_Vars.currentplayer->bondvehicleoffset, &sp1e8);

		bbikeUpdateVertical(&sp1e8);

		prop = g_Vars.currentplayer->prop;

#if VERSION >= VERSION_NTSC_1_0
		for (j = 0; prop->rooms[j] != -1; j++) {
			if (prop->rooms[j] == g_Vars.currentplayer->floorroom) {
				propDeregisterRooms(prop);
				g_Vars.currentplayer->prop->rooms[0] = g_Vars.currentplayer->floorroom;
				g_Vars.currentplayer->prop->rooms[1] = -1;
				break;
			}
		}
#endif
	}

	bheadAdjustAnimation(0);
	bheadUpdate(0, 0);
	mtx4LoadXRotation((360.0f - g_Vars.currentplayer->vv_verta360) * 0.017450513318181f, &sp164);

	mtx00016d58(&sp124, 0.0f, 0.0f, 0.0f,
			-g_Vars.currentplayer->headlook.x, -g_Vars.currentplayer->headlook.y, -g_Vars.currentplayer->headlook.z,
			g_Vars.currentplayer->headup.x, g_Vars.currentplayer->headup.y, g_Vars.currentplayer->headup.z);

	mtx4MultMtx4InPlace(&sp124, &sp164);
	mtx3ToMtx4(obj->realrot, &sp124);
	mtx00015f04(1.0f / obj->model->scale, &sp124);
	mtx4LoadYRotation(hoverpropGetTurnAngle(obj), &spe4);
	quaternion0f097044(&spe4, spd4);
	quaternion0f097044(&sp124, spc4);
	quaternion0f0976c0(spc4, spd4);
	quaternionSlerp(spd4, spc4, 0.8f, spb4);
	quaternionToMtx(spb4, &sp124);
	mtx4MultMtx4InPlace(&sp124, &sp164);

	if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_OFF) {
		g_Vars.currentplayer->bondentert += g_Vars.lvupdate60freal / 60.0f;

		if (g_Vars.currentplayer->bondentert >= 1.0f) {
			g_Vars.currentplayer->bondentert = 1.0f;
		}

		g_Vars.currentplayer->bondentert2 = (cosf(g_Vars.currentplayer->bondentert * M_BADTAU * 0.5f) + 1.0f) * 0.5f;

		if (g_Vars.currentplayer->bondentert >= 1.0f) {
			g_Vars.currentplayer->bondvehiclemode = VEHICLEMODE_ENGINESTART;

			psStopSound(g_Vars.currentplayer->hoverbike, PSTYPE_GENERAL, 0xffff);

			psCreate(NULL, g_Vars.currentplayer->prop, SFX_BIKE_TAKEOFF, -1,
					-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);

			psCreate(NULL, g_Vars.currentplayer->prop, SFX_BIKE_PULSE, -1,
					-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);

			psCreate(NULL, g_Vars.currentplayer->hoverbike, SFX_BIKE_ENGINE, -1,
					-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
		}

		quaternion0f097044(&g_Vars.currentplayer->bondentermtx, spa4);
		quaternion0f097044(&sp164, sp94);
		quaternion0f0976c0(sp94, spa4);
		quaternionSlerp(spa4, sp94, 1.0f - g_Vars.currentplayer->bondentert2, sp84);
		quaternionToMtx(sp84, &sp164);
	}

	g_Vars.currentplayer->bond2.unk1c.x = sp164.m[2][0];
	g_Vars.currentplayer->bond2.unk1c.y = sp164.m[2][1];
	g_Vars.currentplayer->bond2.unk1c.z = sp164.m[2][2];
	g_Vars.currentplayer->bond2.unk28.x = sp164.m[1][0];
	g_Vars.currentplayer->bond2.unk28.y = sp164.m[1][1];
	g_Vars.currentplayer->bond2.unk28.z = sp164.m[1][2];

	if (g_Vars.currentplayer->bondvehiclemode == VEHICLEMODE_OFF) {
		pos.x = (g_Vars.currentplayer->bondenterpos.x - g_Vars.currentplayer->bondenteraim.x) * g_Vars.currentplayer->bondentert2 + g_Vars.currentplayer->prop->pos.x;
		pos.y = (g_Vars.currentplayer->bondenterpos.y - g_Vars.currentplayer->prop->pos.y) * g_Vars.currentplayer->bondentert2 + g_Vars.currentplayer->prop->pos.y;
		pos.z = (g_Vars.currentplayer->bondenterpos.z - g_Vars.currentplayer->bondenteraim.z) * g_Vars.currentplayer->bondentert2 + g_Vars.currentplayer->prop->pos.z;

		bmove0f0cc19c(&pos);
	} else {
		bmove0f0cc19c(&g_Vars.currentplayer->prop->pos);
	}

	sp70 = g_Vars.currentplayer->speedverta / 0.7f;
	breathing = bheadGetBreathingValue();

	if (sp70 > 1.0f) {
		sp70 = 1.0f;
	} else if (sp70 < -1.0f) {
		sp70 = -1.0f;
	}

	bgun0f09d8dc(breathing, 0, sp70, 0.0f, g_Vars.currentplayer->speedsideways);
	bgunSetAdjustPos(g_Vars.currentplayer->vv_verta360 * 0.017450513318181f);
	playerUpdatePerimInfo();
	bmoveUpdateRooms(g_Vars.currentplayer);
	objectiveCheckRoomEntered(g_Vars.currentplayer->prop->rooms[0]);
	doorsCheckAutomatic();
}
