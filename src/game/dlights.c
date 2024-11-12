#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/dlights.h"
#include "game/gfxmemory.h"
#include "game/propsnd.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/smoke.h"
#include "game/sparks.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/zbuf.h"
#include "game/mplayer/scenarios.h"
#include "game/portal.h"
#include "game/propobj.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_17ce0.h"
#include "lib/lib_2f490.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#include "platform.h"

const char var7f1a78e0[] = "LIGHTS : Hit occured on light %d in room %d\n";
const char var7f1a7910[] = "L2(%d) -> ";
const char var7f1a791c[] = "L2 -> BUILD LIGHTS TRANSFER TABLE - Starting\n";
const char var7f1a794c[] = "L2(%d) -> ";
const char var7f1a7958[] = "L2_BuildTransferTables -> Found %d portals\n";
const char var7f1a7984[] = "L2(%d) -> ";
const char var7f1a7990[] = "L2_BuildTransferTables -> Found %d rooms\n";
const char var7f1a79bc[] = "L2(%d) -> ";
const char var7f1a79c8[] = "L2_BuildTransferTables -> Alloc %u bytes of free memory\n";
const char var7f1a7a04[] = "L2(%d) -> ";
const char var7f1a7a10[] = "L2_BuildTransferTables -> Alloc %u bytes for scratch\n";
const char var7f1a7a48[] = "L2(%d) -> ";
const char var7f1a7a54[] = "L2 -> Building room based light transmission table\n";
const char var7f1a7a88[] = "L2(%d) -> ";
const char var7f1a7a94[] = "L2 -> Finished\n";
const char var7f1a7aa4[] = "L2(%d) -> ";
const char var7f1a7ab0[] = "L2 -> Generating room parameters from BG data\n";
const char var7f1a7ae0[] = "L2(%d) -> ";
const char var7f1a7aec[] = "L2 -> Light2_ProcessBgParams room %d does not have a 3D bounding box => Room Volume is bodged!\n";
const char var7f1a7b4c[] = "L2(%d) -> ";
const char var7f1a7b58[] = "%s%sL2 -> Surface area bodged for room %d - using %f\n";
const char var7f1a7b90[] = "";
const char var7f1a7b94[] = "";
const char var7f1a7b98[] = "L2(%d) -> ";
const char var7f1a7ba4[] = "L2 -> Finished\n";

s32 *var8009cad0;
u32 var8009cad4;
s32 *var8009cad8;
s32 g_NumPortals;
s32 var8009cae0;
s32 var8009cae4;
f32 (*var8009cae8)(s32 roomnum, f32 mult, s32 portalnum1, s32 portalnum2);
u8 var8009caec;
u8 var8009caed;
u8 var8009caee;
u8 var8009caef;
u8 var8009caf0;

struct var80061420 *var80061420 = NULL;
u32 var80061424 = 0x00000000;
struct coord *var80061428 = NULL;
u16 **var8006142c = NULL;
u16 **var80061430 = NULL;
f32 *var80061434 = NULL;
bool *var80061438 = NULL;
f32 var8006143c = 50;
u32 var80061440 = 0x00000000;
u32 var80061444 = 1;
u32 var80061448 = 0x00000000;
bool g_IsSwitchingGoggles = false;
u32 var80061450 = 0x00000000;
u32 var80061454 = 0xffffffff;

#if VERSION >= VERSION_NTSC_1_0
s32 g_LightsPrevTickMode = 0;
#endif

u32 func0f000920(s32 portalnum1, s32 portalnum2)
{
	if (portalnum1 != portalnum2) {
		s32 upper = (portalnum1 > portalnum2) ? portalnum1 : portalnum2;
		s32 lower = (portalnum1 < portalnum2) ? portalnum1 : portalnum2;

		return var80061430[upper][lower];
	}

	return 0;
}

struct light *roomGetLight(s32 roomnum, s32 lightnum)
{
	return (struct light *)&g_BgLightsFileData[(g_Rooms[roomnum].lightindex + lightnum) * 0x22];
}

u8 roomGetFinalBrightness(s32 roomnum)
{
	s32 brightness = g_Rooms[roomnum].br_flash + g_Rooms[roomnum].br_settled_regional;

	if (brightness > 255) {
		brightness = 255;
	}

	if (brightness < 0) {
		brightness = 0;
	}

	return brightness;
}

u8 roomGetFinalBrightnessForPlayer(s32 roomnum)
{
	s32 brightness = g_Rooms[roomnum].br_flash;

	if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
		brightness += var8009caec;
	} else {
		brightness += g_Rooms[roomnum].br_settled_regional;
	}

	if (brightness > 255) {
		brightness = 255;
	}

	if (brightness < 0) {
		brightness = 0;
	}

	return brightness;
}

u8 func0f000b18(u32 arg0)
{
	return 255;
}

u8 roomGetSettledRegionalBrightnessForPlayer(s32 roomnum)
{
	u32 brightness;

	if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
		return var8009caec;
	}

	if (g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) {
		brightness = g_Rooms[roomnum].br_settled_regional;
	} else {
		brightness = 255;
	}

	return brightness;
}

u8 roomGetSettledLocalBrightness(s32 room)
{
	return g_Rooms[room].br_settled_local & 0xff;
}

s32 roomGetFlashBrightness(s32 roomnum)
{
	if (g_Rooms[roomnum].br_flash > 255) {
		return 255;
	}

	if (g_Rooms[roomnum].br_flash < 0) {
		return 0;
	}

	return (g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) ? g_Rooms[roomnum].br_flash : 0;
}

f32 roomGetLightOpCurFrac(s32 roomnum)
{
	return g_Rooms[roomnum].lightop_cur_frac;
}

f32 roomGetFinalBrightnessFrac(s32 roomnum)
{
	f32 frac = (g_Rooms[roomnum].br_flash + g_Rooms[roomnum].br_settled_regional) / (1.0f / 255.0f);

	if (frac > 1) {
		frac = 1;
	}

	if (frac < 0) {
		frac = 0;
	}

	return frac;
}

f32 roomGetSettledRegionalBrightnessFrac(s32 roomnum)
{
	return g_Rooms[roomnum].br_settled_regional / 255.0f;
}

f32 roomGetSettledLocalBrightnessFrac(s32 roomnum)
{
	return g_Rooms[roomnum].br_settled_local / 255.0f;
}

/**
 * The resulting position is not a world position. It is relative to the room.
 */
bool lightGetBboxCentre(s32 roomnum, u32 lightnum, struct coord *pos)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	s32 i;
	light += lightnum;

	pos->x = 0;
	pos->y = 0;
	pos->z = 0;

	for (i = 0; i < ARRAYCOUNT(light->bbox); i++) {
		pos->x += light->bbox[i].x;
		pos->y += light->bbox[i].y;
		pos->z += light->bbox[i].z;
	}

	pos->x *= 0.25f;
	pos->y *= 0.25f;
	pos->z *= 0.25f;

	return true;
}

bool lightIsHealthy(s32 roomnum, s32 lightnum)
{
	bool healthy;

	if (roomnum && roomGetLight(roomnum, lightnum)->healthy) {
		healthy = true;
	} else {
		healthy = false;
	}

	return healthy;
}

bool lightIsVulnerable(s32 roomnum, s32 lightnum)
{
	struct light *light = roomGetLight(roomnum, lightnum);

	return light->vulnerable;
}

bool lightIsOn(s32 roomnum, s32 lightnum)
{
	bool on;

	if (roomnum && roomGetLight(roomnum, lightnum)->on) {
		on = true;
	} else {
		on = false;
	}

	return on;
}

void roomSetFlashBrightness(s32 roomnum, s32 value)
{
	g_Rooms[roomnum].br_flash = value;
}

void lightGetDirection(s32 roomnum, u32 lightnum, struct coord *dir)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	light += lightnum;

	dir->x = light->dirx;
	dir->y = light->diry;
	dir->z = light->dirz;
}

