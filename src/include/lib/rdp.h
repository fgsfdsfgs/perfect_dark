#ifndef _IN_LIB_RDP_H
#define _IN_LIB_RDP_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void rdpInit(void);
void rdpCreateTask(Gfx *gdlstart, Gfx *gdlend, u32 arg2, uintptr_t msg);

#endif
