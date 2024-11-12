#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess/common.h"

struct n64_fontchar {
	u8 index;
	s8 baseline;
	u8 height;
	u8 width;
	s32 kerningindex;
	u32 pixeldata;
};

struct n64_fontcharjpn {
	u16 index;
	s8 baseline;
	u8 height;
	u8 width;
	s16 kerningindex;
	u32 pixeldata;
};

u8 *preprocessFont(u8 *src, u32 srclen, u32 *outSize)
{
	int num_chars = 94;
#if VERSION == VERSION_PAL_FINAL
	// PAL has more characters in these fonts
	extern u8* _fonthandelgothicsmSegmentRomStart;
	extern u8* _fonthandelgothicxsSegmentRomStart;
	extern u8* _fonthandelgothicmdSegmentRomStart;
	if (src == _fonthandelgothicsmSegmentRomStart
		|| src == _fonthandelgothicxsSegmentRomStart
		|| src == _fonthandelgothicmdSegmentRomStart) {
		num_chars = 135;
	}
#endif

	size_t dstlen = srclen - (num_chars * sizeof(struct n64_fontchar)) + (num_chars * sizeof(struct fontchar)) + 0x20;
	u8* dst = sysMemZeroAlloc(dstlen);

	// Kerning table
	s32 *src_kerning_table = (s32 *)src;
	s32 *dst_kerning_table = (s32 *)dst;

	for (int i = 0; i < 13 * 13; i++) {
		dst_kerning_table[i] = PD_BE32(src_kerning_table[i]);
	}

	u32 src_offset = PD_ALIGN(13 * 13 * 4, sizeof(u32));
	u32 dst_offset = PD_ALIGN(13 * 13 * 4, sizeof(uintptr_t));

	// Character table
#if VERSION == VERSION_JPN_FINAL
		struct n64_fontcharjpn *src_char_table = (struct n64_fontcharjpn *) &src[src_offset];
		struct fontchar *dst_char_table = (struct fontchar *) &dst[dst_offset];
		u32 diff = (dst_offset + num_chars * sizeof(struct fontchar)) - (src_offset + num_chars * sizeof(struct n64_fontcharjpn));

		for (int i = 0; i < num_chars; i++) {
			dst_char_table[i].index = PD_BE16(src_char_table[i].index);
			dst_char_table[i].baseline = src_char_table[i].baseline;
			dst_char_table[i].height = src_char_table[i].height;
			dst_char_table[i].width = src_char_table[i].width;
			dst_char_table[i].kerningindex = PD_BE16(src_char_table[i].kerningindex);
			dst_char_table[i].pixeldata = (void *)(uintptr_t)(src_char_table[i].pixeldata ? PD_BE32(src_char_table[i].pixeldata) + diff : 0);
		}

		src_offset += num_chars * sizeof(struct n64_fontcharjpn);
		dst_offset += num_chars * sizeof(struct fontchar);
#else
		struct n64_fontchar *src_char_table = (struct n64_fontchar *) &src[src_offset];
		struct fontchar *dst_char_table = (struct fontchar *) &dst[dst_offset];
		u32 diff = (dst_offset + num_chars * sizeof(struct fontchar)) - (src_offset + num_chars * sizeof(struct n64_fontchar));

		for (int i = 0; i < num_chars; i++) {
			dst_char_table[i].index = src_char_table[i].index;
			dst_char_table[i].baseline = src_char_table[i].baseline;
			dst_char_table[i].height = src_char_table[i].height;
			dst_char_table[i].width = src_char_table[i].width;
			dst_char_table[i].kerningindex = PD_BE32(src_char_table[i].kerningindex);
			dst_char_table[i].pixeldata = (void *)(uintptr_t)(src_char_table[i].pixeldata ? PD_BE32(src_char_table[i].pixeldata) + diff : 0);
		}

		src_offset += num_chars * sizeof(struct n64_fontchar);
		dst_offset += num_chars * sizeof(struct fontchar);
#endif

	// Pixel data
	u32 len = srclen - src_offset;
	memcpy(&dst[dst_offset], &src[src_offset], len);

	if (outSize) *outSize = ALIGN16(dst_offset + len);

	return dst;
}

u8 *preprocessJpnFont(u8* data, u32 size, u32* outSize)
{
	// ???
	return NULL;
}
