#include <ultra64.h>
#include "constants.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "lib/crash.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/rzip.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "mod.h"
#include "platform.h"
#endif

struct texture *g_Textures;
u32 var800aabc4;
struct texpool g_TexSharedPool;
struct texcacheitem g_TexCacheItems[150];
s32 g_TexCacheCount;
s32 g_TexNumToLoad;
u8 *g_TexBitstring;
u32 g_TexAccumValue;
s32 g_TexAccumNumBits;
u32 var800ab54c;
u32 g_TexBase;
u8 *g_TextureConfigSegment;
s32 g_TexNumConfigs;
struct tex **g_TexWords;
struct textureconfig *g_TexWallhitConfigs;
Gfx *g_TexGdl1;
Gfx *g_TexGdl2;
Gfx *g_TexGdl3;
struct texturepair *g_ExplosionTexturePairs;
struct textureconfig *g_TexBeamConfigs;
struct textureconfig *g_TexLaserConfigs;
struct textureconfig *g_TexGroup03Configs;
struct textureconfig *g_TexGeCrosshairConfigs;
struct textureconfig *g_TexRedLinesConfigs;
struct textureconfig *g_TexShadowConfigs;
struct textureconfig *g_TexShieldConfigs;
struct textureconfig *g_TexShardConfigs;
struct textureconfig *g_TexScreenConfigs;
struct textureconfig *g_TexSkyWaterConfigs;
struct textureconfig *g_TexGroup11Configs;
struct textureconfig *g_TexLightGlareConfigs;
struct textureconfig *g_TexSparkConfigs;
struct textureconfig *g_TexGeneralConfigs;
struct textureconfig *g_TexRadarConfigs;

u32 var800841b0 = 0x0006ddd0;
u32 var800841b4 = 0x00000000;

// The number of channels, excluding 1-bit alpha channels.
s32 g_TexFormatNumChannels[] = { 4, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1 };

// Whether each format supports a 1-bit alpha channel.
s32 g_TexFormatHas1BitAlpha[] = { 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 };

// For non-paletted images, size in decimal of each colour channel.
// Eg. 32 means each channel can store up to 32 values (5-bits per channel).
// For paletted images, same thing but for the palette indices instead.
s32 g_TexFormatChannelSizes[] = { 256, 32, 256, 32, 256, 16, 8, 256, 16, 256, 16, 256, 16 };

s32 g_TexFormatBitsPerPixel[] = { 32, 16, 24, 15, 16, 8, 4, 8, 4, 16, 16, 16, 16 };

// Mapping to GBI format
s32 g_TexFormatGbiMappings[] = {
	G_IM_FMT_RGBA,
	G_IM_FMT_RGBA,
	G_IM_FMT_RGBA,
	G_IM_FMT_RGBA,
	G_IM_FMT_IA,
	G_IM_FMT_IA,
	G_IM_FMT_IA,
	G_IM_FMT_I,
	G_IM_FMT_I,
	G_IM_FMT_CI,
	G_IM_FMT_CI,
	G_IM_FMT_CI,
	G_IM_FMT_CI,
};

s32 g_TexFormatDepths[] = {
	G_IM_SIZ_32b,
	G_IM_SIZ_16b,
	G_IM_SIZ_32b,
	G_IM_SIZ_16b,
	G_IM_SIZ_16b,
	G_IM_SIZ_8b,
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_4b,
};

s32 g_TexFormatLutModes[] = {
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_NONE,
	G_TT_RGBA16,
	G_TT_RGBA16,
	G_TT_IA16,
	G_TT_IA16,
};

void func0f16e810(u32 arg0)
{
	// empty
}

/**
 * Inflate images (levels of detail) from a zlib-compressed texture.
 *
 * Zlib-compressed textures are always paletted and always use 16-bit colours.
 * The texture header contains palette information, then each image follows with
 * its own header and zlib compresed data.
 *
 * The texture header is:
 *
 * ffffffff nnnnnnnn [palette]
 *
 * f = pixel format (see TEXFORMAT constants)
 * n = number of colours in the palette minus 1
 * [palette] = 16 bits * number of colours
 *
 * Each images's header is:
 *
 * wwwwwwww hhhhhhhh [data]
 *
 * w = width in pixels
 * h = height in pixels
 * [data] = zlib compressed list of indices into the palette
 *
 * The zlib data is prefixed with the standard 5-byte rarezip header.
 */
s32 texInflateZlib(u8 *src, u8 *dst, bool hasloddata, s32 numlods, struct texpool *pool, bool unusedarg)
{
	s32 i;
	s32 imagebytesout;
	s32 numimages;
	bool writetocache;
	s32 format;
	bool foundthething;
	s32 totalbytesout;
	s32 width;
	s32 height;
	s32 numcolours;
	u8 *loddst;
	u8 *lodsrc;
	s32 lod;
	u8 scratch2[0x800];
	u16 palette[256];
	u8 scratch[5120];

	writetocache = false;
	totalbytesout = 0;

	texSetBitstring(src);

	if (hasloddata && numlods) {
		numimages = numlods;
	} else {
		numimages = 1;
	}

	pool->rightpos->numlods = numlods;
	pool->rightpos->hasloddata = hasloddata;

	if (hasloddata) {
		writetocache = true;

		for (i = 0; i < g_TexCacheCount; i++) {
			if (g_TexCacheItems[i].texturenum == pool->rightpos->texturenum) {
				writetocache = false;
			}
		}
	}

	format = texReadBits(8);
	numcolours = texReadBits(8) + 1;

	for (i = 0; i < numcolours; i++) {
		palette[i] = texReadBits(16);
	}

	foundthething = false;

	for (lod = 0; lod < numimages; lod++) {
		width = texReadBits(8);
		height = texReadBits(8);

		if (lod == 0) {
			pool->rightpos->width = width;
			pool->rightpos->height = height;
			pool->rightpos->unk0a = numcolours - 1;
			pool->rightpos->gbiformat = g_TexFormatGbiMappings[format];
			pool->rightpos->depth = g_TexFormatDepths[format];
			pool->rightpos->lutmodeindex = g_TexFormatLutModes[format] >> G_MDSFT_TEXTLUT;
		} else if (writetocache) {
			g_TexCacheItems[g_TexCacheCount].widths[lod - 1] = width;
			g_TexCacheItems[g_TexCacheCount].heights[lod - 1] = height;
		}

		if (rzipInflate(g_TexBitstring, scratch2, scratch) == 0) {
#if VERSION < VERSION_NTSC_1_0
			char message[128];
			sprintf(message, "DMA-Crash %s %d Ram: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					"texdecompress.c", 357,
					g_TexBitstring[0], g_TexBitstring[1], g_TexBitstring[2], g_TexBitstring[3],
					g_TexBitstring[4], g_TexBitstring[5], g_TexBitstring[6], g_TexBitstring[7],
					g_TexBitstring[8], g_TexBitstring[9], g_TexBitstring[10], g_TexBitstring[11],
					g_TexBitstring[12], g_TexBitstring[13], g_TexBitstring[14], g_TexBitstring[15]);
			crashSetMessage(message);
			CRASH();
#endif
		}

		imagebytesout = texAlignIndices(scratch2, width, height, format, &dst[totalbytesout]);
		texSetBitstring(rzipGetSomething());

		if (hasloddata == true) {
			if (IS4MB() && lod == 2 && !foundthething) {
				pool->rightpos->numlods = lod;
				foundthething = true;
			}

			if (totalbytesout + imagebytesout > 0x800 || foundthething) {
				if (!foundthething) {
					pool->rightpos->numlods = lod;
					foundthething = true;
				}
			} else {
				texSwizzle(&dst[totalbytesout], width, height, format);
				totalbytesout += imagebytesout;
			}
		} else {
			totalbytesout += imagebytesout;
		}
	}

	if (writetocache) {
		g_TexCacheItems[g_TexCacheCount].texturenum = pool->rightpos->texturenum;

		g_TexCacheCount++;

		if (g_TexCacheCount >= ARRAYCOUNT(g_TexCacheItems)) {
			g_TexCacheCount = 0;
		}
	}

	// If the texture data doesn't contain multiple LODs but the header has a numlods value,
	// generate the other LODs by shrinking the image.
	if (!hasloddata) {
		if (numlods >= 2) {
			s32 tmpwidth = width;
			s32 tmpheight = height;

			lodsrc = dst;
			loddst = &dst[totalbytesout];

			for (lod = 1; lod < numlods; lod++) {
				imagebytesout = texShrinkPaletted(lodsrc, loddst, tmpwidth, tmpheight, format, palette, numcolours);

				if (IS4MB() && lod == 2) {
					pool->rightpos->numlods = lod;
					break;
				}

				if (totalbytesout + imagebytesout > 0x800) {
					pool->rightpos->numlods = lod;
					break;
				}

				texSwizzle(lodsrc, tmpwidth, tmpheight, format);

				totalbytesout += imagebytesout;

				tmpwidth = (tmpwidth + 1) >> 1;
				tmpheight = (tmpheight + 1) >> 1;

				lodsrc = loddst;
				loddst += imagebytesout;
			}

			texSwizzle(lodsrc, tmpwidth, tmpheight, format);
		} else {
			texSwizzle(dst, width, height, format);
		}
	}

	for (i = 0; i < numcolours; i++) {
		dst[totalbytesout + 0] = palette[i] >> 8;
		dst[totalbytesout + 1] = palette[i] & 0xff;
		totalbytesout += 2;
	}

	if (numcolours & 1) {
#if VERSION < VERSION_NTSC_1_0
		u32 stack;
#endif

		dst[totalbytesout + 0] = dst[totalbytesout - 2];
		dst[totalbytesout + 1] = dst[totalbytesout - 1];
		totalbytesout += 2;
		pool->rightpos->unk0a++;
	}

	totalbytesout = (totalbytesout + 7) & ~7;

	return totalbytesout;
}

