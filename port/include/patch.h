#ifndef _IN_PATCH_H
#define _IN_PATCH_H

#include <PR/ultratypes.h>

struct patchbytes {
	u32 ofs;
	u32 len;
	const char *src;
	const char *dst;
};

struct patchlights {
	s16 lightnum;
	s16 dx;
	s16 dy;
	s16 dz;
};

struct patchroom {
	s16 roomnum;
	s16 cmdnum;
	u8 cmdbyte;
};

s32 patchDeflate1173(u8 *src, u32 srclen, u8 *dst, u32 dstlen);
void patchLight(u8 *src, s16 lightnum, s16 dx, s16 dy, s16 dz);
void patchSetupFile(const char * name, u32 numpatches, struct patchbytes *patches);
void patchBgFile(const char * name, u32 numpatches, struct patchlights *patches, struct patchroom *patchroom);
void patchInit(void);

#endif
