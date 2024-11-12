#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess/common.h"

static int getNextNonZero(u8 *src, u32 offset, size_t len)
{
	u32 *ptr = (u32 *) &src[offset];
	u32 *end = (u32 *) &src[len];

	while (ptr < end) {
		if (*ptr != 0) {
			return PD_BE32(*ptr);
		}

		ptr++;
	}

	return 0;
}

static int getNumStrings(u8 *src, size_t len)
{
	u32 ptr_table_len = getNextNonZero(src, 0, len);

	if (ptr_table_len) {
		return ptr_table_len / sizeof(u32);
	}

	return 1;
}

static u32 convertLangFile(u8 *dst, u8 *src, size_t srclen)
{
	// Convert it
	u32 *src_offsets = (u32 *) src;
	size_t src_ptr_table_len = PD_BE32(src_offsets[0]);
	size_t src_str_table_len = srclen - src_ptr_table_len;
	int num_strings = getNumStrings(src, srclen);
	size_t dst_ptr_table_len = num_strings * sizeof(uintptr_t);
	size_t dst_str_table_len = src_str_table_len;

	int cur_dst_offset = dst_ptr_table_len;

	uintptr_t *dst_offsets = (uintptr_t *) dst;

	for (int i = 0; i < num_strings; i++) {
		u32 src_offset = PD_BE32(src_offsets[i]);

		if (src_offset) {
			// Write pointer
			dst_offsets[i] = (cur_dst_offset);

			// Write string
			u32 end = getNextNonZero(src, (i + 1) * 4, num_strings * 4);

			if (!end) {
				end = srclen;
			}

			size_t len = end - src_offset;
			len = ALIGN4(len);
			memcpy(&dst[cur_dst_offset], &src[src_offset], len);

			cur_dst_offset += len;
		} else {
			// Write pointer
			dst_offsets[i] = 0;
		}
	}

	return ALIGN16(cur_dst_offset);
}

u8 *preprocessLangFile(u8 *data, u32 size, u32 *outSize) {
	u32 newSizeEstimated = romdataFileGetEstimatedSize(size, LOADTYPE_LANG);
	u8 *dst = sysMemZeroAlloc(newSizeEstimated);

	u32 newSize = convertLangFile(dst, data, size);

	if (newSize > newSizeEstimated) {
		sysFatalError("overflow when trying to preprocess a lang file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
