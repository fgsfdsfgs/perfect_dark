#include <ultra64.h>
#include "constants.h"
#include "game/dlights.h"
#include "game/chr.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/playermgr.h"
#include "game/room.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "game/options.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#define WALLHITTYPE_SOFT   0
#define WALLHITTYPE_BULLET 1
#define WALLHITTYPE_SCORCH 2
#define WALLHITTYPE_BLOOD  3
#define WALLHITTYPE_PAINT  4

#define IS_BLOOD_DROP(texnum) (texnum >= WALLHITTEX_BLOOD4 && texnum <= WALLHITTEX_BLOOD4)

const char var7f1b5a10[] = "WallHit_MakeSpaceRoom : ERROR - Couldn't find any space in room %d\n";

struct wallhit *g_Wallhits;
struct wallhit *g_FreeWallhits;
struct wallhit *g_ActiveWallhits;

s32 var8007f740 = 0;
u8 g_WallhitBloodColour[4] = {0x40, 0x0a, 0x0a, 0x00};
f32 var8007f748 = 1;
f32 var8007f74c = 1;
u32 var8007f750 = 0;
f32 var8007f754 = 0;
f32 var8007f758 = 0;

struct wallhittex {
	f32 width;
	f32 height;
	u8 type;
};

struct wallhittex g_WallhitTexes[] = {
	/*0x00*/ { 10,  10,  WALLHITTYPE_BULLET }, // WALLHITTEX_WATER
	/*0x01*/ { 6,   6,   WALLHITTYPE_BULLET }, // WALLHITTEX_BULLET1
	/*0x02*/ { 8,   8,   WALLHITTYPE_SOFT   }, // WALLHITTEX_SOFT
	/*0x03*/ { 6,   6,   WALLHITTYPE_BULLET }, // WALLHITTEX_GLASS1
	/*0x04*/ { 8,   8,   WALLHITTYPE_BULLET }, // WALLHITTEX_GLASS2
	/*0x05*/ { 12,  12,  WALLHITTYPE_BULLET }, // WALLHITTEX_GLASS3
	/*0x06*/ { 6,   6,   WALLHITTYPE_BULLET }, // WALLHITTEX_BULLET2
	/*0x07*/ { 100, 100, WALLHITTYPE_SCORCH }, // WALLHITTEX_SCORCH
	/*0x08*/ { 24,  24,  WALLHITTYPE_PAINT  }, // WALLHITTEX_PAINT
	/*0x09*/ { 20,  20,  WALLHITTYPE_BLOOD  }, // WALLHITTEX_BLOOD1
	/*0x0a*/ { 20,  20,  WALLHITTYPE_BLOOD  }, // WALLHITTEX_BLOOD2
	/*0x0b*/ { 20,  20,  WALLHITTYPE_BLOOD  }, // WALLHITTEX_BLOOD3
	/*0x0c*/ { 20,  20,  WALLHITTYPE_BLOOD  }, // WALLHITTEX_BLOOD4
	/*0x0d*/ { 6,   6,   WALLHITTYPE_BULLET }, // WALLHITTEX_BPGLASS1
	/*0x0e*/ { 8,   8,   WALLHITTYPE_BULLET }, // WALLHITTEX_BPGLASS2
	/*0x0f*/ { 12,  12,  WALLHITTYPE_BULLET }, // WALLHITTEX_BPGLASS3
	/*0x10*/ { 4,   4,   WALLHITTYPE_BULLET }, // WALLHITTEX_WOOD
	/*0x11*/ { 6,   6,   WALLHITTYPE_BULLET }, // WALLHITTEX_METAL
};

s16 wallhitFinaliseAxis(f32 value)
{
	if (value > var8007f754) {
		var8007f754 = value;
	} else if (-value > var8007f758) {
		var8007f758 = -value;
	}

	if (value > 8000) {
		value = 8000;
	} else if (value < -8000) {
		value = -8000;
	}

	if (value > 0) {
		value += 0.5f;
	} else {
		value -= 0.5f;
	}

	return value;
}

void wallhitFree(struct wallhit *wallhit)
{
	struct wallhit *iter;
	s32 i;

	wallhit->timermax = 0;
	wallhit->timercur = 0;
	wallhit->inuse = false;

	// Update the global list
	if (wallhit == g_ActiveWallhits) {
		g_ActiveWallhits = wallhit->globalnext;
		wallhit->globalnext = NULL;
	} else {
		iter = g_ActiveWallhits;

		while (wallhit != iter->globalnext) {
			iter = iter->globalnext;
		}

		iter->globalnext = wallhit->globalnext;
	}

	wallhit->globalnext = g_FreeWallhits;
	g_FreeWallhits = wallhit;

	// Update the room/prop's wallhit list
	if (wallhit->objprop == NULL) {
		if (wallhit->xlu) {
			if (wallhit == g_Rooms[wallhit->roomnum].xluwallhits) {
				g_Rooms[wallhit->roomnum].xluwallhits = wallhit->localnext;
			} else {
				iter = g_Rooms[wallhit->roomnum].xluwallhits;

				while (wallhit != iter->localnext) {
					iter = iter->localnext;
				}

				iter->localnext = wallhit->localnext;
			}

			wallhit->localnext = NULL;
		} else {
			if (wallhit == g_Rooms[wallhit->roomnum].opawallhits) {
				g_Rooms[wallhit->roomnum].opawallhits = wallhit->localnext;
			} else {
				iter = g_Rooms[wallhit->roomnum].opawallhits;

				while (wallhit != iter->localnext) {
					iter = iter->localnext;
				}

				iter->localnext = wallhit->localnext;
			}

			wallhit->localnext = NULL;
		}
	} else {
		struct prop *prop = wallhit->objprop;

		if (wallhit->xlu) {
			if (wallhit == prop->xluwallhits) {
				prop->xluwallhits = wallhit->localnext;
			} else {
				iter = prop->xluwallhits;

				while (wallhit != iter->localnext) {
					iter = iter->localnext;
				}

				iter->localnext = wallhit->localnext;
			}

			wallhit->localnext = NULL;
		} else {
			if (wallhit == prop->opawallhits) {
				prop->opawallhits = wallhit->localnext;
			} else {
				iter = prop->opawallhits;

				while (wallhit != iter->localnext) {
					iter = iter->localnext;
				}

				iter->localnext = wallhit->localnext;
			}

			wallhit->localnext = NULL;
		}
	}

	wallhit->objprop = NULL;

	g_WallhitsNumUsed--;
	g_WallhitsNumFree++;

	for (i = 0; i < ARRAYCOUNT(wallhit->basecolours); i++) {
		wallhit->basecolours[i].a = 0;
		wallhit->finalcolours[i].a = 0;
	}
}

