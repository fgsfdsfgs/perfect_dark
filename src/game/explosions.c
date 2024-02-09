#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/dlights.h"
#include "game/chr.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/propsnd.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/explosions.h"
#include "game/smoke.h"
#include "game/bg.h"
#include "game/room.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "game/mplayer/mplayer.h"
#include "game/propobj.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/collision.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "data.h"
#include "types.h"

#ifdef PLATFORM_N64
#define SHAKE_TIME 6
#else
#define SHAKE_TIME 12
#endif

struct explosion *g_Explosions;
s32 g_MaxExplosions;

s32 g_ExplosionShakeTotalTimer = 0;
s32 g_ExplosionShakeIntensityTimer = 0;
f32 g_ExplosionDamageTxScale = 1;
u32 var8007e4ac = 0x0000004b;
u32 var8007e4b0 = 0x000001e0;
u32 var8007e4b4 = 0x000000a8;

struct explosiontype g_ExplosionTypes[] = {
	//       rangeh
	//       |    rangev
	//       |    |    changerateh
	//       |    |    |    changeratev
	//       |    |    |    |     innersize
	//       |    |    |    |     |     blastradius
	//       |    |    |    |     |     |     damageradius
	//       |    |    |    |     |     |     |     duration
	//       |    |    |    |     |     |     |     |    propagationrate
	//       |    |    |    |     |     |     |     |    |  flarespeed
	//       |    |    |    |     |     |     |     |    |  |  smoketype
	//       |    |    |    |     |     |     |     |    |  |  |                       sound
	//       |    |    |    |     |     |     |     |    |  |  |                       |       damage
	//       |    |    |    |     |     |     |     |    |  |  |                       |       |
	/*00*/ { 0.1, 0.1, 0,   0,    0.1,  0,    0,    1,   1, 1, SMOKETYPE_NONE,         0x0000, 0     },
	/*01*/ { 1,   1,   0,   0,    1,    0,    0,    30,  1, 1, SMOKETYPE_BULLETIMPACT, 0x0000, 0     },
	/*02*/ { 20,  20,  0,   0,    30,   50,   50,   40,  1, 3, SMOKETYPE_MINI,         0x8099, 0.125 },
	/*03*/ { 50,  50,  0,   0,    50,   100,  100,  45,  1, 4, SMOKETYPE_MINI,         0x809a, 0.5   },
	/*04*/ { 60,  80,  2,   0.6,  100,  130,  240,  60,  2, 5, SMOKETYPE_ELECTRICAL,   0x809e, 1     },
	/*05*/ { 60,  120, 2,   0.6,  150,  160,  280,  60,  2, 5, SMOKETYPE_ELECTRICAL,   0x809e, 2     },
	/*06*/ { 20,  20,  0,   0,    22,   40,   40,   60,  1, 3, SMOKETYPE_MINI,         0x8099, 0.5   },
	/*07*/ { 35,  40,  0,   0,    35,   70,   70,   60,  1, 4, SMOKETYPE_MINI,         0x809a, 1     },
	/*08*/ { 50,  80,  2,   0.6,  50,   100,  160,  60,  2, 5, SMOKETYPE_ELECTRICAL,   0x809e, 2     },
	/*09*/ { 60,  120, 2,   0.6,  50,   130,  180,  60,  2, 5, SMOKETYPE_ELECTRICAL,   0x809e, 2     },
	/*10*/ { 40,  40,  0.8, 0.5,  70,   80,   160,  80,  4, 5, SMOKETYPE_SMALL,        0x80a0, 1     },
	/*11*/ { 50,  50,  1.2, 0.8,  100,  100,  200,  90,  1, 4, SMOKETYPE_SMALL,        0x809e, 2     },
	/*12*/ { 70,  60,  2,   1.2,  150,  140,  280,  90,  2, 5, SMOKETYPE_MEDIUM,       0x809e, 4     },
	/*13*/ { 80,  60,  4,   1.4,  200,  200,  400,  90,  2, 5, SMOKETYPE_LARGE,        0x809f, 4     },
	/*14*/ { 50,  50,  0,   0,    120,  150,  300,  150, 4, 4, SMOKETYPE_SMALL,        0x809f, 4     },
	/*15*/ { 1,   1,   0,   0,    1,    0,    0,    1,   1, 1, SMOKETYPE_BULLETIMPACT, 0x809c, 0     },
	/*16*/ { 1,   1,   0,   0,    1,    0,    0,    1,   1, 1, SMOKETYPE_BULLETIMPACT, 0x809c, 0     },
	/*17*/ { 80,  60,  10,  5,    1500, 2200, 3600, 500, 1, 2, SMOKETYPE_NONE,         0x80a5, 4     },
	/*18*/ { 80,  60,  3,   1,    300,  450,  640,  60,  1, 2, SMOKETYPE_NONE,         0x809f, 4     },
	/*19*/ { 90,  75,  2.5, 0.87, 250,  375,  600,  180, 2, 5, SMOKETYPE_LARGE,        0x809f, 4     },
	/*20*/ { 160, 120, 6,   2,    600,  450,  640,  60,  1, 2, SMOKETYPE_NONE,         0x809f, 4     },
	/*21*/ { 40,  30,  2,   0.7,  100,  140,  270,  45,  2, 5, SMOKETYPE_SMALL,        0x809f, 3.5   },
	/*22*/ { 20,  20,  0,   0,    30,   100,  200,  40,  1, 3, SMOKETYPE_MINI,         0x8099, 0.25  },
	/*23*/ { 100, 80,  4,   1.4,  210,  220,  500,  90,  2, 5, SMOKETYPE_LARGE,        0x809f, 4     },
	/*24*/ { 80,  60,  4,   1.4,  500,  200,  400,  90,  2, 5, SMOKETYPE_LARGE,        0x809f, 4     },
	/*25*/ { 640, 480, 32,  11.2, 1600, 1000, 1000, 180, 2, 5, SMOKETYPE_NONE,         0x80a4, 4     },
};

bool explosionCreateSimple(struct prop *prop, struct coord *pos, RoomNum *rooms, s16 type, s32 playernum)
{
	return explosionCreate(prop, pos, rooms, type, playernum, false, NULL, 0, NULL);
}

bool explosionCreateComplex(struct prop *prop, struct coord *pos, RoomNum *rooms, s16 type, s32 playernum)
{
	struct coord sp100;
	struct coord sp88;
	struct explosiontype *etype;
	bool makescorch = true;
	RoomNum room;
	f32 y;
	struct prop *collisionprop;

	if (type == EXPLOSIONTYPE_NONE) {
		return false;
	}

	if (prop) {
		room = cdFindFloorRoomYColourNormalPropAtPos(&prop->pos, prop->rooms, &y, NULL, &sp88, &collisionprop);
		sp100.x = prop->pos.x;
		sp100.y = y;
		sp100.z = prop->pos.z;
	} else {
		room = cdFindFloorRoomYColourNormalPropAtPos(pos, rooms, &y, NULL, &sp88, &collisionprop);
		sp100.x = pos->x;
		sp100.y = y;
		sp100.z = pos->z;
	}

	etype = &g_ExplosionTypes[type];

	if (collisionprop || room <= 0
			|| !(pos->y - y <= (etype->rangev + etype->changeratev * etype->duration + etype->innersize) * 0.5f || pos->y - y <= 75)) {
		makescorch = false;
	}

	return explosionCreate(prop, pos, rooms, type, playernum, makescorch, &sp100, room, &sp88);
}