/**
 * Copy a list of palette indices to the dst buffer, but ensure each row is
 * aligned to an 8 byte boundary.
 *
 * Return the number of output bytes.
 */
s32 texAlignIndices(u8 *src, s32 width, s32 height, s32 format, u8 *dst)
{
	u8 *outptr = dst;
	s32 x;
	s32 y;
	s32 indicesperbyte;

	if (format == TEXFORMAT_RGBA16_CI8 || format == TEXFORMAT_IA16_CI8) {
		indicesperbyte = 1;
	} else if (format == TEXFORMAT_RGBA16_CI4 || format == TEXFORMAT_IA16_CI4) {
		indicesperbyte = 2;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x += indicesperbyte) {
			*outptr = *src;
			outptr++;
			src++;
		}

		outptr = (u8 *)(((uintptr_t)outptr + 7) & ~7);
	}

	return outptr - dst;
}

s32 texGetAverageRed(u16 colour1, u16 colour2, u16 colour3, u16 colour4)
{
	s32 value = 0;

	value += (((colour1 >> 11) & 0x1f) << 3) | ((colour1 >> 13) & 7);
	value += (((colour2 >> 11) & 0x1f) << 3) | ((colour2 >> 13) & 7);
	value += (((colour3 >> 11) & 0x1f) << 3) | ((colour3 >> 13) & 7);
	value += (((colour4 >> 11) & 0x1f) << 3) | ((colour4 >> 13) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

s32 texGetAverageGreen(u16 colour1, u16 colour2, u16 colour3, u16 colour4)
{
	s32 value = 0;

	value += (((colour1 >> 6) & 0x1f) << 3) | ((colour1 >> 8) & 7);
	value += (((colour2 >> 6) & 0x1f) << 3) | ((colour2 >> 8) & 7);
	value += (((colour3 >> 6) & 0x1f) << 3) | ((colour3 >> 8) & 7);
	value += (((colour4 >> 6) & 0x1f) << 3) | ((colour4 >> 8) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

s32 texGetAverageBlue(u16 colour1, u16 colour2, u16 colour3, u16 colour4)
{
	s32 value = 0;

	value += (((colour1 >> 1) & 0x1f) << 3) | ((colour1 >> 3) & 7);
	value += (((colour2 >> 1) & 0x1f) << 3) | ((colour2 >> 3) & 7);
	value += (((colour3 >> 1) & 0x1f) << 3) | ((colour3 >> 3) & 7);
	value += (((colour4 >> 1) & 0x1f) << 3) | ((colour4 >> 3) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

s32 texGetAverageAlpha(u16 colour1, u16 colour2, u16 colour3, u16 colour4)
{
	s32 value = 0
		+ (colour1 & 1 ? 0xff : 0)
		+ (colour2 & 1 ? 0xff : 0)
		+ (colour3 & 1 ? 0xff : 0)
		+ (colour4 & 1 ? 0xff : 0);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

/**
 * Shrink a paletted texture to half its size by averaging each each 2x2 group
 * of pixels.
 *
 * Return the number of bytes written.
 */
s32 texShrinkPaletted(u8 *src, u8 *dst, s32 srcwidth, s32 srcheight, s32 format, u16 *palette, s32 numcolours)
{
	s32 j;
	s32 i;
	s32 alignedsrcwidth;
	s32 aligneddstwidth;
	s32 dstheight = (srcheight + 1) >> 1;
	u16 colour1;
	u16 colour2;
	u16 colour3;
	u16 colour4;
	s32 r;
	s32 g;
	s32 b;
	s32 a;
	s32 nextrow;
	s32 nextcol;
	s32 c;
	u8 *dst8;
	u8 *src8;
	u8 palette32[1024];

	switch (format) {
	case TEXFORMAT_RGBA16_CI8:
	case TEXFORMAT_IA16_CI8:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 7) & 0xff8;
		alignedsrcwidth = (srcwidth + 7) & 0xff8;
		break;
	case TEXFORMAT_RGBA16_CI4:
	case TEXFORMAT_IA16_CI4:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 15) & 0xff0;
		alignedsrcwidth = (srcwidth + 15) & 0xff0;
		break;
	}

	if (format == TEXFORMAT_RGBA16_CI8 || format == TEXFORMAT_RGBA16_CI4) {
		for (i = 0; i < numcolours; i++) {
			colour1 = palette[i];

			palette32[i * 4 + 0] = ((((colour1 >> 11) & 0x1f) * 8) | ((colour1 >> 13) & 7));
			palette32[i * 4 + 1] = ((((colour1 >> 6) & 0x1f) * 8) | ((colour1 >> 8) & 7));
			palette32[i * 4 + 2] = ((((colour1 >> 1) & 0x1f) * 8) | ((colour1 >> 3) & 7));
			palette32[i * 4 + 3] = ((colour1 & 1) ? 0xff : 0);
		}
	}

	dst8 = dst;
	src8 = src;

	switch (format) {
	case TEXFORMAT_RGBA16_CI8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				colour1 = palette[src8[j]];
				colour2 = palette[src8[nextcol]];
				colour3 = palette[src8[nextrow + j]];
				colour4 = palette[src8[nextrow + nextcol]];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 1] = texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a);
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_IA16_CI8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				colour1 = palette[src8[j]];
				colour2 = palette[src8[nextcol]];
				colour3 = palette[src8[nextrow + j]];
				colour4 = palette[src8[nextrow + nextcol]];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 1] = texFindClosestColourIndexIA(palette, numcolours, c, a);
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_RGBA16_CI4:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth >> 1 : 0;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				colour1 = palette[(src8[j >> 1] >> 4) & 0xf];
				colour2 = palette[src8[j >> 1] >> ((j + 1 < srcwidth ? 0 : 4)) & 0xf];
				colour3 = palette[(src8[nextrow + (j >> 1)] >> 4) & 0xf];
				colour4 = palette[src8[nextrow + (j >> 1)] >> ((j + 1 < srcwidth ? 0 : 4)) & 0xf];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 2] = texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a) << 4;

				colour1 = palette[(src8[(j + 2) >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[(j + 2) >> 1] >> (j + 3 < srcwidth ? 0 : 4)) & 0xf];
				colour3 = palette[(src8[nextrow + ((j + 2) >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + ((j + 2) >> 1)] >> (j + 3 < srcwidth ? 0 : 4)) & 0xf];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 2] |= texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a) & 0xff;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	case TEXFORMAT_IA16_CI4:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth >> 1 : 0;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				// @bug: The brackets are wrong in colour2 and colour4 which
				// causes the index shift to be part of the ternary condition.
				// It's done correctly in TEXFORMAT_RGBA16_CI4 (above).
				// This buggy calculation is repeated further below.
				colour1 = palette[(src8[j >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[j >> 1] >> (j + 1 < srcwidth) ? 0 : 4) & 0xf];
				colour3 = palette[(src8[nextrow + (j >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + (j >> 1)] >> (j + 1 < srcwidth) ? 0 : 4) & 0xf];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 2] = texFindClosestColourIndexIA(palette, numcolours, c, a) << 4;

				colour1 = palette[(src8[(j + 2) >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[(j + 2) >> 1] >> (j + 3 < srcwidth) ? 0 : 4) & 0xf];
				colour3 = palette[(src8[nextrow + ((j + 2) >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + ((j + 2) >> 1)] >> (j + 3 < srcwidth) ? 0 : 4) & 0xf];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 2] |= texFindClosestColourIndexIA(palette, numcolours, c, a) & 0xff;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	}

	return 0;
}

s32 texFindClosestColourIndexRGBA(u8 *palette, s32 numcolours, s32 r, s32 g, s32 b, s32 a)
{
	s32 minindex = 0;
	s32 minvalue = 99999999;
	s32 curvalue;
	s32 tmp;
	s32 i;

	for (i = 0; i < numcolours; i++) {
		tmp = palette[i * 4 + 0] - r;
		curvalue = tmp * tmp;

		tmp = palette[i * 4 + 1] - g;
		curvalue += tmp * tmp;

		tmp = palette[i * 4 + 2] - b;
		curvalue += tmp * tmp;

		tmp = palette[i * 4 + 3] - a;
		curvalue += tmp * tmp;

		if (curvalue < minvalue) {
			minindex = i;
			minvalue = curvalue;
		}
	}

	return minindex;
}

s32 texFindClosestColourIndexIA(u16 *palette, s32 numcolours, s32 intensity, s32 alpha)
{
	s32 bestindex = 0;
	s32 bestvalue = 99999999;
	s32 i;

	for (i = 0; i < numcolours; i++) {
		s32 value = palette[i];
		s32 a = ((value >> 8) & 0xff) - intensity;
		s32 b = (value & 0xff) - alpha;
		s32 sum = a * a + b * b;

		if (sum < bestvalue) {
			bestindex = i;
			bestvalue = sum;
		}
	}

	return bestindex;
}

/**
 * Inflate images (levels of detail) from a non-zlib texture.
 *
 * Each image can have a different compression method and pixel format,
 * which is described in a three byte header per image:
 *
 * ffffwwww wwwwhhhh hhhhcccc
 *
 * f = pixel format (see TEXFORMAT constants)
 * w = width in pixels
 * h = height in pixels
 * c = compression method (see TEXCOMPMETHOD constants)
 */
s32 texInflateNonZlib(u8 *src, u8 *dst, bool hasloddata, s32 numlods, struct texpool *pool, bool unusedarg)
{
	u8 scratch[0x2000];
	u8 lookup[0x1000];
	u32 stack;
	s32 i;
	s32 numimages;
	s32 width;
	s32 height;
	s32 compmethod;
	s32 j;
	s32 totalbytesout = 0;
	s32 imagebytesout;
	s32 format;
	s32 value;
	u8 *lodsrc;
	u8 *loddst;
	bool writetocache = false;

	texSetBitstring(src);

	numimages = hasloddata && numlods ? numlods : 1;

	pool->rightpos->numlods = numlods;
	pool->rightpos->hasloddata = hasloddata;

	if (hasloddata) {
		writetocache = true;

		for (i = 0; i < g_TexCacheCount; i++) {
			if (g_TexCacheItems[i].texturenum == pool->rightpos->texturenum) {
				writetocache = false;
			}
		}
	}

	for (i = 0; i < numimages; i++) {
		format = texReadBits(4);
		width = texReadBits(8);
		height = texReadBits(8);
		compmethod = texReadBits(4);

		if (i == 0) {
			pool->rightpos->width = width;
			pool->rightpos->height = height;
			pool->rightpos->gbiformat = g_TexFormatGbiMappings[format];
			pool->rightpos->depth = g_TexFormatDepths[format];
			pool->rightpos->lutmodeindex = g_TexFormatLutModes[format] >> G_MDSFT_TEXTLUT;
		} else if (writetocache) {
			g_TexCacheItems[g_TexCacheCount].widths[i - 1] = width;
			g_TexCacheItems[g_TexCacheCount].heights[i - 1] = height;
		}

		if (width * height > 0x2000) {
			return 0;
		}

		switch (compmethod) {
		case TEXCOMPMETHOD_UNCOMPRESSED0:
		case TEXCOMPMETHOD_UNCOMPRESSED1:
			imagebytesout = texReadUncompressed(&dst[totalbytesout], width, height, format);
			break;
		case TEXCOMPMETHOD_HUFFMAN:
			texInflateHuffman(scratch, g_TexFormatNumChannels[format] * width * height, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_HUFFMANPERHCHANNEL:
			for (j = 0; j < g_TexFormatNumChannels[format]; j++) {
				texInflateHuffman(&scratch[width * height * j], width * height, g_TexFormatChannelSizes[format]);
			}

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_RLE:
			texInflateRle(scratch, g_TexFormatNumChannels[format] * width * height);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_LOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			imagebytesout = texInflateLookup(width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_HUFFMANLOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			texInflateHuffman(scratch, width * height, value);
			imagebytesout = texInflateLookupFromBuffer(scratch, width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_RLELOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			texInflateRle(scratch, width * height);
			imagebytesout = texInflateLookupFromBuffer(scratch, width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_HUFFMANBLUR:
			value = texReadBits(3);
			texInflateHuffman(scratch, g_TexFormatNumChannels[format] * width * height, g_TexFormatChannelSizes[format]);
			texBlur(scratch, width, g_TexFormatNumChannels[format] * height, value, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_RLEBLUR:
			value = texReadBits(3);
			texInflateRle(scratch, g_TexFormatNumChannels[format] * width * height);
			texBlur(scratch, width, g_TexFormatNumChannels[format] * height, value, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		}

		if (hasloddata == true) {
			texSwizzle(&dst[totalbytesout], width, height, format);
		}

		imagebytesout = (imagebytesout + 7) & ~7;
		totalbytesout += imagebytesout;

		if (g_TexAccumNumBits == 0) {
			g_TexBitstring++;
		} else {
			g_TexAccumNumBits = 0;
		}
	}

	if (writetocache) {
		g_TexCacheItems[g_TexCacheCount].texturenum = pool->rightpos->texturenum;

		g_TexCacheCount++;

		// Resetting this variable to 0 here suggests that the g_TexCacheItems
		// array is used in a circular manner, and that g_TexCacheCount is just
		// the index of the oldest/next element. But earlier in this function
		// there's a loop that iterates up to g_TexCacheCount, which doesn't
		// make any sense if this value is used as a pointer in a circular list.
		// Could be a @bug, or maybe they intended to reset the cache every time
		// it fills up.
		if (g_TexCacheCount >= ARRAYCOUNT(g_TexCacheItems)) {
			g_TexCacheCount = 0;
		}
	}

	// If the texture data doesn't contain multiple LODs but the header has a numlods value,
	// generate the other LODs by shrinking the image.
	if (!hasloddata) {
		if (numlods >= 2) {
			s32 tmpwidth = width;
			s32 tmpheight = height;

			lodsrc = dst;
			if (1);
			loddst = &dst[totalbytesout];

			for (i = 1; i < numlods; i++) {
				imagebytesout = texShrinkNonPaletted(lodsrc, loddst, tmpwidth, tmpheight, format);

				texSwizzle(lodsrc, tmpwidth, tmpheight, format);

				totalbytesout += imagebytesout;

				tmpwidth = (tmpwidth + 1) >> 1;
				tmpheight = (tmpheight + 1) >> 1;

				lodsrc = loddst;
				loddst += imagebytesout;
			}

			texSwizzle(lodsrc, tmpwidth, tmpheight, format);
		} else {
			texSwizzle(dst, width, height, format);
		}
	}

	return totalbytesout;
}

/**
 * Shrink a non-paletted texture to half its size by averaging each each 2x2
 * group of pixels.
 *
 * Return the number of bytes written.
 *
 * If the source width is an odd number, the destination's final column is
 * calculated by sampling the final source column twice. Likewise for the height.
 */
s32 texShrinkNonPaletted(u8 *src, u8 *dst, s32 srcwidth, s32 srcheight, s32 format)
{
	s32 i;
	s32 j;
	s32 alignedsrcwidth;
	s32 aligneddstwidth;
	u32 *dst32 = (u32 *) dst;
	u16 *dst16 = (u16 *) dst;
	u8 *dst8 = dst;
	u32 *src32 = (u32 *) src;
	u16 *src16 = (u16 *) src;
	u8 *src8 = src;
	s32 dstheight = (srcheight + 1) >> 1;
	s32 r;
	s32 g;
	s32 b;
	s32 a;
	s32 c;
	u32 tl32;
	u32 tr32;
	u32 bl32;
	u32 br32;
	u16 tl16;
	u16 tr16;
	u16 bl16;
	u16 br16;
	u8 tl8;
	u8 tr8;
	u8 bl8;
	u8 br8;
	s32 nextrow;
	s32 nextcol;

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 3) & 0xffc;
		alignedsrcwidth = (srcwidth + 3) & 0xffc;
		break;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
	case TEXFORMAT_IA16:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 3) & 0xffc;
		alignedsrcwidth = (srcwidth + 3) & 0xffc;
		break;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 7) & 0xff8;
		alignedsrcwidth = (srcwidth + 7) & 0xff8;
		break;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 15) & 0xff0;
		alignedsrcwidth = (srcwidth + 15) & 0xff0;
		break;
	}

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl32 = src32[j];
				tr32 = src32[nextcol];
				bl32 = src32[nextrow + j];
				br32 = src32[nextrow + nextcol];

				r = ((((tl32 >> 24) & 0xff) + ((tr32 >> 24) & 0xff) + ((bl32 >> 24) & 0xff) + ((br32 >> 24) & 0xff)) >> 2) & 0xff;
				g = ((((tl32 >> 16) & 0xff) + ((tr32 >> 16) & 0xff) + ((bl32 >> 16) & 0xff) + ((br32 >> 16) & 0xff)) >> 2) & 0xff;
				b = ((((tl32 >>  8) & 0xff) + ((tr32 >>  8) & 0xff) + ((bl32 >>  8) & 0xff) + ((br32 >>  8) & 0xff)) >> 2) & 0xff;
				a = ((((tl32 >>  0) & 0xff) + ((tr32 >>  0) & 0xff) + ((bl32 >>  0) & 0xff) + ((br32 >>  0) & 0xff) + 1) >> 2) & 0xff;

				dst32[j >> 1] = r << 24 | g << 16 | b << 8 | a;
			}

			dst32 += aligneddstwidth;
			src32 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl16 = src16[j];
				tr16 = src16[nextcol];
				bl16 = src16[nextrow + j];
				br16 = src16[nextrow + nextcol];

				r = ((((tl16 >> 11) & 0x1f) + ((tr16 >> 11) & 0x1f) + ((bl16 >> 11) & 0x1f) + ((br16 >> 11) & 0x1f)) >> 2) & 0x1f;
				g = ((((tl16 >>  6) & 0x1f) + ((tr16 >>  6) & 0x1f) + ((bl16 >>  6) & 0x1f) + ((br16 >>  6) & 0x1f)) >> 2) & 0x1f;
				b = ((((tl16 >>  1) & 0x1f) + ((tr16 >>  1) & 0x1f) + ((bl16 >>  1) & 0x1f) + ((br16 >>  1) & 0x1f)) >> 2) & 0x1f;
				a = ((((tl16 >>  0) & 0x01) + ((tr16 >>  0) & 0x01) + ((bl16 >>  0) & 0x01) + ((br16 >>  0) & 0x01) + 2) >> 2) & 0x01;

				dst16[j >> 1] = r << 11 | g << 6 | b << 1 | a;
			}

			dst16 += aligneddstwidth;
			src16 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 2;
	case TEXFORMAT_IA16:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl16 = src16[j];
				tr16 = src16[nextcol];
				bl16 = src16[nextrow + j];
				br16 = src16[nextrow + nextcol];

				c = ((tl16 >> 8) & 0xff) + ((tr16 >> 8) & 0xff) + ((bl16 >> 8) & 0xff) + ((br16 >> 8) & 0xff) + 2;
				c = c >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xff) {
					c = 0xff;
				}

				a = (tl16 & 0xff) + (tr16 & 0xff) + (bl16 & 0xff) + (br16 & 0xff) + 2; // optimised out

				// @bug: Should be a >> 2
				a = c >> 2;

				if (a < 0) {
					a = 0;
				}

				if (a > 0xff) {
					a = 0xff;
				}

				dst16[j >> 1] = c << 8 | a;
			}

			dst16 += aligneddstwidth;
			src16 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 2;
	case TEXFORMAT_IA8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl8 = src8[j];
				tr8 = src8[nextcol];
				bl8 = src8[nextrow + j];
				br8 = src8[nextrow + nextcol];

				c = (((tl8 >> 4) & 0xf) + ((tr8 >> 4) & 0xf) + ((bl8 >> 4) & 0xf) + ((br8 >> 4) & 0xf) + 2) >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xf) {
					c = 0xf;
				}

				c <<= 4;

				a = ((tl8 & 0xf) + (tr8 & 0xf) + (bl8 & 0xf) + (br8 & 0xf) + 2) >> 2;

				if (a < 0) {
					a = 0;
				}

				if (a > 0xf) {
					a = 0xf;
				}

				dst8[j >> 1] = c | a;
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_I8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				// @bug: The code reads from uninitialised variable br8.
				// This is assumed to be due to a double write to bl8 as shown:
				tl8 = src8[j];
				tr8 = src8[nextcol];
				bl8 = src8[nextrow + j]; // optimised out
				bl8 = src8[nextrow + nextcol];

				c = (tl8 + tr8 + bl8 + br8 + 2) >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xff) {
					c = 0xff;
				}

				dst8[j >> 1] = c;
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_IA4:
		for (i = 0; i < srcheight; i += 2) {
			nextcol = i + 1;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				tl8 = src8[j >> 1];
				tr8 = src8[(nextcol < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1)];
				bl8 = src8[(j >> 1) + 1];
				br8 = src8[(nextcol < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1) + 1];

				c = (((((tl8 >> 5) & 7) + ((tl8 >> 1) & 7) + ((tr8 >> 5) & 7) + ((tr8 >> 1) & 7)) << 3) & 0xe0)
					| (((((bl8 >> 5) & 7) + ((bl8 >> 1) & 7) + ((br8 >> 5) & 7) + ((br8 >> 1) & 7)) >> 1) & 0xe);

				a = (((((tl8 >> 4) & 1) + (tl8 & 1) + ((tr8 >> 4) & 1) + (tr8 & 1) + 1) << 2) & 0x10)
					| (((((bl8 >> 4) & 1) + (bl8 & 1) + ((br8 >> 4) & 1) + (br8 & 1) + 1) >> 2) & 1);

				dst8[j >> 2] = c | a;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	case TEXFORMAT_I4:
		for (i = 0; i < srcheight; i += 2) {
			for (j = 0; j < alignedsrcwidth; j += 4) {
				tl8 = src8[j >> 1];
				tr8 = src8[(i + 1 < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1)];
				bl8 = src8[(j >> 1) + 1];
				br8 = src8[(i + 1 < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1) + 1];

				c = ((((tl8 >> 4) & 0xf) + (tl8 & 0xf) + ((tr8 >> 4) & 0xf) + (tr8 & 0xf)) << 2) & 0xf0;
				a = ((((bl8 >> 4) & 0xf) + (bl8 & 0xf) + ((br8 >> 4) & 0xf) + (br8 & 0xf)) >> 2) & 0xf;

				dst8[j >> 2] = c | a;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	}

	return 0;
}

/**
 * Inflate Huffman data.
 *
 * This function operates on single channels rather than whole colours.
 * For example, for an RGBA32 image this function may be called once for each
 * channel with chansize = 256. This means the resulting data is in the format
 * RRR...GGG...BBB...AAA..., and the caller must convert it into a proper pixel
 * format.
 *
 * A typical Huffman implementation stores a tree, where each node contains
 * the lookup value and its frequency (number of uses). However, Rare's
 * implementation only stores a list of frequencies. It uses the chansize
 * to know how many values there are.
 */
void texInflateHuffman(u8 *dst, s32 numiterations, s32 chansize)
{
	u16 frequencies[2048];
	s16 nodes[2048][2];
	s32 i;
	s32 rootindex;
	s32 sum;
	u16 minfreq1;
	u16 minfreq2;
	s32 minindex1;
	s32 minindex2;
	bool done = false;

	// Read the frequencies list
	for (i = 0; i < chansize; i++) {
		frequencies[i] = texReadBits(8);
	}

	// Initialise the tree
	for (i = 0; i < 2048; i++) {
		nodes[i][0] = -1;
		nodes[i][1] = -1;
	}

	// Find the two smallest frequencies
	minfreq1 = 9999;
	minfreq2 = 9999;

	for (i = 0; i < chansize; i++) {
		if (frequencies[i] < minfreq1) {
			if (minfreq2 < minfreq1) {
				minfreq1 = frequencies[i];
				minindex1 = i;
			} else {
				minfreq2 = frequencies[i];
				minindex2 = i;
			}
		} else if (frequencies[i] < minfreq2) {
			minfreq2 = frequencies[i];
			minindex2 = i;
		}
	}

	// Build the tree.
	// For each node in tree, a branch value < 10000 means this branch
	// leads to another node, and the value is the target node's index.
	// A branch value >= 10000 means the branch is a leaf node,
	// and the value is the channel value + 10000.
	while (!done) {
		sum = frequencies[minindex1] + frequencies[minindex2];

		if (sum == 0) {
			sum = 1;
		}

		frequencies[minindex1] = 9999;
		frequencies[minindex2] = 9999;

		if (nodes[minindex1][0] < 0 && nodes[minindex1][1] < 0) {
			nodes[minindex1][0] = minindex1 + 10000;
			rootindex = minindex1;
			frequencies[minindex1] = sum;

			if (nodes[minindex2][0] < 0 && nodes[minindex2][1] < 0) {
				nodes[minindex1][1] = minindex2 + 10000;
			} else {
				nodes[minindex1][1] = minindex2;
			}
		} else if (nodes[minindex2][0] < 0 && nodes[minindex2][1] < 0) {
			nodes[minindex2][0] = minindex2 + 10000;
			rootindex = minindex2;
			frequencies[minindex2] = sum;

			if (nodes[minindex1][0] < 0 && nodes[minindex1][1] < 0) {
				nodes[minindex2][1] = minindex1 + 10000;
			} else {
				nodes[minindex2][1] = minindex1;
			}
		} else {
			for (rootindex = 0; nodes[rootindex][0] >= 0 || nodes[rootindex][1] >= 0 || frequencies[rootindex] < 9999; rootindex++);

			frequencies[rootindex] = sum;
			nodes[rootindex][0] = minindex1;
			nodes[rootindex][1] = minindex2;
		}

		// Find the two smallest frequencies again for the next iteration
		minfreq1 = 9999;
		minfreq2 = 9999;

		for (i = 0; i < chansize; i++) {
			if (frequencies[i] < minfreq1) {
				if (minfreq1 > minfreq2) {
					minfreq1 = frequencies[i];
					minindex1 = i;
				} else {
					minfreq2 = frequencies[i];
					minindex2 = i;
				}
			} else if (frequencies[i] < minfreq2) {
				minfreq2 = frequencies[i];
				minindex2 = i;
			}
		}

		if (minfreq1 == 9999 || minfreq2 == 9999) {
			done = true;
		}
	}

	// Read bits off the bitstring, traverse the tree
	// and write the channel values to dst
	for (i = 0; i < numiterations; i++) {
		s32 indexorvalue = rootindex;

		while (indexorvalue < 10000) {
			indexorvalue = nodes[indexorvalue][texReadBits(1)];
		}

		if (chansize <= 256) {
			dst[i] = indexorvalue - 10000;
		} else {
			u16 *tmp = (u16 *)dst;
			tmp[i] = indexorvalue - 10000;
		}
	}
}

/**
 * Inflate runlength-encoded data.
 *
 * This data consists of a 10 bit header followed by a list of directives,
 * where each directive can either be a literal block or a repeat (run) of
 * blocks within a sliding window.
 *
 * The header format is:
 *
 * 3 bits btfieldsize: The size in bits of the backtrack distance fields
 * 3 bits rlfieldsize: The size in bits of the runlen fields
 * 4 bits blocksize: The size in bits of each block of data
 *
 * In the data, the first bit is 0 if it's a literal block or 1 if it's a run.
 *
 * For literal blocks, the next <blocksize> bits should be read and appended to
 * the output stream.
 *
 * For runs, the next <btfieldsize> bits are the backtrack length (in blocks)
 * plus one, and the next <rlfieldsize> bits are the run length (in blocks)
 * minus a calculated fudge value.
 *
 * The fudge value is calculated based on the field sizes. For small runs it is
 * more space efficient to use multiple literal directives rather than a run
 * directive. Because of this, smaller runs are not used and the run lengths
 * in the data can be offset accordingly - this offset is the fudge value.
 *
 * Every run must be followed by a literal block without the 1-bit marker.
 * The algorithm does not support back to back runs.
 */
void texInflateRle(u8 *dst, s32 blockstotal)
{
	s32 btfieldsize = texReadBits(3);
	s32 rlfieldsize = texReadBits(3);
	s32 blocksize = texReadBits(4);
	s32 cost;
	s32 fudge;
	s32 blocksdone;
	s32 i;

	// Calculate the fudge value
	cost = btfieldsize + rlfieldsize + blocksize + 1;
	fudge = 0;

	while (cost > 0) {
		cost = cost - blocksize - 1;
		fudge++;
	}

	blocksdone = 0;

	while (blocksdone < blockstotal) {
		if (texReadBits(1) == 0) {
			// Found a literal directive
			if (blocksize <= 8) {
				dst[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			} else {
				u16 *tmp = (u16 *)dst;
				tmp[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			}
		} else {
			// Found a run directive
			s32 startblockindex = blocksdone - texReadBits(btfieldsize) - 1;
			s32 runnumblocks = texReadBits(rlfieldsize) + fudge;

			if (blocksize <= 8) {
				for (i = startblockindex; i < startblockindex + runnumblocks; i++) {
					dst[blocksdone] = dst[i];
					blocksdone++;
				}

				// The next instruction must be a literal
				dst[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			} else {
				u16 *tmp = (u16 *)dst;

				for (i = startblockindex; i < startblockindex + runnumblocks; i++) {
					tmp[blocksdone] = tmp[i];
					blocksdone++;
				}

				// The next instruction must be a literal
				tmp[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			}
		}
	}
}

/**
 * Populate a lookup table by reading it out of the bit string.
 *
 * The first 11 bits denote the number of colours in the lookup table.
 * The data following this is a list of colours, where each colour is sized
 * according to the texture's format.
 *
 * This function does NOT work with pixel formats of 8 bits or less.
 */
s32 texBuildLookup(u8 *lookup, s32 bitsperpixel)
{
	s32 numcolours = texReadBits(11);
	s32 i;

	if (bitsperpixel <= 16) {
		u16 *dst = (u16 *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(bitsperpixel);
		}
	} else if (bitsperpixel <= 24) {
		u32 *dst = (u32 *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(bitsperpixel);
		}
	} else {
		u32 *dst = (u32 *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(24) << 8 | texReadBits(bitsperpixel - 24);
		}
	}

	return numcolours;
}

s32 texGetBitSize(s32 decimal)
{
	s32 count = 0;

	decimal--;

	while (decimal > 0) {
		decimal >>= 1;
		count++;
	}

	return count;
}

void texReadAlphaBits(u8 *dst, s32 count)
{
	s32 i;

	for (i = 0; i < count; i++) {
		dst[i] = texReadBits(1);
	}
}

/**
 * Read pixel data from the bitstream and write to dst,
 * ensuring each row is aligned according to the pixel format.
 *
 * Return the number of output bytes.
 */
s32 texReadUncompressed(u8 *dst, s32 width, s32 height, s32 format)
{
	u32 *dst32 = (u32 *)(((uintptr_t)dst + 0xf) & ~0xf);
	u16 *dst16 = (u16 *)(((uintptr_t)dst + 7) & ~7);
	u8 *dst8 = (u8 *)(((uintptr_t)dst + 7) & ~7);
	s32 x;
	s32 y;

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = texReadBits(16) << 16;
				dst32[x] |= texReadBits(16);
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = texReadBits(24) << 8 | 0xff;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = texReadBits(16);
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = texReadBits(15) << 1 | 1;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = texReadBits(8);
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = texReadBits(8);
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Read grouped channel values and convert it to a proper pixel format.
 *
 * For example, for RGBA32 images the input is in the format
 * RRR...GGG...BBB...AAA... and is converted to RGBARGBARGBA...
 *
 * The existence and size of the channels depends on the pixel format.
 */
s32 texChannelsToPixels(u8 *src, s32 width, s32 height, u8 *dst, s32 format)
{
	u32 *dst32 = (u32 *)dst;
	u16 *dst16 = (u16 *)dst;
	u8 *dst8 = (u8 *)dst;
	s32 x;
	s32 y;
	s32 pos = 0;
	s32 mult = width * height;

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = src[pos] << 24 | src[pos + mult] << 16 | src[pos + mult * 2] << 8 | src[pos + mult * 3];
				pos++;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = src[pos] << 24 | src[pos + mult] << 16 | src[pos + mult * 2] << 8 | 0xff;
				pos++;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = PD_BE16(src[pos] << 11 | src[pos + mult] << 6 | src[pos + mult * 2] << 1 | src[pos + mult * 3]);
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = src[pos] << 8 | src[pos + mult];
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = PD_BE16(src[pos] << 11 | src[pos + mult] << 6 | src[pos + mult * 2] << 1 | 1);
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
		for (y = 0; y < height; y++) {
			if ((width + 7) & 0xff8);

			for (x = 0; x < width; x++) {
				dst8[x] = src[pos] << 4 | src[pos + mult];
				pos++;
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = src[pos];
				pos++;
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
		for (y = 0; y < height; y++) {
			if ((width + 15) & 0xff0);

			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = src[pos] << 5 | src[pos + mult * 3] << 4 | src[pos + 1] << 1 | src[pos + mult * 3 + 1];
				pos += 2;
			}

			if (width & 1) {
				pos--;
			}

			dst8 += (width + 15) & 0xff0;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = src[pos] << 4 | src[pos + 1];
				pos += 2;
			}

			if (width & 1) {
				pos--;
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Inflate a texture using the provided lookup table.
 *
 * The lookup table is a bitstring of colours in the pixel format described by
 * the format argument. The number of colours in the lookup table is given by
 * the numcolours argument.
 *
 * The data in the global source bitstring is expected to be a tightly packed
 * list of indices into the lookup table. The number of bits for each index
 * is calculated based on the number of colours in the lookup table. For
 * example, if the lookup table contains 8 colours then the indices will be 0-7,
 * which requires 3 bits per index.
 *
 * Return the number of bytes written to dst.
 */
s32 texInflateLookup(s32 width, s32 height, u8 *dst, u8 *lookup, s32 numcolours, s32 format)
{
	u32 *lookup32 = (u32 *)lookup;
	u16 *lookup16 = (u16 *)lookup;
	u32 *dst32 = (u32 *)dst;
	u16 *dst16 = (u16 *)dst;
	u8 *dst8 = (u8 *)dst;
	s32 x;
	s32 y;
	s32 bitspercolour = texGetBitSize(numcolours);

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = lookup32[texReadBits(bitspercolour)];
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = lookup32[texReadBits(bitspercolour)] << 8;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = lookup16[texReadBits(bitspercolour)];
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = lookup16[texReadBits(bitspercolour)] << 1 | 1;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = lookup16[texReadBits(bitspercolour)];
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = lookup16[texReadBits(bitspercolour)] << 4;

				if (x + 1 < width) {
					dst8[x >> 1] |= lookup[(texReadBits(bitspercolour) * 2) + 1];
				}
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Like texInflateLookup, but the indices are provided in the src argument
 * as u8s or u16s rather than read from the global bitstring as tightly packed
 * bits.
 *
 * Whether u8s or u16s are expected depends on whether the number of colours
 * in the lookup table. If there are more than 256 colours then it must use
 * u16s, otherwise it expects u8s.
 */
s32 texInflateLookupFromBuffer(u8 *src, s32 width, s32 height, u8 *dst, u8 *lookup, s32 numcolours, s32 format)
{
	s32 x;
	s32 y;
	u32 *lookup32 = (u32 *)lookup;
	u16 *lookup16 = (u16 *)lookup;
	u8 *src8;
	u16 *src16;
	u32 *dst32 = (u32 *)dst;
	u16 *dst16 = (u16 *)dst;
	u8 *dst8 = (u8 *)dst;

	if (numcolours <= 256) {
		src8 = (u8 *)src;
	} else {
		src16 = (u16 *)src;
	}

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst32[x] = lookup32[src8[x]];
				} else {
					dst32[x] = lookup32[src16[x]];
				}
			}

			dst32 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst32[x] = lookup32[src8[x]] << 8 | 0xff;
				} else {
					dst32[x] = lookup32[src16[x]] << 8 | 0xff;
				}
			}

			dst32 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst16[x] = PD_BE16(lookup16[src8[x]]);
				} else {
					dst16[x] = PD_BE16(lookup16[src16[x]]);
				}
			}

			dst16 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst16[x] = PD_BE16(lookup16[src8[x]] << 1 | 1);
				} else {
					dst16[x] = PD_BE16(lookup16[src16[x]] << 1 | 1);
				}
			}

			dst16 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			if ((width + 7) & 0xff8);

			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst8[x] = lookup16[src8[x]];
				} else {
					dst8[x] = lookup16[src16[x]];
				}
			}

			dst8 += (width + 7) & 0xff8;
			src8 += width;
			src16 += width;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				if (numcolours <= 256) {
					dst8[x >> 1] = lookup16[src8[x]] << 4 | lookup16[src8[x + 1]];
				} else {
					dst8[x >> 1] = lookup16[src16[x]] << 4 | lookup16[src16[x + 1]];
				}
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
			src8 += width;
			src16 += width;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * For every second row, swap every pair of words within that row.
 */
#ifdef PLATFORM_N64
void texSwizzle(u8 *dst, s32 width, s32 height, s32 format)
#else
s32 texConfigToFormat(const struct textureconfig *tex)
{
	switch (tex->format) {
		case G_IM_FMT_I:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_I4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_I8;
				default:
					break;
			}
			break;
		case G_IM_FMT_IA:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_IA4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_IA8;
				case G_IM_SIZ_16b:
					return TEXFORMAT_IA16;
				default:
					break;
			}
			break;
		case G_IM_FMT_CI:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_IA16_CI4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_IA16_CI8;
				default:
					break;
			}
			break;
		case G_IM_FMT_RGBA:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_RGBA16_CI4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_RGBA16_CI8;
				case G_IM_SIZ_16b:
					return TEXFORMAT_RGBA16;
				case G_IM_SIZ_32b:
					return TEXFORMAT_RGBA32;
				default:
					break;
			}
			break;
		default:
			break;
	}

	return TEXFORMAT_I8;
}

