#ifndef _IN_PREPROCESS_GBI_H
#define _IN_PREPROCESS_GBI_H

#include "preprocess/common.h"

// gbi related functions
void gbiReset(void);
void gbiSetSegment(int segment, u32 offset);
void gbiSetVtx(u32 src_offset, u32 dst_offset);
u32 gbiConvertGdl(u8 *dst, u32 dstpos, u8 *src, u32 srcpos, int segment_cmds);
void gbiConvertVtx(u8* dst, u32 dstpos, int count);
void gbiGdlRewriteAddrs(u8* dst, u32 offset);
void gbiAddTexAddr(u32 src_offset, u32 dst_offset);

#endif