f32 explosionGetHorizontalRangeAtFrame(struct explosion *exp, s32 frame)
{
	struct explosiontype *type = &g_ExplosionTypes[exp->type];
	f32 changerate = PALUPF(type->changerateh);
	f32 result;

	if (exp->type == EXPLOSIONTYPE_GASBARREL && frame > TICKS(32)) {
		result = frame * PALUPF(3.0f) + 40.0f;

		if (result > 300) {
			result = 300;
		}
	} else {
		result = type->rangeh + changerate * frame;
	}

	return result;
}

f32 explosionGetVerticalRangeAtFrame(struct explosion *exp, s32 frame)
{
	struct explosiontype *type = &g_ExplosionTypes[exp->type];
	f32 changerate = PALUPF(type->changeratev);
	f32 result;

	if (exp->type == EXPLOSIONTYPE_GASBARREL && frame > TICKS(32)) {
		result = 20;
	} else {
		result = type->rangev + changerate * frame;
	}

	return result;
}

void explosionGetBboxAtFrame(struct coord *lower, struct coord *upper, s32 frame, struct prop *prop)
{
	struct explosion *exp = prop->explosion;
	struct explosiontype *type = &g_ExplosionTypes[exp->type];

	f32 rangeh = explosionGetHorizontalRangeAtFrame(exp, frame);
	f32 rangev = explosionGetVerticalRangeAtFrame(exp, frame);

	rangeh = rangeh * 0.5f + type->innersize * 1.5f;
	rangev = rangev * 0.5f + type->innersize * 1.5f;

	lower->x = prop->pos.x - rangeh;
	lower->y = prop->pos.y - rangev;
	lower->z = prop->pos.z - rangeh;

	upper->x = prop->pos.x + rangeh;
	upper->y = prop->pos.y + rangev;
	upper->z = prop->pos.z + rangeh;
}

void explosionAlertChrs(f32 *radius, struct coord *noisepos)
{
	u32 stack[2];
	s32 *end = (s32 *)&doorDestroyGlass;
	s32 i;

	for (i = 0; i < g_NumChrSlots; i++) {
		if (g_ChrSlots[i].model
				&& chrGetTargetProp(&g_ChrSlots[i]) == g_Vars.currentplayer->prop
				&& g_ChrSlots[i].prop
				&& g_ChrSlots[i].prop->type == PROPTYPE_CHR
				&& (g_ChrSlots[i].prop->flags & PROPFLAG_ENABLED)) {
			f32 distance = chrGetDistanceToCoord(&g_ChrSlots[i], noisepos);

			if (distance == 0) {
				distance = 2;
			} else {
				distance = (10.0f * *radius * g_ChrSlots[i].hearingscale) / distance;
			}

			if (distance > 1) {
				chrRecordLastHearTargetTime(&g_ChrSlots[i]);
			}
		}
	}

#if PIRACYCHECKS
	{
		u32 checksum = 0;
		s32 *ptr = (s32 *)&glassDestroy;

		while (ptr < end) {
			checksum ^= *ptr;
			checksum <<= 1;
			ptr++;
		}

		if (checksum != CHECKSUM_PLACEHOLDER) {
			struct explosiontype *type = &g_ExplosionTypes[0];
			s32 i;

			for (i = 0; i != ARRAYCOUNT(g_ExplosionTypes) - 1; i++) {
				type->rangeh = 80;
				type->rangev = 60;
				type->changerateh = 15;
				type->changeratev = 5;
				type->innersize = 1500;
				type->blastradius = 200;
				type->damageradius = 3600;
				type++;
			}
		}
	}
#endif
}