void texSwizzle(u8 *dst, s32 width, s32 height, s32 format)
{
	/**
	 * The N64 GPU wants swizzled textures, we don't.
	 * Thus this function is stubbed out, but its functionality is still made
	 * available for unswizzling the embedded textures in preprocess.c.
	 */
}

void texSwizzleInternal(u8 *dst, s32 width, s32 height, s32 format, u32 dstlen)
#endif
{
	s32 x;
	s32 y;
	s32 wordsperrow;
	u32 *row = (u32 *)dst;
	u32 *end = (u32 *)(dst + dstlen);
	s32 tmp;

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		wordsperrow = (width + 3) & 0xffc;
		break;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
	case TEXFORMAT_IA16:
		wordsperrow = ((width + 3) & 0xffc) >> 1;
		break;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
	case TEXFORMAT_RGBA16_CI8:
	case TEXFORMAT_IA16_CI8:
		wordsperrow = ((width + 7) & 0xff8) >> 2;
		break;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
	case TEXFORMAT_RGBA16_CI4:
	case TEXFORMAT_IA16_CI4:
		wordsperrow = ((width + 0xf) & 0xff0) >> 3;
		break;
	}

	row += wordsperrow;

	if (format == TEXFORMAT_RGBA32 || format == TEXFORMAT_RGB24) {
		for (y = 1; y < height; y += 2) {
#ifdef PLATFORM_N64
			for (x = 0; x < wordsperrow; x += 4) {
#else
			for (x = 0; x < wordsperrow && row + x < end; x += 4) {
#endif
				tmp = row[x + 0];
				row[x + 0] = row[x + 2];
				row[x + 2] = tmp;

				tmp = row[x + 1];
				row[x + 1] = row[x + 3];
				row[x + 3] = tmp;
			}

			row += wordsperrow * 2;
		}
	} else {
		for (y = 1; y < height; y += 2) {
#ifdef PLATFORM_N64
			for (x = 0; x < wordsperrow; x += 2) {
#else
			for (x = 0; x < wordsperrow && row + x < end; x += 2) {
#endif
				tmp = row[x + 0];
				row[x + 0] = row[x + 1];
				row[x + 1] = tmp;
			}

			row += wordsperrow * 2;
		}
	}
}

/**
 * Blur the pixels in the image with the surrounding pixels.
 */
void texBlur(u8 *pixels, s32 width, s32 height, s32 method, s32 chansize)
{
	s32 x;
	s32 y;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			s32 cur = pixels[y * width + x] + chansize * 2;
			s32 left = x > 0 ? pixels[y * width + x - 1] : 0;
			s32 above = y > 0 ? pixels[(y - 1) * width + x] : 0;
			s32 aboveleft = x > 0 && y > 0 ? pixels[(y - 1) * width + x - 1] : 0;

			switch (method) {
			case 0:
				pixels[y * width + x] = (cur + left) % chansize;
				break;
			case 1:
				pixels[y * width + x] = (cur + above) % chansize;
				break;
			case 2:
				pixels[y * width + x] = (cur + aboveleft) % chansize;
				break;
			case 3:
				pixels[y * width + x] = (cur + (left + above - aboveleft)) % chansize;
				break;
			case 4:
				pixels[y * width + x] = (cur + ((above - aboveleft) / 2 + left)) % chansize;
				break;
			case 5:
				pixels[y * width + x] = (cur + ((left - aboveleft) / 2 + above)) % chansize;
				break;
			case 6:
				pixels[y * width + x] = (cur + ((left + above) / 2)) % chansize;
				break;
			}
		}
	}
}