void func0f0010b4(void)
{
	if (var80061424) {
		var80061424 = 0;
	}

	var80061424 = 1;
}

void roomSetDefaults(struct room *room)
{
	room->br_light_min = 0;
	room->br_light_max = 255;
	room->br_light_each = 0;
	room->br_settled_local = 128;
	room->br_flash = 0;
	room->br_settled_regional = 0;
	room->lightop = LIGHTOP_NONE;
	room->flags &= ~(ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_LIGHTS_DIRTY | ROOMFLAG_RENDERALWAYS | ROOMFLAG_BRIGHTNESS_CALCED);
	room->volume = 1;
	room->surfacearea = 1;
	room->lightop_cur_frac = 1;
	room->lightop_to_frac = 0;
	room->lightop_from_frac = 0;
	room->lightop_duration240 = 0;
}

Gfx *lightsSetForRoom(Gfx *gdl, RoomNum roomnum)
{
	Lights1 *lights = gfxAllocate(sizeof(Lights1));

	u8 brightness = roomGetFinalBrightnessForPlayer(roomnum);
	u8 a0 = (u32)(brightness * 0.5882353f);

	lights->a.l.col[0] = a0;
	lights->a.l.col[1] = a0;
	lights->a.l.col[2] = a0;
	lights->a.l.colc[0] = a0;
	lights->a.l.colc[1] = a0;
	lights->a.l.colc[2] = a0;

	lights->l[0].l.col[0] = brightness;
	lights->l[0].l.col[1] = brightness;
	lights->l[0].l.col[2] = brightness;
	lights->l[0].l.colc[0] = brightness;
	lights->l[0].l.colc[1] = brightness;
	lights->l[0].l.colc[2] = brightness;
	lights->l[0].l.dir[0] = 0x4d;
	lights->l[0].l.dir[1] = 0x4d;
	lights->l[0].l.dir[2] = 0x2e;

	gSPSetLights1(gdl++, (*lights));

	gSPLookAtX(gdl++, &camGetLookAt()->l[0]);
	gSPLookAtY(gdl++, &camGetLookAt()->l[1]);

	return gdl;
}

Gfx *lightsSetDefault(Gfx *gdl)
{
	static Lights1 var80061460 = gdSPDefLights1(0x96, 0x96, 0x96, 0xff, 0xff, 0xff, 0x4d, 0x4d, 0x2e);

	gSPSetLights1(gdl++, var80061460);

	gSPLookAtX(gdl++, &camGetLookAt()->l[0]);
	gSPLookAtY(gdl++, &camGetLookAt()->l[1]);

	return gdl;
}

void roomInitLights(s32 roomnum)
{
	struct room *room = &g_Rooms[roomnum];
	struct light *light;
	s32 i;

	bgGetRoomBrightnessRange(roomnum, &room->br_light_min, &room->br_light_max);

	room->br_light_min = room->br_light_min / 4;

	if (room->numlights) {
		room->br_light_each = (f32)(room->br_light_max - room->br_light_min) / (f32)room->numlights;
	} else {
		room->br_light_each = 0;
	}

	if (room->br_light_min > 255) {
		room->br_light_min = 255;
	}

	if (room->br_light_max > 255) {
		room->br_light_max = 255;
	}

	room->br_base = room->br_light_min;

	if (room->numlights == 0) {
		room->br_base += (room->br_light_max - room->br_light_min) * 4 / 5;
	}

	switch (g_Vars.stagenum) {
	case STAGE_EXTRACTION:
	case STAGE_DEFECTION:
		if (roomnum == 0x003d) { // near the top comms hub
			room->br_base = 2;
		}
		break;
	}

	if (((g_StageIndex == STAGEINDEX_INFILTRATION || g_StageIndex == STAGEINDEX_RESCUE || g_StageIndex == STAGEINDEX_ESCAPE || g_StageIndex == STAGEINDEX_MAIANSOS)
				&& roomnum == 0x000f) // freight elevator shaft
			|| ((g_StageIndex == STAGEINDEX_DEFECTION || g_StageIndex == STAGEINDEX_EXTRACTION || g_StageIndex == STAGEINDEX_MBR)
				&& roomnum == 0x0001) // moon
			|| ((g_StageIndex == STAGEINDEX_SKEDARRUINS || g_StageIndex == STAGEINDEX_WAR)
				&& roomnum == 0x0002) // fake sky
			|| ((s32)g_StageIndex == STAGEINDEX_TEST_OLD
				&& roomnum == 0x0001)
			|| (g_StageIndex == STAGEINDEX_ATTACKSHIP
				&& roomnum == 0x0071)) { // planet
		room->flags |= ROOMFLAG_RENDERALWAYS;
	} else {
		room->flags &= ~ROOMFLAG_RENDERALWAYS;
	}

	room->flags |= ROOMFLAG_LIGHTS_DIRTY;

#if VERSION < VERSION_NTSC_1_0
	if (cheatIsActive(CHEAT_PERFECTDARKNESS) && (room->flags & ROOMFLAG_RENDERALWAYS) == 0) {
		room->lightop = LIGHTOP_SET;
		room->lightop_to_frac = 0.0f;
	}
#endif

	light = (struct light *)&g_BgLightsFileData[(u32)g_Rooms[roomnum].lightindex * 0x22];

	for (i = 0; i < room->numlights; i++) {
#if VERSION < VERSION_NTSC_1_0
		if (cheatIsActive(CHEAT_PERFECTDARKNESS)) {
			light->brightness = 0;
			light->sparkable = (random() % 2) ? true : false;
			light->healthy = false;
			light->on = false;
			light->sparking = false;
			light->vulnerable = false;
		} else
#endif
		{
			light->brightness = g_Rooms[roomnum].br_light_each;
			light->sparkable = true;
			light->healthy = true;
			light->on = true;
			light->sparking = false;
			light->vulnerable = true;

			switch (g_Vars.stagenum) {
			case STAGE_CITRAINING:
			case STAGE_DEFENSE:
				if (roomnum == 0x000a) { // firing range
					light->vulnerable = false;
				}
				break;
			case STAGE_DEFECTION:
			case STAGE_EXTRACTION:
				if (roomnum == 0x003e && (i == 0 || i == 1)) { // top comms hub
					light->vulnerable = false;
				}
				break;
			}
		}

		light++;
	}
}

bool lightsHandleHit(struct coord *gunpos, struct coord *hitpos, s32 roomnum)
{
	s32 i;
	f32 f2;
	struct coord spa4;
	struct coord sp98;
	struct coord sp8c;
	struct light *light;

	if (roomnum == 0 || g_Rooms[roomnum].numlights == 0) {
		return false;
	}

	spa4.x = gunpos->x - g_BgRooms[roomnum].pos.x;
	spa4.y = gunpos->y - g_BgRooms[roomnum].pos.y;
	spa4.z = gunpos->z - g_BgRooms[roomnum].pos.z;

	sp98.x = hitpos->x - g_BgRooms[roomnum].pos.x;
	sp98.y = hitpos->y - g_BgRooms[roomnum].pos.y;
	sp98.z = hitpos->z - g_BgRooms[roomnum].pos.z;

	sp8c.x = sp98.x - spa4.x;
	sp8c.y = sp98.y - spa4.y;
	sp8c.z = sp98.z - spa4.z;

	f2 = 2.0f / sqrtf(sp8c.x * sp8c.x + sp8c.y * sp8c.y + sp8c.z * sp8c.z);

	sp8c.x *= f2;
	sp8c.y *= f2;
	sp8c.z *= f2;

	sp98.x += sp8c.x;
	sp98.y += sp8c.y;
	sp98.z += sp8c.z;

	light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];

	for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
		if (light->healthy && light->vulnerable) {
			if (func0002f490(&light->bbox[0], &light->bbox[1], &light->bbox[3], NULL, &spa4, &sp98, &sp8c, 0, 0)
					|| func0002f490(&light->bbox[1], &light->bbox[2], &light->bbox[3], NULL, &spa4, &sp98, &sp8c, 0, 0)) {
				struct coord soundpos;

				soundpos.x = light->bbox[0].x;
				soundpos.y = light->bbox[0].y;
				soundpos.z = light->bbox[0].z;

				roomSetLightBroken(roomnum, i);
				psCreate(0, 0, SFX_HIT_GLASS, -1, -1, PSFLAG_0400, 0, PSTYPE_NONE, &soundpos, -1.0f, 0, roomnum, -1.0f, -1.0f, -1.0f);
				return true;
			}
		}

		light++;
	}

	return false;
}