bool explosionCreate(struct prop *sourceprop, struct coord *exppos, RoomNum *exprooms,
		s16 type, s32 playernum, bool makescorch, struct coord *arg6, RoomNum room, struct coord *arg8)
{
	u32 stack;
	struct explosion *exp = NULL;
	s32 i;

	if (type == EXPLOSIONTYPE_NONE || exprooms[0] == -1) {
		return false;
	}

	// Bullet holes: only crate the flame (explosion) if within 4 metres
	if (type == EXPLOSIONTYPE_BULLETHOLE) {
		f32 lodscale = camGetLodScaleZ();
		struct coord *campos = &g_Vars.currentplayer->cam_pos;
		f32 xdist = exppos->x - campos->x;
		f32 ydist = exppos->y - campos->y;
		f32 zdist = exppos->z - campos->z;
		f32 sum = xdist * xdist + ydist * ydist + zdist * zdist;

		if (sum * lodscale * lodscale > 400 * 400) {
			if (random() % 2 == 0) {
				if (sourceprop) {
					smokeCreateSimple(&sourceprop->pos, sourceprop->rooms, g_ExplosionTypes[type].smoketype);
				} else {
					smokeCreateSimple(exppos, exprooms, g_ExplosionTypes[type].smoketype);
				}
			}

			return true;
		}
	}

	// Try to find an unused slot
	for (i = 0; i < g_MaxExplosions; i++) {
		if (g_Explosions[i].prop == NULL) {
			exp = &g_Explosions[i];
			break;
		}
	}

	// If there's no unused slots, find the oldest bullethole flame and replace it
	if (exp == NULL) {
		s32 maxage = -1;
		s32 index = -1;

		for (i = 0; i < g_MaxExplosions; i++) {
			if (g_Explosions[i].type == EXPLOSIONTYPE_BULLETHOLE && g_Explosions[i].age > maxage) {
				maxage = g_Explosions[i].age;
				index = i;
			}
		}

		if (index >= 0) {
			propExecuteTickOperation(g_Explosions[index].prop, TICKOP_FREE);
			g_Explosions[index].prop = NULL;
			exp = &g_Explosions[index];
		}
	}

	if (exp) {
		struct prop *expprop = propAllocate();

		if (type != EXPLOSIONTYPE_16 && type != EXPLOSIONTYPE_BULLETHOLE) {
			g_ExplosionShakeTotalTimer = SHAKE_TIME;
		}

		if (expprop) {
			s32 exproom;
			f32 value1;
			f32 value2;
			s32 k;
			s32 index;
			s32 portalnum;
			s32 indexplus1;
			s32 indexplus2;
			struct coord portalbbmin;
			struct coord portalbbmax;
			struct coord portal2bbmin;
			struct coord portal2bbmax;
			struct coord spd4;
			struct coord spc8;
			RoomNum otherroom;
			RoomNum otherroom2;
			f32 mult = 1;
			s32 stack4;
			s32 portalnum2;
			struct coord spac;
			u32 stack2;
			u32 stack3;
			s32 j;

			expprop->type = PROPTYPE_EXPLOSION;
			expprop->explosion = exp;
			expprop->pos.x = exppos->x;
			expprop->pos.y = exppos->y;
			expprop->pos.z = exppos->z;

			for (i = 0; exprooms[i] != -1 && i < ARRAYCOUNT(expprop->rooms) - 1; i++) {
				expprop->rooms[i] = exprooms[i];

				roomFlashLighting(exprooms[i], g_ExplosionTypes[type].rangeh, 255);
			}

			expprop->rooms[i] = -1;

			propActivateThisFrame(expprop);
			propEnable(expprop);

			exp->type = type;
			exp->prop = expprop;
			exp->source = sourceprop;
			exp->age = 0;
			exp->makescorch = makescorch;
			exp->owner = playernum;

			if (type != EXPLOSIONTYPE_BULLETHOLE && type != EXPLOSIONTYPE_PHOENIX) {
				propSetDangerous(expprop);
			}

			exproom = expprop->rooms[0];

			explosionGetBboxAtFrame(&spd4, &spc8, g_ExplosionTypes[type].duration, expprop);

			spd4.x *= mult;
			spd4.y *= mult;
			spd4.z *= mult;

			spc8.x *= mult;
			spc8.y *= mult;
			spc8.z *= mult;

			exp->bbs[0].bbmin.x = g_Rooms[exproom].bbmin[0];
			exp->bbs[0].bbmin.y = g_Rooms[exproom].bbmin[1];
			exp->bbs[0].bbmin.z = g_Rooms[exproom].bbmin[2];
			exp->bbs[0].bbmax.x = g_Rooms[exproom].bbmax[0];
			exp->bbs[0].bbmax.y = g_Rooms[exproom].bbmax[1];
			exp->bbs[0].bbmax.z = g_Rooms[exproom].bbmax[2];
			exp->bbs[0].room = exproom;
			exp->bbs[0].room2 = -1;
			exp->numbb = 1;

			if (exp->type == EXPLOSIONTYPE_HUGE25) {
				exp->numbb = 0;
			} else {
				exp->bbs[0].bbmin.x = g_Rooms[exproom].bbmin[0];
				exp->bbs[0].bbmin.y = g_Rooms[exproom].bbmin[1];
				exp->bbs[0].bbmin.z = g_Rooms[exproom].bbmin[2];
				exp->bbs[0].bbmax.x = g_Rooms[exproom].bbmax[0];
				exp->bbs[0].bbmax.y = g_Rooms[exproom].bbmax[1];
				exp->bbs[0].bbmax.z = g_Rooms[exproom].bbmax[2];
				exp->bbs[0].room = exproom;
				exp->bbs[0].room2 = -1;
				exp->numbb = 1;

				for (k = 0; k < g_Rooms[exproom].numportals; k++) {
					portalnum = g_RoomPortals[g_Rooms[exproom].roomportallistoffset + k];

					bgCalculatePortalBbox(portalnum, &portalbbmin, &portalbbmax);

					if (bgIsBboxOverlapping(&portalbbmin, &portalbbmax, &spd4, &spc8)) {
						otherroom2 = -1;
						index = 0;

						if (exproom == g_BgPortals[portalnum].roomnum1) {
							otherroom = g_BgPortals[portalnum].roomnum2;
						} else {
							otherroom = g_BgPortals[portalnum].roomnum1;
						}

						spac.f[0] = (g_PortalMetrics + portalnum)->normal.f[0];
						spac.f[1] = (g_PortalMetrics + portalnum)->normal.f[1];
						spac.f[2] = (g_PortalMetrics + portalnum)->normal.f[2];

						if (spac.f[0] < 0.0f) {
							spac.f[0] = -spac.f[0];
						}

						if (spac.f[1] < 0.0f) {
							spac.f[1] = -spac.f[1];
						}

						if (spac.f[2] < 0.0f) {
							spac.f[2] = -spac.f[2];
						}

						if (spac.f[0] < spac.f[1]) {
							index = 1;
						}

						if (spac.f[index] < spac.f[2]) {
							index = 2;
						}

						indexplus1 = (index + 1) % 3;
						indexplus2 = (index + 2) % 3;

						value1 = portalbbmax.f[indexplus1] - portalbbmin.f[indexplus1];
						value2 = portalbbmax.f[indexplus2] - portalbbmin.f[indexplus2];

						if (value2 < value1) {
							value1 = value2;
						}

						portalbbmin.f[index] -= value1;
						portalbbmax.f[index] += value1;

						if (portalbbmin.f[index] < g_Rooms[exproom].bbmin[index]) {
							portalbbmin.f[index] = g_Rooms[exproom].bbmin[index];
						}

						if (portalbbmax.f[index] > g_Rooms[exproom].bbmax[index]) {
							portalbbmax.f[index] = g_Rooms[exproom].bbmax[index];
						}

						if (portalbbmin.f[index] > g_Rooms[otherroom].bbmin[index]) {
							portalbbmin.f[index] = g_Rooms[otherroom].bbmin[index];
						}

						if (portalbbmax.f[index] < g_Rooms[otherroom].bbmax[index]) {
							portalbbmax.f[index] = g_Rooms[otherroom].bbmax[index];
						}

						for (j = 0; j < g_Rooms[otherroom].numportals; j++) {
							portalnum2 = g_RoomPortals[g_Rooms[otherroom].roomportallistoffset + j];

							if (portalnum2 != portalnum) {
								bgCalculatePortalBbox(portalnum2, &portal2bbmin, &portal2bbmax);

								if (portal2bbmin.f[indexplus1] <= portalbbmin.f[indexplus1] + 10.0f * mult
										&& portal2bbmin.f[indexplus2] <= portalbbmin.f[indexplus2] + 10.0f * mult
										&& portal2bbmax.f[indexplus1] >= portalbbmax.f[indexplus1] - 10.0f * mult
										&& portal2bbmax.f[indexplus2] >= portalbbmax.f[indexplus2] - 10.0f * mult) {
									if (otherroom == g_BgPortals[portalnum2].roomnum1) {
										otherroom2 = g_BgPortals[portalnum2].roomnum2;
									} else {
										otherroom2 = g_BgPortals[portalnum2].roomnum1;
									}

									if (portalbbmin.f[index] > g_Rooms[otherroom2].bbmin[index]) {
										portalbbmin.f[index] = g_Rooms[otherroom2].bbmin[index];
									}

									if (portalbbmax.f[index] < g_Rooms[otherroom2].bbmax[index]) {
										portalbbmax.f[index] = g_Rooms[otherroom2].bbmax[index];
									}
									break;
								}
							}
						}

						portalbbmin.f[0] *= mult;
						portalbbmin.f[1] *= mult;
						portalbbmin.f[2] *= mult;

						portalbbmax.f[0] *= mult;
						portalbbmax.f[1] *= mult;
						portalbbmax.f[2] *= mult;

						exp->bbs[exp->numbb].bbmin.x = portalbbmin.f[0];
						exp->bbs[exp->numbb].bbmin.y = portalbbmin.f[1];
						exp->bbs[exp->numbb].bbmin.z = portalbbmin.f[2];
						exp->bbs[exp->numbb].bbmax.x = portalbbmax.f[0];
						exp->bbs[exp->numbb].bbmax.y = portalbbmax.f[1];
						exp->bbs[exp->numbb].bbmax.z = portalbbmax.f[2];
						exp->bbs[exp->numbb].room = otherroom;
						exp->bbs[exp->numbb].room2 = otherroom2;

						exp->numbb++;

						if (exp->numbb >= 5) {
							break;
						}
					}
				}

				exp->bbs[0].bbmin.x *= mult;
				exp->bbs[0].bbmin.y *= mult;
				exp->bbs[0].bbmin.z *= mult;
				exp->bbs[0].bbmax.x *= mult;
				exp->bbs[0].bbmax.y *= mult;
				exp->bbs[0].bbmax.z *= mult;
			}

			if (makescorch) {
				exp->unk3d0.x = arg6->x;
				exp->unk3d0.y = arg6->y;
				exp->unk3d0.z = arg6->z;
				exp->room = room;
				exp->unk3dc.x = arg8->x;
				exp->unk3dc.y = arg8->y;
				exp->unk3dc.z = arg8->z;
			} else {
				exp->unk3d0.x = 999999.875f;
			}

			exp->parts[0].frame = 1;
			exp->parts[0].pos.x = exppos->x;
			exp->parts[0].pos.y = exppos->y;
			exp->parts[0].pos.z = exppos->z;
			exp->parts[0].size = g_ExplosionTypes[type].innersize * (RANDOMFRAC() * 0.5f + 1);
			exp->parts[0].rot = RANDOMFRAC() * M_BADTAU;
			exp->parts[0].bb = 0;

			if (g_Vars.mplayerisrunning) {
				smokeClearSomeTypes();
			}

			explosionAlertChrs(&g_ExplosionTypes[type].rangeh, exppos);
		}
	}

	return exp != NULL;
}