void texInitPool(struct texpool *pool, u8 *start, s32 len)
{
	pool->start = start;
	pool->end = (struct tex *)(start + len);
	pool->leftpos = start;
	pool->rightpos = (struct tex *)((uintptr_t)start + len);
}

struct tex *texFindInPool(s32 texturenum, struct texpool *pool)
{
	struct tex *end;
	struct tex *cur;
	s32 i;

	if (pool == NULL) {
		pool = &g_TexSharedPool;
	}

	if (pool == &g_TexSharedPool) {
		cur = pool->head;

		while (cur) {
			if (cur->texturenum == texturenum) {
				return cur;
			}

			if (!cur->next) {
				return NULL;
			}

			cur = (struct tex *) PHYS_TO_K0(cur->next);
		}

		return NULL;
	}

	end = pool->end;
	cur = pool->rightpos;

	while (cur < end) {
		if (cur->texturenum == texturenum) {
			return cur;
		}

		cur++;
	}

	return NULL;
}

s32 texGetPoolFreeBytes(struct texpool *pool)
{
	return (uintptr_t) pool->rightpos - (uintptr_t) pool->leftpos;
}

u8 *texGetPoolLeftPos(struct texpool *pool)
{
	return pool->leftpos;
}

void texLoadFromDisplayList(Gfx *gdl, struct texpool *pool, s32 arg2)
{
	u8 *bytes = (u8 *)gdl;
	u8 ofs = 4;
#ifdef PLATFORM_64BIT
	ofs = 8;
#endif


	while (bytes[GFX_W0_BYTE(0)] != (u8)G_ENDDL) {
		// Look for GBI sequence: fd...... abcd....
		if (bytes[GFX_W0_BYTE(0)] == G_SETTIMG && bytes[GFX_W1_BYTE(0)] == 0xab && bytes[GFX_W1_BYTE(1)] == 0xcd) {
			texLoad((texnum_t *)((uintptr_t)bytes + ofs), pool, arg2);
		}

		bytes += sizeof(Gfx);
	}
}