void roomSetLightsFaulty(s32 roomnum, s32 chance)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	s32 i;

	if (g_Rooms[roomnum].numlights) {
		for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
			if ((random() % 100) < chance) {
				light->healthy = false;
				light->on = false;
			}

			light++;
		}
	}

#if VERSION >= VERSION_NTSC_1_0
	g_Rooms[roomnum].br_base = 50;
#else
	g_Rooms[roomnum].br_base = 15;
#endif

	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

void roomSetLightBroken(s32 roomnum, s32 lightnum)
{
	struct light *light = roomGetLight(roomnum, lightnum);
	light->healthy = false;
	light->on = false;

	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

void lightsReset(void)
{
	if (var80061444) {
		func0f004c6c();
	}
}

void func0f001c0c(void)
{
	s32 i;
	s32 sp68;
	s32 table1size;
	s32 table2size;
	s32 table3size;
	s32 table4size;
	s32 sp54;
	u8 *ptr;
	u8 *s5;
	u8 *sp48;
	s32 *sp44;
	s32 j;
	s32 stack[4];

	osGetCount();

	var80061440 = 0;

	lightsCalculateRoomDimensions();

	if (1);
	for (g_NumPortals = 0; g_BgPortals[g_NumPortals].verticesoffset != 0; g_NumPortals++);

	if (g_NumPortals == 0) {
		return;
	}

	table1size = align16(g_Vars.roomcount * 4);
	table2size = align16(g_NumPortals * 4);
	table3size = align16(g_Vars.roomcount * 4);
	table4size = align16((u32)var8009cae0 * (u32)var8009cae0);
#ifdef PLATFORM_64BIT
	sp68 = align16(g_Vars.roomcount * 8 * 2);
#else
	sp68 = align16(g_Vars.roomcount * 8);
#endif

	mempGetStageFree();

	/**
	 * This lighting initialisation needs to build temporary tables in memory.
	 * The memp system allows freeing of the most recent allocation *only*,
	 * so it's not easy to build these tables and dispose of them afterwards.
	 *
	 * Instead of using memp, it uses the z-buffer as a temporary scratch space.
	 * This is safe because there are no RDP tasks in progress at this point.
	 * The game waits for them to complete before loading the stage.
	 *
	 * Note that for some stages the z-buffer allocation must be higher than the
	 * lo-res size to support the lighting needs here.
	 */
	ptr = zbufGetAllocation();

	var80061434 = (f32 *)ptr;
	ptr += table1size;

	var80061438 = (bool *)ptr;
	ptr += table2size;

	sp44 = (s32 *)(ptr);
	ptr += table3size;

	sp48 = (u8 *)ptr;
	ptr += table4size;

	s5 = (u8 *)ptr;

	var80061420 = mempAlloc(sp68, MEMPOOL_STAGE);

	for (i = 0; i < g_NumPortals; i++) {
		if (PORTAL_IS_CLOSED(i)) {
			var80061438[i] = false;
		} else {
			var80061438[i] = true;
			if (1);
		}
	}

	if (g_Vars.stagenum == STAGE_EXTRACTION || g_Vars.stagenum == STAGE_DEFECTION) {
		var80061438[98] = false;
		var80061438[100] = false;
	}

	func0f00215c(sp48);

	for (i = 1, table3size = 0; i < g_Vars.roomcount; i++) {
		sp44[i] = func0f177a54((void *)(i * var8009cae0 + sp48), g_Vars.roomcount, (void *)(&s5[i * var8009cae0]), 1);
		table3size += align4(sp44[i]);
	}

	ptr = mempAlloc(align16(table3size), MEMPOOL_STAGE);

	sp68 += align16(table3size);

	sp54 = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		s32 size = align4(sp44[i]);

		var80061420[i].unk00 = ptr;

		ptr += size;
		sp54 += size;

		for (j = 0; j < sp44[i]; j++) {
			var80061420[i].unk00[j] = *(&s5[i * var8009cae0] + j);
		}
	}

	table3size = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		sp44[i] = func0f177a54((void *)(sp48 + i), g_Vars.roomcount, (void *)(&s5[i * var8009cae0]), var8009cae0);

		table3size += align4(sp44[i]);
	}

	ptr = mempAlloc(align16(table3size), MEMPOOL_STAGE);

	align16(table3size);

	for (i = 1; i < g_Vars.roomcount; i++) {
		var80061420[i].unk04 = ptr;

		ptr += align4(sp44[i]);

		for (j = 0; j < sp44[i]; j++) {
			var80061420[i].unk04[j] = *(&s5[i * var8009cae0] + j);
		}
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags |= ROOMFLAG_ONSCREEN;
	}

	lightingTick();

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags &= ~ROOMFLAG_ONSCREEN;
	}

	osGetCount();

	if (sp68);
}

f32 func0f002334(s32 roomnum, f32 mult, s32 portalnum1, s32 portalnum2);

void func0f00215c(u8 *arg0)
{
	s32 i;
	s32 j;

	var8009cae8 = &func0f002334;

	var8006143c = 50.0f;
	var8009cae4 = 20;
	var80061440 = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		u8 *ptr = &arg0[i * var8009cae0];

		func0f00259c(i);

		for (j = 0; j < 1; j++) {
			ptr[j] = 0;
		}

		for (j = 1; j < g_Vars.roomcount; j++) {
			if (var80061434[i] < var80061434[j]) {
				var80061434[j] = var80061434[i];
			}

			ptr[j] = var80061434[j];
		}
	}
}

f32 func0f002334(s32 roomnum, f32 mult, s32 portalnum1, s32 portalnum2)
{
	f32 surfacearea = 0;
	f32 result;

	if (portalnum1 != -1) {
		surfacearea = bgCalculatePortalSurfaceArea(portalnum1);
	}

	result = (bgCalculatePortalSurfaceArea(portalnum2) / (g_Rooms[roomnum].surfacearea - surfacearea)) * mult;
	return result;
}

void lightsCalculateRoomDimensions(void)
{
	s32 i;
	s32 j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		bool valid = true;

		g_Rooms[i].volume = 1.0f;
		g_Rooms[i].surfacearea = 1.0f;

		for (j = 0; j < 3; j++) {
			f32 diff = g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j];

			if (diff > 0.0f) {
				g_Rooms[i].volume *= (g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j]) / 100.0f;
			} else {
				valid = false;
			}
		}

		g_Rooms[i].volume += 1.0f;

		if (g_Rooms[i].volume > 60.0f) {
			g_Rooms[i].volume = 60.0f;
		}

		if (valid) {
			f32 xdiff = g_Rooms[i].bbmax[0] - g_Rooms[i].bbmin[0];
			f32 ydiff = g_Rooms[i].bbmax[1] - g_Rooms[i].bbmin[1];
			f32 zdiff = g_Rooms[i].bbmax[2] - g_Rooms[i].bbmin[2];

			if (!(xdiff > 0)) {
				xdiff = -xdiff;
			}

			if (!(ydiff > 0)) {
				ydiff = -ydiff;
			}

			if (!(zdiff > 0)) {
				zdiff = -zdiff;
			}

			g_Rooms[i].surfacearea = 2.0f * (xdiff * ydiff + xdiff * zdiff + ydiff * zdiff);
		} else {
			g_Rooms[i].surfacearea = 20000000.0f;
		}
	}
}