/**
 * Start a shake without any explosion.
 *
 * This function is unused.
 */
void explosionShake(void)
{
	g_ExplosionShakeTotalTimer = SHAKE_TIME;
	g_ExplosionShakeIntensityTimer = SHAKE_TIME;
}

void explosionsUpdateShake(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	u32 stack[4];
	f32 sp54;
	f32 sp50;
	s32 i;
	f32 intensity;

	if (g_ExplosionShakeTotalTimer == 0) {
		viShake(0);
		return;
	}

	sp54 = cosf(0.8f) * arg1->f[0] - sinf(0.8f) * arg1->f[2];
	sp50 = sinf(0.8f) * arg1->f[0] + cosf(0.8f) * arg1->f[2];

	intensity = 0.0f;

	for (i = 0; i < g_MaxExplosions; i++) {
		struct prop *prop = g_Explosions[i].prop;

		if (prop) {
			f32 xdiff = prop->pos.x - arg0->x;
			f32 ydiff = prop->pos.y - arg0->y;
			f32 zdiff = prop->pos.z - arg0->z;

			f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
			f32 mult;

			if (dist == 0.0f) {
				dist = 0.0001f;
			}

			mult = g_ExplosionTypes[g_Explosions[i].type].innersize / dist;

			intensity += mult * 15.0f;
		}
	}

	if (g_ExplosionShakeIntensityTimer > 0) {
		g_ExplosionShakeIntensityTimer--;
		intensity++;
	}

	g_ExplosionShakeTotalTimer--;

	if (g_ExplosionShakeTotalTimer & 2) {
		arg2->y = intensity;
		intensity = -intensity;
	} else {
		arg2->y = -intensity;
	}

	arg2->x = intensity * sp54;
	arg2->z = intensity * sp50;

	viShake(g_ExplosionShakeTotalTimer * intensity);
}

/**
 * Check if a prop is either fully or partially overlapping the given explosion.
 *
 * minpos and maxpos are the bounding boxes of the prop.
 */
bool explosionOverlapsProp(struct explosion *exp, struct prop *prop, struct coord *minpos, struct coord *maxpos)
{
	bool result = false;
	s32 i;
	RoomNum rooms[8];

	if (exp->type == EXPLOSIONTYPE_HUGE25) {
		result = true;
	} else {
		for (i = 0; i < exp->numbb; i++) {
			rooms[0] = exp->bbs[i].room;

			if (exp->bbs[i].room2 != -1) {
				rooms[1] = exp->bbs[i].room2;
			} else {
				rooms[1] = -1;
			}

			rooms[2] = -1;

			if (arrayIntersects(prop->rooms, rooms)
					&& minpos->x <= exp->bbs[i].bbmax.x
					&& minpos->y <= exp->bbs[i].bbmax.y
					&& minpos->z <= exp->bbs[i].bbmax.z
					&& maxpos->x >= exp->bbs[i].bbmin.x
					&& maxpos->y >= exp->bbs[i].bbmin.y
					&& maxpos->z >= exp->bbs[i].bbmin.z) {
				result = true;
				break;
			}
		}
	}

	return result;
}