void wallhitsFreeByProp(struct prop *prop, s8 layer)
{
	struct prop *copy = prop;

	if (layer) {
		while (copy->xluwallhits) {
			wallhitFade(copy->xluwallhits, 1);
			wallhitFree(copy->xluwallhits);
		}
	} else {
		while (copy->opawallhits) {
			wallhitFade(copy->opawallhits, 1);
			wallhitFree(copy->opawallhits);
		}
	}
}

bool chrIsUsingPaintball(struct chrdata *chr)
{
	s32 prevplayernum = g_Vars.currentplayernum;
	bool paintball;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		setCurrentPlayerNum(playermgrGetPlayerNumByProp(chr->prop));
	} else {
		setCurrentPlayerNum(rngRandom() % PLAYERCOUNT());
	}

	paintball = optionsGetPaintball(g_Vars.currentplayerstats->mpindex);

	setCurrentPlayerNum(prevplayernum);

	return paintball;
}

void wallhitChooseBloodColour(struct prop *prop)
{
	if (prop && prop->chr && (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER)) {
		struct chrdata *chr = prop->chr;
		chrGetBloodColour(chr->bodynum, g_WallhitBloodColour, NULL);
	} else {
		g_WallhitBloodColour[0] = 0x40;
		g_WallhitBloodColour[1] = 0x0a;
		g_WallhitBloodColour[2] = 0x0a;
	}
}

void wallhitFade(struct wallhit *wallhit, u32 arg1)
{
	if (!wallhit->fading) {
		if (wallhit->objprop) {
			g_WallhitCountsPerRoom[0]--;
		} else {
			g_WallhitCountsPerRoom[wallhit->roomnum]--;
		}

		if (wallhit->timermax == 0) {
			wallhit->timermax = arg1;
			wallhit->timercur = arg1;
		}

		wallhit->fading = true;

		g_WallhitsNumSettled--;
		g_WallhitsNumUsed++;

		if (g_WallhitTexes[wallhit->texturenum].type == WALLHITTYPE_BLOOD) {
			g_WallhitsNumBloodSettled--;
		} else {
			g_WallhitsNumNonbloodSettled--;
		}

		wallhit->expanding = false;
	}
}

/**
 * Remove a single wallhit from the given room.
 *
 * Room may be -1 to remove a wallhit from any room, or 0 to remove a prop
 * wallhit, or any number > 0 to remove a wallhit from that room.
 *
 * Wallhits are removed in such a way to try to meet the target blood to
 * non-blood ratio. If a blood wallhit will be removed, a blood drop will
 * be favoured over a blood puddle. The actual wallhit to be removed will
 * be the oldest one that meets that criteria.
 */
bool wallhitRemoveOneInRoom(s32 room)
{
	if (room == -1 || g_WallhitCountsPerRoom[room]) {
		f32 ratio = 0.0f;
		u32 blooddropframe = U32_MAX;
		u32 bloodpuddleframe = U32_MAX;
		u32 otherframe = U32_MAX;
		s32 blooddropindex = -1;
		s32 bloodpuddleindex = -1;
		s32 otherindex = -1;
		s32 numblood;
		s32 numother;
		s32 numtotal;
		s32 i;

		for (i = 0, numblood = 0, numother = 0; i < g_WallhitsMax; i++) {
			if (g_Wallhits[i].inuse
					&& !g_Wallhits[i].fading
					&& (room == -1
						|| (room == 0 && g_Wallhits[i].objprop != NULL)
						|| (room && room == g_Wallhits[i].roomnum))) {
				if (g_WallhitTexes[g_Wallhits[i].texturenum].type == WALLHITTYPE_BLOOD) {
					numblood++;

					if (IS_BLOOD_DROP(g_Wallhits[i].texturenum)) {
						if (g_Wallhits[i].createdframe < blooddropframe) {
							blooddropframe = g_Wallhits[i].createdframe;
							blooddropindex = i;
						}
					} else {
						if (g_Wallhits[i].createdframe < bloodpuddleframe) {
							bloodpuddleframe = g_Wallhits[i].createdframe;
							bloodpuddleindex = i;
						}
					}
				} else {
					numother++;

					if (g_Wallhits[i].createdframe < otherframe) {
						otherframe = g_Wallhits[i].createdframe;
						otherindex = i;
					}
				}
			}
		}

		numtotal = numblood + numother;

		if (numtotal > 0) {
			ratio = (f32) numblood / (f32) numtotal;
		}

		if (ratio > g_WallhitTargetBloodRatio && (blooddropindex != -1 || bloodpuddleindex != -1)) {
			if (blooddropindex != -1) {
				wallhitFade(&g_Wallhits[blooddropindex], TICKS(30));
				return true;
			} else {
				wallhitFade(&g_Wallhits[bloodpuddleindex], TICKS(30));
				return true;
			}
		} else {
			if (otherindex != -1) {
				wallhitFade(&g_Wallhits[otherindex], TICKS(30));
				return true;
			}
		}
	}

	return false;
}

/**
 * Remove a single wallhit.
 *
 * The wallhit will be removed from a room that's offscreen and not in standby
 * if possible, otherwise a room that's in standby. If wallhits only exist in
 * onscreen rooms then all rooms are considered, and prop hits too.
 *
 * The chosen room will be the one with the most wallhits within one of those
 * three categories.
 */
void wallhitRemoveOne(void)
{
	s32 room;
	u32 i;
	bool done = false;

	for (i = 0; !done && i < 3; i++) {
		s32 bestroom = -1;
		s32 bestvalue = -1;

		for (room = 0; room < g_Vars.roomcount; room++) {
			s32 onscreen = room == 0 ? 1 : (g_Rooms[room].flags & ROOMFLAG_ONSCREEN);
			s32 standby = room == 0 ? 1 : (g_Rooms[room].flags & ROOMFLAG_STANDBY);
			bool consider;

			if (i == 0) {
				consider = !onscreen && !standby;
			} else if (i == 1) {
				consider = !onscreen;
			} else {
				consider = true;
			}

			if (consider) {
				if (g_WallhitCountsPerRoom[room] > bestvalue) {
					bestvalue = g_WallhitCountsPerRoom[room];
					bestroom = room;
				}
			}
		}

		if (1);
		if (1);

		if (bestroom != -1) {
			s32 min = bestroom == 0 ? g_MinPropWallhits : g_MinBgWallhitsPerRoom;

			if (g_WallhitCountsPerRoom[bestroom] > min) {
				if (wallhitRemoveOneInRoom(bestroom)) {
					done = true;
				}
			}
		}
	}

	if (!done) {
		wallhitRemoveOneInRoom(-1);
	}

	if (1);
}

