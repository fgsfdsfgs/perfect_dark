#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/dyntex.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

/**
 * dyntex - dynamic textures
 *
 * This file handles textures which animate automatically, such as water.
 *
 * The dyntex system maintains three conceptually nested arrays: rooms, types
 * and vertices.
 *
 * Rooms are the first tier. Rooms will only exist in the array if they contain
 * animated textures. Rooms contain types.
 *
 * Types are a type of animation. Linear is the most common, but there's also
 * ocean waves, and some specific types such as Attack Ship triangular arrows.
 * Types contain vertices.
 *
 * Vertices contain an offset to the graphics vertex, as well as a copy of its
 * S and T values.
 *
 * The caller should call dyntexSetCurrentRoom and dyntexSetCurrentType, then
 * add vertices with dyntexAddVertex. Lastly, dyntexTickRoom should be called
 * on each tick for each nearby room. dyntexTickRoom can be called multiple
 * times on the same frame (such as if there's two players), as dyntex will
 * ensure it's only updated once per frame.
 *
 * Data is added to dyntex during gameplay as rooms are loaded. When a room is
 * unloaded the data remains in the dyntex arrays. When a room is loaded again
 * dyntex will not add it a second time.
 */

struct dyntexroom {
	u16 roomnum;
	u16 typelistoffset;
	u16 numtypes;
	s32 updatedframe;
};

struct dyntextype {
	u16 type : 7;
	u16 initialised : 1;
	u8 numvertices;
	u16 vertexlistoffset;
};

struct dyntexvtx {
	u16 offset;
	s16 s;
	s16 t;
};

s32 g_DyntexVerticesMax;
s32 g_DyntexTypesMax;
s32 g_DyntexRoomsMax;
struct dyntexvtx *g_DyntexVertices;
struct dyntextype *g_DyntexTypes;
struct dyntexroom *g_DyntexRooms;

s32 g_DyntexCurRoom = -1;
s32 g_DyntexCurType = -1;
bool g_DyntexRoomPopulated = false;
bool g_DyntexTypePopulated = false;
s32 g_DyntexRoomsCount = 0;
s32 g_DyntexTypesCount = 0;
s32 g_DyntexVerticesCount = 0;

void dyntexUpdateLinear(Vtx *vertices, struct dyntextype *type)
{
	s16 tmp = (s32) (g_Lv80SecIntervalFrac * 10.0f * 4096.0f) % 4096;
	s32 i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t + tmp;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s;
	}
}

void dyntexUpdateReset(Vtx *vertices, struct dyntextype *type)
{
	s32 i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->s = 0;
		vertex->t = 0;
	}
}

void dyntexUpdateMonitor(Vtx *vertices, struct dyntextype *type)
{
	s16 tmp = (s32) (g_Lv80SecIntervalFrac * 4.0f * 4096.0f) % 4096;
	s32 i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t - tmp;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s;
	}
}

void dyntexUpdateOcean(Vtx *vertices, struct dyntextype *type)
{
	f32 f24 = g_Lv80SecIntervalFrac * 5.0f;
	f32 angle;
	s32 i;

	static u32 ripsize = 65;
	static u32 modula = 22;

	mainOverrideVariable("modula", &modula);
	mainOverrideVariable("ripsize", &ripsize);

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		angle = ((g_DyntexVertices[type->vertexlistoffset + i].t % modula) / (f32) modula + f24) * M_BADTAU;
		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t + (s16) (sinf(angle) * ripsize);

		angle = (((g_DyntexVertices[type->vertexlistoffset + i].s + 22) % modula) / (f32) modula + f24) * M_BADTAU;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s + (s16) (cosf(angle) * ripsize);
	}
}

void dyntexUpdateArrows(Vtx *vertices, struct dyntextype *type)
{
	s32 tmp = ((s32) ((1.0f - g_Lv80SecIntervalFrac) * 60.0f * 8.0f) % 8) * 256;
	s32 i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s + tmp;
		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t;
	}
}

