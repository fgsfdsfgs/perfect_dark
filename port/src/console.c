#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "platform.h"
#include "types.h"
#include "constants.h"
#include "data.h"
#include "bss.h"
#include "config.h"
#include "video.h"
#include "input.h"
#include "console.h"
#include "system.h"
#include "utils.h"
#include "net/net.h"
#include "game/hudmsg.h"
#include "game/game_1531a0.h"
#include "lib/vi.h"

#define CON_ROWS 80
#define CON_COLS 56
#define CON_VISROWS 10
#define CON_MSGROWS 4
#define CON_MSGTIMER 3.f

static char conBuf[CON_ROWS][CON_COLS + 1];
static char conInput[CON_COLS + 1];
static char *conVisRows[CON_VISROWS];
static s32 conPrintRow = 0;
static s32 conPrintCol = 0;
static s32 conMsgRows = 0;
static f32 conMsgTimer = 0.f;
static s32 conInputCol = 0;
static u32 conTextColour = 0x00ff00ff;
static s32 conOpen = 0;
static s32 conButton = 0;

void conInit(void)
{
	memset(conBuf, 0, sizeof(conBuf));
	memset(conInput, 0, sizeof(conInput));
	memset(conVisRows, 0, sizeof(conVisRows));
	conPrintRow = conPrintCol = 0;
	conOpen = 0;
	conMsgRows = 0;
	conMsgTimer = 0.0f;
	conInputCol = 0;
}

void conPrint(s32 showmsg, const char *str)
{
	if (!str || !*str) {
		return;
	}

	const s32 oldRow = conPrintRow;

	for (const char *s = str; *s; ++s) {
		char ch = *s;
		switch (ch) {
			case '\n':
				if (conPrintCol) {
					conPrintCol = 0;
					conPrintRow = (conPrintRow + 1) % CON_ROWS;
				}
				break;
			case '\r':
				break;
			case '\t':
				ch = ' ';
				/* fallthrough */
			default:
				conBuf[conPrintRow][conPrintCol++] = ch;
				if (conPrintCol == CON_COLS) {
					conPrintCol = 0;
					conPrintRow = (conPrintRow + 1) % CON_ROWS;
				}
				break;
		}
		conBuf[conPrintRow][conPrintCol] = '\0';
	}

	if (conPrintRow != oldRow) {
		// scroll time
		for (s32 i = 0; i < ARRAYCOUNT(conVisRows); ++i) {
			s32 row = conPrintRow - i - 1;
			if (row < 0) {
				row = CON_ROWS + row;
			}
			conVisRows[i] = &conBuf[row][0];
		}
		if (showmsg) {
			if (conMsgRows < CON_MSGROWS) {
				if (conPrintRow > oldRow) {
					conMsgRows += conPrintRow - oldRow;
				} else {
					conMsgRows += conPrintRow + (CON_ROWS - oldRow);
				}
			}
			conMsgTimer = sysGetSeconds() + CON_MSGTIMER;
		}
	}
}

void conPrintLn(s32 showmsg, const char *s)
{
	char tmp[4096];
	snprintf(tmp, sizeof(tmp), "%s\n", s);
	conPrint(showmsg, tmp);
}

void conPrintf(s32 showmsg, const char *fmt, ...)
{
	char tmp[4096];
	tmp[sizeof(tmp) - 1] = '\0';
	tmp[0] = '\0';

	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, args);
	va_end(args);

	conPrint(showmsg, tmp);
}

static inline Gfx *conRenderMsgs(Gfx *gdl)
{
	if (conMsgRows) {
		s32 x, y;
		const u32 c = (conTextColour & 0xffffff00) | 0xa0;
		for (s32 i = 0; i < conMsgRows; ++i) {
			char *s = conVisRows[i];
			if (s) {
				x = 4;
				y = 4 + 8 * (conMsgRows - i - 1);
				gdl = textRenderProjected(gdl, &x, &y, s, g_CharsHandelGothicXs, g_FontHandelGothicXs, c, viGetWidth(), viGetHeight(), 0, 0);
			}
		}
		const f32 t = sysGetSeconds();
		if (conMsgTimer <= t) {
			conMsgTimer = t + CON_MSGTIMER;
			--conMsgRows;
		}
	} else {
		conMsgTimer = 0;
	}

	return gdl;
}

Gfx *conRender(Gfx *gdl)
{
	if (!g_CharsHandelGothicXs || !g_FontHandelGothicXs) {
		return gdl;
	}

	gdl = text0f153628(gdl);

	if (!conOpen) {
		gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, G_ASPECT_LEFT_EXT);
		gdl = conRenderMsgs(gdl);
	} else {
		s32 x, y;
		gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, G_ASPECT_CENTER_EXT);
		gdl = hudmsgRenderBox(gdl, 16, 0, SCREEN_WIDTH_LO - 16, 4 + 8 * (CON_VISROWS + 1), 1.f, conTextColour, 0.9f);
		for (s32 i = 0; i < CON_VISROWS; ++i) {
			char *s = conVisRows[i];
			if (s) {
				x = 18;
				y = 4 + 8 * (CON_VISROWS - i - 1);
				gdl = textRenderProjected(gdl, &x, &y, s, g_CharsHandelGothicXs, g_FontHandelGothicXs, conTextColour, viGetWidth(), viGetHeight(), 0, 0);
			}
		}
		char tmp[CON_COLS + 3];
		snprintf(tmp, sizeof(tmp), "> %s", conInput);
		x = 18;
		y = 4 + 8 * CON_VISROWS;
		gdl = textRenderProjected(gdl, &x, &y, tmp, g_CharsHandelGothicXs, g_FontHandelGothicXs, conTextColour, viGetWidth(), viGetHeight(), 0, 0);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

	gdl = text0f153780(gdl);

	return gdl;
}

void conTick(void)
{
	const s32 button = inputKeyPressed(VK_GRAVE);
	if (button && !conButton) {
		conOpen = !conOpen;
		g_MenuKeyboardPlayer = -1;
		if (conOpen) {
			inputClearLastTextChar();
			inputStartTextInput();
		} else {
			inputStopTextInput();
		}
	}

	conButton = button;

	if (conOpen) {
		if (inputTextHandler(conInput, CON_COLS, &conInputCol, false)) {
			if (g_NetMode) {
				netChat(NULL, conInput);
			}
			conInput[0] = '\0';
			conInputCol = 0;
		}
	}
}

s32 conIsOpen(void)
{
	return conOpen;
}