void wallhitsTick(void)
{
	f32 sp12c;
	f32 fov;
	s32 numallocated;
	s32 i;
	s32 j;
	u32 stack[3];
	f32 midx;
	f32 midy;
	f32 midz;
	f32 f22;
	f32 f24;
	struct wallhit *wallhit;
	struct coord spc8[4];
	u32 stack2[4];

	static s32 var8007f834 = 0;

	sp12c = (g_Vars.lvupdate240 + 2.0f) * 0.25f;
	fov = currentPlayerGetGunZoomFov();

	mainOverrideVariable("wallhit", &var8007f750);

	var8007f740 = 0;

	if (fov == 0.0f || fov == 60.0f) {
		var8007f748 = 1;
	} else {
		f32 tmp = fov / g_Vars.currentplayer->zoominfovy;
		var8007f748 = 60.0f / fov - 1.00f / tmp + 1;
	}

	var8007f74c = 1.0f / var8007f748;

	numallocated = g_WallhitsNumFree + g_WallhitsNumUsed;

	if (numallocated < g_WallhitsCriticalSpareLimit) {
		wallhitRemoveOne();
	} else if (numallocated < g_WallhitsGoalSpareLimit) {
		var8007f834++;

		if (var8007f834 == 8) {
			var8007f834 = 0;
			wallhitRemoveOne();
		}
	}

	wallhit = g_Wallhits;

	for (i = 0; i < g_WallhitsMax; i++, wallhit++) {
		f32 f0 = sp12c;

		if (!wallhit->inuse) {
			continue;
		}

		if (wallhit->timerspeed != 8) {
			f0 *= 0.6f * ((wallhit->timerspeed - 8.0f) * 0.125f);
		}

		if (wallhit->timermax) {
			u32 amount = (u32)(f0 + 0.5f);

			if (wallhit->expanding) {
				if (wallhit->timercur > wallhit->timermax) {
					wallhit->timermax = 0;
					wallhit->timercur = 0;
					wallhit->inuse = true;
				}

				wallhit->timercur += amount;
			} else {
				if (amount < wallhit->timercur) {
					wallhit->timercur -= amount;
				} else {
					wallhitFree(wallhit);
				}
			}

			if (wallhit->timermax) {
				f24 = (f32) wallhit->timercur / wallhit->timermax;

				if (f24 > 1.0f) {
					f24 = 1.0f;
				}

				f22 = f24;

				if (wallhit->expanding) {
					f32 frac = 0.2f;
					f32 f30;
					s32 minindex;
					f32 tmp;
					s32 j;

					tmp = 1.5707964f * f24;
					f30 = (1.0f - frac) * sinf(tmp);
					f22 = 1.0f - tmp + 0.6f;

					wallhit->vertices2 = gfxAllocateVertices(4);

					midx = var800845dc.x; \
					midy = var800845dc.y; \
					midz = var800845dc.z;

					// Copy the vertices into a float array
					for (j = 0; j < ARRAYCOUNT(wallhit->vertices); j++) {
						spc8[j].x = wallhit->vertices[j].x;
						spc8[j].y = wallhit->vertices[j].y;
						spc8[j].z = wallhit->vertices[j].z;
					}

					// Sum the vertices and divide them by 4 to get the centre
					minindex = 0;

					for (j = 0; j < ARRAYCOUNT(spc8); j++) {
						midx = midx + spc8[j].x;
						midy = midy + spc8[j].y;
						midz = midz + spc8[j].z;

						// This should be j != 0, but minindex is unused
						// so it doesn't affect anything
						if (minindex != 0 && spc8[j].y < spc8[minindex].y) {
							minindex = j;
						}
					}

					midx = 0.25f * midx;
					midy = 0.25f * midy;
					midz = 0.25f * midz;

					// Calculate and apply the new size
					for (j = 0; j < ARRAYCOUNT(spc8); j++) {
						s32 j2;
						f32 xradius = spc8[j].x - midx;
						f32 yradius = spc8[j].y - midy;
						f32 zradius = spc8[j].z - midz;

						wallhit->vertices2[j].x = midx + xradius * (0.2f + f30);
						goto foo; foo:;
						j2 = j;
						wallhit->vertices2[j2].y = midy + yradius * (0.2f + f30);
						wallhit->vertices2[j2].z = midz + zradius * (0.2f + f30);
						wallhit->vertices2[j2].s = wallhit->vertices[j].s;
						wallhit->vertices2[j2].t = wallhit->vertices[j].t;
						wallhit->vertices2[j2].colour = wallhit->vertices[j].colour;
					}

					f24 *= 2.0f;

					if (f24 > 1.0f) {
						f24 = 1.0f;
					}

					;
				}

				for (j = 0; j < ARRAYCOUNT(wallhit->basecolours); j++) {
					u32 alpha;

					if (f22 > 1.0f) {
						f22 = 1.0f;
					}

					alpha = wallhit->basecolours[j].a * f24;

					if (alpha > 255) {
						alpha = 255;
					}

					wallhit->finalcolours[j].a = alpha;
				}
			} else {
				if (wallhit->inuse) {
					wallhit->vertices2 = NULL;

					for (j = 0; j < ARRAYCOUNT(wallhit->basecolours); j++) {
						wallhit->finalcolours[j].a = wallhit->basecolours[j].a;
					}
				} else {
					wallhit->vertices2 = NULL;
				}
			}
		}

		wallhit->unk6f_05 = true;
	}
}

