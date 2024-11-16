#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "bss.h"

#include "preprocess/common.h"
#include "preprocess/gbi.h"

/**
 * 4 bytes decompressed size of primary data
 * 4 bytes compressed size of section 1 in its entirety
 * 4 bytes compressed size of primary data
 * Section 1:
 *     Primary: (zipped)
 *         4 bytes null
 *         4 bytes pointer to room table
 *         4 bytes pointer to portal table
 *         4 bytes pointer to bgcmds
 *         4 bytes pointer to lights table
 *         4 bytes null
 *         (room table)
 *         (portal table)
 *         (bgcmds)
 *         (lights table)
 *     room 1 (zipped)
 *     room 2 (zipped)
 *     ...
 * Section 2:
 *     2 bytes decompressed size of section (mask with 0x7fff)
 *     2 bytes compressed size of section
 *     Texture ID list (zipped)
 * Section 3:
 *     2 bytes decompressed size of section (mask with 0x7fff)
 *     2 bytes compressed size of section
 *     Zipped:
 *         (room bbox table)
 *         (list of roomgfxdata sizes)
 *         (list of light counts per room)
 */

struct fileheader {
	u32 primary_infsize;
	u32 section1_cmpsize;
	u32 primary_cmpsize;
};

struct sectionheader {
	u16 infsize;
	u16 cmpsize;
};

struct n64_primaryheader {
	u32 unused1;
	u32 ptr_rooms;
	u32 ptr_portals;
	u32 ptr_bgcmds;
	u32 ptr_lights;
	u32 unused2;
};

struct host_primaryheader {
	uintptr_t unused1;
	uintptr_t ptr_rooms;
	uintptr_t ptr_portals;
	uintptr_t ptr_bgcmds;
	uintptr_t ptr_lights;
	uintptr_t unused2;
};

struct n64_bgroom {
	u32 ptr_gfxdata;
	struct coord pos;
	u8 br_light_min;
	u8 br_light_max;
};

struct n64_roomblock {
	u8 type;
	u32 ptr_next;
	u32 ptr_gdl;
	u32 ptr_vertices;
	u32 ptr_colours;
};

struct n64_roomgfxdata {
	u32 ptr_vertices;
	u32 ptr_colours;
	u32 ptr_opablocks;
	u32 ptr_xlublocks;
	s16 lightsindex;
	s16 numlights;
	s16 numvertices;
	s16 numcolours;
	// roomblocks[]
};

struct vtx {
	s16 coord[3];
	u8 flag;
	u8 color;
	u16 s;
	u16 t;
};

static void convertPrimaryRooms(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos)
{
	struct n64_bgroom *n64_rooms = (struct n64_bgroom *) &src[*srcpos];
	struct bgroom *host_rooms = (struct bgroom *) &dst[*dstpos];

	s32 numRooms = 0;

	for (int i = 1; n64_rooms[i].ptr_gfxdata != 0; i++) {
		numRooms++;
	}

	for (int i = 0; i < numRooms + 1; i++) {
		host_rooms[i].unk00 = PD_BE32(n64_rooms[i].ptr_gfxdata);
		host_rooms[i].pos = PD_SWAPPED_VAL(n64_rooms[i].pos);
		host_rooms[i].br_light_min = n64_rooms[i].br_light_min;
		host_rooms[i].br_light_max = n64_rooms[i].br_light_max;
	}

	*srcpos += sizeof(*n64_rooms) * (numRooms + 2);
	*dstpos += sizeof(*host_rooms) * (numRooms +2);
}