void func0f00259c(s32 roomnum)
{
	s32 i;
	f32 sp58;
	f32 f20 = 0.0f;

	for (i = 0; i < g_Vars.roomcount; i++) {
		var80061434[i] = 0.0f;
	}

	var80061434[roomnum] = sqrtf(g_Rooms[roomnum].volume) * 255.0f;
	if (1);

	if (g_Rooms[roomnum].numportals != 0) {
		func0f002844(roomnum, var80061434[roomnum], 0, -1);
		if (1);
		if (1);
	}

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		f20 += bgCalculatePortalSurfaceArea(g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i]);
	}

	sp58 = (g_Rooms[roomnum].surfacearea - f20) / g_Rooms[roomnum].surfacearea;

	for (i = 1; i < g_Vars.roomcount; i++) {
		var80061434[i] *= 3.0f / sqrtf(g_Rooms[i].volume);
	}

	if (var80061434[roomnum] > 255.0f) {
		var80061434[roomnum] = 255.0f;
	}

	if (sp58 < 0.1f) {
		g_Rooms[roomnum].br_light_min = g_Rooms[roomnum].br_light_max >> 1;
		var80061434[roomnum] = g_Rooms[roomnum].br_light_max;
	}
}

void func0f002844(s32 roomnum, f32 arg1, s32 arg2, s32 portalnum)
{
	s32 i;
	s32 otherroomnum = -1;

	var80061440++;

	if (portalnum != -1) {
		if (roomnum == g_BgPortals[portalnum].roomnum1) {
			otherroomnum = (s32) g_BgPortals[portalnum].roomnum2;
		} else {
			otherroomnum = (s32) g_BgPortals[portalnum].roomnum1;
		}
	}

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		s32 iterportalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		s32 iterroomnum;

		if (var80061438[iterportalnum]) {
			if (roomnum == g_BgPortals[iterportalnum].roomnum1) {
				iterroomnum = g_BgPortals[iterportalnum].roomnum2;
			} else {
				iterroomnum = g_BgPortals[iterportalnum].roomnum1;
			}

			if (iterroomnum != otherroomnum) {
				f32 f0 = var8009cae8(roomnum, arg1, portalnum, iterportalnum);

				if (f0 > var8006143c && arg2 < var8009cae4) {
					var80061434[roomnum] -= f0;
					var80061434[iterroomnum] += f0;

					if (var80061434[roomnum] < 0.0f) {
						var80061434[roomnum] = 0.0f;
					}

					func0f002844(iterroomnum, f0, arg2 + 1, iterportalnum);
				}
			}
		}
	}
}

void func0f002a98(void)
{
	s32 i;

	var8009cae0 = align4(g_Vars.roomcount);
#if VERSION >= VERSION_NTSC_1_0
	g_LightsPrevTickMode = 0;
#endif
	g_Vars.remakewallhitvtx = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		roomSetDefaults(&g_Rooms[i]);
		roomInitLights(i);
	}

	var80061420 = NULL;

	if (IS4MB()) {
		var80061444 = 0;
	}
}

void roomSetLightsOn(s32 roomnum, s32 enable)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	s32 i;

	if (g_Rooms[roomnum].numlights) {
		for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
			if (light->healthy) {
				light->on = enable;
			}

			light++;
		}
	}

	if (enable) {
		g_Rooms[roomnum].flags &= ~ROOMFLAG_LIGHTSOFF;
	} else {
		g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTSOFF;
	}

	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

/**
 * @bug: The lightop_timer240 values are not set correctly.
 * They should be duration60 * 4. This only affects LIGHTOP_TRANSITION,
 * which makes it start the transition 3/4 of the way into it.
 */
void roomSetLightOp(s32 roomnum, s32 operation, u8 br_to, u8 br_from, u8 duration60)
{
	if (cheatIsActive(CHEAT_PERFECTDARKNESS) == false) {
		g_Rooms[roomnum].lightop = operation;

		switch (operation) {
		case LIGHTOP_SET:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			break;
		case LIGHTOP_SETRANDOM:
			g_Rooms[roomnum].lightop_to_frac = br_to;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = duration60;
			break;
		case LIGHTOP_TRANSITION:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = duration60;
			break;
		case LIGHTOP_SINELOOP:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = 0;
			break;
		case LIGHTOP_HIGHLIGHT:
			break;
		}
	}
}

bool lightTickBroken(s32 roomnum, s32 lightnum)
{
	struct light *light = (struct light *)(g_BgLightsFileData + ((g_Rooms[roomnum].lightindex + lightnum) * 0x22));

	if (!light->sparkable) {
		return false;
	}

	if (light->sparking) {
		if ((random() % 8) == 0) {
			light->sparking = false;
		} else if ((random() % 2) == 0) {
			struct coord spc8;
			struct coord spbc;
			struct coord spb0;
			struct coord spa4;
			struct coord sp98;
			struct coord sp8c;
			struct coord sp80;
			struct coord centre;
			f32 rand1 = 2.0f * RANDOMFRAC() - 1.0f; // range -1 to 1
			f32 rand2 = 2.0f * RANDOMFRAC() - 1.0f; // range -1 to 1
			s32 sparktype = -1;
			RoomNum smokerooms[2];
			struct bgroom *room;

			spc8.x = light->bbox[1].x - light->bbox[0].x;
			spc8.y = light->bbox[1].y - light->bbox[0].y;
			spc8.z = light->bbox[1].z - light->bbox[0].z;

			spc8.x = rand1 * spc8.x;
			spc8.y = rand1 * spc8.y;
			spc8.z = rand1 * spc8.z;

			spbc.x = light->bbox[2].x - light->bbox[0].x;
			spbc.y = light->bbox[2].y - light->bbox[0].y;
			spbc.z = light->bbox[2].z - light->bbox[0].z;

			spbc.x = rand2 * spbc.x;
			spbc.y = rand2 * spbc.y;
			spbc.z = rand2 * spbc.z;

			// @bug? These all use x
			sp98.x = light->bbox[0].x + spc8.x + spbc.x;
			sp98.y = light->bbox[0].x + spc8.y + spbc.y;
			sp98.z = light->bbox[0].x + spc8.z + spbc.z;

			sp8c.x = light->dirx;
			sp8c.y = light->diry;
			sp8c.z = light->dirz;

			sp80.x = -sp8c.x;
			sp80.y = -sp8c.y;
			sp80.z = -sp8c.z;

			func0f177164(&sp98, &spa4, VERSION >= VERSION_NTSC_1_0 ? 1546 : 1570, "dlights.c");

			spa4.x += sp80.x;
			spa4.y += sp80.y;
			spa4.z += sp80.z;

			func0f177164(&spa4, &spa4, VERSION >= VERSION_NTSC_1_0 ? 1548 : 1572, "dlights.c");

			room = (void *) (roomnum * sizeof(struct bgroom));

			switch (random() % 4) {
			case 0:
				if (roomnum && roomnum && roomnum);
				sparktype = SPARKTYPE_LIGHT1;
				break;
			case 1:
				sparktype = SPARKTYPE_LIGHT2;
				break;
			case 2:
				sparktype = SPARKTYPE_LIGHT3;
				break;
			case 3:
				sparktype = SPARKTYPE_LIGHT4;
				break;
			}

			lightGetBboxCentre(roomnum, lightnum, &centre);

			room = (void *) ((u8 *) g_BgRooms + (uintptr_t) room);
			centre.x += room->pos.x;
			centre.y += room->pos.y;
			centre.z += room->pos.z;

			sparksCreate(roomnum, NULL, &centre, &spa4, &sp8c, sparktype);

			if ((random() % 4) == 0) {
				smokerooms[0] = roomnum;
				smokerooms[1] = -1;

				smokeCreateSimple(&centre, smokerooms, SMOKETYPE_BULLETIMPACT);
			}

			roomFlashLighting(roomnum, 64, 80);
			psCreate(NULL, NULL, psGetRandomSparkSound(), -1, -1, PSFLAG_0400, 0, PSTYPE_FOOTSTEP, &centre, -1.0f, 0, roomnum, -1.0f, -1.0f, -1.0f);
			return true;
		}
	} else {
		if ((random() % 80) == 0) {
			light->sparking = true;
		}
	}

	return false;
}