const char var7f1b5a5c[] = "g_MaxRound = %s%s%f";
const char var7f1b5a70[] = "";
const char var7f1b5a74[] = "";
const char var7f1b5a78[] = "g_MinRound = %s%s%f";
const char var7f1b5a8c[] = "";
const char var7f1b5a90[] = "";
const char var7f1b5a94[] = "Done %d Z buffer calcs";
const char var7f1b5aac[] = "ZOOM : g_ZoomFactor=%s%s%f";
const char var7f1b5ac8[] = "";
const char var7f1b5acc[] = "";
const char var7f1b5ad0[] = "ZOOM : g_ZoomScalar=%s%s%f";
const char var7f1b5aec[] = "";
const char var7f1b5af0[] = "";
const char var7f1b5af4[] = "ZOOM : scale=%s%s%f";
const char var7f1b5b08[] = "";
const char var7f1b5b0c[] = "";
const char var7f1b5b10[] = "WallHit_Tick : Status - RED";
const char var7f1b5b2c[] = "WallHit_Tick : Status - YELLOW (%u)";
const char var7f1b5b50[] = "WallHit_Tick : Status - GREEN";
const char var7f1b5b70[] = "WallHit_Tick : %d Used";
const char var7f1b5b88[] = "WallHit_Tick : %d Free";
const char var7f1b5ba0[] = "WallHit_Tick : %d Pending";
const char var7f1b5bbc[] = "WallHit_Tick : %d Blood";
const char var7f1b5bd4[] = "WallHit_Tick : %d Other";
const char var7f1b5bec[] = "WallHit_Tick : %d Ratio";
const char var7f1b5c04[] = "WallHit_Tick : %d(%d) Prop Hits";
const char var7f1b5c24[] = "tLifeTime=%s%s%f, tScalarGbl=%f";
const char var7f1b5c44[] = "";
const char var7f1b5c48[] = "";

void wallhitCreate(struct coord *relpos, struct coord *arg1, struct coord *arg2, s16 arg3[3],
		s16 arg4[3], s16 texnum, RoomNum room, struct prop *objprop,
		s8 mtxindex, s8 arg9, struct chrdata *chr, bool xlu)
{
	f32 scale = RANDOMFRAC() * 0.1f + 0.6f;
	f32 width = g_WallhitTexes[texnum].width * scale;
	f32 height = g_WallhitTexes[texnum].height * scale;

	wallhitCreateWith20Args(relpos, arg1, arg2, arg3,
			arg4, texnum, room, objprop,
			NULL, mtxindex, arg9, chr,
			width, height, 0xff, 0xff,
			0, 0, 0, xlu);
}

