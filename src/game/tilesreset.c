#include <ultra64.h>
#include "constants.h"
#include "game/tiles.h"
#include "game/bg.h"
#include "game/file.h"
#include "bss.h"
#include "data.h"
#include "types.h"

void stageParseTiles(void);

void tilesReset(void)
{
	s32 index = bgGetStageIndex(g_Vars.stagenum);

	if (index < 0) {
		index = 0;
	}

	g_LoadType = LOADTYPE_TILES;
	g_TileFileData.u8 = fileLoadToNew(g_Stages[index].tilefileid, FILELOADMETHOD_DEFAULT, LOADTYPE_TILES);
	g_TileNumRooms = *g_TileFileData.u32;
	g_TileRooms = g_TileFileData.u32 + 1;

	stageParseTiles();
}

#define mult6(a) (((a) << 1) + ((a) << 2))

void stageParseTiles(void)
{
	struct geo *geo = (struct geo *)(g_TileFileData.u8 + g_TileRooms[0]);
	struct geo *end = (struct geo *)(g_TileFileData.u8 + g_TileRooms[g_TileNumRooms]);

	while (geo < end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;
			tile->xmin = mult6(tile->xmin) + 14;
			tile->xmax = mult6(tile->xmax) + 14;
			tile->ymin = mult6(tile->ymin) + 16;
			tile->ymax = mult6(tile->ymax) + 16;
			tile->zmin = mult6(tile->zmin) + 18;
			tile->zmax = mult6(tile->zmax) + 18;
			geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 6 + 0x18e);
		} else if (geo->type == GEOTYPE_TILE_F) {
			geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 12 + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			geo = (struct geo *)((u8 *)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			geo = (struct geo *)((u8 *)geo + sizeof(struct geocyl));
		}
	}
}