static void convertPrimaryPortals(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos, u32 *src_portalvtxs, u32 *dst_portalvtxs)
{
	struct bgportal *n64_portals = (struct bgportal *) &src[*srcpos];
	struct bgportal *host_portals = (struct bgportal *) &dst[*dstpos];

	// Count portals
	int numportals = 0;

	for (int i = 0; n64_portals[i].verticesoffset != 0; i++) {
		numportals++;
	}

	// Convert portals
	for (int i = 0; i < numportals; i++) {
		host_portals[i].verticesoffset = PD_BE16(n64_portals[i].verticesoffset);
		host_portals[i].roomnum1 = PD_BE16(n64_portals[i].roomnum1);
		host_portals[i].roomnum2 = PD_BE16(n64_portals[i].roomnum2);
		host_portals[i].flags = n64_portals[i].flags;
	}

	*srcpos += sizeof(*n64_portals) * (numportals + 1);
	*dstpos += sizeof(*host_portals) * (numportals + 1);

	*src_portalvtxs = *srcpos;
	*dst_portalvtxs = *dstpos;

	while (1) {
		struct portalvertices *n64_pvertices = (struct portalvertices *)&src[*srcpos];
		struct portalvertices *host_pvertices = (struct portalvertices *)&dst[*dstpos];

		host_pvertices->count = n64_pvertices->count;

		for (int j = 0; j < host_pvertices->count; j++) {
			host_pvertices->vertices[j] = PD_SWAPPED_VAL(n64_pvertices->vertices[j]);
		}

		*srcpos += 4 + host_pvertices->count * 12;
		*dstpos += 4 + host_pvertices->count * 12;

		if (host_pvertices->count == 0)  break;
	}
}

static void convertPrimaryBgCmds(u8 *dst, u32 *dstpos, u8 *src, u32 dst_portalvtxs, u32 src_portalvtxs)
{
	struct bgcmd *n64_cmds = (struct bgcmd *) src;
	struct bgcmd *host_cmds = (struct bgcmd *) &dst[*dstpos];
	int num_cmds = 0;

	for (int i = 0; n64_cmds[i].type != 0; i++) {
		num_cmds++;
	}

	num_cmds++;

	for (int i = 0; i < num_cmds; i++) {
		host_cmds[i].type = n64_cmds[i].type;
		host_cmds[i].len = n64_cmds[i].len;
		host_cmds[i].param = PD_BE32(n64_cmds[i].param);

		if (host_cmds[i].type == 0x64) {
			u32 offset = host_cmds[i].param - src_portalvtxs;
			host_cmds[i].param = dst_portalvtxs + offset;
		}
	}

	*dstpos += sizeof(*host_cmds) * num_cmds;
}

static void convertPrimaryLights(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos, u32 end)
{
	struct light *n64_lights = (struct light *) &src[*srcpos];
	struct light *host_lights = (struct light *) &dst[*dstpos];
	int num_lights = (end - *srcpos) / sizeof(struct light);

	for (int i = 0; i < num_lights; i++) {
		host_lights[i].roomnum = PD_BE16(n64_lights[i].roomnum);
		host_lights[i].colour = PD_BE16(n64_lights[i].colour);
		host_lights[i].brightness = n64_lights[i].brightness;
		host_lights[i].sparkable = n64_lights[i].sparkable;
		host_lights[i].healthy = n64_lights[i].healthy;
		host_lights[i].on = n64_lights[i].on;
		host_lights[i].sparking = n64_lights[i].sparking;
		host_lights[i].vulnerable = n64_lights[i].vulnerable;
		host_lights[i].brightnessmult = n64_lights[i].brightnessmult;
		host_lights[i].dirx = n64_lights[i].dirx;
		host_lights[i].diry = n64_lights[i].diry;
		host_lights[i].dirz = n64_lights[i].dirz;

		for (int j = 0; j < 4; j++) {
			host_lights[i].bbox[j].x = PD_BE16(n64_lights[i].bbox[j].x);
			host_lights[i].bbox[j].y = PD_BE16(n64_lights[i].bbox[j].y);
			host_lights[i].bbox[j].z = PD_BE16(n64_lights[i].bbox[j].z);
		}
	}

	*srcpos += sizeof(*n64_lights) * num_lights;
	*dstpos += sizeof(*host_lights) * num_lights;
}

