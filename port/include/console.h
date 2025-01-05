#ifndef _IN_CONSOLE_H
#define _IN_CONSOLE_H

#include <PR/ultratypes.h>
#include "types.h"

void conInit(void);
Gfx *conRender(Gfx *gdl);
void conTick(void);
void conPrint(s32 showmsg, const char *s);
void conPrintf(s32 showmsg, const char *fmt, ...);
void conPrintLn(s32 showmsg, const char *s);
s32 conIsOpen(void);

#endif