void explosionInflictDamage(struct prop *expprop)
{
	s32 stack;
	struct explosion *exp = expprop->explosion;
	struct explosiontype *type = &g_ExplosionTypes[exp->type];
	s16 *propnumptr;
	s16 propnums[256];
	bool isfirstframe = exp->age <= 0;
	s32 i;
	f32 k;
	s32 j;
	f32 damageradius;

	if (g_Vars.lvupdate60 <= 0) {
		return;
	}

	if (type->damage <= 0.0f) {
		return;
	}

	if (isfirstframe) {
		damageradius = type->damageradius;
	} else {
#if PAL
		damageradius = type->blastradius + (type->damageradius - type->blastradius) * exp->age / (type->duration * 0.8333333f);
#else
		damageradius = type->blastradius + (type->damageradius - type->blastradius) * exp->age / type->duration;
#endif

		if (damageradius > type->damageradius) {
			damageradius = type->damageradius;
		}
	}

#if PAL
	if (exp->age > (s32)((type->duration + 7.0f * type->flarespeed) * 0.8333333f)) {
		return;
	}
#else
	if (exp->age > (s32)(type->duration + 7.0f * type->flarespeed)) {
		return;
	}
#endif

	// Flicker room lighting
	for (i = 0; expprop->rooms[i] != -1; i++) {
		if (random() % 2048 <= 240) {
			roomFlashLighting(expprop->rooms[i], type->rangeh, 255);
		}
	}

	// Break lights
	for (i = 0; expprop->rooms[i] != -1; i++) {
		s32 roomnum = expprop->rooms[i];

		if (roomnum != 0) {
			s32 numlights = g_Rooms[roomnum].numlights;
			f32 xdist = expprop->pos.f[0];
			f32 ydist = expprop->pos.f[1];
			f32 zdist = expprop->pos.f[2];
			struct coord sp164;

			xdist -= g_BgRooms[roomnum].pos.f[0];
			ydist -= g_BgRooms[roomnum].pos.f[1];
			zdist -= g_BgRooms[roomnum].pos.f[2];

			for (j = 0; j < numlights; j++) {
				if (lightIsHealthy(roomnum, j)
						&& lightIsVulnerable(roomnum, j)
						&& lightGetBboxCentre(roomnum, j, &sp164)) {
					struct coord sp158;
					struct coord sp14c;

					sp14c.f[0] = sp164.f[0] - xdist; \
					sp14c.f[1] = sp164.f[1] - ydist; \
					sp14c.f[2] = sp164.f[2] - zdist;

					sp158.f[0] = damageradius;
					sp158.f[1] = damageradius;
					sp158.f[2] = damageradius;

					if (func0f1773c8(&sp14c, &sp158)) {
						roomSetLightBroken(roomnum, j);
					}
				}
			}
		}
	}

	// Damage props
	roomGetProps(expprop->rooms, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (prop != exp->source && prop->timetoregen == 0) {
			if (prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_WEAPON || prop->type == PROPTYPE_DOOR) {
				f32 xdist;
				f32 ydist;
				f32 zdist;
				struct coord sp130;
				struct coord sp124;
				struct defaultobj *obj = prop->obj;
				bool candamage = false;

				xdist = prop->pos.x - expprop->pos.x;
				ydist = prop->pos.y - expprop->pos.y;
				zdist = prop->pos.z - expprop->pos.z;

				if (candamage);

#if VERSION >= VERSION_NTSC_1_0
				if (obj)
#endif
				{
					if (xdist <= damageradius && xdist >= -damageradius
							&& ydist <= damageradius && ydist >= -damageradius
							&& zdist <= damageradius && zdist >= -damageradius) {

						if (setup0f092304(obj, &sp130, &sp124)) {
							if (explosionOverlapsProp(exp, prop, &sp130, &sp124)) {
								candamage = true;
							}
						} else {
							candamage = true;
						}
					}

					if (candamage && prop->type == PROPTYPE_WEAPON) {
						struct weaponobj *weapon = prop->weapon;

						if (weapon && weapon->weaponnum == WEAPON_SKROCKET) {
							weapon->timer240 = 0;
						}
					}

					if (candamage) {
						f32 f0;
						f32 xfrac;
						f32 yfrac;
						f32 zfrac;
						f32 minfrac;

						xfrac = f0 = xdist / damageradius;

						if (xfrac < 0.0f) {
							xfrac = -xfrac;
						}

						xfrac = 1.0f - xfrac;

						yfrac = f0 = ydist / damageradius;

						if (yfrac < 0.0f) {
							yfrac = -yfrac;
						}

						yfrac = 1.0f - yfrac;

						zfrac = f0 = zdist / damageradius;

						if (zfrac < 0.0f) {
							zfrac = -zfrac;
						}

						zfrac = 1.0f - zfrac;

						minfrac = xfrac;

						if (yfrac < minfrac) {
							minfrac = yfrac;
						}

						if (zfrac < minfrac) {
							minfrac = zfrac;
						}

						minfrac = (minfrac * 0.7f + 0.3f) * type->damage;

						if (g_Vars.antiplayernum >= 0
								&& g_Vars.antiplayernum == exp->owner // NOTE: this might cause issues
								&& (obj->flags2 & OBJFLAG2_IMMUNETOANTI)) {
							// anti cannot damage this obj
						} else if (isfirstframe) {
							// Unblock path if this object is a path blocker
							objUpdateLinkedScenery(obj, expprop);

							// Damage the object
							if ((obj->hidden & OBJHFLAG_00001000) == 0
									&& (obj->flags2 & (OBJFLAG2_LINKEDTOSAFE | OBJFLAG2_IMMUNETOEXPLOSIONS)) == 0) {
								func0f085050(prop, (RANDOMFRAC() * 0.5f + 1.0f) * minfrac, &prop->pos, 0x22, exp->owner);
							}

							// Give object momentum if it's a hover obj
							if ((obj->hidden & OBJHFLAG_MOUNTED) == 0
									&& (obj->hidden & OBJHFLAG_GRABBED) == 0
									&& (obj->flags3 & OBJFLAG3_PUSHABLE)) {
								f32 dist;
								struct coord spf4;
								spf4.x = prop->pos.x - expprop->pos.x;
								spf4.y = 0.0f;
								spf4.z = prop->pos.z - expprop->pos.z;

								if (spf4.f[0] != 0.0f || spf4.f[2] != 0.0f) {
									dist = sqrtf(spf4.f[0] * spf4.f[0] + spf4.f[2] * spf4.f[2]);

									if (dist > 0.0f) {
										f32 tmp = minfrac * 4.0f / dist;
										spf4.x *= tmp;
										spf4.z *= tmp;
									}
								}

								objApplyMomentum(obj, &spf4, 0.0f, true, true);
							}
						} else if (objIsHealthy(obj)) {
							// Sustained damage
							minfrac *= 0.05f * g_Vars.lvupdate60freal;

							if ((obj->hidden & OBJHFLAG_00001000) == 0
									&& (obj->flags2 & (OBJFLAG2_LINKEDTOSAFE | OBJFLAG2_IMMUNETOEXPLOSIONS)) == 0) {
								func0f085050(prop, (RANDOMFRAC() * 0.5f + 1.0f) * minfrac,
										&prop->pos, 0x22, exp->owner);
							}
						}
					}
				}
			} else if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
				f32 xdist = prop->pos.f[0] - expprop->pos.f[0];
				f32 ydist = prop->pos.f[1] - expprop->pos.f[1];
				f32 zdist = prop->pos.f[2] - expprop->pos.f[2];
				f32 radius;
				f32 ymax;
				f32 ymin;
				struct coord spcc;
				struct coord spc0;

				bool candamage = false;

				if (prop->type == PROPTYPE_CHR);

				if (xdist <= damageradius && xdist >= -damageradius
						&& ydist <= damageradius && ydist >= -damageradius
						&& zdist <= damageradius && zdist >= -damageradius) {
					propGetBbox(prop, &radius, &ymax, &ymin);

					radius -= 20.0f;

					if (radius <= 0.0f) {
						radius = 0.0f;
					}

					spcc.f[0] = prop->pos.f[0] - radius;
					spcc.f[1] = ymin;
					spcc.f[2] = prop->pos.f[2] - radius;

					spc0.f[0] = prop->pos.f[0] + radius;
					spc0.f[1] = ymax;
					spc0.f[2] = prop->pos.f[2] + radius;

					if (explosionOverlapsProp(exp, prop, &spcc, &spc0)) {
						candamage = true;
					}
				}

				if (candamage) {
					struct prop *ownerprop = NULL;
					f32 xfrac = xdist / damageradius;
					f32 yfrac = ydist / damageradius;
					f32 zfrac = zdist / damageradius;
					struct coord spa0 = {0, 0, 0};
					struct chrdata *chr = prop->chr;
					f32 minfrac;

					if (xfrac < 0.0f) {
						xfrac = -xfrac;
					}

					if (yfrac < 0.0f) {
						yfrac = -yfrac;
					}

					if (zfrac < 0.0f) {
						zfrac = -zfrac;
					}

					xfrac = 1.0f - xfrac;
					yfrac = 1.0f - yfrac;
					zfrac = 1.0f - zfrac;

					minfrac = xfrac;

					if (yfrac < minfrac) {
						minfrac = yfrac;
					}

					if (zfrac < minfrac) {
						minfrac = zfrac;
					}

					minfrac *= minfrac;
					minfrac = minfrac * type->damage * 8.0f;

					if (isfirstframe) {
						if (xdist != 0.0f || zdist != 0.0f) {
							f32 dist = sqrtf(xdist * xdist + zdist * zdist);

							if (dist > 0.0f) {
								xdist *= 1.0f / dist;
								zdist *= 1.0f / dist;

								spa0.x = xdist;
								spa0.y = 0.0f;
								spa0.z = zdist;
							}
						}
					} else {
						minfrac *= 0.05f * g_Vars.lvupdate60freal;
					}

					if (g_Vars.normmplayerisrunning) {
						struct chrdata *ownerchr = mpGetChrFromPlayerIndex(exp->owner);

						if (ownerchr) {
							ownerprop = ownerchr->prop;
						}
					} else if (exp->owner == g_Vars.bondplayernum) {
						ownerprop = g_Vars.bond->prop;
					} else if (g_Vars.coopplayernum >= 0 && exp->owner == g_Vars.coopplayernum) {
						ownerprop = g_Vars.coop->prop;
					} else if (g_Vars.antiplayernum >= 0 && exp->owner == g_Vars.antiplayernum) {
						ownerprop = g_Vars.anti->prop;
					}

					chrDamageByExplosion(chr, minfrac, &spa0, ownerprop, &expprop->pos);

					if (prop->type == PROPTYPE_CHR && !isfirstframe) {
						chrDisfigure(chr, &expprop->pos, damageradius);
					}
				}
			}
		}

		propnumptr++;
	}
}