const char var7f1a7bcc[] = "L2 - g_bfGlobalLightRebuild = %d";
const char var7f1a7bf0[] = "Acoustic Shadowing is %s";
const char var7f1a7c0c[] = "Enabled";
const char var7f1a7c14[] = "Disabled";
const char var7f1a7c20[] = "L2 - Fading Rm%d - Mode=%d%%";
const char var7f1a7c40[] = "RWI : Re-light all affected char props : g_bfGlobalLightRebuild";
const char var7f1a7c80[] = "L2 - %d Rooms have been processed";
const char var7f1a7ca4[] = "L2 - %d Chars need lighting";
const char var7f1a7cc0[] = "L2(%d) -> ";
const char var7f1a7ccc[] = "L2 -> Building portal range table (Num Portals = %d)\n";
const char var7f1a7d04[] = "L2(%d) -> ";
const char var7f1a7d10[] = "L2 -> Allocated %uK for the compressed acoustic shadow table\n";
const char var7f1a7d50[] = "L2(%d) -> ";
const char var7f1a7d5c[] = "L2 -> Finished building portal range table\n";

void lightingTick(void)
{
	s32 i;

	roomsTickLighting();

	if (g_Vars.remakewallhitvtx) {
		wallhitsRecolour();

		g_Vars.remakewallhitvtx = false;

		for (i = 1; i < g_Vars.roomcount; i++) {
			g_Rooms[i].flags &= ~ROOMFLAG_NEEDRESHADE;
		}
	}
}

#if VERSION >= VERSION_NTSC_1_0
void lightsConfigureForPerfectDarknessCutscene(void)
{
	s32 i;
	s32 j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];
		g_Rooms[i].lightop = LIGHTOP_SET;
		g_Rooms[i].lightop_to_frac = 0.5f;

		for (j = 0; j < g_Rooms[i].numlights; j++) {
			light->sparkable = random() % 2 ? true : false;
			light->healthy = true;
			light->on = true;
			light->sparking = false;
			light->vulnerable = true;
			light->brightness = g_Rooms[i].br_light_each;

			light++;
		}
	}
}
#endif

#if VERSION >= VERSION_NTSC_1_0
void lightsConfigureForPerfectDarknessGameplay(void)
{
	s32 i;
	s32 j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];
		g_Rooms[i].lightop = LIGHTOP_SET;
		g_Rooms[i].lightop_to_frac = 0;

		for (j = 0; j < g_Rooms[i].numlights; j++) {
			light->sparkable = random() % 2 ? true : false;
			light->healthy = false;
			light->on = false;
			light->sparking = false;
			light->vulnerable = false;
			light->brightness = 0;

			light++;
		}
	}
}
#endif

#if VERSION >= VERSION_NTSC_1_0
void lightsTickPerfectDarkness(void)
{
	if (g_Vars.tickmode != g_LightsPrevTickMode) {
		if (TICKMODE_CUTSCENE == g_Vars.tickmode && TICKMODE_CUTSCENE != g_LightsPrevTickMode) {
			lightsConfigureForPerfectDarknessCutscene();
		} else if (TICKMODE_NORMAL == g_Vars.tickmode && TICKMODE_NORMAL != g_LightsPrevTickMode) {
			lightsConfigureForPerfectDarknessGameplay();
		}

		g_LightsPrevTickMode = g_Vars.tickmode;
	}
}
#endif