extern u8 EXT_SEG _texturesdataSegmentRomStart;

/**
 * Load and decompress a texture from ROM.
 *
 * The given pointer points to a word which determines what to load.
 * The formats of the word are:
 *
 *     abcdxxxx -> load texture number xxxx
 *     0000xxxx -> load texture number xxxx
 *     (memory address) -> the texture is already loaded, so do nothing
 *
 * After loading and decompressing the texture, the value that's pointed to is
 * changed to be a pointer to... something.
 *
 * There are two types of textures:
 *
 * - Zlib-compressed textures, which are always paletted
 * - Non-zlib textures, which use a variety of (non-zlib) compression methods
 *   and are sometimes paletted
 *
 * Both types have support for multiple levels of detail (ie. multiple images
 * of varying size) within each texture. There are enough bits in the header
 * byte to support 64 levels of detail, but this function caps it to 5. Some
 * textures actually specify up to 7 levels of detail. However testing suggests
 * that the additional levels of detail are not even read.
 *
 * This function reads the above information from the first byte of texture data,
 * then calls the texInflateZlib or texInflateNonZlib to inflate the images.
 *
 * The format of the first byte is:
 * uzllllll
 *
 * u = unknown
 * z = texture is compressed with zlib
 * l = number of levels of detail within the texture
 */
