#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/debug.h"
#include "game/chr.h"
#include "game/propsnd.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/playermgr.h"
#include "game/botact.h"
#include "game/pad.h"
#include "game/padhalllv.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "data.h"
#include "types.h"

s32 botactGetAmmoTypeByFunction(s32 weaponnum, s32 funcnum)
{
	if (weaponnum >= WEAPON_FALCON2 && weaponnum <= WEAPON_SUICIDEPILL) {
		struct inventory_ammo *ammo = weaponGetAmmoByFunction(weaponnum, funcnum);

		if (ammo) {
			return ammo->type;
		}
	}

	return 0;
}

s32 botactGetClipCapacityByFunction(s32 weaponnum, u32 funcnum)
{
	if (weaponnum >= WEAPON_FALCON2 && weaponnum <= WEAPON_SUICIDEPILL) {
		struct inventory_ammo *ammo = weaponGetAmmoByFunction(weaponnum, funcnum);

		if (ammo) {
			return ammo->clipsize;
		}
	}

	return 0;
}

void botactReload(struct chrdata *chr, s32 handnum, bool withsound)
{
	struct aibot *aibot = chr->aibot;

	aibot->timeuntilreload60[handnum] = 0;
	aibot->maulercharge[handnum] = 0;

	if (chr->weapons_held[handnum] && !botactIsWeaponThrowable(aibot->weaponnum, aibot->gunfunc)) {
		s32 capacity = botactGetClipCapacityByFunction(aibot->weaponnum, aibot->gunfunc);

		if (capacity > 0) {
			s32 tryamount = capacity - aibot->loadedammo[handnum];
			s32 actualamount = botactTryRemoveAmmoFromReserve(aibot, aibot->weaponnum, aibot->gunfunc, tryamount);

			if (actualamount > 0) {
				aibot->loadedammo[handnum] += actualamount;

				if (withsound) {
					if (aibot->weaponnum == WEAPON_FARSIGHT) {
						psCreate(NULL, chr->prop, SFX_RELOAD_FARSIGHT, -1,
								-1, PSFLAG_0400, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
					} else {
						psCreate(NULL, chr->prop, SFX_RELOAD_DEFAULT, -1,
								-1, PSFLAG_0400, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
					}
				}
			}
		}
	}
}

s32 botactGetAmmoQuantityByWeapon(struct aibot *aibot, s32 weaponnum, s32 funcnum, bool include_equipped)
{
	s32 qty = 0;
	s32 ammotype;
	s32 equippedammotype;

	if (aibot) {
		if (aibot->flags & BOTFLAG_UNLIMITEDAMMO) {
			ammotype = botactGetAmmoTypeByFunction(weaponnum, funcnum);
			qty = bgunGetCapacityByAmmotype(ammotype);
		} else {
			ammotype = botactGetAmmoTypeByFunction(weaponnum, funcnum);
			qty = aibot->ammoheld[ammotype];
		}

		if (include_equipped) {
			ammotype = botactGetAmmoTypeByFunction(weaponnum, funcnum);
			equippedammotype = botactGetAmmoTypeByFunction(aibot->weaponnum, aibot->gunfunc);

			if (equippedammotype == ammotype) {
				qty += aibot->loadedammo[HAND_LEFT] + aibot->loadedammo[HAND_RIGHT];
			}
		}
	}

	return qty;
}

s32 botactGetAmmoQuantityByType(struct aibot *aibot, s32 ammotype, bool include_equipped)
{
	s32 qty = 0;

	if (aibot) {
		if (aibot->flags & BOTFLAG_UNLIMITEDAMMO) {
			qty = bgunGetCapacityByAmmotype(ammotype);
		} else {
			qty = aibot->ammoheld[ammotype];
		}

		if (include_equipped
				&& botactGetAmmoTypeByFunction(aibot->weaponnum, aibot->gunfunc) == ammotype) {
			qty += aibot->loadedammo[HAND_LEFT] + aibot->loadedammo[HAND_RIGHT];
		}
	}

	return qty;
}

/**
 * Attempt to remove the given quantity of ammo from the aibot's reserve and
 * return the amount actually removed.
 *
 * The amount removed will be less than the attempted amount if the aibot
 * doesn't have enough ammo in reserve.
 */
s32 botactTryRemoveAmmoFromReserve(struct aibot *aibot, s32 weaponnum, s32 funcnum, s32 tryqty)
{
	s32 amountremoved;
	s32 *ammoheld = &aibot->ammoheld[botactGetAmmoTypeByFunction(weaponnum, funcnum)];

	if (!aibot || *ammoheld <= 0 || tryqty <= 0) {
		return 0;
	}

	if (aibot->flags & BOTFLAG_UNLIMITEDAMMO) {
		return tryqty;
	}

	dprint();
	*ammoheld -= tryqty;

	if (*ammoheld < 0) {
		amountremoved = tryqty + *ammoheld;
		*ammoheld = 0;

		if (dprint()) {
			return amountremoved;
		}
	} else {
		amountremoved = tryqty;
		dprint();
	}

	return amountremoved;
}

void botactGiveAmmoByWeapon(struct aibot *aibot, s32 weaponnum, s32 funcnum, s32 qty)
{
	s32 max;
	s32 *heldquantity = &aibot->ammoheld[botactGetAmmoTypeByFunction(weaponnum, funcnum)];

	if (aibot && (aibot->flags & BOTFLAG_UNLIMITEDAMMO) == 0 && qty > 0) {
		dprint();
		*heldquantity += qty;

		if (heldquantity);

		max = bgunGetCapacityByAmmotype(botactGetAmmoTypeByFunction(weaponnum, funcnum));

		if (*heldquantity > max) {
			*heldquantity = max;
		}

		dprint();
	}
}

void botactGiveAmmoByType(struct aibot *aibot, u32 ammotype, s32 quantity)
{
	s32 max;
	s32 *heldquantity = &aibot->ammoheld[ammotype];

	if (!aibot || (aibot->flags & BOTFLAG_UNLIMITEDAMMO) || quantity <= 0) {
		return;
	}

	dprint();

	*heldquantity += quantity;

	if (heldquantity);

	max = bgunGetCapacityByAmmotype(ammotype);

	if (*heldquantity > max) {
		*heldquantity = max;
	}

	dprint();
}

bool botactShootFarsight(struct chrdata *chr, s32 arg1, struct coord *vector, struct coord *arg3)
{
	struct aibot *aibot;
	struct chrdata *oppchr;
	struct prop *oppprop;
	s32 i;
	s32 rand;
	f32 speed;

	if (!chr || !chr->aibot) {
		return false;
	}

	aibot = chr->aibot;

	if (aibot->weaponnum == WEAPON_FARSIGHT) {
		rand = rngRandom() % 100;

		// 3 in 10 chance of this passing
		if (rand < 30) {
			struct modelnode *node = NULL;
			struct model *model = NULL;
			s32 side = -1;
			s32 hitpart = HITPART_GENERAL;
			struct gset gset = {WEAPON_FARSIGHT, 0, 0, FUNC_PRIMARY};
			f32 damage = gsetGetDamage(&gset);
			s32 fallback = 30;
			s32 value = fallback;

			for (i = 0; i < g_MpNumChrs; i++) {
				oppchr = g_MpAllChrPtrs[i];
				oppprop = g_MpAllChrPtrs[i]->prop;

				if (oppprop->type == PROPTYPE_PLAYER) {
					struct player *player = g_Vars.players[playermgrGetPlayerNumByProp(oppprop)];
					speed = player->speedforwards * player->speedforwards
							+ player->speedsideways * player->speedsideways;

					if (speed > 0) {
						value = fallback * 0.05f;
					}
				} else {
					if (oppchr->actiontype != ACT_STAND) {
						value = fallback * 0.05f;
					}
				}

				// value is 30 if player was still, or 1.5 if moving. So if 30
				// then this will always pass, or if 1.5 then this has 1 in 15
				// chance of passing.
				if (oppchr != chr
						&& value > rand
						&& func0f06b39c(arg3, vector, &oppprop->pos, chrGetHitRadius(oppchr))) {
					bgunPlayPropHitSound(&gset, oppprop, -1);

					if (oppchr->model && chrGetShield(oppchr) > 0) {
						chrCalculateShieldHit(oppchr, &oppprop->pos, vector, &node, &hitpart, &model, &side);
					}

					chrEmitSparks(oppchr, oppprop, hitpart, &oppprop->pos, vector, chr);
					func0f0341dc(oppchr, damage, vector, &gset, chr->prop, HITPART_GENERAL, oppprop, node, model, side, 0);
				}
			}

			return true;
		}

		return false;
	}

	return true;
}

s32 botactGetWeaponModel(s32 weapon)
{
	return playermgrGetModelOfWeapon(weapon);
}

bool botactIsWeaponThrowable(s32 weaponnum, bool is_secondary)
{
	switch (weaponnum) {
	case WEAPON_LAPTOPGUN:
	case WEAPON_DRAGON:
	case WEAPON_COMBATKNIFE:
		return is_secondary;
	case WEAPON_GRENADE:
	case WEAPON_NBOMB:
	case WEAPON_TIMEDMINE:
	case WEAPON_PROXIMITYMINE:
	case WEAPON_REMOTEMINE:
		return true;
	}

	return false;
}

u32 botactGetProjectileThrowInterval(u32 weapon)
{
	switch (weapon) {
	case WEAPON_COMBATKNIFE:
		return TICKS(120);
	case WEAPON_GRENADE:
	case WEAPON_NBOMB:
		return TICKS(90);
	case WEAPON_CROSSBOW:
	case WEAPON_TRANQUILIZER:
	case WEAPON_LASER:
	case WEAPON_TIMEDMINE:
	case WEAPON_PROXIMITYMINE:
	case WEAPON_REMOTEMINE:
	default:
		return TICKS(60);
	}
}

s32 botactGetWeaponByAmmoType(s32 ammotype)
{
	switch (ammotype) {
	case AMMOTYPE_NBOMB:       return WEAPON_NBOMB;
	case AMMOTYPE_GRENADE:     return WEAPON_GRENADE;
	case AMMOTYPE_KNIFE:       return WEAPON_COMBATKNIFE;
	case AMMOTYPE_REMOTE_MINE: return WEAPON_REMOTEMINE;
	case AMMOTYPE_PROXY_MINE:  return WEAPON_PROXIMITYMINE;
	case AMMOTYPE_TIMED_MINE:  return WEAPON_TIMEDMINE;
	}

	return 0;
}

void botactThrow(struct chrdata *chr)
{
	struct coord sp228 = {0, 0, 0};
	Mtxf sp164;
	struct coord sp152;
	struct prop *prop = chr->prop;
	Mtxf sp84;
	f32 sp80 = chrGetAimAngle(chr);
	u32 stack;
	struct gset gset = {0};
	struct prop *target = chrGetTargetProp(chr);
	struct coord sp56;
	f32 mult;

	gset.weaponnum = chr->aibot->weaponnum;
	gset.weaponfunc = chr->aibot->gunfunc;

	if (chrIsTargetInFov(chr, 30, 0)) {
		sp56.x = target->pos.x;
		sp56.z = target->pos.z;

		if (chr->aibot->weaponnum == WEAPON_GRENADE || chr->aibot->weaponnum == WEAPON_NBOMB) {
			sp56.y = target->chr->manground;
		} else {
			sp56.y = target->pos.y;

			if (target->type == PROPTYPE_PLAYER) {
				sp56.y -= 25.0f;
			}
		}

		chrCalculateTrajectory(&prop->pos, 16.666666f, &sp56, &sp152);
	} else {
		// These numbers are about 2 billionths away from BADDEG2RAD(20),
		// but tweaking the multiplier in BADDEG2RAD doesn't make this match
		// without creating mismatches in other places :(
		sp152.x = cosf(0.34901028871536f) * sinf(sp80);
		sp152.y = sinf(0.34901028871536f);
		sp152.z = cosf(0.34901028871536f) * cosf(sp80);
	}

	mult = 16.666666f;

	sp228.x = sp152.x * mult;
	sp228.y = sp152.y * mult;
	sp228.z = sp152.z * mult;

	mtx4LoadIdentity(&sp164);

	if (chr->aibot->weaponnum == WEAPON_COMBATKNIFE) {
		mtx4LoadZRotation(M_BADPI * 1.5f, &sp164);
		mtx4LoadXRotation(M_BADPI, &sp84);
		mtx4MultMtx4InPlace(&sp84, &sp164);
	}

	mtx4LoadXRotation(0.34901028871536f, &sp84);
	mtx00015be0(&sp84, &sp164);
	mtx4LoadYRotation(sp80, &sp84);
	mtx00015be0(&sp84, &sp164);

	bgunCreateThrownProjectile2(chr, &gset, &prop->pos, prop->rooms, &sp164, &sp228);

	if (gset.weaponnum == WEAPON_REMOTEMINE) {
		chr->aibot->flags |= BOTFLAG_THREWREMOTEMINE;
	}
}

/**
 * Get the shoot interval of the given weapon, in time60.
 */
s32 botactGetShootInterval60(s32 weaponnum, s32 funcnum)
{
	s32 stack[2];
	s32 result = 1;
	struct weapon *weapon = weaponFindById(weaponnum);

	if (weapon) {
		struct weaponfunc *func = weapon->functions[funcnum];

		if (func) {
			if (func->type == INVENTORYFUNCTYPE_SHOOT_SINGLE) {
				struct weaponfunc_shoot *func2 = (struct weaponfunc_shoot *)func;
				result = func2->unk24 + func2->unk25;
			} else if (func->type == INVENTORYFUNCTYPE_SHOOT_AUTOMATIC) {
				struct weaponfunc_shoot *func2 = (struct weaponfunc_shoot *)func;
				result = func2->unk24 + func2->unk25;
			} else if (func->type == INVENTORYFUNCTYPE_SHOOT_PROJECTILE) {
				struct weaponfunc_shoot *func2 = (struct weaponfunc_shoot *)func;
				result = func2->unk24 + func2->unk25;
			} else if (func->type == INVENTORYFUNCTYPE_MELEE && weaponnum != WEAPON_REAPER) {
				result = 60;
			}
		}
	}

	return result;
}

/**
 * Do pathfinding for a bot's Slayer rocket in fly-by-wire mode and populate
 * the projectil's waypads.
 *
 * Return true if a route was found, false if not.
 */
bool botactFindRocketRoute(struct chrdata *chr, struct coord *frompos, struct coord *topos, RoomNum *fromrooms, RoomNum *torooms, struct projectile *projectile)
{
	struct waypoint *from = waypointFindClosestToPos(frompos, fromrooms);
	struct waypoint *to = waypointFindClosestToPos(topos, torooms);
	struct waypoint *waypoints[MAX_CHRWAYPOINTS];
	s32 numwaypoints;

	if (from && to) {
		navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
		numwaypoints = navFindRoute(from, to, waypoints, 6);
		navSetSeed(0, 0);

		if (numwaypoints > 1) {
			s32 i = 0;

			while (waypoints[i]) {
				projectile->waypads[i] = waypoints[i]->padnum;
				i++;
			}

			projectile->step = 0;
			projectile->numwaypads = i;

			return true;
		}
	}

	return false;
}

/**
 * Populate pos with the position of the given pad
 * for a Slayer rocket in fly-by-wire mode.
 *
 * It's the ground position of the pad plus 1.5 metres.
 */
void botactGetRocketNextStepPos(u16 padnum, struct coord *pos)
{
	struct pad pad;
	RoomNum rooms[2];

	padUnpack(padnum, PADFIELD_ROOM | PADFIELD_POS, &pad);

	rooms[0] = pad.room;
	rooms[1] = -1;

	pos->x = pad.pos.x;
	pos->y = cdFindFloorYColourTypeAtPos(&pad.pos, rooms, 0, 0) + 150;
	pos->z = pad.pos.z;
}

/**
 * Create a Slayer rocket in fly-by-wire mode (ie. remote controlled).
 */
void botactCreateSlayerRocket(struct chrdata *chr)
{
	struct weaponobj *rocket = weaponCreateProjectileFromWeaponNum(MODEL_CHRSKROCKETMIS, WEAPON_SKROCKET, chr);

	if (rocket) {
		Mtxf sp260;
		Mtxf sp196;
		Mtxf sp132;
		struct coord sp120 = {0, 0, 0};
		f32 yrot;
		f32 xrot;
		struct coord sp100;

		yrot = chrGetAimAngle(chr);
		xrot = chrGetPitchAngle(chr);

		sp100.x = cosf(xrot) * sinf(yrot);
		sp100.y = sinf(xrot);
		sp100.z = cosf(xrot) * cosf(yrot);

		mtx4LoadXRotation(xrot, &sp196);
		mtx4LoadYRotation(yrot, &sp132);
		mtx00015be0(&sp132, &sp196);
		mtx4LoadIdentity(&sp260);

		bgun0f09ebcc(&rocket->base, &chr->prop->pos, chr->prop->rooms, &sp196, &sp100, &sp260, chr->prop, &chr->prop->pos);

		if (rocket->base.hidden & OBJHFLAG_PROJECTILE) {
			struct prop *target = chrGetTargetProp(chr);
			rocket->timer240 = -1;
			rocket->base.projectile->unk010 = 7.5;
			rocket->base.projectile->unk014 = xrot;
			rocket->base.projectile->unk018 = yrot;
			rocket->base.projectile->smoketimer240 = 0;
			rocket->base.projectile->pickuptimer240 = 0x20000000;

			// Fire rocket sound
			psCreate(NULL, rocket->base.prop, SFX_LAUNCH_ROCKET_8053, -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);

			if (!botactFindRocketRoute(chr, &chr->prop->pos, &target->pos, chr->prop->rooms, target->rooms, rocket->base.projectile)) {
				rocket->timer240 = 0; // blow up rocket
			} else {
				botactGetRocketNextStepPos(rocket->base.projectile->waypads[0], &rocket->base.projectile->nextsteppos);
				chr->aibot->skrocket = rocket->base.prop;
			}
		}
	}
}