void dyntexTickRoom(s32 roomnum, Vtx *vertices)
{
	s32 index = -1;
	s32 i;

	for (i = 0; i < g_DyntexRoomsCount; i++) {
		if (g_DyntexRooms[i].roomnum == roomnum) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		return;
	}

	if (g_Vars.lvframenum == g_DyntexRooms[index].updatedframe) {
		return;
	}

	for (i = 0; i < g_DyntexRooms[index].numtypes; i++) {
		struct dyntextype *type = &g_DyntexTypes[g_DyntexRooms[index].typelistoffset + i];
		s32 mins = 32767;
		s32 maxs = -32766;
		s32 mint = 32767;
		s32 maxt = -32766;

		if (!type->initialised) {
			s32 adds = 0;
			s32 addt = 0;

			if (1);

			// @bug: Using i for both outer and inner loops
			for (i = 0; i < type->numvertices; i++) {
				Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

				g_DyntexVertices[type->vertexlistoffset + i].s = vertex->s;
				g_DyntexVertices[type->vertexlistoffset + i].t = vertex->t;

				if (vertex->s < mins) {
					mins = vertex->s;
				}

				if (vertex->t < mint) {
					mint = vertex->t;
				}

				if (vertex->s > maxs) {
					maxs = vertex->s;
				}

				if (vertex->t > maxt) {
					maxt = vertex->t;
				}
			}

			type->initialised = true;

			if (mins < -0x5d00) {
				adds = 0x2000;
			}

			if (mint < -0x5d00) {
				addt = 0x2000;
			}

			if (maxs > 0x5d00) {
				adds = -0x2000;
			}

			if (maxt > 0x5d00) {
				addt = -0x2000;
			}

			if (adds || addt) {
				for (i = 0; i < type->numvertices; i++) {
					g_DyntexVertices[type->vertexlistoffset + i].s += adds;
					g_DyntexVertices[type->vertexlistoffset + i].t += addt;
				}
			}
		}

		switch (g_DyntexTypes[g_DyntexRooms[index].typelistoffset + i].type) {
		case DYNTEXTYPE_RIVER:
			dyntexUpdateLinear(vertices, type);
			break;
		case DYNTEXTYPE_MONITOR:
			dyntexUpdateMonitor(vertices, type);
			break;
		case DYNTEXTYPE_OCEAN:
			dyntexUpdateOcean(vertices, type);
			break;
		case DYNTEXTYPE_ARROWS:
			dyntexUpdateArrows(vertices, type);
			break;
		case DYNTEXTYPE_TELEPORTAL:
			// Deep Sea - teleports enabled and not SA disabled
			if (chrHasStageFlag(0, 0x00000100) && !chrHasStageFlag(0, 0x00010000)) {
				dyntexUpdateLinear(vertices, type);
			}
			break;
		case DYNTEXTYPE_POWERRING:
			if (chrHasStageFlag(0, 0x00010000)) {
				// Attack Ship engines are destroyed
				dyntexUpdateReset(vertices, type);
			} else {
				// Attack Ship engines are healthy
				dyntexUpdateLinear(vertices, type);
			}
			break;
		case DYNTEXTYPE_POWERJUICE:
			if (!chrHasStageFlag(0, 0x00010000)) {
				// Attack Ship engines are healthy
				dyntexUpdateLinear(vertices, type);
			}
			break;
		}
	}

	g_DyntexRooms[index].updatedframe = g_Vars.lvframenum;
}

void dyntexAddVertex(Vtx *vertex)
{
	if (g_DyntexCurRoom < 0) {
		return;
	}

	if (g_DyntexVerticesCount == g_DyntexVerticesMax) {
		return;
	}

	if (!g_DyntexRoomPopulated) {
		if (g_DyntexTypesCount >= g_DyntexTypesMax || g_DyntexRoomsCount >= g_DyntexRoomsMax) {
			return;
		}

		g_DyntexRooms[g_DyntexRoomsCount].roomnum = g_DyntexCurRoom;
		g_DyntexRooms[g_DyntexRoomsCount].typelistoffset = g_DyntexTypesCount;
		g_DyntexRooms[g_DyntexRoomsCount].numtypes = 0;
		g_DyntexRooms[g_DyntexRoomsCount].updatedframe = 0;

		g_Rooms[g_DyntexCurRoom].flags |= ROOMFLAG_HASDYNTEX;

		g_DyntexRoomsCount++;
		g_DyntexRoomPopulated = true;
	}

	if (!g_DyntexTypePopulated) {
		if (g_DyntexTypesCount >= g_DyntexTypesMax) {
			return;
		}

		g_DyntexTypes[g_DyntexTypesCount].type = g_DyntexCurType;
		g_DyntexTypes[g_DyntexTypesCount].initialised = false;
		g_DyntexTypes[g_DyntexTypesCount].numvertices = 0;
		g_DyntexTypes[g_DyntexTypesCount].vertexlistoffset = g_DyntexVerticesCount;
		g_DyntexTypesCount++;

		g_DyntexRooms[g_DyntexRoomsCount - 1].numtypes++;
		g_DyntexTypePopulated = true;
	}

	g_DyntexVertices[g_DyntexVerticesCount].offset = (u16)vertex;
	g_DyntexVerticesCount++;

	g_DyntexTypes[g_DyntexTypesCount - 1].numvertices++;
}

