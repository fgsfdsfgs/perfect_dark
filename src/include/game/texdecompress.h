#ifndef IN_GAME_TEXDECOMPRESS_H
#define IN_GAME_TEXDECOMPRESS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void func0f16e810(u32 arg0);
s32 texInflateZlib(u8 *src, u8 *dst, bool arg2, s32 forcenumimages, struct texpool *pool, s32 unusedarg);
s32 texAlignIndices(u8 *arg0, s32 width, s32 height, s32 format, u8 *dst);
s32 texGetAverageRed(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
s32 texGetAverageGreen(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
s32 texGetAverageBlue(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
s32 texGetAverageAlpha(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
s32 texShrinkPaletted(u8 *src, u8 *dst, s32 srcwidth, s32 srcheight, s32 format, u16 *palette, s32 numcolours);
s32 texFindClosestColourIndexRGBA(u8 *palette, s32 numcolours, s32 r, s32 g, s32 b, s32 a);
s32 texFindClosestColourIndexIA(u16 *palette, s32 numcolours, s32 intensity, s32 alpha);
s32 texInflateNonZlib(u8 *src, u8 *dst, s32 arg2, s32 forcenumimages, struct texpool *pool, s32 unusedarg);
s32 texShrinkNonPaletted(u8 *src, u8 *dst, s32 srcwidth, s32 srcheight, s32 format);
void texInflateHuffman(u8 *dst, s32 numiterations, s32 chansize);
void texInflateRle(u8 *arg0, s32 arg1);
s32 texBuildLookup(u8 *arg0, s32 bpp);
s32 texGetBitSize(s32 arg0);
void texReadAlphaBits(u8 *dst, s32 count);
s32 texReadUncompressed(u8 *dst, s32 width, s32 height, s32 format);
s32 texChannelsToPixels(u8 *src, s32 width, s32 height, u8 *dst, s32 format);
s32 texInflateLookup(s32 width, s32 height, u8 *dst, u8 *lookup, s32 numcolours, s32 format);
s32 texInflateLookupFromBuffer(u8 *src, s32 width, s32 height, u8 *dst, u8 *lookup, s32 numcolours, s32 format);
void texSwizzle(u8 *arg0, s32 width, s32 height, s32 format);
void texBlur(u8 *pixels, s32 width, s32 height, s32 method, s32 chansize);
void texInitPool(struct texpool *pool, u8 *start, s32 len);
struct tex *texFindInPool(s32 texturenum, struct texpool *pool);
s32 texGetPoolFreeBytes(struct texpool *pool);
u8 *texGetPoolLeftPos(struct texpool *pool);
void texLoadFromDisplayList(Gfx *gdl, struct texpool *pool, s32 arg2);
void texLoad(texnum_t *updateword, struct texpool *pool, bool unusedarg);
void texLoadFromConfigs(struct textureconfig *configs, s32 numconfigs, struct texpool *pool, uintptr_t arg3);
void texLoadFromTextureNum(u32 arg0, struct texpool *pool);
#ifndef PLATFORM_N64
void texSwizzleInternal(u8 *dst, s32 width, s32 height, s32 format, u32 dstlen);
s32 texConfigToFormat(const struct textureconfig *tex);
#endif

#endif