void roomsTickLighting(void)
{
#if VERSION >= VERSION_NTSC_1_0
	s32 i;
	s32 numprocessed = 0;
#else
	s32 numprocessed = 0;
	s32 i;
#endif
	s32 j;
	bool wasdirty = false;
	struct light *light;
	f32 amount;
	s32 timer240;
	f32 angle;
	f32 average;
	u32 stack;

#if VERSION >= VERSION_NTSC_1_0
	if (cheatIsActive(CHEAT_PERFECTDARKNESS)) {
		lightsTickPerfectDarkness();
	}
#else
	static s32 prevtickmode = 0;

	if (prevtickmode != g_Vars.tickmode) {
		if (prevtickmode == TICKMODE_CUTSCENE && g_Vars.tickmode == TICKMODE_NORMAL) {
			g_IsSwitchingGoggles = 2;
		}

		prevtickmode = g_Vars.tickmode;
	}
#endif

	if (var80061420 == NULL) {
		return;
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags &= ~ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		// Tick any light operations
		g_Rooms[i].lightop_timer240 -= g_Vars.lvupdate240;

		switch (g_Rooms[i].lightop) {
		case LIGHTOP_SET:
			g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_to_frac;

			if (g_Rooms[i].lightop_cur_frac < 0.0f) {
				g_Rooms[i].lightop_cur_frac = 0.0f;
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			roomSetLightOp(i, LIGHTOP_NONE, 0, 0, 0);
			break;
		case LIGHTOP_SETRANDOM:
			if (g_Rooms[i].lightop_timer240 < 0) {
				if (RANDOMFRAC() * 100.0f < g_Rooms[i].lightop_to_frac) {
					g_Rooms[i].lightop_cur_frac = 1.0f;
				} else {
					g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_from_frac;

					if (g_Rooms[i].lightop_cur_frac < 0.0f) {
						g_Rooms[i].lightop_cur_frac = 0.0f;
					}
				}

				g_Rooms[i].lightop_timer240 = g_Rooms[i].lightop_duration240;
				g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			}
			break;
		case LIGHTOP_TRANSITION:
			if (g_Rooms[i].lightop_timer240 > 0) {
				g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_from_frac;
				g_Rooms[i].lightop_cur_frac += g_Rooms[i].lightop_timer240 / g_Rooms[i].lightop_duration240 * (g_Rooms[i].lightop_to_frac - g_Rooms[i].lightop_from_frac);

				if (g_Rooms[i].lightop_cur_frac < 0.0f) {
					g_Rooms[i].lightop_cur_frac = 0.0f;
				}
			} else {
				roomSetLightOp(i, LIGHTOP_NONE, 0, 0, 0);
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		case LIGHTOP_SINELOOP:
			timer240 = g_Rooms[i].lightop_timer240 > 0 ? g_Rooms[i].lightop_timer240 : -g_Rooms[i].lightop_timer240;

			angle = (timer240 % (s32) g_Rooms[i].lightop_duration240) * M_TAU / g_Rooms[i].lightop_duration240;
			average = (g_Rooms[i].lightop_to_frac + g_Rooms[i].lightop_from_frac) * 0.5f;

			g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_to_frac + (cosf(angle) + 1.0f) * average;

			if (g_Rooms[i].lightop_cur_frac < 0.0f) {
				g_Rooms[i].lightop_cur_frac = 0.0f;
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		case LIGHTOP_HIGHLIGHT:
			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		}

		if (g_IsSwitchingGoggles) {
			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
		}

		if (g_Rooms[i].flags & ROOMFLAG_LIGHTS_DIRTY) {
			// Calculate the settled local brightness
			if (g_Rooms[i].numlights != 0) {
				s32 numlightson = 0;
				struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];

				for (j = 0; j < g_Rooms[i].numlights; j++) {
					if (light->on) {
						numlightson++;
					}

					light++;
				}

				if (g_Rooms[i].flags & ROOMFLAG_LIGHTSOFF) {
					amount = 2.0f;
				} else if (numlightson != 0) {
					amount = g_Rooms[i].br_base;
				} else {
					amount = (f32)g_Rooms[i].br_base / 2;
				}
			} else {
				if (g_Rooms[i].flags & ROOMFLAG_LIGHTSOFF) {
					amount = 2.0f;
				} else {
					amount = g_Rooms[i].br_base;
				}
			}

			amount *= g_Rooms[i].lightop_cur_frac;
			g_Rooms[i].br_settled_local = amount;

			// Add brightness for each light that is on
			light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];

			for (j = 0; j < g_Rooms[i].numlights; j++) {
				if (light->on) {
					amount = g_Rooms[i].lightop_cur_frac * light->brightness;
					g_Rooms[i].br_settled_local += (s32)amount;
				}

				light++;
			}

			if (g_Rooms[i].br_settled_local > 255) {
				g_Rooms[i].br_settled_local = 255;
			}

			g_Rooms[i].flags &= ~ROOMFLAG_LIGHTS_DIRTY;

			wasdirty = 1;
		}

		// Tick any flash lighting
		if (g_Rooms[i].br_flash != 0) {
			s32 increment = g_Vars.lvupdate240 * 2;

			if (var80061420 != NULL) {
				s32 spa0 = 0;
				s32 sp9c = 0;

				s32 ret = func0f177c8c(var80061420[i].unk04, &spa0, &sp9c);

				while (ret != -1) {
					if (ret != 0) {
						g_Rooms[sp9c].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
					}

					ret = func0f177c8c(var80061420[i].unk04, &spa0, &sp9c);
				}
			}

			if (g_Rooms[i].br_flash > 0) {
				if (increment > g_Rooms[i].br_flash) {
					increment = g_Rooms[i].br_flash;
				}

				g_Rooms[i].br_flash -= increment;
			} else {
				// @bug: In this branch br_flash is zero or negative.
				// Both instances of br_flash should be negated here.
				if (increment < g_Rooms[i].br_flash) {
					increment = g_Rooms[i].br_flash;
				}

				g_Rooms[i].br_flash += increment;
			}

			g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
		}

		g_Rooms[i].flags &= ~ROOMFLAG_LIGHTS_DIRTY;
	}

	// If switching googles, mark all rooms as dirty so their brightness will be
	// recalculated the next time they appear on screen.
	if (g_IsSwitchingGoggles || wasdirty) {
		for (i = 1; i < g_Vars.roomcount; i++) {
			g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_PERM;
		}
	}

	// Calculate settled regional lighting for any rooms that need it
	for (i = 1; i < g_Vars.roomcount; i++) {
		if (i != 0) {
			if ((g_Rooms[i].flags & ROOMFLAG_RENDERALWAYS) || (g_Rooms[i].flags & (ROOMFLAG_ONSCREEN | ROOMFLAG_STANDBY))) {
				if (g_Rooms[i].flags & (ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_BRIGHTNESS_DIRTY_TEMP)) {
					s32 sum = 0;
					s32 sp90 = 0;
					s32 sp8c = 0;

					s32 ret = func0f177c8c(var80061420[i].unk00, &sp90, &sp8c);

					while (ret != -1) {
						if (sp8c != 0) {
							s32 add = 0;

							if (sp8c) {
								add += (s32)((1.0f / 255.0f) * ret * g_Rooms[sp8c].br_settled_local);
							}

							sum += add;
						}

						ret = func0f177c8c(var80061420[i].unk00, &sp90, &sp8c);
					}

					if (sum > 255) {
						sum = 255;
					}

					g_Rooms[i].br_settled_regional = sum;
					g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_CALCED;
					g_Rooms[i].flags |= ROOMFLAG_NEEDRESHADE;
					g_Rooms[i].flags &= ~(ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_BRIGHTNESS_DIRTY_TEMP);

					if (g_Rooms[i].lightop == LIGHTOP_HIGHLIGHT) {
						s32 r = roomGetFinalBrightnessForPlayer(i);
						s32 g = r;
						s32 b = r;

						scenarioHighlightRoom(i, &r, &g, &b);

						g_Rooms[i].highlightfrac_r = r * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_g = g * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_b = b * (1.0f / 255.0f);
					} else {
						g_Rooms[i].highlightfrac_r = roomGetFinalBrightnessForPlayer(i) * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_g = g_Rooms[i].highlightfrac_r;
						g_Rooms[i].highlightfrac_b = g_Rooms[i].highlightfrac_r;
					}

					numprocessed++;
				}
			}
		}
	}

	if (g_Vars.joydisableframestogo <= 0 && (numprocessed || g_IsSwitchingGoggles)) {
		struct prop *prop = g_Vars.activeprops;

		while (prop) {
			if (prop->type == PROPTYPE_CHR) {
				for (i = 0; prop->rooms[i] != -1; i++) {
					if (g_Rooms[prop->rooms[i]].flags & ROOMFLAG_NEEDRESHADE) {
						if (2 == g_IsSwitchingGoggles) {
							struct chrdata *chr = prop->chr;
							propCalculateShadeColour(chr->prop, chr->nextcol, chr->floorcol);
						} else {
							struct chrdata *chr = prop->chr;
							chr->unk32c_18 = true;
						}
					}
				}
			}

			prop = prop->next;
		}

		g_Vars.remakewallhitvtx = 0xf;
	}

	if (g_IsSwitchingGoggles) {
		g_IsSwitchingGoggles = false;
	}
}

void lightsTick(void)
{
	struct hand *hand1 = &g_Vars.currentplayer->hands[0];
	struct hand *hand2 = &g_Vars.currentplayer->hands[1];

	func0f005bb0();

	if (hand1->flashon || hand2->flashon) {
		roomFlashLighting(g_Vars.currentplayer->prop->rooms[0], 64, 80);
	}
}

void func0f004384(void)
{
	// empty
}

/**
 * Set a lighting flash in the given room and its neighbours.
 *
 * The room must not have ROOMFLAG_OUTDOORS.
 */
void roomFlashLighting(s32 roomnum, s32 start, s32 limit)
{
	if (var80061420 && !(g_Rooms[roomnum].flags & ROOMFLAG_OUTDOORS ? 1 : 0)) {
		s32 value;
		s32 sp78 = 0;
		s32 neighbournum = 0;

		value = func0f177c8c(var80061420[roomnum].unk04, &sp78, &neighbournum);

		while (value != -1) {
			f32 increment = value * (1.0f / 255.0f) * start * 5.0f;

			if (start > 0) {
				if (increment > start) {
					increment = start;
				}
			} else {
				if (increment < start) {
					increment = start;
				}
			}

			// @bug: Should be checking neighbournum flags, not roomnum
			if (!(g_Rooms[roomnum].flags & ROOMFLAG_OUTDOORS ? 1 : 0)) {
				roomFlashLocalLighting(neighbournum, increment, limit);
			}

			value = func0f177c8c(var80061420[roomnum].unk04, &sp78, &neighbournum);
		}
	}
}

void roomFlashLocalLighting(s32 roomnum, s32 increment, s32 limit)
{
	if (roomnum) {
		if (g_Rooms[roomnum].flags & ROOMFLAG_ONSCREEN) {
			if (increment > 0) {
				// Increasing
				if (g_Rooms[roomnum].br_flash < limit) {
					g_Rooms[roomnum].br_flash += increment;

					if (g_Rooms[roomnum].br_flash > limit) {
						g_Rooms[roomnum].br_flash = limit;
					}
				}
			} else {
				// Decreasing
				if (g_Rooms[roomnum].br_flash > limit) {
					g_Rooms[roomnum].br_flash += increment;

					if (g_Rooms[roomnum].br_flash < limit) {
						g_Rooms[roomnum].br_flash = limit;
					}
				}
			}
		}
	}
}

void roomHighlight(s32 roomnum)
{
	s32 i;
	s32 tmpr;
	s32 tmpg;
	s32 tmpb;
	s32 alpha;
	s32 red;
	s32 green;
	s32 blue;
	s32 extra;
	s32 numcolours;
	Col *src;
	Col *dst;
	u8 br_settled_regional;
	s32 max;
	f32 mult;
	u32 stack;

	if (g_BgFrameCount != g_Rooms[roomnum].hlupdatedframe && g_Rooms[roomnum].loaded240 != 0) {
		g_Rooms[roomnum].hlupdatedframe = g_BgFrameCount;

		if ((g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) == 0) {
			g_Rooms[roomnum].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_PERM;
		}

		br_settled_regional = roomGetSettledRegionalBrightnessForPlayer(roomnum);
		numcolours = g_Rooms[roomnum].gfxdata->numcolours;
		dst = gfxAllocateColours(numcolours);
		g_Rooms[roomnum].colours = dst;

		extra = g_Rooms[roomnum].br_flash;
		src = (Col *)((uintptr_t)g_Rooms[roomnum].gfxdata->vertices + g_Rooms[roomnum].gfxdata->numvertices * sizeof(Vtx));
		src = (Col *)ALIGN8((uintptr_t)src);

		if (g_Rooms[roomnum].flags & ROOMFLAG_RENDERALWAYS) {
			g_Rooms[roomnum].colours = src;
			return;
		}

		for (i = 0; i < numcolours; i++) {
			// @bug? Why is this looking up vertices using a colour index?
			if (g_Rooms[roomnum].gfxdata->vertices[i].flags & 0x01) {
				dst[i].r = src[i].r;
				dst[i].g = src[i].g;
				dst[i].b = src[i].b;
				dst[i].a = src[i].a * (1.0f / 255.0f * br_settled_regional);
			} else {
				if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
					tmpr = tmpg = tmpb = (src[i].r > src[i].g && src[i].r > src[i].b)
						? src[i].r
						: src[i].g > src[i].b ? src[i].g : src[i].b;
				} else {
					tmpr = src[i].r;
					tmpg = src[i].g;
					tmpb = src[i].b;
				}

				alpha = src[i].a;
				max = tmpr;

				if (tmpg > max) {
					max = tmpg;
				}

				if (tmpb > max) {
					max = tmpb;
				}

				if (max > br_settled_regional) {
					mult = br_settled_regional / (f32)max;
				} else {
					mult = 1.0f;
				}

				red = tmpr * mult;
				green = tmpg * mult;
				blue = tmpb * mult;
				max *= mult;

				if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
					extra = 0;
				} else if (extra + max > 285) {
					extra = 285 - max;
				}

				if (red + green + blue != 0
						|| g_Rooms[roomnum].lightop_cur_frac == 0.0f
						|| (g_Rooms[roomnum].flags & ROOMFLAG_LIGHTSOFF)) {
					red += extra;
					green += extra;
					blue += extra;
				}

				if (g_Rooms[roomnum].lightop == LIGHTOP_HIGHLIGHT) {
					scenarioHighlightRoom(roomnum, &red, &green, &blue);
				}

				if (red > 255) {
					red = 255;
				}

				if (green > 255) {
					green = 255;
				}

				if (blue > 255) {
					blue = 255;
				}

				if (red < 0) {
					red = 0;
				}

				if (green < 0) {
					green = 0;
				}

				if (blue < 0) {
					blue = 0;
				}

				dst[i].r = red;
				dst[i].g = green;
				dst[i].b = blue;
				dst[i].a = alpha;
			}
		}
	}
}