void wallhitCreateWith20Args(struct coord *relpos, struct coord *arg1, struct coord *arg2, s16 arg3[3],
		s16 arg4[3], s16 texnum, RoomNum room, struct prop *objprop,
		struct prop *chrprop, s8 mtxindex, s8 arg10, struct chrdata *chr,
		f32 width, f32 height, u8 minalpha, u8 maxalpha,
		s32 rotdeg, u32 timermax, u32 timerspeed, bool xlu)
{
	struct coord sp1f4;
	struct coord sp1e8;
	struct coord sp1dc;
	struct coord sp1d0;
	bool xiszero;
	bool ziszero;
	bool yiszero;
	struct coord sp1b8;
	struct coord sp1ac;
	struct coord sp17c[4];
	s32 type;
	f32 f0;
	u8 alpha;
	struct wallhit *wallhit;
	f32 mult = 1.0f;
	f32 brightnessfrac;
#if VERSION >= VERSION_NTSC_1_0
	bool paintball;
#endif
	u32 stack[6];
	struct coord sp13c;
	struct coord sp130;
	u32 stack2[3];
	struct coord sp118;
	s32 i;
	s32 room2;
	u32 range;
	struct coord sp100;

	sp1b8.x = arg1->x;
	sp1b8.y = arg1->y;
	sp1b8.z = arg1->z;

#if VERSION >= VERSION_NTSC_1_0
	func0f177164(&sp1b8, &sp1b8, 956, "wallhit.c");
#else
	func0f177164(&sp1b8, &sp1b8, 955, "wallhit.c");
#endif

#if VERSION >= VERSION_NTSC_1_0
	paintball = chrIsUsingPaintball(chr);

	if (paintball && g_WallhitTexes[texnum].type != WALLHITTYPE_BLOOD) {
		if (texnum != WALLHITTEX_SCORCH) {
			width = 15.0f;
			height = 15.0f;
		}

		texnum = WALLHITTEX_PAINT;
		timermax = TICKS(10);
	}

	switch (texnum) {
	case WALLHITTEX_BULLET2:
	case WALLHITTEX_BLOOD1:
	case WALLHITTEX_BLOOD2:
	case WALLHITTEX_BLOOD3:
	case WALLHITTEX_BLOOD4:
	case WALLHITTEX_BPGLASS1:
	case WALLHITTEX_BPGLASS2:
	case WALLHITTEX_BPGLASS3:
	case WALLHITTEX_METAL:
		break;
	default:
	case WALLHITTEX_SCORCH:
	case WALLHITTEX_PAINT:
	case WALLHITTEX_WOOD:
		rotdeg = rngRandom() % 360;
		break;
	}

	type = paintball ? WALLHITTYPE_PAINT : g_WallhitTexes[texnum].type;
#else
	switch (texnum) {
	case WALLHITTEX_BULLET2:
	case WALLHITTEX_PAINT:
	case WALLHITTEX_BLOOD1:
	case WALLHITTEX_BLOOD2:
	case WALLHITTEX_BLOOD3:
	case WALLHITTEX_BLOOD4:
	case WALLHITTEX_BPGLASS1:
	case WALLHITTEX_BPGLASS2:
	case WALLHITTEX_BPGLASS3:
	case WALLHITTEX_METAL:
		break;
	default:
	case WALLHITTEX_SCORCH:
	case WALLHITTEX_WOOD:
		rotdeg = rngRandom() % 360;
		break;
	}

	if (chrIsUsingPaintball(chr) && g_WallhitTexes[texnum].type != WALLHITTYPE_BLOOD) {
		if (texnum != WALLHITTEX_SCORCH) {
			width = 15.0f;
			height = 15.0f;
		}

		rotdeg = rngRandom() % 360;
		type = WALLHITTYPE_PAINT;
		timermax = TICKS(10);
	} else {
		type = g_WallhitTexes[texnum].type;
	}
#endif

	if (g_FreeWallhits != NULL) {
		// Check if we are at a limit and need to free some old wallhits
		s32 room2 = objprop ? 0 : room;
		s32 max;

		if (objprop) {
			max = g_MaxPropWallhits - 1;
		} else {
			max = g_MaxBgWallhitsPerRoom - 1;
		}

		if (g_WallhitCountsPerRoom[room2] > max) {
			if (!wallhitRemoveOneInRoom(room2)) {
				return;
			}
		} else if (g_WallhitCountsPerRoom[room] > g_MaxBgWallhitsPerRoom) {
			if (!wallhitRemoveOneInRoom(room)) {
				return;
			}
		}

		// Adjust counters
		g_WallhitCountsPerRoom[room2]++;
		g_WallhitsNumFree--;
		g_WallhitsNumSettled++;

		if (g_WallhitTexes[texnum].type == WALLHITTYPE_BLOOD) {
			g_WallhitsNumBloodSettled++;
		} else {
			g_WallhitsNumNonbloodSettled++;
		}

		// Remove the head of the free list and prepend it to the active list
		wallhit = g_FreeWallhits;
		g_FreeWallhits = wallhit->globalnext;
		wallhit->globalnext = g_ActiveWallhits;
		g_ActiveWallhits = wallhit;

		wallhit->xlu = xlu;
		wallhit->localnext = NULL;

		// Prepend to the prop-local or room-local list
		if (objprop != NULL) {
			if (xlu) {
				wallhit->localnext = objprop->xluwallhits;
				objprop->xluwallhits = wallhit;
			} else {
				wallhit->localnext = objprop->opawallhits;
				objprop->opawallhits = wallhit;
			}

			wallhit->objprop = objprop;
		} else {
			if (xlu) {
				wallhit->localnext = g_Rooms[room].xluwallhits;
				g_Rooms[room].xluwallhits = wallhit;
			} else {
				wallhit->localnext = g_Rooms[room].opawallhits;
				g_Rooms[room].opawallhits = wallhit;
			}

			wallhit->objprop = NULL;
		}

		sp1ac.f[0] = relpos->x;
		sp1ac.f[1] = relpos->y;
		sp1ac.f[2] = relpos->z;

		xiszero = ABS(arg1->x) < var800845d4 ? true : false;
		yiszero = ABS(arg1->y) < var800845d4 ? true : false;
		ziszero = ABS(arg1->z) < var800845d4 ? true : false;

		if (xiszero && ziszero) {
			sp1f4.x = -1.0f;
			sp1f4.y = 0.0f;
			sp1f4.z = 0.0f;

			sp1e8.x = 0.0f;
			sp1e8.y = 0.0f;
			sp1e8.z = (arg1->y >= 0.0f ? 1.0f : -1.0f) * -1.0f;
		} else if (xiszero && yiszero) {
			sp1f4.x = arg1->z >= 0.0f ? 1.0f : -1.0f;
			sp1f4.y = 0.0f;
			sp1f4.z = 0.0f;

			sp1e8.x = 0.0f;
			sp1e8.y = -1.0f;
			sp1e8.z = 0.0f;
		} else if (yiszero && ziszero) {
			sp1f4.x = 0.0f;
			sp1f4.y = (arg1->x >= 0.0f ? 1.0f : -1.0f) * -1.0f;
			sp1f4.z = 0.0f;

			sp1e8.x = 0.0f;
			sp1e8.y = 0.0f;
			sp1e8.z = 1.0f;
		} else if (arg3 && arg4) {
			sp13c.x = arg3[0];
			sp13c.y = arg3[1];
			sp13c.z = arg3[2];

			sp130.x = arg4[0];
			sp130.y = arg4[1];
			sp130.z = arg4[2];

#if VERSION >= VERSION_NTSC_1_0
			func0f177164(&sp13c, &sp13c, 1151, "wallhit.c");
			func0f177164(&sp130, &sp130, 1152, "wallhit.c");
#else
			func0f177164(&sp13c, &sp13c, 1142, "wallhit.c");
			func0f177164(&sp130, &sp130, 1143, "wallhit.c");
#endif

			f0 = (sp13c.x * sp130.x + sp13c.y * sp130.y + sp13c.z * sp130.z) * -1.0f;

			sp118.x = f0 * sp13c.x + sp130.x;
			sp118.y = f0 * sp13c.y + sp130.y;
			sp118.z = f0 * sp13c.z + sp130.z;

			func0f1770ac(&sp13c, &sp118, &sp100);

			sp1f4.x = sp13c.x;
			sp1f4.y = sp13c.y;
			sp1f4.z = sp13c.z;

			sp1e8.x = sp118.x;
			sp1e8.y = sp118.y;
			sp1e8.z = sp118.z;
		} else {
			f32 f0 = sqrtf(sp1b8.x * sp1b8.x + sp1b8.z * sp1b8.z);
			f32 xvalue = sp1b8.x / f0;
			f32 zvalue = sp1b8.z / f0;

			sp1f4.x = zvalue;
			sp1f4.y = 0.0f;
			sp1f4.z = -xvalue;

			sp1e8.x = sp1b8.y * xvalue;
			sp1e8.y = -f0;
			sp1e8.z = sp1b8.y * zvalue;
		}

		osSyncPrintf("AFT : tVecU (along) = (%s%s%f,%f,%f)\n", "", "", sp1f4.x, sp1f4.y, sp1f4.z);
		osSyncPrintf("AFT : tVecV (up)    = (%s%s%f,%f,%f)\n", "", "", sp1e8.x, sp1e8.y, sp1e8.z);

		if (rotdeg != 0) {
			u32 stack[6];
			f32 spd0 = sinf(rotdeg * 0.017453292f);
			f32 spcc = cosf(rotdeg * 0.017453292f);
			u32 stack2[12];

			sp1dc.x = spcc * sp1f4.x + spd0 * sp1e8.x;
			sp1dc.y = spcc * sp1f4.y + spd0 * sp1e8.y;
			sp1dc.z = spcc * sp1f4.z + spd0 * sp1e8.z;

			sp1d0.x = -spd0 * sp1f4.x + spcc * sp1e8.x;
			sp1d0.y = -spd0 * sp1f4.y + spcc * sp1e8.y;
			sp1d0.z = -spd0 * sp1f4.z + spcc * sp1e8.z;
		} else {
			sp1dc.x = sp1f4.x;
			sp1dc.y = sp1f4.y;
			sp1dc.z = sp1f4.z;

			sp1d0.x = sp1e8.x;
			sp1d0.y = sp1e8.y;
			sp1d0.z = sp1e8.z;
		}

		if (objprop) {
			struct defaultobj *obj = objprop->obj;
			Mtxf *mtx = &obj->model->matrices[mtxindex];
			struct doorobj *door = objprop->door;
			struct coord sp84;
			struct coord sp78;

			if (obj->type == OBJTYPE_DOOR && (door->doorflags & DOORFLAG_FLIP)) {
				sp1d0.x = -1.0f * sp1d0.x;
				sp1d0.y = -1.0f * sp1d0.y;
				sp1d0.z = -1.0f * sp1d0.z;
			}

			sp84.x = sp1dc.x;
			sp84.y = sp1dc.y;
			sp84.z = sp1dc.z;

			sp78.x = sp1d0.x;
			sp78.y = sp1d0.y;
			sp78.z = sp1d0.z;

			mtx4RotateVecInPlace(mtx, &sp84);
			mtx4RotateVecInPlace(mtx, &sp78);

			width /= sqrtf(sp84.x * sp84.x + sp84.y * sp84.y + sp84.z * sp84.z);
			height /= sqrtf(sp78.x * sp78.x + sp78.y * sp78.y + sp78.z * sp78.z);

			if (xlu) {
				obj->hidden2 |= OBJH2FLAG_HASXLU;
			} else {
				obj->hidden2 |= OBJH2FLAG_HASOPA;
			}
		} else {
			struct coord *roompos = roomGetPosPtr(room);

			if (arg2 != NULL) {
				f32 xdist = arg2->x - relpos->x;
				f32 ydist = arg2->y - relpos->y;
				f32 zdist = arg2->z - relpos->z;
				f32 sum = xdist * sp1b8.x + ydist * sp1b8.y + zdist * sp1b8.z;

				if (sum < 0.0f) {
					sp1d0.x = -1.0f * sp1d0.x;
					sp1d0.y = -1.0f * sp1d0.y;
					sp1d0.z = -1.0f * sp1d0.z;
				}
			}

			sp1ac.x = mult * sp1ac.x;
			sp1ac.y = mult * sp1ac.y;
			sp1ac.z = mult * sp1ac.z;

			sp1ac.x -= roompos->x;
			sp1ac.y -= roompos->y;
			sp1ac.z -= roompos->z;

			width *= mult;
			height *= mult;
		}

		sp1dc.x = width * sp1dc.x;
		sp1dc.y = width * sp1dc.y;
		sp1dc.z = width * sp1dc.z;

		sp1d0.x = height * sp1d0.x;
		sp1d0.y = height * sp1d0.y;
		sp1d0.z = height * sp1d0.z;

		sp17c[0].x = sp1dc.x + sp1d0.x;
		sp17c[0].y = sp1dc.y + sp1d0.y;
		sp17c[0].z = sp1dc.z + sp1d0.z;

		sp17c[1].x = sp1dc.x - sp1d0.x;
		sp17c[1].y = sp1dc.y - sp1d0.y;
		sp17c[1].z = sp1dc.z - sp1d0.z;

		sp17c[2].x = -sp17c[0].x;
		sp17c[2].y = -sp17c[0].y;
		sp17c[2].z = -sp17c[0].z;

		sp17c[3].x = sp1d0.x - sp1dc.x; \
		sp17c[3].y = sp1d0.y - sp1dc.y; \
		sp17c[3].z = sp1d0.z - sp1dc.z;

		wallhit->relpos.x = relpos->x;
		wallhit->relpos.y = relpos->y;
		wallhit->relpos.z = relpos->z;

		wallhit->mtxindex = mtxindex;
		wallhit->unk6f_01 = arg10;
		wallhit->roomnum = room;
		wallhit->inuse = true;
		wallhit->fading = false;
		wallhit->texturenum = texnum;
		wallhit->chrprop = chrprop;
		wallhit->objprop = objprop;
		wallhit->vertices2 = NULL;
		wallhit->timermax = timermax;
		wallhit->timercur = 0;
		wallhit->expanding = true;
		wallhit->timerspeed = timerspeed ? timerspeed : 8;
		wallhit->createdframe = g_Vars.lvframenum;
		wallhit->unk6f_05 = false;

		for (i = 0; i < ARRAYCOUNT(sp17c); i++) {
			struct coord sp58;
			s16 x;
			s16 y;
			s16 z;

			sp58.x = sp17c[i].x + sp1ac.x;
			sp58.y = sp17c[i].y + sp1ac.y;
			sp58.z = sp17c[i].z + sp1ac.z;

			x = wallhitFinaliseAxis(sp58.x);
			y = wallhitFinaliseAxis(sp58.y);
			z = wallhitFinaliseAxis(sp58.z);

			wallhit->vertices[i].x = x;
			wallhit->vertices[i].y = y;
			wallhit->vertices[i].z = z;
			wallhit->vertices[i].colour = i * 4;
		}

		wallhit->vertices[0].s = 0;
		wallhit->vertices[0].t = g_TexWallhitConfigs[texnum].height * 32;
		wallhit->vertices[1].s = 0;
		wallhit->vertices[1].t = 0;
		wallhit->vertices[2].s = g_TexWallhitConfigs[texnum].width * 32;
		wallhit->vertices[2].t = 0;
		wallhit->vertices[3].s = g_TexWallhitConfigs[texnum].width * 32;
		wallhit->vertices[3].t = g_TexWallhitConfigs[texnum].height * 32;

		if (wallhit->objprop) {
			struct prop *prop = wallhit->objprop;

			while (prop->parent) {
				prop = prop->parent;
			}

			room2 = prop->rooms[0];
		} else {
			room2 = wallhit->roomnum;
		}

		{
			u8 r;
			u8 g;
			u8 b;
			u8 a;

			brightnessfrac = roomGetFinalBrightnessForPlayer(room2) * (1.0f / 255.0f);

			range = maxalpha - (u32)minalpha;

			if (range) {
				alpha = minalpha + (rngRandom() % range);
			} else {
				alpha = 0;
			}

			for (i = 0; i < ARRAYCOUNT(wallhit->basecolours); i++) {
				switch (type) {
				case WALLHITTYPE_BULLET:
					r = g = b = 255 - (rngRandom() % 40);
					a = alpha ? alpha : 255;
					break;
				case WALLHITTYPE_SOFT:
					r = g = b = rngRandom() % 70;
					a = alpha ? alpha : 255 - (rngRandom() % 50);
					break;
				case WALLHITTYPE_SCORCH:
					r = g = b = rngRandom() % 50;
					a = alpha ? alpha : 255 - (rngRandom() % 80);
					break;
				case WALLHITTYPE_BLOOD:
					r = g_WallhitBloodColour[0];
					g = g_WallhitBloodColour[1];
					b = g_WallhitBloodColour[2];
					a = alpha ? alpha : 255;
					break;
				case WALLHITTYPE_PAINT:
					r = (rngRandom() % 2) ? 0xff : 0;
					g = (rngRandom() % 2) ? 0xff : 0;
					b = (rngRandom() % 2) ? 0xff : 0;
					a = alpha ? alpha : 255;
					break;
				default:
					osSyncPrintf("Wallhit colour %d not implemented, substituting black\n", type);
					r = g = b = 0;
					a = alpha ? alpha : 255;
					break;
				}

				wallhit->basecolours[i].r = r;
				wallhit->basecolours[i].g = g;
				wallhit->basecolours[i].b = b;
				wallhit->basecolours[i].a = a;

				wallhit->finalcolours[i].r = r * brightnessfrac;
				wallhit->finalcolours[i].g = g * brightnessfrac;
				wallhit->finalcolours[i].b = b * brightnessfrac;
				wallhit->finalcolours[i].a = wallhit->timermax ? 0 : a;
			}
		}
	}
}