u32 explosionTick(struct prop *prop)
{
	struct explosion *exp = prop->explosion;
	struct explosiontype *type = &g_ExplosionTypes[exp->type];
	s32 i;
	s32 j;
	s32 k;
	f32 hrange;
	f32 vrange;
	f32 lvupdate;
	struct coord bbmin;
	struct coord bbmax;
	s16 maxage;
	s32 numpartstocreate;
	struct coord sp11c;
	struct coord sp110;
	u32 stack[2];
	struct coord spfc;
	struct coord spf0;
	s32 bb;
	bool xlu;
	struct chrdata *chr;
	f32 scorchsize;
	struct hitthing hitthing;

	maxage = TICKS(type->duration);

	if (g_Vars.lvupdate60 == 0) {
		return TICKOP_NONE;
	}

	lvupdate = g_Vars.lvupdate60 < TICKS(15) ? g_Vars.lvupdate60 : (s32)TICKS(15);

#if PAL
	if (exp->age >= 7 && exp->age < maxage)
#else
	if (exp->age >= 8 && exp->age < maxage)
#endif
	{
		hrange = explosionGetHorizontalRangeAtFrame(exp, exp->age);
		vrange = explosionGetVerticalRangeAtFrame(exp, exp->age);

		sp11c.x = prop->pos.x - hrange * 0.5f;
		sp11c.y = prop->pos.y - vrange * 0.5f;
		sp11c.z = prop->pos.z - hrange * 0.5f;

		sp110.x = prop->pos.x + hrange * 0.5f;
		sp110.y = prop->pos.y + vrange * 0.5f;
		sp110.z = prop->pos.z + hrange * 0.5f;

		// Barrel explosions ascend upwards
		if (exp->type == EXPLOSIONTYPE_GASBARREL && exp->age < TICKS(32)) {
			prop->pos.y += PALUPF(10.0f) * lvupdate;
		}

		// Create new parts
		numpartstocreate = (s32)((f32)type->propagationrate * exp->age / maxage) + 1;

		for (i = 0; i < numpartstocreate; i++) {
			for (j = 0; j < 40; j++) {
				if (exp->parts[j].frame == 0) {
					if (exp->numbb == 0 || exp->type == EXPLOSIONTYPE_HUGE25) {
						spfc.f[0] = sp11c.f[0];
						spfc.f[1] = sp11c.f[1];
						spfc.f[2] = sp11c.f[2];

						spf0.f[0] = sp110.f[0];
						spf0.f[1] = sp110.f[1];
						spf0.f[2] = sp110.f[2];

						bb = 0;
					} else {
						bb = j % exp->numbb;

						spfc.x = exp->bbs[bb].bbmin.x;
						spfc.y = exp->bbs[bb].bbmin.y;
						spfc.z = exp->bbs[bb].bbmin.z;

						spf0.x = exp->bbs[bb].bbmax.x;
						spf0.y = exp->bbs[bb].bbmax.y;
						spf0.z = exp->bbs[bb].bbmax.z;

						if (spfc.x < sp11c.x) {
							spfc.x = sp11c.x;
						}

						if (spfc.y < sp11c.y) {
							spfc.y = sp11c.y;
						}

						if (spfc.z < sp11c.z) {
							spfc.z = sp11c.z;
						}

						if (sp110.x < spf0.x) {
							spf0.x = sp110.x;
						}

						if (sp110.y < spf0.y) {
							spf0.y = sp110.y;
						}

						if (sp110.z < spf0.z) {
							spf0.z = sp110.z;
						}

						if (spf0.x <= spfc.x || spf0.y <= spfc.y || spf0.z <= spfc.z) {
							bb = 0;

							spfc.x = exp->bbs[bb].bbmin.x;
							spfc.y = exp->bbs[bb].bbmin.y;
							spfc.z = exp->bbs[bb].bbmin.z;

							spf0.x = exp->bbs[bb].bbmax.x;
							spf0.y = exp->bbs[bb].bbmax.y;
							spf0.z = exp->bbs[bb].bbmax.z;

							if (spfc.x < sp11c.x) {
								spfc.x = sp11c.x;
							}

							if (spfc.y < sp11c.y) {
								spfc.y = sp11c.y;
							}

							if (spfc.z < sp11c.z) {
								spfc.z = sp11c.z;
							}

							if (sp110.x < spf0.x) {
								spf0.x = sp110.x;
							}

							if (sp110.y < spf0.y) {
								spf0.y = sp110.y;
							}

							if (sp110.z < spf0.z) {
								spf0.z = sp110.z;
							}
						}
					}

					exp->parts[j].pos.f[0] = spfc.f[0] + RANDOMFRAC() * (spf0.f[0] - spfc.f[0]);
					exp->parts[j].pos.f[1] = spfc.f[1] + RANDOMFRAC() * (spf0.f[1] - spfc.f[1]);
					exp->parts[j].pos.f[2] = spfc.f[2] + RANDOMFRAC() * (spf0.f[2] - spfc.f[2]);
					exp->parts[j].bb = bb;
					exp->parts[j].frame = 1;
					exp->parts[j].size = (1.0f + RANDOMFRAC() * 0.5f) * type->innersize;
					exp->parts[j].rot = RANDOMFRAC() * M_BADTAU;
					break;
				}
			}
		}
	}

	explosionGetBboxAtFrame(&bbmin, &bbmax, exp->age, prop);
	bgFindEnteredRooms(&bbmin, &bbmax, prop->rooms, 7, false);
	explosionInflictDamage(prop);

	// Play boom sound if this is the first frame
	if (exp->age == 0) {
		psCreate(NULL, NULL, type->sound, -1, -1, 0, 0, PSTYPE_NONE, &exp->prop->pos, -1.0f, exp->prop->rooms, -1, -1.0f, -1.0f, -1.0f);
	}

	for (k = 0; k < (s32)lvupdate; k++) {
		exp->age++;

		for (j = 0; j < 40; j++) {
			if (exp->parts[j].frame > 0) {
				exp->parts[j].frame++;
			}
		}

		// Create smoke
		if (((exp->age == TICKS(15) && exp->type == EXPLOSIONTYPE_GASBARREL)
					|| (exp->age == maxage - TICKS(20) && exp->type != EXPLOSIONTYPE_GASBARREL))
				&& (exp->type != EXPLOSIONTYPE_BULLETHOLE || (random() % 2) == 0)) {
			if (exp->source) {
				smokeCreateSimple(&exp->source->pos, exp->source->rooms, type->smoketype);
			} else {
				smokeCreateSimple(&prop->pos, prop->rooms, type->smoketype);
			}
		}

		// Make scorch at half duration
		if (exp->age == (maxage >> 1) && exp->makescorch) {
			xlu = false;
			scorchsize = 2.0f * type->innersize;

			if (scorchsize > 100.0f) {
				scorchsize = 100.0f;
			}

			scorchsize *= 0.8f + 0.2f * RANDOMFRAC();

			if (g_Vars.normmplayerisrunning) {
				chr = mpGetChrFromPlayerIndex(exp->owner);
			} else if (g_Vars.antiplayernum >= 0 && exp->owner == g_Vars.antiplayernum) {
				chr = g_Vars.anti->prop->chr;
			} else if (g_Vars.coopplayernum >= 0 && exp->owner == g_Vars.coopplayernum) {
				chr = g_Vars.coop->prop->chr;
			} else {
				chr = g_Vars.bond->prop->chr;
			}

			if (g_Rooms[exp->room].gfxdata) {
				if (g_Rooms[exp->room].gfxdata->xlublocks && bgTestHitInRoom(&prop->pos, &exp->unk3d0, exp->room, &hitthing)) {
					xlu = hitthing.unk2c == 2;
				}

				wallhitCreateWith20Args(&exp->unk3d0, &exp->unk3dc, &prop->pos, NULL,
						0, WALLHITTEX_SCORCH, exp->room, 0,
						0, -1, 0, chr,
						scorchsize, scorchsize, 0xff, 0xff,
						0, 0, 0, xlu);
			}
		}
	}

	// Free explosion if finished
#if PAL
	if (exp->age >= maxage + (s32)((16.0f * type->flarespeed) * 0.8333333f))
#else
	if (exp->age >= maxage + (s32)(16.0f * type->flarespeed))
#endif
	{
		if (exp->type != EXPLOSIONTYPE_BULLETHOLE) {
			propUnsetDangerous(exp->prop);
		}

		exp->prop = NULL;
		return TICKOP_FREE;
	}

	return TICKOP_NONE;
}