void texLoad(texnum_t *updateword, struct texpool *pool, bool unusedarg)
{
	u8 compbuffer[4 * 1024 + 0x40];
	u8 *compptr;
	s32 hasloddata;
	s32 iszlib;
	s32 numlods;
	struct tex *tex;
	u8 *alignedcompbuffer;
	u32 stack;
	struct tex *tail;
	u32 freebytes;
	u8 usingsharedpool = 0;
	s8 buffer5kb[5 * 1024 + 0x40];
	s32 thisoffset;
	s32 nextoffset;
	s16 *texnumptr;
	s32 bytesout;

	usingsharedpool = 0;

	if (pool == NULL) {
		pool = &g_TexSharedPool;
	}

	if (pool == &g_TexSharedPool) {
		usingsharedpool = 1;
	}

	// If the value at updateword isn't already a pointer
	if ((*updateword & 0xffff0000) == 0 || (*updateword & 0xffff0000) == 0xabcd0000) {
		g_TexNumToLoad = *updateword & 0xffff;

		tex = texFindInPool(g_TexNumToLoad, pool);

		if (tex == NULL) {
			if (g_TexNumToLoad >= NUM_TEXTURES) {
				return;
			}

			alignedcompbuffer = (u8 *) (((uintptr_t)compbuffer + 0xf) >> 4 << 4);

			if (alignedcompbuffer);
			if (tex);

			osWritebackDCacheAll();
			osInvalDCache(alignedcompbuffer, DCACHE_SIZE);

			thisoffset = g_Textures[g_TexNumToLoad].dataoffset;
			nextoffset = g_Textures[g_TexNumToLoad + 1].dataoffset;

			if (thisoffset == nextoffset) {
				// The texture has no data
				return;
			}

#ifndef PLATFORM_N64
			// try to load external replacement if present
			if (modTextureLoad(g_TexNumToLoad, alignedcompbuffer, 4096) > 0) {
				compptr = alignedcompbuffer;
			} else
#endif
			{
				// Copy the compressed texture to RAM
				dmaExec(alignedcompbuffer,
						(romptr_t) REF_SEG _texturesdataSegmentRomStart + (thisoffset & 0xfffffff8),
						((uintptr_t) (nextoffset - thisoffset) + 0x1f) >> 4 << 4);
				compptr = (u8 *) alignedcompbuffer + (thisoffset & 7);
			}
			thisoffset = 0;
			hasloddata = (*compptr & 0x80) >> 7;
			iszlib = (*compptr & 0x40) >> 6;
			numlods = *compptr & 0x3f;

			if (numlods > 5) {
				numlods = 5;
			}

			compptr++;

			// If there's not enough memory to load the texture, set the texture
			// pointer to the start of the pool. It'll be garbage data but the
			// only other option is a crash. GBI commands contain texture IDs
			// instead of pointers, and they must be replaced with pointers.
			if (usingsharedpool) {
				freebytes = mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_ONBOARD) + mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_EXPANSION);
			} else {
				freebytes = texGetPoolFreeBytes(pool);
			}

			if ((!iszlib && freebytes < 4300) || (iszlib && freebytes < 2600)) {
				*updateword = osVirtualToPhysical(pool->start);
				return;
			}

			// If we're using the shared pool:
			// - rightpos is the head of a linked list, so grab it and find the tail
			// - set rightpos to a spot in the buffer that can fit a tex before it
			// - set leftpos to 0x10 after rightpos
			if (usingsharedpool) {
				tail = pool->rightpos;
				pool->rightpos = (struct tex *) ((((uintptr_t) buffer5kb + 0xf) >> 4 << 4) + sizeof(struct tex));
				pool->leftpos = ((u8 *) pool->rightpos + sizeof(struct tex));

				while (tail) {
					if (tail->next == 0) {
						break;
					}

					tail = (struct tex *) PHYS_TO_K0(tail->next);
				}
			}

			// Write the texturenum into the allocation
			texnumptr = (s16 *) pool->leftpos;
			*texnumptr = g_TexNumToLoad;
			pool->leftpos += 8;

			// Write a tex into the allocation
			pool->rightpos--;
			tex = pool->rightpos;
			tex->texturenum = g_TexNumToLoad;
			tex->data = pool->leftpos;
			tex->unk0c_03 = false;

			// Extract the texture data to the allocation (pool->leftpos)
			if (iszlib) {
				bytesout = texInflateZlib(compptr, pool->leftpos, hasloddata, numlods, pool, unusedarg);
			} else {
				bytesout = texInflateNonZlib(compptr, pool->leftpos, hasloddata, numlods, pool, unusedarg);
			}

			// If we're using the shared pool, the data must be copied out of
			// the stack and into the heap.
			if (usingsharedpool) {
				u8 *ptr = mempAllocFromRight(ALIGN16(bytesout + 2 * sizeof(struct tex)), MEMPOOL_STAGE);
				pool->rightpos = (struct tex *) ptr;

				bcopy(tex, ptr, sizeof(struct tex));

				tex = (struct tex *) ptr;
				ptr += sizeof(struct tex);

				bcopy(pool->leftpos - 8, ptr, bytesout + 8);

				pool->rightpos->data = ptr + 8;
				pool->rightpos->next = 0;

				if (tail != NULL) {
					tail->next = (uintptr_t) pool->rightpos
					#ifdef PLATFORM_N64
						& 0xffffff;
					#else
						;
					#endif
				} else {
					pool->head = pool->rightpos;
				}

				pool->start = (u8 *) pool->rightpos;
			}

			pool->leftpos += bytesout;

			if (!usingsharedpool) {
				texGetPoolFreeBytes(pool);
			}
		}

		*updateword = osVirtualToPhysical(tex->data);
	}
}

void texLoadFromConfigs(struct textureconfig *configs, s32 numconfigs, struct texpool *pool, uintptr_t arg3)
{
	s32 i;

	for (i = 0; i < numconfigs; i++) {
		if ((uintptr_t)configs[i].texturenum < NUM_TEXTURES) {
			texLoad(&configs[i].texturenum, pool, true);
			configs[i].unk0b = 1;
		} else {
			configs[i].texturenum += arg3;
		}
	}
}

void texLoadFromTextureNum(u32 texturenum, struct texpool *pool)
{
	texnum_t texturenumcopy = texturenum;

	texLoad(&texturenumcopy, pool, true);
}

s32 func0f173510(s32 arg0, s32 arg1, s32 arg3)
{
	return arg0;
}