/**
 * Maybe a LOD calculation?
 */
s32 wallhit0f140750(struct coord *coord)
{
	f32 x;
	f32 y;
	f32 z;
	f32 tmp;

	x = g_Vars.currentplayer->projectionmtx->m[3][0] - coord->f[0];
	y = g_Vars.currentplayer->projectionmtx->m[3][1] - coord->f[1];
	z = g_Vars.currentplayer->projectionmtx->m[3][2] - coord->f[2];

	var8007f740++;

	if (x < 0) {
		x = -x;
	}

	if (y < 0) {
		y = -y;
	}

	if (z < 0) {
		z = -z;
	}

	if (y > x) {
		x = y;
	}

	if (z > x) {
		x = z;
	}

	tmp = x * var8007f74c;

	if (tmp > 1600) {
		return 4;
	}

	if (tmp > 400) {
		return 8;
	}

	if (tmp > 300) {
		return 16;
	}

	if (tmp > 200) {
		return 32;
	}

	if (tmp > 100) {
		return 64;
	}

	return 128;
}

Gfx *wallhitRenderOpaBgHits(s32 roomnum, Gfx *gdl)
{
	struct wallhit *wallhit;
	Col *colours;
	s32 prevtexturenum;
	s32 prev6b;

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gSPSetGeometryMode(gdl++, G_CULL_BACK);
#if VERSION >= VERSION_NTSC_1_0
	gDPSetTextureDetail(gdl++, G_TD_CLAMP);
#endif
	gDPSetColorDither(gdl++, G_CD_NOISE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);

	prevtexturenum = -1;
	prev6b = -1;

	gdl = roomApplyMtx(gdl, roomnum);

	wallhit = g_Rooms[roomnum].opawallhits;

	while (wallhit) {
		if (wallhit->inuse && wallhit->unk6f_05) {
			if (wallhit->xlu) {
				wallhit->unk6b = 1;
			} else {
				wallhit->unk6b = wallhit0f140750(&wallhit->relpos);
			}

			if (wallhit->texturenum != prevtexturenum || wallhit->unk6b != prev6b) {
				texSelect(&gdl, &g_TexWallhitConfigs[wallhit->texturenum], 2, wallhit->unk6b, 2, 1, NULL);

				prevtexturenum = wallhit->texturenum;
				prev6b = wallhit->unk6b;
			}

			colours = gfxAllocateColours(4);
			colours[0] = wallhit->finalcolours[0];
			colours[1] = wallhit->finalcolours[1];
			colours[2] = wallhit->finalcolours[2];
			colours[3] = wallhit->finalcolours[3];

			gSPColor(gdl++, osVirtualToPhysical(colours), 4);

			if (wallhit->vertices2 != NULL) {
				gSPVertex(gdl++, wallhit->vertices2, 4, 0);
			} else {
				gSPVertex(gdl++, osVirtualToPhysical(&wallhit->vertices), 4, 0);
			}

			gSPTri2(gdl++, 0, 1, 2, 0, 2, 3);
		}

		wallhit = wallhit->localnext;
	}

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_BAYER);

	return gdl;
}