u32 explosionTickPlayer(struct prop *prop)
{
	Mtxf *matrix = camGetWorldToScreenMtxf();

	prop->z = -(matrix->m[0][2] * prop->pos.x + matrix->m[1][2] * prop->pos.y + matrix->m[2][2] * prop->pos.z + matrix->m[3][2]);

	if (prop->z < 100) {
		prop->z *= 0.5f;
	} else {
		prop->z -= 100;
	}

	prop->flags |= PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONTHISSCREENTHISTICK;

	return TICKOP_NONE;
}

Gfx *explosionRender(struct prop *prop, Gfx *gdl, bool xlupass)
{
	struct explosion *exp = prop->explosion;
	s32 roomnum;
	s32 i;
	s32 j;

	if (!xlupass) {
		return gdl;
	}

	j = 0;
	roomnum = prop->rooms[j];

	while (roomnum != -1) {
		if (g_Rooms[roomnum].gfxdata && g_Rooms[roomnum].loaded240 && (g_Rooms[roomnum].flags & ROOMFLAG_ONSCREEN)) {
			break;
		}

		j++;
		roomnum = prop->rooms[j];
	}

	if (roomnum != -1) {
		struct screenbox screenbox;
		struct coord *coord = roomGetPosPtr(roomnum);
		Col *colours;
		s32 tmp;

		if (func0f08e5a8(prop->rooms, &screenbox) > 0) {
			gdl = bgScissorWithinViewport(gdl, screenbox.xmin, screenbox.ymin, screenbox.xmax, screenbox.ymax);
		} else {
			gdl = bgScissorToViewport(gdl);
		}

		gSPClearGeometryMode(gdl++, G_CULL_BOTH | G_FOG);
		gSPMatrix(gdl++, osVirtualToPhysical(camGetOrthogonalMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		gdl = roomApplyMtx(gdl, roomnum);

		gSPDisplayList(gdl++, g_TexGdl2);

		colours = gfxAllocateColours(1);

		if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
			colours[0].word = 0xffffffff;
		} else if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
			u32 alpha = 0x80;
			u32 red;
			u32 green;
			f32 expdist = sqrtf(ERASERSQDIST(prop->pos.f));

			if (g_Vars.currentplayer->eraserpropdist < expdist) {
				return gdl;
			}

			if (g_Vars.currentplayer->eraserpropdist - 150.0f < expdist) {
				alpha = (1.0f - (expdist - (g_Vars.currentplayer->eraserpropdist - 150.0f)) / 150.0f) * 128.0f;
			}

			expdist = expdist / g_Vars.currentplayer->eraserpropdist;

			if (expdist > 1.0f) {
				expdist = 1.0f;
			}

			red = expdist * 127.0f;
			green = (1.0f - expdist) * 127.0f;

			colours[0].word = PD_BE32(red << 24 | green << 16 | alpha | 0x80800000);
		} else {
			static u32 var8007e93c = 0xffffffff;
			mainOverrideVariable("ecol", &var8007e93c);
			colours[0].word = 0xffffffff;
			colours[0].word = var8007e93c;
		}

		gSPColor(gdl++, osVirtualToPhysical(colours), 1);

		for (i = 14; i >= 0; i--) {
			gDPSetTextureImage(gdl++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, g_ExplosionTexturePairs[i].texturenum1);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 1567, 0);

			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, g_ExplosionTexturePairs[i].texturenum2);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, 5, 0, 0, 223, 0);

			gDPPipeSync(gdl++);

			for (j = 0; j < ARRAYCOUNT(exp->parts); j++) {
				if (exp->parts[j].frame > 0) {
#if PAL
					if (i == (s32)((f32)(exp->parts[j].frame - 1) / (g_ExplosionTypes[exp->type].flarespeed * 0.83333331346512f))) {
						gdl = explosionRenderPart(exp, &exp->parts[j], gdl, coord, i);
					}
#else
					if (i == (s32)((f32)(exp->parts[j].frame - 1) / g_ExplosionTypes[exp->type].flarespeed)) {
						gdl = explosionRenderPart(exp, &exp->parts[j], gdl, coord, i);
					}
#endif
				}
			}
		}

		gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

