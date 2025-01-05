#include <ultra64.h>
#include "constants.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/savebuffer.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#if VERSION >= VERSION_NTSC_1_0
Vp *var80070f10 = NULL;
#else
Vp var80070f10;
#endif

s32 var8009de90;
s32 var8009de94;
s32 g_MenuProjectFromX;
s32 g_MenuProjectFromY;

void func0f0d4690(Mtxf *mtx)
{
	struct coord pos;

	mtx4LoadIdentity(mtx);

	pos.x = -159.75f;
	pos.y = PAL ? 136.25f : 120.25f;
	pos.z = 0;

	pos.x = (.5f - viGetWidth()) * 0.5f;
	pos.y = (.5f + viGetHeight()) * 0.5f;
	pos.z = 0;

	mtx4SetTranslation(&pos, mtx);
	mtx00015e4c(-1, mtx);

	if (g_ScaleX == 2) {
		mtx00015df0(2, mtx);
	}
}

void func0f0d475c(Mtxf *mtx)
{
	func0f0d4690(mtx);
	mtx00015df0(0.1f, mtx);
	mtx00015e4c(0.1f, mtx);
}

Gfx *func0f0d479c(Gfx *gdl)
{
	Mtxf mtx;
	Mtxf *mtx1;
	Mtxf *mtx2;

	mtx1 = gfxAllocateMatrix();
	mtx2 = gfxAllocateMatrix();

	mtx00016760();
	func0f0d475c(&mtx);
	mtxF2L(&mtx, mtx2);
	mtx4LoadIdentity(&mtx);

	guFrustumF(mtx.m,
			-(f32) viGetWidth() * 0.5f, viGetWidth() * 0.5f,
			-(f32) viGetHeight() * 0.5f, viGetHeight() * 0.5f,
			10, 10000, 1);

	mtxF2L(&mtx, mtx1);
	mtx00016784();

	gSPMatrix(gdl++, osVirtualToPhysical(mtx2), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gSPMatrix(gdl++, osVirtualToPhysical(mtx1), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

#if VERSION >= VERSION_NTSC_1_0
	if (var80070f10 == NULL) {
		u32 size = align16(sizeof(Vp));
		var80070f10 = gfxAllocate(size);

		if (var80070f10 != NULL) {
			var80070f10->vp.vscale[0] = viGetWidth() << 1;
			var80070f10->vp.vscale[1] = viGetHeight() << 1;
			var80070f10->vp.vscale[2] = 1;
			var80070f10->vp.vscale[3] = 0;

			var80070f10->vp.vtrans[0] = viGetWidth() << 1;
			var80070f10->vp.vtrans[1] = viGetHeight() << 1;
			var80070f10->vp.vtrans[2] = 0x1ff;
			var80070f10->vp.vtrans[3] = 0;
		}
	}

	gSPViewport(gdl++, var80070f10);
#else
	var80070f10.vp.vscale[0] = 640;
	var80070f10.vp.vscale[1] = 480;
	var80070f10.vp.vscale[2] = 640;
	var80070f10.vp.vscale[3] = 0;

	var80070f10.vp.vtrans[0] = 640;
	var80070f10.vp.vtrans[1] = 480;
	var80070f10.vp.vtrans[2] = 0x1ff;
	var80070f10.vp.vtrans[3] = 0;

	var80070f10.vp.vscale[0] = viGetWidth() << 1;
	var80070f10.vp.vscale[1] = viGetHeight() << 1;
	var80070f10.vp.vscale[2] = 1;
	var80070f10.vp.vscale[3] = 0;

	var80070f10.vp.vtrans[0] = viGetWidth() << 1;
	var80070f10.vp.vtrans[1] = viGetHeight() << 1;
	var80070f10.vp.vtrans[2] = 0x1ff;
	var80070f10.vp.vtrans[3] = 0;

	gSPViewport(gdl++, &var80070f10);
#endif

	gDPPipeSync(gdl++);

	return gdl;
}

Gfx *func0f0d49c8(Gfx *gdl)
{
	gSPViewport(gdl++, OS_K0_TO_PHYSICAL(viGetCurrentPlayerViewport()));
	gSPMatrix(gdl++, osVirtualToPhysical(camGetPerspectiveMtxL()), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *func0f0d4a3c(Gfx *gdl, s32 arg1)
{
	Mtxf mtx;
	Mtxf *mtxptr = gfxAllocateMatrix();

	gDPPipeSync(gdl++);

	if (arg1 == 0) {
		texSelect(&gdl, &g_TexGeneralConfigs[6], 2, 0, 2, 1, NULL);
	} else if (arg1 == 1) {
		texSelect(&gdl, &g_TexGeneralConfigs[11], 2, 0, 2, 1, NULL);
	}

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
	gSPSetGeometryMode(gdl++, G_SHADE);
	gSPSetGeometryMode(gdl++, G_SHADING_SMOOTH);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	func0f0d4690(&mtx);
	mtxF2L(&mtx, mtxptr);

	gSPMatrix(gdl++, osVirtualToPhysical(mtxptr), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *func0f0d4c80(Gfx *gdl)
{
	Mtxf mtx;
	Mtxf *mtxptr = gfxAllocateMatrix();

	func0f0d4690(&mtx);
	mtx00015df0(0.1f, &mtx);
	mtx00015e4c(0.1f, &mtx);
	mtxF2L(&mtx, mtxptr);

	gSPMatrix(gdl++, osVirtualToPhysical(mtxptr), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *menugfxDrawPlane(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2, u32 colour1, u32 colour2, s32 type)
{
	Col *colours;
	Vtx *vertices;
	f32 tmp1;
	s16 a1;
	s16 t1;
	f32 sp34;
	f32 sp30;
	s16 sp2e;
	s16 sp2c;
	s16 sp2a;
	s16 sp28;
	s16 a1_2;
	s16 scale = 10;
	f32 tmp2;

	static u32 depthsub = 1000;
	static u32 txmul = 20;
	static u32 rsub = 5;

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(4);

	sp34 = 1.0f;
	sp30 = 1.0f;

	mainOverrideVariable("txmul", &txmul);

	if (y1 < var8009de90 && y2 < var8009de90) {
		return gdl;
	}

	if (y1 > var8009de94 && y2 > var8009de94) {
		return gdl;
	}

	if (y1 < var8009de90) {
		y1 = var8009de90;
	}

	if (y2 < var8009de90) {
		y2 = var8009de90;
	}

	if (y1 > var8009de94) {
		y1 = var8009de94;
	}

	if (y2 > var8009de94) {
		y2 = var8009de94;
	}

	sp2e = (x1 + y1) * txmul;
	sp2c = (x2 + y2) * txmul;
	sp2a = 0;
	sp28 = 16384;

	if (type == MENUPLANE_01) {
		sp30 = 2.0f;
	}

	mainOverrideVariable("depthsub", &depthsub);

	a1 = 200;

	if (type == MENUPLANE_02 || type == MENUPLANE_03) {
		if (type == MENUPLANE_02) {
			sp2e = 0;
			sp2c = 1024;
		} else {
			sp2e = 1024;
			sp2c = 2048;
		}

		sp34 = 4.0f;
		sp30 = 4.0f;
		a1 = 6000;
	}

	if (type == MENUPLANE_08 || type == MENUPLANE_09 || type == MENUPLANE_11) {
		sp2e = 0;
		sp2c = 2048;
		a1 = 2000;
		sp34 = 4.0f;
		sp30 = 4.0f;

		if (type == MENUPLANE_09) {
			sp30 = 2.0f;
		}
	}

	if (type == MENUPLANE_04) {
		a1 = 2000;
		sp34 = 1.0f;
		sp30 = 1.0f;
	}

	if (type == MENUPLANE_05 || type == MENUPLANE_06 || type == MENUPLANE_10) {
		a1 = 1000;
		sp2e = 0;
		sp2c = 4096;
		sp30 = 4.0f;

		if (type == MENUPLANE_06) {
			sp2e = 384;
			sp2c = 4480;
			sp30 = 8.0f;
		} else if (type == MENUPLANE_10) {
			sp2e = 384;
			sp2c = 4480;
			sp30 = 8.0f;
		} else {
			sp34 = 2.0f;
		}
	}

	if (type == MENUPLANE_07) {
		mainOverrideVariable("rsub", &rsub);

		a1 = -rsub;

		sp30 = 8.0f;
		sp2a = 256;
		sp28 = 0;
	}

	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = -10;

	vertices[1].x = x2;
	vertices[1].y = y2;
	vertices[1].z = -10;

	tmp1 = (f32) g_MenuProjectFromX * a1 / scale;
	tmp2 = (f32) g_MenuProjectFromY * a1 / scale;

	vertices[2].x = vertices[0].v[0] + (s16) tmp1;
	vertices[2].y = vertices[0].v[1] + (s16) tmp2;
	vertices[2].z = -10 - a1;

	vertices[3].x = vertices[1].v[0] + (s16) tmp1;
	vertices[3].y = vertices[1].v[1] + (s16) tmp2;
	vertices[3].z = -10 - a1;

	if (type == MENUPLANE_10) {
		t1 = g_20SecIntervalFrac * sp34 * 64.0f * 32.0f;
	} else {
		t1 = (g_20SecIntervalFrac - 0.5f) * sp34 * 64.0f * 32.0f;
	}

	if (type == MENUPLANE_10) {
		a1_2 = (g_20SecIntervalFrac - 0.5f) * sp30 * 64.0f * 32.0f;
	} else {
		a1_2 = g_20SecIntervalFrac * sp30 * 64.0f * 32.0f;
	}

	vertices[0].s = sp2e + t1;
	vertices[0].t = sp2a + a1_2;
	vertices[1].s = sp2c + t1;
	vertices[1].t = sp2a + a1_2;
	vertices[3].s = sp2c + t1;
	vertices[3].t = sp28 + a1_2;
	vertices[2].s = sp2e + t1;
	vertices[2].t = sp28 + a1_2;

	if (type == MENUPLANE_07) {
		vertices[0].colour = 0;
		vertices[1].colour = 0;
		vertices[2].colour = 4;
		vertices[3].colour = 4;
	} else {
		vertices[0].colour = 0;
		vertices[1].colour = 4;
		vertices[2].colour = 0;
		vertices[3].colour = 4;
	}

	colours[0].word = PD_BE32(colour1);
	colours[1].word = PD_BE32(colour2);

	gSPColor(gdl++, osVirtualToPhysical(colours), 2);
	gSPVertex(gdl++, osVirtualToPhysical(vertices), 4, 0);
	gSPTri2(gdl++, 0, 1, 3, 3, 2, 0);

	return gdl;
}

/**
 * Write the specified amount of bits to the buffer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 *
 * This function only sets bits to on and does not unset them.
 */
void savebufferOr(struct savebuffer *buffer, u32 value, s32 numbits)
{
	u32 bit = 1 << (numbits - 1);

	for (; bit; bit >>= 1) {
		if (bit & value) {
			s32 bitindex = buffer->bitpos % 8;
			u8 mask = 1 << (7 - bitindex);
			s32 byteindex = buffer->bitpos / 8;

			buffer->bytes[byteindex] |= mask;
		}

		buffer->bitpos++;
	}
}

#if VERSION >= VERSION_NTSC_1_0
/**
 * Write the specified amount of bits to the buffer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 */
void savebufferWriteBits(struct savebuffer *buffer, u32 value, s32 numbits, u8 *dst)
{
	u32 bit = 1 << (numbits - 1);

	for (; bit; bit >>= 1) {
		s32 bitindex = buffer->bitpos % 8;
		u8 mask = 1 << (7 - bitindex);
		s32 byteindex = buffer->bitpos / 8;

		if (bit & value) {
			dst[byteindex] |= mask;
		} else {
			dst[byteindex] &= ~mask;
		}

		buffer->bitpos++;
	}
}
#endif

/**
 * Read the specified amount of bits from the buffer and return it as an
 * integer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 */
u32 savebufferReadBits(struct savebuffer *buffer, s32 numbits)
{
	u32 bit = 1 << (numbits - 1);
	u32 value = 0;

	for (; bit; bit >>= 1) {
		s32 bitindex = buffer->bitpos % 8;
		u8 mask = 1 << (7 - bitindex);
		s32 byteindex = buffer->bitpos / 8;

		if (buffer->bytes[byteindex] & mask) {
			value |= bit;
		}

		buffer->bitpos++;
	}

	return value;
}

void savebufferClear(struct savebuffer *buffer)
{
	s32 i;

	buffer->bitpos = 0;

	for (i = 0; i < sizeof(buffer->bytes);) {
		buffer->bytes[i] = 0;
		i++;
	}
}

void savebufferWriteData(struct savebuffer *buffer, u8 *data, u8 len)
{
	s32 i;

	buffer->bitpos = 0;

	for (i = 0; i < len; i++) {
		buffer->bytes[i] = data[i];
	}
}

void func0f0d54c4(struct savebuffer *buffer)
{
	s32 tmp = buffer->bitpos;

	if (tmp / 8 && buffer->bitpos);
}

/**
 * Read a zero-terminated string from the buffer and move the buffer's internal
 * pointer past the end of the string.
 */
void savebufferReadString(struct savebuffer *buffer, char *dst, bool addlinebreak)
{
	bool foundnull = false;
	s32 index = 0;
	s32 i;

	for (i = 0; i < 10; i++) {
		s32 byte = savebufferReadBits(buffer, 8);

		if (!foundnull) {
			if (byte == '\0') {
				foundnull = true;
			} else {
				dst[i] = byte;
				index = i;
			}
		}
	}

	if (addlinebreak) {
		index++;
		dst[index] = '\n';
	}

	index++;
	dst[index] = '\0';
}

void func0f0d55a4(struct savebuffer *buffer, char *src)
{
	bool done = false;
	s32 i;

	for (i = 0; i < 10; i++) {
		if (!done) {
			if (src[i] == '\0') {
				done = true;
			} else if (src[i] == '\n') {
				done = true;
			} else {
				u32 c = src[i];
				savebufferOr(buffer, c, 8);
			}
		}

		if (done) {
			savebufferOr(buffer, '\0', 8);
		}
	}
}

void func0f0d564c(u8 *data, char *dst, bool addlinebreak)
{
	struct savebuffer buffer;

	savebufferWriteData(&buffer, data, 10);
	savebufferReadString(&buffer, dst, addlinebreak);
}

#if VERSION >= VERSION_NTSC_1_0
void func0f0d5690(u8 *dst, char *src)
{
	struct savebuffer buffer;
	bool done = false;
	s32 i;

	savebufferWriteData(&buffer, dst, 10);

	for (i = 0; i < 10; i++) {
		if (!done) {
			if (src[i] == '\0') {
				done = true;
			} else if (src[i] == '\n') {
				done = true;
			} else {
				u32 c = src[i];
				savebufferWriteBits(&buffer, c, 8, dst);
			}
		}

		if (done) {
			savebufferWriteBits(&buffer, '\0', 8, dst);
		}
	}
}
#endif

void savebufferWriteGuid(struct savebuffer *buffer, struct fileguid *guid)
{
	savebufferOr(buffer, guid->fileid, 7);
	savebufferOr(buffer, guid->deviceserial, 13);
}

void savebufferReadGuid(struct savebuffer *buffer, struct fileguid *guid)
{
	guid->fileid = savebufferReadBits(buffer, 7);
	guid->deviceserial = savebufferReadBits(buffer, 13);
}

void formatTime(char *dst, s32 time60, s32 precision)
{
	s32 parts[5];
	bool donefirst = false;
	s32 len = 0;
	s32 i;

	parts[4] = time60 % 60 * 100 / 60; // hundredths
	parts[3] = time60 / 60; // seconds
	parts[2] = parts[3] / 60; // minutes
	parts[1] = parts[2] / 60; // hours
	parts[0] = parts[1] / 24; // days

	parts[3] %= 60; // seconds
	parts[2] %= 60; // minutes
	parts[1] %= 24; // hours

	for (i = 0; i <= precision; i++) {
		if (donefirst) {
			len += sprintf(&dst[len], ":%02d", parts[i]);
		} else if (parts[i] != 0 || i >= TIMEPRECISION_MINUTES) {
			len += sprintf(&dst[len], "%d", parts[i]);
			donefirst = true;
		}
	}
}

#if VERSION >= VERSION_NTSC_1_0
void func0f0d5a7c(void)
{
	var80070f10 = 0;
}
#endif