Gfx *wallhitRenderXluBgHits(s32 roomnum, Gfx *gdl)
{
	struct wallhit *wallhit;
	Col *colours;
	s32 prevtexturenum;
	s32 prev6b;

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
#if VERSION >= VERSION_NTSC_1_0
	gDPSetTextureDetail(gdl++, G_TD_CLAMP);
#endif
	gDPSetColorDither(gdl++, G_CD_NOISE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);

	prevtexturenum = -1;
	prev6b = -1;

	gdl = roomApplyMtx(gdl, roomnum);

	wallhit = g_Rooms[roomnum].xluwallhits;

	while (wallhit) {
		if (wallhit->inuse && wallhit->unk6f_05) {
			wallhit->unk6b = 1;

			if (wallhit->texturenum != prevtexturenum || wallhit->unk6b != prev6b) {
				texSelect(&gdl, &g_TexWallhitConfigs[wallhit->texturenum], 2, wallhit->unk6b, 2, 1, NULL);

				prevtexturenum = wallhit->texturenum;
				prev6b = wallhit->unk6b;
			}

			colours = gfxAllocateColours(4);
			colours[0] = wallhit->finalcolours[0];
			colours[1] = wallhit->finalcolours[1];
			colours[2] = wallhit->finalcolours[2];
			colours[3] = wallhit->finalcolours[3];

			gSPColor(gdl++, osVirtualToPhysical(colours), 4);

			if (wallhit->vertices2 != NULL) {
				gSPVertex(gdl++, wallhit->vertices2, 4, 0);
			} else {
				gSPVertex(gdl++, osVirtualToPhysical(&wallhit->vertices), 4, 0);
			}

			gSPTri2(gdl++, 0, 1, 2, 0, 2, 3);
		}

		wallhit = wallhit->localnext;
	}

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_BAYER);

	return gdl;
}