void func0f004c6c(void)
{
	s32 sp44;
	s32 sp40;
	s32 sp3c;
	s32 sp38;
	s32 sp34;
	s32 i;
	s32 j;
	s32 s4;
	u8 *ptr;
	u8 *backupptr;

#ifdef PLATFORM_64BIT
	sp44 = align16(0x2000 * 2);
	sp40 = align16(g_NumPortals * 4 * 2);
	sp3c = align16(g_NumPortals * 0xc * 2);
	sp38 = align16(g_NumPortals * 4 * 2);
	sp34 = align16(g_NumPortals * 2 * 2);
#else
	sp44 = align16(0x2000);
	sp40 = align16(g_NumPortals * 4);
	sp3c = align16(g_NumPortals * 0xc);
	sp38 = align16(g_NumPortals * 4);
	sp34 = align16(g_NumPortals * 2);
#endif

	for (i = 0, s4 = sp38; i < g_NumPortals; i++) {
		if (i != 0) {
			s4 += i * 2;
		}
	}

	s4 = align16(s4);
	ptr = mempAlloc(align16(s4), MEMPOOL_STAGE);
	var80061430 = (void *)ptr;

	ptr += sp38;

	for (i = 0; i < g_NumPortals; i++) {
		if (i != 0) {
			var80061430[i] = (void *)ptr;
			ptr += i * 2;
		} else {
			var80061430[i] = 0;
		}
	}

	s4 += sp3c;
	s4 += sp44;
	s4 += sp40;
	s4 += sp38;
	s4 += g_NumPortals * sp34;

	align16((s32)s4);

	ptr = mempGetNextStageAllocation();
	var8009cad0 = (void *)ptr;
	ptr += sp44;

	var8009cad8 = (void *)ptr;
	ptr += sp40;

	var8006142c = (void *)ptr;
	ptr += sp38;

	backupptr = ptr;

	ptr += g_NumPortals * sp34;
	var80061428 = (void *)ptr;
	ptr = backupptr;

	s4 = sp38;

	for (i = 0; i < g_NumPortals; i++) {
		var8006142c[i] = (void *)ptr;
		ptr += sp34;
		s4 += sp34;

		for (j = 0; j < g_NumPortals; j++) {
			var8006142c[i][j] = 0x8009;
		}
	}

	for (i = 0; i < g_NumPortals; i++) {
		var8009cad8[i] = portalGetXluFrac(i) > 0.5f;

		portalGetAvgVertexPos(i, &var80061428[i]);
	}

	if (g_Vars.stagenum == STAGE_INVESTIGATION) {
		var8009cad8[0] = 1;
	}

	for (i = 0; i < g_NumPortals; i++) {
		var8006142c[i][i] = 0;
	}

	func0f00505c();

	for (i = 0; i < g_NumPortals; i++) {
		for (j = 0; j < i; j++) {
			u16 a = var8006142c[i][j];
			u16 b = var8006142c[j][i];

			var80061430[i][j] = a < b ? a : b;
		}
	}
}

void func0f00505c(void)
{
	s32 j;
	s32 sp78;
	s32 i;
	s32 k;
	s32 portalnum;
	s32 portalnum2;
	s32 roomnum;
	s32 l;
	u16 dist;
	u32 stack;

	for (i = 0; i < g_NumPortals; i++) {
		for (j = 0, var8009cad0[0] = i, sp78 = 1; j != sp78; j = (j + 1) & 0x7ff) {
			portalnum = var8009cad0[j];

			for (k = 0; k < 2; k++) {
				if (k != 0) {
					roomnum = g_BgPortals[portalnum].roomnum2;
				} else {
					roomnum = g_BgPortals[portalnum].roomnum1;
				}

				for (l = 0; l < g_Rooms[roomnum].numportals; l++) {
					portalnum2 = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + l];

					if (portalnum2 != portalnum && var8009cad8[portalnum2] != 0) {
						if (var8006142c[portalnum][portalnum2] >= 0x8000) {
							f32 xdiff = var80061428[portalnum].x - var80061428[portalnum2].x;
							f32 ydiff = var80061428[portalnum].y - var80061428[portalnum2].y;
							f32 zdiff = var80061428[portalnum].z - var80061428[portalnum2].z;

							f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

							var8006142c[portalnum][portalnum2] = dist;
							var8006142c[portalnum2][portalnum] = dist;
						}

						dist = (var8006142c[i][portalnum] + var8006142c[portalnum2][portalnum]);

						if (dist <= 5800 && ((i == portalnum) != 0 || dist < var8006142c[i][portalnum2])) {
							var8006142c[i][portalnum2] = dist;
							var8009cad0[sp78] = portalnum2;
							sp78 = (sp78 + 1) & 0x7ff;
						}
					}
				}
			}
		}
	}
}