void dyntexSetCurrentType(s16 type)
{
	// Air Base - don't animate anything (exterior water)
	if (g_StageIndex == STAGEINDEX_AIRBASE) {
		return;
	}

	// Investigation - don't animate the puddle of water behind the glass
	if (g_StageIndex == STAGEINDEX_INVESTIGATION && type == DYNTEXTYPE_RIVER) {
		return;
	}

	// Villa - don't animate shallow water
	if (g_StageIndex == STAGEINDEX_VILLA && type == DYNTEXTYPE_RIVER) {
		return;
	}

	// Power juice and power rings exist on Deep Sea and Attack Ship.
	//
	// Deep Sea - in the SA megaweapon
	// Attack Ship - in the engine's power node
	//
	// These both use a linear animation, but Attack Ship's are conditional on
	// the ship's engines running. To avoid doing a stage check on every tick,
	// Deep Sea's are retyped to the river type which uses an unconditional
	// linear animation. Attack Ship's remains as is.
	if (g_StageIndex != STAGEINDEX_ATTACKSHIP && (type == DYNTEXTYPE_POWERJUICE || type == DYNTEXTYPE_POWERRING)) {
		type = DYNTEXTYPE_RIVER;
	}

	if (type != g_DyntexCurType) {
		g_DyntexTypePopulated = false;
	}

	g_DyntexCurType = type;
}

void dyntexSetCurrentRoom(RoomNum roomnum)
{
	s32 i;

	if (roomnum >= 0) {
		for (i = 0; i < g_DyntexRoomsCount; i++) {
			if (g_DyntexRooms[i].roomnum == roomnum) {
				return;
			}
		}
	}

	if (g_DyntexRooms != NULL) {
		g_DyntexRoomPopulated = false;
		g_DyntexTypePopulated = false;
		g_DyntexCurRoom = roomnum;
		g_DyntexCurType = -1;
	}
}

void dyntexReset(void)
{
	u32 size3;
	u32 size2;
	u32 size1;

	g_DyntexCurRoom = -1;
	g_DyntexCurType = -1;
	g_DyntexRoomPopulated = false;
	g_DyntexRoomsCount = 0;
	g_DyntexTypesCount = 0;
	g_DyntexVerticesCount = 0;

	g_DyntexVerticesMax = 1200;
	g_DyntexTypesMax = 50;
	g_DyntexRoomsMax = 50;

	size1 = ALIGN64(g_DyntexTypesMax * sizeof(struct dyntextype));
	g_DyntexTypes = mempAlloc(size1, MEMPOOL_STAGE);

	size2 = ALIGN64(g_DyntexVerticesMax * sizeof(struct dyntexvtx));
	g_DyntexVertices = mempAlloc(size2, MEMPOOL_STAGE);

	size3 = ALIGN64(g_DyntexRoomsMax * sizeof(struct dyntexroom));
	g_DyntexRooms = mempAlloc(size3, MEMPOOL_STAGE);

	if (g_DyntexVerticesMax);
	if (g_DyntexTypesMax);
	if (size1);
}

u32 var8007f704 = 0x0000001d;
u32 var8007f708 = 0x0000004a;
u32 var8007f70c = 0x00000006;
u32 var8007f710 = 0x00000006;
u32 var8007f714 = 0x00000038;
u32 var8007f718 = 0x000003d5;
u32 var8007f71c = 0x00000018;
u32 var8007f720 = 0x00000018;
u32 var8007f724 = 0x00000034;
u32 var8007f728 = 0x000002f7;
u32 var8007f72c = 0x00000012;
u32 var8007f730 = 0x00000012;

void dyntex0f13c4e8(void)
{
	// empty
}

bool dyntexHasRoom(void)
{
	return g_DyntexCurRoom >= 0;
}