Gfx *wallhitRenderPropHits(Gfx *gdl, struct prop *prop, bool xlu)
{
	Col *colours;
	struct defaultobj *obj = prop->obj;
	bool hasany = false;
	struct wallhit *wallhit;
	s16 prevmtxindex = -1;
	s32 prevtexturenum = -1;
	s32 prev6b = -1;

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		return gdl;
	}

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);

	if (!xlu) {
		gSPSetGeometryMode(gdl++, G_CULL_BACK);
	}

#if VERSION >= VERSION_NTSC_1_0
	gDPSetTextureDetail(gdl++, G_TD_CLAMP);
#endif
	gDPSetColorDither(gdl++, G_CD_NOISE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);

	wallhit = xlu ? prop->xluwallhits : prop->opawallhits;

	while (wallhit) {
		if (wallhit->inuse) {
			hasany = true;

			if (wallhit->mtxindex != prevmtxindex) {
				Mtxf *mtx = &obj->model->matrices[wallhit->mtxindex];
				if (wallhit->mtxindex);
				prevmtxindex = wallhit->mtxindex;
				gSPMatrix(gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
			}

			if (!xlu) {
				if (wallhit->xlu) {
					wallhit->unk6b = 1;
				} else {
					struct coord sp74;
					sp74.x = wallhit->relpos.x + prop->pos.x;
					sp74.y = wallhit->relpos.y + prop->pos.y;
					sp74.z = wallhit->relpos.z + prop->pos.z;

					wallhit->unk6b = wallhit0f140750(&sp74);
				}
			} else {
				wallhit->unk6b = 1;
			}

			if (prevtexturenum != wallhit->texturenum || prev6b != wallhit->unk6b) {
				texSelect(&gdl, &g_TexWallhitConfigs[wallhit->texturenum], 2, wallhit->unk6b, 2, 1, NULL);

				prevtexturenum = wallhit->texturenum;
				prev6b = wallhit->unk6b;
			}

			colours = gfxAllocateColours(4);
			colours[0] = wallhit->finalcolours[0];
			colours[1] = wallhit->finalcolours[1];
			colours[2] = wallhit->finalcolours[2];
			colours[3] = wallhit->finalcolours[3];

			gSPColor(gdl++, osVirtualToPhysical(colours), 4);

			if (wallhit->vertices2 != NULL) {
				gSPVertex(gdl++, wallhit->vertices2, 4, 0);
			} else {
				gSPVertex(gdl++, osVirtualToPhysical(&wallhit->vertices), 4, 0);
			}

			gSPTri2(gdl++, 0, 1, 2, 0, 2, 3);
		}

		wallhit = wallhit->localnext;
	}

	if (!hasany) {
		obj->hidden2 &= ~(1 << xlu);
	}

	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_BAYER);

	return gdl;
}

Gfx *wallhitRenderBgHits(s32 roomnum, Gfx *gdl)
{
	if (g_Rooms[roomnum].opawallhits != NULL) {
		gdl = wallhitRenderOpaBgHits(roomnum, gdl);
	}

	if (g_Rooms[roomnum].xluwallhits != NULL) {
		gdl = wallhitRenderXluBgHits(roomnum, gdl);
	}

	return gdl;
}

void wallhitsRecolour(void)
{
	s32 i;
	s32 j;
	u32 stack;
	struct wallhit *wallhit;
	f32 r;
	f32 g;
	f32 b;

	for (i = 0, wallhit = g_Wallhits; i < g_WallhitsMax; i++) {
		if (wallhit->roomnum > 0) {
			s32 room = -1;

			if (wallhit->objprop != NULL) {
				struct prop *prop = wallhit->objprop;

				while (prop->parent) {
					prop = prop->parent;
				}

				for (j = 0; prop->rooms[j] != -1; j++) {
					if (g_Rooms[prop->rooms[j]].flags & ROOMFLAG_NEEDRESHADE) {
						room = prop->rooms[j];

						r = g_Rooms[room].highlightfrac_r;
						g = g_Rooms[room].highlightfrac_g;
						b = g_Rooms[room].highlightfrac_b;
						break;
					}
				}
			} else {
				if (g_Rooms[wallhit->roomnum].flags & ROOMFLAG_NEEDRESHADE) {
					room = wallhit->roomnum;

					r = g_Rooms[room].highlightfrac_r;
					g = g_Rooms[room].highlightfrac_g;
					b = g_Rooms[room].highlightfrac_b;
				}
			}

			if (room > 0) {
				for (j = 0; j < ARRAYCOUNT(wallhit->basecolours); j++) {
					wallhit->finalcolours[j].r = wallhit->basecolours[j].r * r;
					wallhit->finalcolours[j].g = wallhit->basecolours[j].g * g;
					wallhit->finalcolours[j].b = wallhit->basecolours[j].b * b;
				}
			}
		}

		wallhit++;
	}
}

void wallhitFadeSplatsForRemovedChr(struct prop *chrprop)
{
	s32 i;

	for (i = 0; i < g_WallhitsMax; i++) {
		struct wallhit *wallhit = &g_Wallhits[i];

		if (wallhit->chrprop
				&& wallhit->roomnum > 0
				&& wallhit->chrprop == chrprop
				&& g_WallhitTexes[wallhit->texturenum].type == WALLHITTYPE_BLOOD) {
			if (IS_BLOOD_DROP(wallhit->texturenum) || (rngRandom() % 100) < 35) {
				wallhitFade(wallhit, TICKS(120));
			} else {
				wallhit->createdframe = g_Vars.lvframenum;
			}
		}
	}
}

void wallhitRemoveOldestWoundedSplatByChr(struct prop *chrprop)
{
	s32 oldestframe = 0x0fffffff;
	s32 oldestindex = -1;
	s32 i;

	for (i = 0; i < g_WallhitsMax; i++) {
		struct wallhit *wallhit = &g_Wallhits[i];

		if (wallhit->chrprop
				&& wallhit->roomnum > 0
				&& wallhit->chrprop == chrprop
				&& !wallhit->fading
				&& g_WallhitTexes[wallhit->texturenum].type == WALLHITTYPE_BLOOD
				&& IS_BLOOD_DROP(wallhit->texturenum)
				&& wallhit->createdframe < oldestframe) {
			oldestframe = wallhit->createdframe;
			oldestindex = i;
		}
	}

	if (oldestindex != -1) {
		wallhitFade(&g_Wallhits[oldestindex], TICKS(120));
	}
}

Gfx *wallhit0f141814(Gfx *gdl, s32 arg1)
{
	return gdl;
}