f32 func0f0053d0(s32 roomnum1, struct coord *pos1, s32 portalnum1, s32 roomnum2, struct coord *pos2, s32 portalnum2, f32 *arg6)
{
	f32 sp6c;
	f32 *sp68;
	f32 sp64;
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;

	sp6c = 32767.0f;
	sp68 = arg6 ? arg6 : &sp6c;
	sp64 = *sp68;

	xdiff = pos1->x - pos2->x;
	xdiff = xdiff > 0.0f ? xdiff : -xdiff;

	if (xdiff < sp64) {
		zdiff = pos1->z - pos2->z;
		zdiff = zdiff > 0.0f ? zdiff : -zdiff;

		if (zdiff < sp64) {
			ydiff = pos1->y - pos2->y;
			ydiff = ydiff > 0.0f ? ydiff : -ydiff;

			if (ydiff < sp64) {
				f32 dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

				if (dist < sp64) {
					if (roomnum1 == roomnum2 || portalnum1 == portalnum2) {
						if (dist < *sp68) {
							*sp68 = dist;
						}
					} else {
						f32 sp50 = func0f000920(portalnum1, portalnum2);

						if (sp50 < sp64) {
							struct coord sp44;
							f32 xdiff2;
							f32 zdiff2;

							portalGetAvgVertexPos(portalnum1, &sp44);
							sp64 -= sp50;

							xdiff2 = sp44.x - pos1->x;
							xdiff2 = xdiff2 > 0.0f ? xdiff2 : -xdiff2;

							if (xdiff2 < sp64) {
								zdiff2 = sp44.z - pos1->z;
								zdiff2 = zdiff2 > 0.0f ? zdiff2 : -zdiff2;

								if (zdiff2 < sp64) {
									f32 sp38 = sqrtf(xdiff2 * xdiff2 + zdiff2 * zdiff2);

									if (sp38 < sp64) {
										struct coord sp2c;
										f32 xdiff3;
										f32 zdiff3;

										portalGetAvgVertexPos(portalnum2, &sp2c);
										sp64 -= sp38;

										xdiff3 = sp2c.x - pos2->x;
										xdiff3 = xdiff3 > 0.0f ? xdiff3 : -xdiff3;

										if (xdiff3 < sp64) {
											zdiff3 = sp2c.z - pos2->z;
											zdiff3 = zdiff3 > 0.0f ? zdiff3 : -zdiff3;

											if (zdiff3 < sp64) {
												f32 dist3 = sqrtf(xdiff3 * xdiff3 + zdiff3 * zdiff3);

												if (dist3 < sp64) {
													sp64 -= dist3;
													*sp68 -= sp64;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return *sp68;
}

void func0f0056f4(s32 roomnum1, struct coord *pos1, s32 roomnum2, struct coord *pos2, s32 arg4, f32 *result, s32 arg6)
{
	f32 dist;

	if (!var80061444
			|| PLAYERCOUNT() >= 3
			|| roomnum1 == roomnum2
			|| roomnum1 == -1
			|| roomnum2 == -1) {
		f32 xdist = pos1->x - pos2->x;

		if (!(xdist > 0.0f)) {
			xdist = -xdist;
		}

		if (xdist < *result) {
			f32 zdist = pos1->z - pos2->z;

			if (!(zdist > 0.0f)) {
				zdist = -zdist;
			}

			if (zdist < *result) {
				f32 ydist = pos1->y - pos2->y;

				if (!(ydist > 0.0f)) {
					ydist = -ydist;
				}

				if (ydist < *result) {
					dist = sqrtf(xdist * xdist + ydist * ydist + zdist * zdist);

					if (dist < *result) {
						*result = dist;
					}
				}
			}
		}
	} else {
		s32 portalnum1;
		s32 portalnum2;
		s32 i;
		s32 j;

		for (i = 0; i < g_Rooms[roomnum1].numportals; i++) {
			portalnum1 = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + i];

			for (j = 0; j < g_Rooms[roomnum2].numportals; j++) {
				portalnum2 = g_RoomPortals[g_Rooms[roomnum2].roomportallistoffset + j];

				dist = func0f0053d0(roomnum1, pos1, portalnum1, roomnum2, pos2, portalnum2, result);

				if (dist < *result) {
					*result = dist;
				}
			}
		}
	}
}

void func0f0059fc(s32 roomnum1, struct coord *pos1, s32 roomnum2, struct coord *pos2, s32 arg4, f32 *result)
{
	s32 portalnum1;
	s32 portalnum2;
	s32 i;
	s32 j;
	f32 dist;

	*result = 32767;

	if (roomnum1 == roomnum2) {
		*result = coordsGetDistance(pos1, pos2);
		return;
	}

	for (i = 0; i < g_Rooms[roomnum1].numportals; i++) {
		portalnum1 = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + i];
		if (1);

		for (j = 0; j < g_Rooms[roomnum2].numportals; j++) {
			portalnum2 = g_RoomPortals[g_Rooms[roomnum2].roomportallistoffset + j];
			if (j);
			if (j);

			dist = func0f0053d0(roomnum1, pos1, portalnum1, roomnum2, pos2, portalnum2, NULL);

			if (dist < *result) {
				*result = dist;
			}
		}
	}
}

/**
 * This function:
 * - Controls the sound effects for the night vision and IR scanner.
 * - Sets g_IsSwitchingGoggles if equipping or unequipping NV/IR on this frame.
 * - Updates the player's usinggoggles property.
 */
void func0f005bb0(void)
{
	s32 brightness = roomGetFinalBrightness(g_Vars.currentplayer->prop->rooms[0]);

	if (((USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) && !g_Vars.currentplayer->usinggoggles)
			|| ((!USINGDEVICE(DEVICE_NIGHTVISION) && !USINGDEVICE(DEVICE_IRSCANNER)) && g_Vars.currentplayer->usinggoggles)) {
		g_IsSwitchingGoggles = true;
	}

	g_Vars.currentplayer->usinggoggles = USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER);

	if (USINGDEVICE(DEVICE_NIGHTVISION) && !lvIsPaused()) {
		// Play the goggle's hum sound
		if (g_Vars.currentplayer->nvhum == NULL) {
			sndStart(var80095200, SFX_0505, &g_Vars.currentplayer->nvhum, -1, -1, -1.0f, -1, -1);
		}

		if (brightness > 128) {
			// Room is too bright for night vision - play overload sound
			if (g_Vars.currentplayer->nvoverload == NULL) {
				sndStart(var80095200, SFX_01BE, &g_Vars.currentplayer->nvoverload, -1, -1, -1.0f, -1, -1);
			}
		} else {
			// Room is dark enough for night vision - stop overload sound if active
			if (g_Vars.currentplayer->nvoverload != NULL) {
				if (sndGetState(g_Vars.currentplayer->nvoverload) != AL_STOPPED) {
					audioStop(g_Vars.currentplayer->nvoverload);
				}
			}
		}
	} else {
		// Paused or not wearing night vision - stop both sounds
		if (g_Vars.currentplayer->nvhum != NULL) {
			if (sndGetState(g_Vars.currentplayer->nvhum) != NULL) {
				audioStop(g_Vars.currentplayer->nvhum);
			}
		}

		if (g_Vars.currentplayer->nvoverload != NULL) {
			if (sndGetState(g_Vars.currentplayer->nvoverload) != NULL) {
				audioStop(g_Vars.currentplayer->nvoverload);
			}
		}
	}
}
