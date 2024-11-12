#ifndef _IN_LIB_MP3_H
#define _IN_LIB_MP3_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void mp3Init(ALHeap *heap);
void mp3PlayFile(uintptr_t romaddr, s32 filesize);
void func00037e1c(void);
void func00037e38(void);
void func00037e68(void);
s32 func00037ea4(void);
void func00037f08(s32 arg0, bool arg1);
void func00037f5c(s32 arg0, bool arg1);
s32 func00037fc0(s32 arg0, Acmd **cmd);
void func00038924(struct mp3vars *vars);
void func00038b90(void *fn);
void mp3Dma(void);

#endif
