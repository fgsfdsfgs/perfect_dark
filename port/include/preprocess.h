#ifndef _IN_PREPROCESS_H
#define _IN_PREPROCESS_H

#include <PR/ultratypes.h>

typedef u8 *(*preprocessfunc)(u8 *data, u32 size, u32 *outSize);

u8* preprocessAnimations(u8 *data, u32 size, u32 *outSize);
u8* preprocessMpConfigs(u8 *data, u32 size, u32 *outSize);
u8* preprocessFont(u8 *data, u32 size, u32 *outSize);
u8* preprocessJpnFont(u8 *data, u32 size, u32 *outSize);
u8* preprocessALBankFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessALCMidiHdr(u8 *data, u32 size, u32 *outSize);
u8* preprocessSequences(u8 *data, u32 size, u32 *outSize);
u8* preprocessTexturesList(u8 *data, u32 size, u32 *outSize);

void preprocessBgSection1(u8* data, u32 size, u32 ofs);
void preprocessBgSection2(u8 *data, u32 size);
void preprocessBgSection3(u8* data, u32 size);
void preprocessBgSection1Header(u8 *data, u32 size);
void preprocessBgSection2Header(u8 *data, u32 size);
void preprocessBgSection3Header(u8 *data, u32 size);
u32 preprocessBgRoom(u8* data, u32 size, u32 room_ofs);

u8 *preprocessLangFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessPadsFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessTilesFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessSetupFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessModelFile(u8 *data, u32 size, u32 *outSize);
u8 *preprocessGunFile(u8 *data, u32 size, u32 *outSize);

#endif