#define BLOCK_LEAF 0
#define BLOCK_PARENT 1

void relinkPtr(uintptr_t* ptr)
{
	if (!*ptr) return;

	struct ptrmarker* marker = ptrFind(*ptr);
	if (!marker) {
		sysFatalError("[BG] WARNING: Unable to relink pointer: %p\n", ptr);
	}
	
	*ptr = marker->ptr_host;
}

static u32 convertRoomGfxData(u8 *dst, u8 *src, u32 infsize, u32 src_ofs)
{
	ptrReset();
	gbiReset();

	u32 dst_roomoffset = src_ofs;

	uintptr_t gdls_addr[64];
	int numgdls = 0;

	struct n64_roomgfxdata *src_header = (struct n64_roomgfxdata*)src;
	struct roomgfxdata *dst_header = (struct roomgfxdata*)dst;

	dst_header->vertices = (void *)(uintptr_t)PD_BE32(src_header->ptr_vertices);
	dst_header->colours = (void *)(uintptr_t)PD_BE32(src_header->ptr_colours);
	dst_header->opablocks = (void *)(uintptr_t)PD_BE32(src_header->ptr_opablocks);
	dst_header->xlublocks = (void *)(uintptr_t)PD_BE32(src_header->ptr_xlublocks);
	dst_header->lightsindex = PD_BE16(src_header->lightsindex);
	dst_header->numlights = PD_BE16(src_header->numlights);
	dst_header->numvertices = PD_BE16(src_header->numvertices);
	dst_header->numcolours = PD_BE16(src_header->numcolours);

	intptr_t endpos = (uintptr_t)dst_header->vertices - src_ofs;
	uintptr_t curpos_src = sizeof(struct n64_roomgfxdata);
	uintptr_t curpos_dst = sizeof(struct roomgfxdata) - sizeof(struct roomblock); // compensate for the [1]

	// roomblocks
	int ncoords = 0;
	
	// save the ptr to roomblocks here to iterate later to fix the pointers
	struct roomblock *ptr_dst_roomblocks = (struct roomblock*)&dst[curpos_dst];

	int numblocks = 0;
	while ((intptr_t)(curpos_src + sizeof(struct n64_roomblock)) <= endpos) {
		struct n64_roomblock *src_roomblock = (struct n64_roomblock*)&src[curpos_src];
		struct roomblock *dest_roomblock = (struct roomblock*)&dst[curpos_dst];

		dest_roomblock->type = src_roomblock->type;
		dest_roomblock->next = (void *)(uintptr_t)PD_BE32(src_roomblock->ptr_next);
		dest_roomblock->gdl = (void *)(uintptr_t)PD_BE32(src_roomblock->ptr_gdl);
		dest_roomblock->vertices = (void *)(uintptr_t)PD_BE32(src_roomblock->ptr_vertices);
		dest_roomblock->colours = (void *)(uintptr_t)PD_BE32(src_roomblock->ptr_colours);

		ptrAdd(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		if (src_roomblock->type == BLOCK_PARENT) {
			ncoords++;
			uintptr_t vtx_ptr = (uintptr_t)dest_roomblock->vertices - src_ofs;
			if (vtx_ptr < endpos) endpos = vtx_ptr;
		}
		else if (src_roomblock->ptr_gdl) {
			gdls_addr[numgdls++] = PD_BE32(src_roomblock->ptr_gdl) - src_ofs;
		}

		curpos_src += sizeof(struct n64_roomblock);
		curpos_dst += sizeof(struct roomblock);
		numblocks++;
	}

	// block coords
	for (size_t i = 0; i < ncoords; i++) {
		u32 *src_coord = (u32*)(src + curpos_src);
		u32 *dst_coord = (u32*)(dst + curpos_dst);

		dst_coord[0] = PD_BE32(src_coord[0]);
		dst_coord[1] = PD_BE32(src_coord[1]);
		dst_coord[2] = PD_BE32(src_coord[2]);
		
		dst_coord[3] = PD_BE32(src_coord[3]);
		dst_coord[4] = PD_BE32(src_coord[4]);
		dst_coord[5] = PD_BE32(src_coord[5]);

		ptrAdd(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		curpos_src += 6 * sizeof(u32);
		curpos_dst += 6 * sizeof(u32);
	}

	curpos_dst = ALIGN8(curpos_dst);

	// vertices
	uintptr_t ptr_src_vertices = curpos_src = ALIGN8((uintptr_t)dst_header->vertices - src_ofs);
	uintptr_t ptr_dst_vertices = curpos_dst + dst_roomoffset;

	ptrAdd(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

	uintptr_t vtx_end = (uintptr_t)dst_header->colours - src_ofs;

	while (dst_header->colours && curpos_src < vtx_end) {
		struct vtx *src_vtx = (struct vtx*)(src + curpos_src); 
		struct vtx *dst_vtx = (struct vtx*)(dst + curpos_dst);

		dst_vtx->coord[0] = PD_BE16(src_vtx->coord[0]);
		dst_vtx->coord[1] = PD_BE16(src_vtx->coord[1]);
		dst_vtx->coord[2] = PD_BE16(src_vtx->coord[2]);
		
		dst_vtx->flag = src_vtx->flag;
		dst_vtx->color = src_vtx->color;

		dst_vtx->s = PD_BE16(src_vtx->s);
		dst_vtx->t = PD_BE16(src_vtx->t);

		curpos_src += sizeof(struct vtx);
		curpos_dst += sizeof(struct vtx);
	}

	curpos_dst = ALIGN8(curpos_dst);

	// colors
	uintptr_t ptr_src_colors = 0;
	uintptr_t ptr_dst_colors = 0;

	if (dst_header->colours) {
		ptr_src_colors = curpos_src = ALIGN8((uintptr_t)dst_header->colours - src_ofs);
		ptr_dst_colors = curpos_dst + dst_roomoffset;

		ptrAdd(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		uintptr_t col_end = (numgdls > 0) ? gdls_addr[0] : infsize;
		size_t col_len = col_end - curpos_src;

		memcpy(dst + curpos_dst, src + curpos_src, col_len);
		curpos_src += col_len;
		curpos_dst += col_len;
	}

	// gdls
	gbiSetVtx(src_header->ptr_vertices, ptr_src_vertices);
	gbiSetSegment(0x0e, (uintptr_t)dst_header->vertices);

	uintptr_t gdlstart = curpos_dst;
	for (size_t i = 0; i < numgdls; i++) {
		ptrAdd(gdls_addr[i] + src_ofs, curpos_dst + dst_roomoffset);
		curpos_dst = gbiConvertGdl(dst, curpos_dst, src, gdls_addr[i], 1);
	}

	// relink ptrs: roomblocks
	for (size_t i = 0; i < numblocks; i++) {
		struct roomblock *block = (struct roomblock*)&ptr_dst_roomblocks[i];

		relinkPtr((uintptr_t*)&block->next);

		if (block->type == BLOCK_LEAF) {
			relinkPtr((uintptr_t*)&block->gdl);
			u32 offset_vtx = (uintptr_t)block->vertices - ptr_src_vertices - src_ofs;
			u32 offset_col = (uintptr_t)block->colours - ptr_src_colors - src_ofs;

			block->vertices = (void *)(ptr_dst_vertices + offset_vtx);
			block->colours = (void *)(ptr_dst_colors + offset_col);
		}
		else {
			relinkPtr((uintptr_t*)&block->gdl); // child
			relinkPtr((uintptr_t*)&block->vertices); // coord
		}
	}
	
	// relink ptrs: header
	relinkPtr((uintptr_t*)&dst_header->vertices);
	relinkPtr((uintptr_t*)&dst_header->colours);
	relinkPtr((uintptr_t*)&dst_header->opablocks);
	relinkPtr((uintptr_t*)&dst_header->xlublocks);

	return curpos_dst;
}

static u32 convertSection1(u8 *dst, u8 *src, u32 ofs)
{
	// Convert primary
	u32 srcpos = 0;
	u32 dstpos = 0;
	struct host_primaryheader *host_primary_header = (struct host_primaryheader *) dst;
	struct n64_primaryheader *n64_primary_header = (struct n64_primaryheader *) src;
	srcpos += sizeof(struct n64_primaryheader);
	dstpos += sizeof(*host_primary_header);

	u32 src_bgcmds = PD_BE32(n64_primary_header->ptr_bgcmds) - ofs;

	host_primary_header->unused1 = 0;
	host_primary_header->unused2 = 0;

	host_primary_header->ptr_rooms = dstpos + ofs;
	convertPrimaryRooms(dst, &dstpos, src, &srcpos);
	
	if (n64_primary_header->ptr_lights) {
		host_primary_header->ptr_lights = dstpos + ofs;
		srcpos = PD_BE32(n64_primary_header->ptr_lights) - ofs;
		convertPrimaryLights(dst, &dstpos, src, &srcpos, src_bgcmds);
	}

	host_primary_header->ptr_portals = dstpos + ofs;
	srcpos = PD_BE32(n64_primary_header->ptr_portals) - ofs;
	u32 src_portalvtxs, dst_portalvtxs;
	convertPrimaryPortals(dst, &dstpos, src, &srcpos, &src_portalvtxs, &dst_portalvtxs);

	host_primary_header->ptr_bgcmds = dstpos + ofs;
	convertPrimaryBgCmds(dst, &dstpos, &src[src_bgcmds], dst_portalvtxs, src_portalvtxs);

	return dstpos;
}

void preprocessBgSection1(u8 *data, u32 size, u32 ofs)
{
	u8 *dst = sysMemZeroAlloc(size);

	u32 newSize = convertSection1(dst, data, ofs);

	if (newSize > size) {
		sysFatalError("overflow when trying to preprocess a bg file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);
}

u32 preprocessBgRoom(u8 *data, u32 size, u32 room_ofs)
{
	size *= 2;
	u8 *dst = sysMemZeroAlloc(size);
	u32 newSize = convertRoomGfxData(dst, data, size, room_ofs);

	if (newSize > size) {
		sysFatalError("overflow when trying to preprocess a bg room, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	return newSize;
}

void preprocessBgSection1Header(u8* data, u32 size)
{
	u32* header = (u32*)data;
	PD_SWAP_VAL(header[0]); // inflatedsize
	PD_SWAP_VAL(header[1]); // section1size
	PD_SWAP_VAL(header[2]); // primcompsize
}

void preprocessBgSection2Header(u8* data, u32 size)
{
	u16* header = (u16*)data;
	PD_SWAP_VAL(header[0]); // inflatedsize
	PD_SWAP_VAL(header[1]); // section2compsize
}

void preprocessBgSection2(u8* data, u32 size)
{
	// section2 is a texture id list
	u16* section2 = (u16*)data;
	for (s32 i = 0; i < size; ++i) {
		PD_SWAP_VAL(section2[i]);
	}
}

void preprocessBgSection3Header(u8* data, u32 size)
{
	u16* header = (u16*)data;
	PD_SWAP_VAL(header[0]);
	PD_SWAP_VAL(header[1]);
}

void preprocessBgSection3(u8* data, u32 size)
{
	// room bounding boxes, 6x s16 per room
	s16* bbox = (s16*)data;
	for (s32 i = 1; i < g_Vars.roomcount; ++i) {
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
		PD_SWAP_VAL(*bbox); ++bbox;
	}

	// gfxdatalen list
	u16* gfxdatalen = (u16*)bbox;
	for (s32 i = 1; i < g_Vars.roomcount; ++i) {
		PD_SWAP_VAL(*gfxdatalen); ++gfxdatalen;
	}
}