#if PAL
		tmp = (g_ExplosionTypes[exp->type].flarespeed * 15.0f) * 0.83333331346512f;
#else
		tmp = g_ExplosionTypes[exp->type].flarespeed * 15.0f;
#endif

		for (j = 0; j < ARRAYCOUNT(exp->parts); j++) {
			if (exp->parts[j].frame > tmp) {
				exp->parts[j].frame = 0;
			}
		}
	}

	return gdl;
}

Gfx *explosionRenderPart(struct explosion *exp, struct explosionpart *part, Gfx *gdl, struct coord *coord, s32 arg4)
{
	Vtx *vertices = gfxAllocateVertices(4);
	Mtxf *mtx = camGetProjectionMtxF();
	struct coord spbc;
	struct coord spb0;
	struct coord spa4;
	struct coord sp98;
	f32 x;
	f32 y;
	f32 z;
	s32 i;
	s32 j;
	f32 size;
	f32 cosine;
	f32 max;
	f32 size2;
	struct coord pos;
	f32 sine;
	s32 bbnum;
	f32 value;

	size = part->size;

	x = part->pos.x;
	y = part->pos.y;
	z = part->pos.z;

	if (exp->numbb > 0) {
		pos.x = x;
		pos.y = y;
		pos.z = z;

		bbnum = part->bb;
		max = 0.0f;

		for (i = 0; i < exp->numbb; i++) {
			if (pos.f[0] >= exp->bbs[i].bbmin.f[0] && pos.f[0] <= exp->bbs[i].bbmax.f[0]
					&& pos.f[1] >= exp->bbs[i].bbmin.f[1] && pos.f[1] <= exp->bbs[i].bbmax.f[1]
					&& pos.f[2] >= exp->bbs[i].bbmin.f[2] && pos.f[2] <= exp->bbs[i].bbmax.f[2]) {
				f32 min = 65536.0f;

				for (j = 0; j < 3; j++) {
					value = pos.f[j] - exp->bbs[i].bbmin.f[j];

					if (value < min) {
						min = value;
					}

					value = exp->bbs[i].bbmax.f[j] - pos.f[j];

					if (value < min) {
						min = value;
					}
				}

				if (min > max) {
					max = min;
					bbnum = i;
				}
			}
		}

		size2 = size * 0.7f;

		for (i = 0; i < 3; i++) {
			if ((exp->bbs[bbnum].bbmax.f[i] - exp->bbs[bbnum].bbmin.f[i]) * 0.8f < size2) {
				size = (exp->bbs[bbnum].bbmax.f[i] - exp->bbs[bbnum].bbmin.f[i]) * 0.8f / 0.7f;

				if (part->size * 0.65f > size) {
					size = part->size * 0.65f;
				}

				size2 = size * 0.7f;
			}
		}

		if (arg4 == 1) {
			size2 *= 0.58928573f;
		} else if (arg4 == 2) {
			size2 *= 0.6964286f;
		} else if (arg4 == 3) {
			size2 *= 0.8214286f;
		} else if (arg4 == 4) {
			size2 *= 0.96428573f;
		}

		for (i = 0; i < 3; i++) {
			if (exp->bbs[bbnum].bbmax.f[i] - exp->bbs[bbnum].bbmin.f[i] < size2) {
				pos.f[i] = (exp->bbs[bbnum].bbmax.f[i] + exp->bbs[bbnum].bbmin.f[i]) * 0.5f;
			} else {
				value = exp->bbs[bbnum].bbmin.f[i] + size2 - pos.f[i];

				if (value > 0.0f) {
					pos.f[i] += value;
				} else {
					value = pos.f[i] - (exp->bbs[bbnum].bbmax.f[i] - size2);

					if (value > 0.0f) {
						pos.f[i] -= value;
					}
				}
			}
		}

		x = pos.x;
		y = pos.y;
		z = pos.z;
	}

	cosine = cosf(part->rot) * size;
	sine = sinf(part->rot) * size;

	spbc.x = mtx->m[0][0] * cosine;
	spbc.y = mtx->m[0][1] * cosine;
	spbc.z = mtx->m[0][2] * cosine;

	spb0.x = mtx->m[0][0] * sine;
	spb0.y = mtx->m[0][1] * sine;
	spb0.z = mtx->m[0][2] * sine;

	spa4.x = mtx->m[1][0] * cosine;
	spa4.y = mtx->m[1][1] * cosine;
	spa4.z = mtx->m[1][2] * cosine;

	sp98.x = mtx->m[1][0] * sine;
	sp98.y = mtx->m[1][1] * sine;
	sp98.z = mtx->m[1][2] * sine;

	vertices[0].x = x - spbc.f[0] - sp98.f[0] - coord->f[0];
	vertices[0].y = y - spbc.f[1] - sp98.f[1] - coord->f[1];
	vertices[0].z = z - spbc.f[2] - sp98.f[2] - coord->f[2];
	vertices[0].s = 1760;
	vertices[0].t = 0;

	vertices[1].x = x + spb0.f[0] - spa4.f[0] - coord->f[0];
	vertices[1].y = y + spb0.f[1] - spa4.f[1] - coord->f[1];
	vertices[1].z = z + spb0.f[2] - spa4.f[2] - coord->f[2];
	vertices[1].s = 0;
	vertices[1].t = 0;

	vertices[2].x = x + spbc.f[0] + sp98.f[0] - coord->f[0];
	vertices[2].y = y + spbc.f[1] + sp98.f[1] - coord->f[1];
	vertices[2].z = z + spbc.f[2] + sp98.f[2] - coord->f[2];
	vertices[2].s = 0;
	vertices[2].t = 1760;

	vertices[3].x = x - spb0.f[0] + spa4.f[0] - coord->f[0];
	vertices[3].y = y - spb0.f[1] + spa4.f[1] - coord->f[1];
	vertices[3].z = z - spb0.f[2] + spa4.f[2] - coord->f[2];
	vertices[3].s = 1760;
	vertices[3].t = 1760;

	for (j = 0; j < 4; j++) {
		vertices[j].colour = 0;
	}

	gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);

	gSPTri2(gdl++, 0, 1, 2, 0, 2, 3);

	return gdl;
}
