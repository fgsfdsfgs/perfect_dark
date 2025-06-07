#include <stdint.h>
#include <ultra64.h>
#include "constants.h"
#include "game/filelist.h"
#include "game/tex.h"
#include "game/savebuffer.h"
#include "game/menu.h"
#include "game/filemgr.h"
#include "game/bossfile.h"
#include "game/game_1531a0.h"
#include "game/gamefile.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/mema.h"
#include "lib/str.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#endif

// bss
struct fileguid g_FilemgrFileToCopy;
struct fileguid var800a21e8;

#if VERSION >= VERSION_JPN_FINAL
struct fileguid g_FilemgrFileToDelete[MAX_PLAYERS];
#else
struct fileguid g_FilemgrFileToDelete;
#endif

s32 g_FilemgrLastPakError;
#if VERSION == VERSION_JPN_FINAL
u8 jpnfill4[8];
#endif
struct gamefile g_GameFile;
u32 var800a22bc;
struct fileguid g_GameFileGuid;

// These might be able to be removed by relocating their symbols
struct menudialogdef g_FilemgrCopyMenuDialog;
struct menudialogdef g_FilemgrConfirmDeleteMenuDialog;
struct menudialogdef g_FilemgrDeleteMenuDialog;
struct menudialogdef g_FilemgrEnterNameMenuDialog;
struct menudialogdef g_FilemgrErrorMenuDialog;
struct menudialogdef g_FilemgrFileInUseMenuDialog;
struct menudialogdef g_FilemgrFileLostMenuDialog;
struct menudialogdef g_FilemgrFileSavedMenuDialog;
struct menudialogdef g_FilemgrFileSelectMenuDialog;
struct menudialogdef g_FilemgrSaveElsewhereMenuDialog;
struct menudialogdef g_FilemgrSaveErrorMenuDialog;
struct menudialogdef g_FilemgrSelectLocationMenuDialog;
struct menudialogdef g_PakDeleteNoteMenuDialog;
struct menudialogdef g_PakGameNotesMenuDialog;
struct menudialogdef g_PakNotOriginalMenuDialog;
struct pakdata *g_EditingPak;

#if VERSION >= VERSION_NTSC_1_0
struct menudialogdef g_FilemgrDuplicateNameMenuDialog;
struct menudialogdef g_FilemgrRenameMenuDialog;
#endif

MenuItemHandlerResult filemgrHandleSetLanguage(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Vars.language = item->param;
		langSetEuropean(g_Vars.language);
		menuPopDialog();
	}

	return 0;
}
// TODO - Lang: Fix it; make that things dynamic
struct menuitem g_ChooseLanguageMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Choose your language:"), // "Choose your language:"
		0,
		0,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		0,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		LANGUAGE_PAL_EN,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("English\n"), // "English"
		0,
		filemgrHandleSetLanguage,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		LANGUAGE_PAL_FR,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("French\n"), // "French"
		0,
		filemgrHandleSetLanguage,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		LANGUAGE_PAL_DE,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("German\n"), // "German"
		0,
		filemgrHandleSetLanguage,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		LANGUAGE_PAL_IT,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Italian\n"), // "Italian"
		0,
		filemgrHandleSetLanguage,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		LANGUAGE_PAL_ES,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Spanish\n"), // "Spanish"
		0,
		filemgrHandleSetLanguage,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_ChooseLanguageMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Perfect Dark\n"),
	g_ChooseLanguageMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

char *filemgrGetDeviceName(s32 index)
{
	char *names[] = {
		_("Controller Pak 1\n"), // "Controller Pak 1"
		_("Controller Pak 2\n"), // "Controller Pak 2"
		_("Controller Pak 3\n"), // "Controller Pak 3"
		_("Controller Pak 4\n"), // "Controller Pak 4"
		_("Game Pak\n"), // "Game Pak"
#if VERSION >= VERSION_NTSC_1_0
		_("Controller Pak Not Found\n"), // "Controller Pak Not Found"
#endif
	};

	if (index < ARRAYCOUNT(names)) {
		return names[index];
	}

	return NULL;
}

MenuItemHandlerResult filemgrDeviceNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if ((g_Menus[g_MpPlayerNum].fm.device1 & 0x7f) >= SAVEDEVICE_INVALID) {
			return true;
		}
	}

	return 0;
}

char *filemgrMenuTextDeviceName(struct menuitem *item)
{
	return filemgrGetDeviceName(g_Menus[g_MpPlayerNum].fm.device1 & 0x7f);
}

void filemgrGetSelectName(char *buffer, struct filelistfile *file, u32 filetype)
{
	s32 days;
	char tmpbuffer1[28];
	char namebuffer[20];
	u32 totalinseconds;
	s32 pos;
	s32 hours;
	s32 minutes;
	s32 seconds;
	s32 totalinhours;

	switch (filetype) {
	case FILETYPE_GAME:
	case FILETYPE_MPSETUP:
		func0f0d564c(file->name, tmpbuffer1, false);
		break;
	case FILETYPE_MPPLAYER:
		// MP Player filenames have the play duration appended to the name
		mpplayerfileGetOverview(file->name, namebuffer, &totalinseconds);
		pos = sprintf(tmpbuffer1, "%s-", namebuffer);

		if (totalinseconds >= 0x7ffffff) { // about 4.25 years
			sprintf(tmpbuffer1 + pos, "==:==");
		} else {
			seconds = totalinseconds % 60;
			totalinseconds = totalinseconds / 60;
			totalinhours = totalinseconds / 60;
			minutes = totalinseconds % 60;
			days = totalinhours / 24;
			hours = totalinhours % 24;

			if (days == 0) {
				// seconds is passed but has no placeholder
				sprintf(tmpbuffer1 + pos, "%d:%02d", hours, minutes, seconds);
			} else {
				sprintf(tmpbuffer1 + pos, "%d:%02d:%02d", days, hours, minutes);
			}
		}
		break;
	}

	sprintf(buffer, "%s\n", tmpbuffer1);
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b2f28[] = "Setup: item = %x\n";
#endif

// game, mpsetup, mpplayer, perfect head
const u32 g_FileTypeSizes[] = {0xa0, 0x31, 0x4e, 0x4a0};

MenuItemHandlerResult filemgrFileNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if (g_Menus[g_MpPlayerNum].fm.filetodelete == 0) {
			return true;
		}
	}

	return 0;
}

char *filemgrMenuTextDeleteFileName(struct menuitem *item)
{
	if (g_Menus[g_MpPlayerNum].fm.filetodelete) {
		filemgrGetSelectName(g_StringPointer,
				g_Menus[g_MpPlayerNum].fm.filetodelete,
				g_Menus[g_MpPlayerNum].fm.filetypetodelete);
		return g_StringPointer;
	}

	return NULL;
}

void filemgrSetDevice1BySerial(s32 deviceserial)
{
	s32 device = pakFindBySerial(deviceserial);

	if (device >= 0) {
		g_Menus[g_MpPlayerNum].fm.device1 = device;
	} else {
#if VERSION >= VERSION_NTSC_1_0
		g_Menus[g_MpPlayerNum].fm.device1 = SAVEDEVICE_INVALID;
#else
		g_Menus[g_MpPlayerNum].fm.device1 = 0x7f;
#endif
	}
}

void filemgrSetDevice1ByFile(struct filelistfile *file)
{
	filemgrSetDevice1BySerial(file->deviceserial);
}

void filemgrSetFileToDelete(struct filelistfile *file, s32 filetype)
{
	g_Menus[g_MpPlayerNum].fm.filetypetodelete = filetype;
	g_Menus[g_MpPlayerNum].fm.filetodelete = file;
	filemgrSetDevice1ByFile(file);
}

char *filemgrMenuTextFailReason(struct menuitem *item)
{
	static char *reasons[] = {
		gettext_noop("The Controller Pak\nwas not found\nin any controller.\n"), // "The Controller Pak was not found in any controller."
		gettext_noop("File was not saved.\n"), // "File was not saved."
		gettext_noop("File would not load.\n"), // "File would not load."
		gettext_noop("Could not delete the file.\n"), // "Could not delete the file."
		gettext_noop("Out of memory.\n"), // "Out of memory."
		gettext_noop("This player is already\nloaded for this game.\n"), // "This player is already loaded for this game."
		gettext_noop("has been removed.\n"), // "has been removed."
		gettext_noop("Controller Pak is damaged or incorrectly inserted.\n"), // "Controller Pak is damaged or incorrectly inserted."
		gettext_noop("Game note delete failed.\n"), // "Game note delete failed."
	};

	return _(reasons[g_Menus[g_MpPlayerNum].fm.errnum]);
}

/**
 * Unused.
 */
char *filemgr0f108484(struct menuitem *item)
{
	sprintf(g_StringPointer, "location: controller pak 1\n");
	return g_StringPointer;
}

MenuItemHandlerResult filemgrDeviceNameForErrorMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKHIDDEN) {
		if ((g_Menus[g_MpPlayerNum].fm.device1 & 0x7f) >= SAVEDEVICE_INVALID) {
			return true;
		}

		switch (g_Menus[g_MpPlayerNum].fm.errnum) {
		case FILEERROR_OUTOFMEMORY:
		case FILEERROR_ALREADYLOADED:
		case FILEERROR_PAKDAMAGED:
		default:
			return true;
		case FILEERROR_SAVEFAILED:
		case FILEERROR_LOADFAILED:
		case FILEERROR_DELETEFAILED:
		case FILEERROR_PAKREMOVED:
		case FILEERROR_DELETENOTEFAILED:
			break;
		}
	}

	return false;
}

char *filemgrMenuTextDeviceNameForError(struct menuitem *item)
{
	sprintf(g_StringPointer, "%s", filemgrGetDeviceName(g_Menus[g_MpPlayerNum].fm.device1 & 0x7f));

	if (g_Menus[g_MpPlayerNum].fm.errnum != FILEERROR_PAKREMOVED) {
		s32 i = 0;

		while (g_StringPointer[i] != '\0') {
			i++;
		}

		g_StringPointer[i - 1] = ':';
		g_StringPointer[i] = '\n';
		g_StringPointer[i + 1] = '\0';
	}

	return g_StringPointer;
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b2f6c[] = "FileMan: Failure Handler\n";
const char var7f1b2f88[] = "Copy Memory Freed\n";
const char var7f1b2f9c[] = "FileMan: Success Handler\n";
const char var7f1b2fb8[] = "Copy Memory Freed\n";
const char var7f1b2fcc[] = ">> block read going write, target file is %x-%x\n";
const char var7f1b3000[] = "SaveElsewhere\n";
const char var7f1b3010[] = "DELETING: %x-%x\n";
const char var7f1b3024[] = "MyResult: %d\n";
const char var7f1b3034[] = "PakOperationSearch>> Search for pak: %x = %d\n";
#endif

void filemgrPushErrorDialog(u16 errnum)
{
	g_Menus[g_MpPlayerNum].fm.errnum = errnum;

	menuPushDialog(&g_FilemgrErrorMenuDialog);
}

struct menuitem g_FilemgrErrorMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&filemgrMenuTextDeviceNameForError,
		0,
		filemgrDeviceNameForErrorMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&filemgrMenuTextFailReason,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrErrorMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Error\n"), // "Error"
	g_FilemgrErrorMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

/**
 * For a file listing, iterate the devices until the one at optionindex is found.
 * Depending on operation, return the device name or index of the first file.
 */
uintptr_t filemgrGetDeviceNameOrStartIndex(s32 listnum, s32 operation, s32 optionindex) // TODO - Lang: Fix it, return value
{
	char *names[] = {
		_("Game Pak\n"), // "Game Pak"
		_("Controller Pak 1\n"), // "Controller Pak 1"
		_("Controller Pak 2\n"), // "Controller Pak 2"
		_("Controller Pak 3\n"), // "Controller Pak 3"
		_("Controller Pak 4\n"), // "Controller Pak 4"
	};

	s32 i;
	s32 remaining = optionindex;

	for (i = 0; i < ARRAYCOUNT(names); i++) {
		if (g_FileLists[listnum]->devicestartindexes[i] != -1) {
			if (remaining == 0) {
				if (operation == MENUOP_GETOPTGROUPTEXT) {
					return (uintptr_t)names[i];
				}

				return g_FileLists[listnum]->devicestartindexes[i];
			}

			remaining--;
		}
	}

	return 0;
}

char *filemgrMenuTextErrorTitle(struct menuitem *item)
{
	char *messages[] = {
		_("Error Loading Game\n"), // "Error Loading Game"
		_("Error Saving Game\n"), // "Error Saving Game"
		_("Error Loading Player\n"), // "Error Loading Player"
		_("Error Saving Player\n"), // "Error Saving Player"
		_("Error Loading PerfectHead\n"), // "Error Loading PerfectHead"
		_("Error Saving PerfectHead\n"), // "Error Saving PerfectHead"
		_("Error Reading File\n"), // "Error Reading File"
		_("Error Writing File\n"), // "Error Writing File"
		_("Error\n"), // "Error"
	};

	switch (g_Menus[g_MpPlayerNum].fm.fileop) {
	case FILEOP_LOAD_GAME:
	case FILEOP_LOAD_MPSETUP:
		return messages[0];
	case FILEOP_SAVE_GAME_000:
	case FILEOP_SAVE_GAME_001:
	case FILEOP_SAVE_GAME_002:
	case FILEOP_SAVE_MPSETUP:
		return messages[1];
	case FILEOP_LOAD_MPPLAYER:
		return messages[2];
	case FILEOP_SAVE_MPPLAYER:
		return messages[3];
	case FILEOP_READ_GAME:
	case FILEOP_READ_MPSETUP:
	case FILEOP_READ_MPPLAYER:
		return messages[6];
	case FILEOP_WRITE_GAME:
	case FILEOP_WRITE_MPSETUP:
	case FILEOP_WRITE_MPPLAYER:
		return messages[7];
	}

	return messages[8];
}

#if VERSION >= VERSION_NTSC_1_0
char *filemgrMenuTextFileType(struct menuitem *item)
{
	char *names[] = {
		_("Single Player Agent File\n"), // "Single Player Agent File"
		_("Combat Simulator Settings File\n"), // "Combat Simulator Settings File"
		_("Combat Simulator Player File\n"), // "Combat Simulator Player File"
		_("PerfectHead Files\n"), // "PerfectHead Files"
	};

	switch (g_Menus[g_MpPlayerNum].fm.fileop) {
	case FILEOP_SAVE_GAME_000:
	case FILEOP_SAVE_GAME_001:
	case FILEOP_SAVE_GAME_002:
	case FILEOP_WRITE_GAME:
	case FILEOP_LOAD_GAME:
	case FILEOP_READ_GAME:
		return names[0];
	case FILEOP_SAVE_MPPLAYER:
	case FILEOP_WRITE_MPPLAYER:
	case FILEOP_LOAD_MPPLAYER:
	case FILEOP_READ_MPPLAYER:
		return names[2];
	}

	return names[0];
}
#endif

void func0f10898c(void)
{
	menuSetBanner(-1, false);

	switch (g_Menus[g_MpPlayerNum].fm.fileop) {
	case FILEOP_WRITE_GAME:
	case FILEOP_WRITE_MPSETUP:
	case FILEOP_WRITE_MPPLAYER:
	case FILEOP_READ_GAME:
	case FILEOP_READ_MPSETUP:
	case FILEOP_READ_MPPLAYER:
		memaFree(g_Menus[g_MpPlayerNum].fm.unke44, align16(g_FileTypeSizes[g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1]));
		break;
	case FILEOP_LOAD_GAME:
	case FILEOP_LOAD_MPPLAYER:
	case FILEOP_LOAD_MPSETUP:
	case FILEOP_103:
	case FILEOP_SAVE_GAME_000:
	case FILEOP_SAVE_GAME_001:
	case FILEOP_SAVE_GAME_002:
	case FILEOP_SAVE_MPPLAYER:
	case FILEOP_SAVE_MPSETUP:
	case FILEOP_005:
		break;
	}
}

void filemgrHandleSuccess(void)
{
	menuSetBanner(-1, false);

	switch (g_Menus[g_MpPlayerNum].fm.fileop) {
	case FILEOP_WRITE_GAME:
	case FILEOP_WRITE_MPSETUP:
	case FILEOP_WRITE_MPPLAYER:
		memaFree(g_Menus[g_MpPlayerNum].fm.unke44,
				align16(g_FileTypeSizes[g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1]));
		break;
	case FILEOP_LOAD_GAME:
		g_Vars.bossfileid = g_Menus[g_MpPlayerNum].fm.fileid;
		g_Vars.bossdeviceserial = g_Menus[g_MpPlayerNum].fm.deviceserial;
		bossfileSave();

		if (IS4MB()) {
			func0f0f820c(&g_MainMenu4MbMenuDialog, MENUROOT_4MBMAINMENU);
		} else {
			func0f0f820c(&g_CiMenuViaPcMenuDialog, MENUROOT_MAINMENU);
		}
		break;
	case FILEOP_READ_GAME:
	case FILEOP_READ_MPSETUP:
	case FILEOP_READ_MPPLAYER:
		filemgrSaveOrLoad(&var800a21e8,
				g_Menus[g_MpPlayerNum].fm.fileop - 98,
				g_Menus[g_MpPlayerNum].fm.mpplayernum);
		break;
	case FILEOP_SAVE_GAME_000:
	case FILEOP_SAVE_GAME_001:
	case FILEOP_SAVE_GAME_002:
	case FILEOP_SAVE_MPPLAYER:
	case FILEOP_SAVE_MPSETUP:
	case FILEOP_005:
	case FILEOP_LOAD_MPPLAYER:
	case FILEOP_LOAD_MPSETUP:
	case FILEOP_103:
		break;
	}
}

MenuItemHandlerResult filemgrRetrySaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		filemgrRetrySave(2);
	}

	return 0;
}

MenuItemHandlerResult filemgrSaveElsewhereYesMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 filetype;

		menuCloseDialog();

		switch (g_Menus[g_MpPlayerNum].fm.fileop) {
		case FILEOP_SAVE_GAME_000:
		case FILEOP_SAVE_GAME_001:
		case FILEOP_SAVE_GAME_002:
		case FILEOP_WRITE_GAME:
			filetype = FILETYPE_GAME;
			break;
		case FILEOP_SAVE_MPPLAYER:
		case FILEOP_WRITE_MPPLAYER:
			filetype = FILETYPE_MPPLAYER;
			break;
		case FILEOP_SAVE_MPSETUP:
		case FILEOP_WRITE_MPSETUP:
			filetype = FILETYPE_MPSETUP;
			break;
		}

		filemgrPushSelectLocationDialog(g_Menus[g_MpPlayerNum].fm.fileop + 9, filetype);
	}

	return 0;
}

MenuItemHandlerResult filemgrCancelSave2MenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuCloseDialog();
		func0f10898c();
		menuUpdateCurFrame();
	}

	return 0;
}

/**
 * Unused.
 */
MenuItemHandlerResult filemgr0f108d14(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		filemgrRetrySave(2);
	}

	return 0;
}

MenuItemHandlerResult filemgrAcknowledgeFileLostMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuCloseDialog();
		func0f10898c();
		menuUpdateCurFrame();
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
void filemgrEraseCorruptFile(void)
{
	s32 device;
	s32 i;

	device = pakFindBySerial(g_Menus[g_MpPlayerNum].fm.deviceserial);

	if (device >= 0) {
		pakDeleteFile(device, g_Menus[g_MpPlayerNum].fm.fileid);
	}

	for (i = 0; i < ARRAYCOUNT(g_FileLists); i++) {
		if (g_FileLists[i]) {
			g_FileLists[i]->timeuntilupdate = 1;
		}
	}

	menuPushDialog(&g_FilemgrFileLostMenuDialog);
}
#endif

MenuDialogHandlerResult filemgrInsertOriginalPakMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog &&
				g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			filemgrRetrySave(0);
		}
	}

	return false;
}

MenuItemHandlerResult filemgrReinsertedOkMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		pakExecuteDebugOperations();
		filemgrRetrySave(1);
	}

	return 0;
}

MenuItemHandlerResult filemgrReinsertedCancelMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		if (FILEOP_IS_SAVE(g_Menus[g_MpPlayerNum].fm.fileop) && g_Menus[g_MpPlayerNum].fm.fileop != FILEOP_SAVE_GAME_001) {
			func0f0f3704(&g_FilemgrSaveElsewhereMenuDialog);
		} else {
			menuPopDialog();
		}
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
char *filemgrMenuTextInsertOriginalPak(struct menuitem *item)
{
	char fullbuffer[100];
	char namebuffer[100];
	s32 i;

	sprintf(namebuffer, filemgrMenuTextFileType(item));

	// Replace first line break in namebuffer with a terminator
	i = 0;

	while (namebuffer[i] != '\0') {
		if (namebuffer[i] == '\n') {
			namebuffer[i] = '\0';
		} else {
			i++;
		}
	}

	// "Please insert the Controller Pak containing your %s into any controller."
	sprintf(fullbuffer, _("Please insert the Controller Pak containing your %s into any controller.\n"), namebuffer);

	textWrap(120, fullbuffer, g_StringPointer, g_CharsHandelGothicSm, g_FontHandelGothicSm);

	return g_StringPointer;
}
#endif

/**
 * Context is:
 * 0 on tick while reinsert dialog is open
 * 1 when user selects OK on reinsert dialog
 * 2 when user selects Retry Save on error dialog
 */
void filemgrRetrySave(s32 context)
{
	s32 device = pakFindBySerial(g_Menus[g_MpPlayerNum].fm.deviceserial);

	if (device == -1) {
		if (context == 1) {
			filemgrPushErrorDialog(FILEERROR_NOPAK);
		}

		if (context == 2) {
			func0f0f3704(&g_PakNotOriginalMenuDialog);
		}
	} else if (filemgrAttemptOperation(device, true)) {
		if (context == 2) {
			g_Menus[g_MpPlayerNum].fm.device1 = device;

			if (FILEOP_IS_SAVE(g_Menus[g_MpPlayerNum].fm.fileop)) {
				filemgrPushErrorDialog(FILEERROR_SAVEFAILED);
			} else {
				filemgrPushErrorDialog(FILEERROR_LOADFAILED);
			}
		} else {
			filemgrSetDevice1BySerial(g_Menus[g_MpPlayerNum].fm.deviceserial);

			if (FILEOP_IS_SAVE(g_Menus[g_MpPlayerNum].fm.fileop)) {
				func0f0f3704(&g_FilemgrSaveErrorMenuDialog);
			} else {
#if VERSION >= VERSION_NTSC_1_0
				filemgrEraseCorruptFile();
#else
				func0f0f3704(&g_FilemgrFileLostMenuDialog);
#endif
			}
		}
	}
}

bool filemgrAttemptOperation(s32 device, bool closeonsuccess)
{
	s32 errnum = 0;
	bool showfilesaved = (g_Menus[g_MpPlayerNum].fm.isretryingsave & 1) != 0;

	const s32 filetypes[] = {
		PAKFILETYPE_GAME,
		PAKFILETYPE_MPSETUP,
		PAKFILETYPE_MPPLAYER,
		PAKFILETYPE_CAMERA,
	};

	s32 newfileid;

	switch (g_Menus[g_MpPlayerNum].fm.fileop) {
	case FILEOP_SAVE_GAME_002:
		showfilesaved = true;
		// fall through
	case FILEOP_SAVE_GAME_000:
	case FILEOP_SAVE_GAME_001:
		errnum = gamefileSave(device,
				g_Menus[g_MpPlayerNum].fm.fileid,
				g_Menus[g_MpPlayerNum].fm.deviceserial);
		break;
	case FILEOP_SAVE_MPPLAYER:
		errnum = mpplayerfileSave(
				(s32) g_Menus[g_MpPlayerNum].fm.unke44, device,
				g_Menus[g_MpPlayerNum].fm.fileid,
				g_Menus[g_MpPlayerNum].fm.deviceserial);
		break;
	case FILEOP_WRITE_GAME:
	case FILEOP_WRITE_MPSETUP:
	case FILEOP_WRITE_MPPLAYER:
		newfileid = 0;
#if VERSION >= VERSION_NTSC_1_0
		func0f0d5690(g_Menus[g_MpPlayerNum].fm.unke44, g_Menus[g_MpPlayerNum].fm.filename);
#endif
		errnum = pakSaveAtGuid(device,
				g_Menus[g_MpPlayerNum].fm.fileid,
				filetypes[g_Menus[g_MpPlayerNum].fm.fileop - 6],
				g_Menus[g_MpPlayerNum].fm.unke44, &newfileid, NULL);
		var80075bd0[g_Menus[g_MpPlayerNum].fm.fileop - 6] = 1;
		break;
	case FILEOP_LOAD_GAME:
		errnum = gamefileLoad(device);
		break;
	case FILEOP_LOAD_MPPLAYER:
		errnum = mpplayerfileLoad(
				(s32) g_Menus[g_MpPlayerNum].fm.unke44,
				device,
				g_Menus[g_MpPlayerNum].fm.fileid,
				g_Menus[g_MpPlayerNum].fm.deviceserial);
		break;
	case FILEOP_READ_GAME:
	case FILEOP_READ_MPSETUP:
	case FILEOP_READ_MPPLAYER:
#if VERSION >= VERSION_NTSC_1_0
		errnum = pakReadBodyAtGuid(device, g_Menus[g_MpPlayerNum].fm.fileid, g_Menus[g_MpPlayerNum].fm.unke44, 0);
#else
		errnum = pakReadBodyAtGuid(device, g_Menus[g_MpPlayerNum].fm.fileid, g_Menus[g_MpPlayerNum].fm.unke44,
				g_FileTypeSizes[g_Menus[g_MpPlayerNum].fm.fileop - FILEOP_READ_GAME]);
#endif
		break;
	}

	if (errnum == 0 && closeonsuccess) {
		menuCloseDialog();
	}

	if (FILEOP_IS_SAVE(g_Menus[g_MpPlayerNum].fm.fileop)) {
		if (errnum == 0) {
			filemgrHandleSuccess();
		}

		if (showfilesaved && errnum == 0) {
			menuPushDialog(&g_FilemgrFileSavedMenuDialog);
		}
	} else {
		if (errnum == 0) {
			filemgrHandleSuccess();
		}
	}

	menuUpdateCurFrame();

	return errnum;
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b3074[] = "FileAttemptOperation - pak %d op %d\n";
const char var7f1b309c[] = ">> blockWrite: file:%x pak:%x\n";
const char var7f1b30bc[] = ">> blockRead: file:%x pak:%x\n";
const char var7f1b30dc[] = "SUCCESS**->%d\n";
const char var7f1b30ec[] = "SUCCESS**->%d\n";
const char var7f1b30fc[] = ">>>>>>>>>>>>> FileWrite: type %d <<<<<<<<<<<<<<<\n";
const char var7f1b3130[] = "!!!!!!!!!!!! Wanted: %d\n";
const char var7f1b314c[] = "&&&&&&&&&&&&&&&&&&&&&&&&&&&PASSED**************\n";
const char var7f1b3180[] = "guid: %x gives pakno: %d\n";
const char var7f1b319c[] = "Invalidating pak %d\n";
const char var7f1b31b4[] = "YOUR TARGET: %x-%x\n";
const char var7f1b31c8[] = "MyResult: %d\n";
const char var7f1b31d8[] = "COULD NOT DELETE\n";
const char var7f1b31ec[] = "Multiplayer %d was using that file...\n";
#endif

/**
 * Save or load the file with the specified GUID.
 * If saving, it's assumed that the file has been saved previously.
 *
 * fileop determines the type of file, as well as whether it's a save or load
 * operation. If set to -1, the previous fileop is reused (eg. for retry).
 *
 * playernum is used when working with MP player files, otherwise ignored.
 *
 * Return true if it worked, otherwise false.
 */
bool filemgrSaveOrLoad(struct fileguid *guid, s32 fileop, uintptr_t playernum)
{
	s32 device;

	if (fileop != -1) {
		// Common case - set new fileop
		g_Menus[g_MpPlayerNum].fm.fileop = fileop;
		g_Menus[g_MpPlayerNum].fm.mpplayernum = playernum;
		g_Menus[g_MpPlayerNum].fm.isretryingsave = 0;
		g_FilemgrLastPakError = 0;
	}

	g_Menus[g_MpPlayerNum].fm.fileid = guid->fileid;
	g_Menus[g_MpPlayerNum].fm.deviceserial = guid->deviceserial;

	if (fileop != -1) {
		// empty
	}

	device = pakFindBySerial(g_Menus[g_MpPlayerNum].fm.deviceserial);

	if (device == -1) {
		// Original pak is no longer connected
		g_Menus[g_MpPlayerNum].fm.isretryingsave |= 1;
		menuPushDialog(&g_PakNotOriginalMenuDialog);
		return false;
	}

	if (filemgrAttemptOperation(device, false) != 0) {
		// Operation failed
		g_Menus[g_MpPlayerNum].fm.isretryingsave |= 1;
		filemgrSetDevice1BySerial(g_Menus[g_MpPlayerNum].fm.deviceserial);

		if (FILEOP_IS_SAVE(g_Menus[g_MpPlayerNum].fm.fileop)) {
			menuPushDialog(&g_FilemgrSaveErrorMenuDialog);
		} else {
			// File couldn't be loaded
#if VERSION >= VERSION_NTSC_1_0
			filemgrEraseCorruptFile();
#else
			menuPushDialog(&g_FilemgrFileLostMenuDialog);
#endif
		}

		return false;
	}

	return true;
}

void filemgrDeleteCurrentFile(void)
{
#if VERSION >= VERSION_JPN_FINAL
	// JPN uses an array for g_FilemgrFileToDelete
	bool error = false;
	s8 device = pakFindBySerial(g_FilemgrFileToDelete[g_MpPlayerNum].deviceserial);
	s32 i;

	if (device >= 0) {
		if (pakDeleteFile(device, g_FilemgrFileToDelete[g_MpPlayerNum].fileid) != 0) {
			error = true;
		}
	} else {
		error = true;
	}

	g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->timeuntilupdate = 1;

	if (error) {
		g_Menus[g_MpPlayerNum].fm.device1 = device;
		filemgrPushErrorDialog(FILEERROR_DELETEFAILED);
	} else {
		// If deleting a loaded MP player, reset them to default
		for (i = 0; i < MAX_PLAYERS; i++) {
			if (g_FilemgrFileToDelete[g_MpPlayerNum].fileid == g_PlayerConfigsArray[i].fileguid.fileid
					&& g_FilemgrFileToDelete[g_MpPlayerNum].deviceserial == g_PlayerConfigsArray[i].fileguid.deviceserial) {
				mpPlayerSetDefaults(i, true);
			}
		}
	}
#else
	bool error = false;
	s8 device = pakFindBySerial(g_FilemgrFileToDelete.deviceserial);
	s32 i;

	if (device >= 0) {
		if (pakDeleteFile(device, g_FilemgrFileToDelete.fileid) != 0) {
			error = true;
		}
	} else {
		error = true;
	}

	g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->timeuntilupdate = 1;

	if (error) {
		g_Menus[g_MpPlayerNum].fm.device1 = device;
		filemgrPushErrorDialog(FILEERROR_DELETEFAILED);
	} else {
		// If deleting a loaded MP player, reset them to default
		for (i = 0; i < MAX_PLAYERS; i++) {
			if (g_FilemgrFileToDelete.fileid == g_PlayerConfigsArray[i].fileguid.fileid
					&& g_FilemgrFileToDelete.deviceserial == g_PlayerConfigsArray[i].fileguid.deviceserial) {
				mpPlayerSetDefaults(i, true);
			}
		}
	}
#endif
}

struct menuitem g_FilemgrFileSavedMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("File Saved.\n"), // "File Saved."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("OK\n"), // "OK"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileSavedMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	gettext_noop("Cool!\n"), // "Cool!"
	g_FilemgrFileSavedMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrSaveErrorMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&filemgrMenuTextDeviceName,
		0,
		filemgrDeviceNameMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("An error occurred while\ntrying to save.\n"), // "An error occurred while trying to save."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Try Again\n"), // "Try Again"
		0,
		filemgrRetrySaveMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Save Elsewhere\n"), // "Save Elsewhere"
		0,
		filemgrSaveElsewhereYesMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		filemgrCancelSave2MenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrSaveErrorMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	&filemgrMenuTextErrorTitle,
	g_FilemgrSaveErrorMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK | MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrFileLostMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&filemgrMenuTextDeviceName,
		0,
		filemgrDeviceNameMenuHandler,
	},
#if VERSION >= VERSION_NTSC_1_0
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("The saved file has\nbeen erased due\nto corruption\nor damage.\n"), // "The saved file has been erased due to corruption or damage."
		0,
		NULL,
	},
#else
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		L_OPTIONS_352, // "An error occurred while trying to load."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_353, // "Try Again"
		0,
		filemgr0f108d14,
	},
#endif
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		filemgrAcknowledgeFileLostMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileLostMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	&filemgrMenuTextErrorTitle,
	g_FilemgrFileLostMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK | MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrSaveElsewhereMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Would you like to save\nyour file elsewhere?\n"), // "Would you like to save your file elsewhere?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Yes\n"), // "Yes"
		0,
		filemgrSaveElsewhereYesMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("No\n"), // "No"
		0,
		filemgrCancelSave2MenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrSaveElsewhereMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Save\n"), // "Save"
	g_FilemgrSaveElsewhereMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK | MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_PakNotOriginalMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
#if VERSION >= VERSION_NTSC_1_0
		&filemgrMenuTextInsertOriginalPak,
#else
		L_OPTIONS_363, // "Please insert the Controller Pak containing your %s into any controller."
#endif
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("OK\n"), // "OK"
		0,
		filemgrReinsertedOkMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		filemgrReinsertedCancelMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakNotOriginalMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	&filemgrMenuTextErrorTitle,
	g_PakNotOriginalMenuItems,
	filemgrInsertOriginalPakMenuDialog,
	MENUDIALOGFLAG_IGNOREBACK | MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

void func0f1097d0(s32 device)
{
	char *typenames[] = {"GAM", "MPG", "MPP", "CAM"};
	const u32 typecodes[] = {0x80, 0x40, 0x20, 0x08};
	void *thing;

	if (g_FileLists[0]) {
		var800a21e8.fileid = g_FileLists[0]->deviceguids[device].fileid;
		var800a21e8.deviceserial = g_FileLists[0]->deviceguids[device].deviceserial;

		thing = memaAlloc(align16(g_FileTypeSizes[g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1]));

		if (thing) {
			filemgrSaveOrLoad(&g_FilemgrFileToCopy, g_Menus[g_MpPlayerNum].fm.filetypeplusone + 103, (uintptr_t) thing);
		} else {
			filemgrPushErrorDialog(FILEERROR_OUTOFMEMORY);
		}

		var80075bd0[g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1] = 1;
	}
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b3234[] = "DestPakNo: %d (guid F:%x-%x:P)\n";
const char var7f1b3254[] = "Copy Memory Alloced\n";
const char var7f1b326c[] = "COULDNT GET THE RAM!\n";
const char var7f1b3284[] = "Saving...\n";
#endif

void filemgrSaveGameToDevice(s32 device)
{
	if (g_FileLists[0]) {
		g_GameFileGuid.fileid = g_FileLists[0]->deviceguids[device].fileid;
		g_GameFileGuid.deviceserial = g_FileLists[0]->deviceguids[device].deviceserial;

		filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_SAVE_GAME_000, 0);
	}
}

#if VERSION >= VERSION_NTSC_1_0
void filemgrGetFileName(char *dst, struct filelistfile *file)
{
	char localbuffer[20];
	u32 playtime;

	localbuffer[0] = '\0';

	switch (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->filetype) {
	case FILETYPE_GAME:
	case FILETYPE_MPSETUP:
		func0f0d564c(file->name, localbuffer, false);
		break;
	case FILETYPE_MPPLAYER:
		mpplayerfileGetOverview(file->name, localbuffer, &playtime);
		break;
	}

	sprintf(dst, "%s", localbuffer);
}
#endif

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b3294[] = "GETFileNameForThePurposesOfTheFileRenamingChecker: Unknown type %d\n";

void filemgrGetRenameName(char *buffer)
{
	s32 i;
	s32 j;

	switch (g_Menus[g_MpPlayerNum].fm.unke3e) {
	case 0:
	case 9:
	case 10:
	case 11:
		strcpy(buffer, g_GameFile.name);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 15:
	case 16:
	case 17:
		strcpy(buffer, g_Menus[g_MpPlayerNum].fm.filename);
		break;
	case 6:
	case 12:
		i = 0;
		j = 0;

		while (g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\0') {
			if (g_PlayerConfigsArray[g_MpPlayerNum].base.name[i] != '\n') {
				buffer[j] = g_PlayerConfigsArray[g_MpPlayerNum].base.name[i];
				j++;
			}

			i++;
		}

		buffer[j] = '\0';
		break;
	case 7:
	case 13:
		strcpy(buffer, g_MpSetup.name);
		break;
	}
}
#endif

#if VERSION >= VERSION_NTSC_1_0
void filemgrSetRenameName(char *name)
{
	switch (g_Menus[g_MpPlayerNum].fm.unke3e) {
	case 0:
	case 9:
	case 10:
	case 11:
		strcpy(g_GameFile.name, name);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 15:
	case 16:
	case 17:
		strcpy(g_Menus[g_MpPlayerNum].fm.filename, name);
		break;
	case 6:
	case 12:
		sprintf(g_PlayerConfigsArray[g_MpPlayerNum].base.name, "%s\n", name);
		break;
	case 7:
	case 13:
		strcpy(g_MpSetup.name, name);
		break;
	}
}

const char var7f1b32dc[] = "SetFileNameForThePurposesOfTheFileRenamingChecker: Unknown type %d\n";
#endif

#if VERSION >= VERSION_NTSC_1_0
bool filemgrIsNameAvailable(s32 device)
{
	static u8 lookup[] = {1, 2, 3, 4, 0};

	struct filelist *filelist;
	char findname[16];
	s32 deviceindex;
	s32 startindex;
	s32 endindex;
	s32 i;
	s32 j;
	char loopname[16];

	deviceindex = lookup[device];
	filelist = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum];

	if (filelist == NULL) {
		return true;
	}

	// Determine which index to start searching at
	startindex = filelist->devicestartindexes[deviceindex];

	if (startindex == -1) {
		return true;
	}

	// Determine which index to stop searching at
	// (ie. start of a new device or end of the list)
	endindex = filelist->numfiles;

	for (i = 4; i > deviceindex; i--) {
		if (filelist->devicestartindexes[i] != -1) {
			endindex = filelist->devicestartindexes[i];
		}
	}

	// Get the filename to search for, make it uppercase and remove trailing line break.
	// @dangerous: findname can overflow if filemgrGetRenameName returns a long name.
	findname[0] = '\0';
	filemgrGetRenameName(findname);

	for (j = 0; findname[j] != '\0';) {
		if (findname[j] >= 'a' && findname[j] <= 'z') {
			findname[j] -= 32;
		}

		if (findname[j] == '\n') {
			findname[j] = '\0';
		} else {
			j++;
		}
	}

	osSyncPrintf("CheckFileName: Comparing range %d-%d\n", startindex, endindex);

	// Iterate files
	for (i = startindex; i < endindex; i++) {
		filemgrGetFileName(loopname, &g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->files[i]);

		// Convert loop filename to uppercase and remove trailing line break
		// @dangerous: loopname can overflow if filemgrGetFileName returns a long name.
		for (j = 0; loopname[j] != '\0';) {
			if (loopname[j] >= 'a' && loopname[j] <= 'z') {
				loopname[j] -= 32;
			}

			if (loopname[j] == '\n') {
				loopname[j] = '\0';
			} else {
				j++;
			}
		}

		// Compare names
		osSyncPrintf("Compare '%s' to '%s' = %d\n", findname, loopname, strcmp(findname, loopname));

		if (strcmp(findname, loopname) == 0) {
			osSyncPrintf("OI! DUPLICATE FILE NAME! NO!\n");
			return false;
		}
	}

	return true;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b3384[] = "()()()()()() Writing MPLAYER\n";
const char var7f1b33a4[] = "()()()()()() Writing MGAME\n";
const char var7f1b33c0[] = "Write Attempt Made...\n";
const char var7f1b33d8[] = "decided location: %d\n";

/**
 * Used for both saving new files and copying files.
 */
void filemgrSaveToDevice(void)
{
	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->spacesfree[g_Menus[g_MpPlayerNum].fm.device2] > 0) {
		if (!filemgrIsNameAvailable(g_Menus[g_MpPlayerNum].fm.device2)) {
			menuPushDialog(&g_FilemgrDuplicateNameMenuDialog);
		} else {
			menuPopDialog();

			if (g_Menus[g_MpPlayerNum].fm.unke3e == 0) {
				filemgrSaveGameToDevice(g_Menus[g_MpPlayerNum].fm.device2);
			} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 5) {
				// empty
			} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 6) {
				struct fileguid guid;
				guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].fileid;
				guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].deviceserial;
				filemgrSaveOrLoad(&guid, FILEOP_SAVE_MPPLAYER, (u32)g_MpPlayerNum);
			} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 7) {
				struct fileguid guid;
				guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].fileid;
				guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].deviceserial;
				filemgrSaveOrLoad(&guid, FILEOP_SAVE_MPSETUP, 0);
			} else if (g_Menus[g_MpPlayerNum].fm.unke3e >= 9) {
				struct fileguid guid;
				guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].fileid;
				guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[g_Menus[g_MpPlayerNum].fm.device2].deviceserial;
				filemgrSaveOrLoad(&guid, -1, 0);
			} else {
				func0f1097d0(g_Menus[g_MpPlayerNum].fm.device2);
			}
		}
	}
}
#endif

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult filemgrConfirmRenameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;

	switch (operation) {
	case MENUOP_GETTEXT:
		filemgrGetRenameName(name);
		break;
	case MENUOP_SETTEXT:
		filemgrSetRenameName(name);
		break;
	case MENUOP_SET:
		filemgrSaveToDevice();
		break;
	}

	return 0;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult filemgrDuplicateRenameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPushDialog(&g_FilemgrRenameMenuDialog);
	}

	return 0;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
MenuItemHandlerResult filemgrDuplicateCancelMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
		menuPopDialog();
	}

	return 0;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *filemgrMenuTextDeviceNameContainingDuplicateFile(struct menuitem *item)
{
	return filemgrGetDeviceName(g_Menus[g_MpPlayerNum].fm.device2);
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *filemgrMenuTextDuplicateFileName(struct menuitem *item)
{
	char buffer[32];

	filemgrGetRenameName(buffer);
	sprintf(g_StringPointer, "%s\n", buffer);

	return g_StringPointer;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
struct menuitem g_FilemgrRenameMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Enter new file name:\n"), // "Enter new file name:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_KEYBOARD,
		0,
		0,
		0,
		0,
		filemgrConfirmRenameMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrRenameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Change File Name\n"), // "Change File Name"
	g_FilemgrRenameMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};
#endif

#if VERSION >= VERSION_NTSC_1_0
struct menuitem g_FilemgrDuplicateMenuMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		&filemgrMenuTextDeviceNameContainingDuplicateFile,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		gettext_noop("already contains\n"), // "already contains"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		gettext_noop("a file named\n"), // "a file named"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_LESSHEIGHT,
		&filemgrMenuTextDuplicateFileName,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Rename File\n"), // "Rename File"
		0,
		filemgrDuplicateRenameMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		gettext_noop("Change Location\n"), // "Change Location"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		filemgrDuplicateCancelMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrDuplicateNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Duplicate File Name\n"), // "Duplicate File Name"
	g_FilemgrDuplicateMenuMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};
#endif

char *filemgrMenuTextLocationName2(struct menuitem *item)
{
	char *names[] = {
		_("Controller Pak 1\n"), // "Controller Pak 1"
		_("Controller Pak 2\n"), // "Controller Pak 2"
		_("Controller Pak 3\n"), // "Controller Pak 3"
		_("Controller Pak 4\n"), // "Controller Pak 4"
		_("Game Pak\n"), // "Game Pak"
		"", // ""
	};

	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum] == NULL) {
		return NULL;
	}

#if VERSION >= VERSION_NTSC_1_0
	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->spacesfree[item->param] < 0) {
		return names[5];
	}
#else
	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->spacesfree[item->param] <= 0) {
		return langGet(names[5]);
	}
#endif

	return names[item->param];
}

char *filemgrMenuTextSaveLocationSpaces(struct menuitem *item)
{
	s32 spacesfree;

	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum] == NULL) {
		return NULL;
	}

	spacesfree = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->spacesfree[item->param];

	if (spacesfree < 0) {
		return "\n";
	}

	if (spacesfree == 0) {
		return _("Full"); // "Full"
	}

	sprintf(g_StringPointer, "%d", spacesfree);
	return g_StringPointer;
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b33fc[] = "GOT OKed!, item->data = %d\n";
const char var7f1b3418[] = "GOT CANCELLED!\n";
const char var7f1b3428[] = "Picking Location, type %d wadtype %d wad %d\n";
const char var7f1b3458[] = "Torching file %d\n";
const char var7f1b346c[] = "Copying file %d\n";
const char var7f1b3480[] = "item: %x\n";
const char var7f1b348c[] = "Switched Wads Back\n";
const char var7f1b34a0[] = "MenuClosed\n";
const char var7f1b34ac[] = "Deleting files, wad %d\n";
#endif

/**
 * item->param is a SAVEDEVICE constant.
 */
MenuItemHandlerResult filemgrSelectLocationMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum] == NULL) {
		return 0;
	}

	if (operation == MENUOP_CHECKDISABLED) {
		if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->spacesfree[item->param] < 1) {
			return true;
		}
	}

	if (operation == MENUOP_SET) {
#if VERSION >= VERSION_NTSC_1_0
		g_Menus[g_MpPlayerNum].fm.device2 = item->param;
		filemgrSaveToDevice();
#else
		menuPopDialog();

		if (g_Menus[g_MpPlayerNum].fm.unke3e == 0) {
			filemgrSaveGameToDevice(item->param);
		} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 5) {
			// empty
		} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 6) {
			struct fileguid guid;
			guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].fileid;
			guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].deviceserial;
			filemgrSaveOrLoad(&guid, FILEOP_SAVE_MPPLAYER, (u32)g_MpPlayerNum);
		} else if (g_Menus[g_MpPlayerNum].fm.unke3e == 7) {
			struct fileguid guid;
			guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].fileid;
			guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].deviceserial;
			filemgrSaveOrLoad(&guid, FILEOP_SAVE_MPSETUP, 0);
		} else if (g_Menus[g_MpPlayerNum].fm.unke3e >= 9) {
			struct fileguid guid;
			guid.fileid = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].fileid;
			guid.deviceserial = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->deviceguids[item->param].deviceserial;
			filemgrSaveOrLoad(&guid, -1, 0);
		} else {
			func0f1097d0(item->param);
		}
#endif
	}

	return 0;
}

MenuItemHandlerResult filemgrCancelSaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPopDialog();
	}

	return 0;
}

MenuItemHandlerResult filemgrDeleteFilesForSaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		filemgrPushDeleteFileDialog(g_Menus[g_MpPlayerNum].fm.listnum);
		g_Menus[g_MpPlayerNum].fm.isdeletingforsave = true;
	}

	return 0;
}

void filemgrPushSelectLocationDialog(s32 arg0, u32 filetype)
{
	g_Menus[g_MpPlayerNum].fm.unke3e = arg0;
	g_Menus[g_MpPlayerNum].fm.listnum = filelistFindOrCreate(filetype);

#if VERSION >= VERSION_NTSC_1_0
	filelistsTick();
#endif

	menuPushDialog(&g_FilemgrSelectLocationMenuDialog);
}

#if VERSION >= VERSION_JPN_FINAL
bool filemgrIsFileInUse(struct filelistfile *file)
{
	s32 i;

	if (menuIsDialogOpen(&g_FilemgrCopyMenuDialog)
			&& file->fileid == g_FilemgrFileToCopy.fileid
			&& file->deviceserial == g_FilemgrFileToCopy.deviceserial) {
		return true;
	}

	if (menuIsDialogOpen(&g_FilemgrFileSelect4MbMenuDialog)) {
		return false;
	}

	if (g_MenuData.root == MENUROOT_FILEMGR) {
		return false;
	}

	if (file->fileid == g_GameFileGuid.fileid && file->deviceserial == g_GameFileGuid.deviceserial) {
		return true;
	}

	if (file->fileid == g_MpSetup.fileguid.fileid && file->deviceserial == g_MpSetup.fileguid.deviceserial) {
		return true;
	}

	// JPN adds this loop
	for (i = 0; i < ARRAYCOUNT(g_Menus); i++) {
		if (i != g_MpPlayerNum
				&& g_Menus[i].curdialog
				&& g_Menus[i].curdialog->definition == &g_FilemgrConfirmDeleteMenuDialog
				&& file->fileid == g_FilemgrFileToDelete[i].fileid
				&& file->deviceserial == g_FilemgrFileToDelete[i].deviceserial) {
			return true;
		}
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		if ((g_MpSetup.chrslots & (1 << i))
				&& g_PlayerConfigsArray[i].fileguid.fileid == file->fileid
				&& g_PlayerConfigsArray[i].fileguid.deviceserial == file->deviceserial) {
			return true;
		}
	}

	return false;
}
#endif

MenuItemHandlerResult filemgrConfirmDeleteMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
#if VERSION >= VERSION_JPN_FINAL
		struct filelistfile file;
		file.fileid = g_FilemgrFileToDelete[g_MpPlayerNum].fileid;
		file.deviceserial = g_FilemgrFileToDelete[g_MpPlayerNum].deviceserial;

		menuPopDialog();

		if (filemgrIsFileInUse(&file)) {
			filemgrPushErrorDialog(FILEERROR_DELETEFAILED);
		} else {
			filemgrDeleteCurrentFile();
		}
#else
		menuPopDialog();
		filemgrDeleteCurrentFile();
#endif
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
char *filemgrMenuTextFileInUseDescription(struct menuitem *item)
{
	if (menuIsDialogOpen(&g_FilemgrCopyMenuDialog)) {
		return _("The file you are copying\ncannot be deleted.\n"); // "The file you are copying cannot be deleted."
	}

	return _("Cannot delete file as\nit is being used.\n"); // "Cannot delete file as it is being used."
}
#endif

/**
 * This is a dirty decomp hack where we intentionally declare
 * an incorrect function signature in order to get a match.
 * pheadGetTexture uses u16 as its last argument
 * but filemgrRenderPerfectHeadThumbnail will only match if
 * it's an s32 with a 0xffff mask.
 */
struct textureconfig *pheadGetTexture(s32 playernum, s32 fileid, s32 deviceserial);

Gfx *filemgrRenderPerfectHeadThumbnail(Gfx *gdl, struct menuitemrenderdata *renderdata, s32 fileid, s32 deviceserial)
{
	struct textureconfig *texture = pheadGetTexture(g_MpPlayerNum, fileid, deviceserial & 0xffff);

	if (texture) {
		gSPDisplayList(gdl++, &var800613a0);
		gSPDisplayList(gdl++, &var80061360);

		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);

		texSelect(&gdl, texture, 1, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetTextureFilter(gdl++, G_TF_POINT);
		gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, renderdata->colour);

		gDPSetCombineLERP(gdl++,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);

		gDPLoadSync(gdl++);
		gDPTileSync(gdl++);

		gSPTextureRectangle(gdl++,
				((renderdata->x + 4) << 2) * g_ScaleX,
				(renderdata->y + 2) << 2,
				((renderdata->x + 20) << 2) * g_ScaleX,
				(renderdata->y + 18) << 2,
				G_TX_RENDERTILE, 0, 512, 1024 / g_ScaleX, -1024);

		gDPLoadSync(gdl++);
		gDPTileSync(gdl++);
		gDPPipeSync(gdl++);

		if (deviceserial) {
			// empty
		}
	}

	return gdl;
}

#if VERSION < VERSION_JPN_FINAL
bool filemgrIsFileInUse(struct filelistfile *file)
{
	s32 i;

#if VERSION >= VERSION_NTSC_1_0
	if (menuIsDialogOpen(&g_FilemgrCopyMenuDialog)
			&& file->fileid == g_FilemgrFileToCopy.fileid
			&& file->deviceserial == g_FilemgrFileToCopy.deviceserial) {
		return true;
	}

	if (menuIsDialogOpen(&g_FilemgrFileSelect4MbMenuDialog)) {
		return false;
	}
#else
	if (g_MenuData.root == MENUROOT_FILEMGR
			&& menuIsDialogOpen(&g_FilemgrCopyMenuDialog)
			&& file->fileid == g_FilemgrFileToCopy.fileid
			&& file->deviceserial == g_FilemgrFileToCopy.deviceserial) {
		return true;
	}
#endif

	if (g_MenuData.root == MENUROOT_FILEMGR) {
		return false;
	}

	if (file->fileid == g_GameFileGuid.fileid && file->deviceserial == g_GameFileGuid.deviceserial) {
		return true;
	}

	if (file->fileid == g_MpSetup.fileguid.fileid && file->deviceserial == g_MpSetup.fileguid.deviceserial) {
		return true;
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		if ((g_MpSetup.chrslots & (1 << i))
				&& g_PlayerConfigsArray[i].fileguid.fileid == file->fileid
				&& g_PlayerConfigsArray[i].fileguid.deviceserial == file->deviceserial) {
			return true;
		}
	}

	return false;
}
#endif

/**
 * item->param is 0 if copying, 1 if deleting.
 */
MenuItemHandlerResult filemgrFileToCopyOrDeleteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data, bool isdelete)
{
	s32 x;
	s32 y;
	struct filelist *list = g_FileLists[0];
	s32 listnum = 0;

	if (item->param == 1) {
		listnum = g_Menus[g_MpPlayerNum].fm.listnum;
		list = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum];
	}

	if (list == NULL) {
		return 0;
	}

	switch (operation) {
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0x0fffff;
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = list->numfiles;
		break;
	case MENUOP_RENDER:
		{
			Gfx *gdl = data->type19.gdl;
			struct menuitemrenderdata *renderdata = data->type19.renderdata2;
			struct filelistfile *file = &list->files[data->list.unk04];

			if (g_Menus[g_MpPlayerNum].fm.filetypeplusone == 4) {
				gdl = filemgrRenderPerfectHeadThumbnail(gdl, renderdata, file->fileid, file->deviceserial);
			} else {
				u32 colour = renderdata->colour;
				char text[32];

				if (isdelete && filemgrIsFileInUse(file)) {
					colour = 0xff333300 | (colour & 0xff);
				}

				x = renderdata->x + 2;
				y = renderdata->y + 2;

				gdl = text0f153628(gdl);

				if (file) {
					filemgrGetSelectName(text, file, g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1);
					gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicSm, g_FontHandelGothicSm,
							colour, viGetWidth(), viGetHeight(), 0, 1);
					y = renderdata->y + 12;
					x = renderdata->x + 2;
				}

				gdl = text0f153780(gdl);
			}

			return (uintptr_t) gdl;
		}
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = 11;
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = list->numdevices;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		return filemgrGetDeviceNameOrStartIndex(listnum, operation, data->list.value);
	case MENUOP_GETGROUPSTARTINDEX:
		data->list.groupstartindex = filemgrGetDeviceNameOrStartIndex(listnum, operation, data->list.value);
		return 0;
	}

	return 0;
}

MenuItemHandlerResult filemgrFileToDeleteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum] == NULL) {
		return 0;
	}

	if (operation == MENUOP_SET) {
		struct filelistfile *file = &g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->files[data->list.value];

		if (file) {
			if (filemgrIsFileInUse(file)) {
				filemgrSetFileToDelete(file, g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->filetype);
				menuPushDialog(&g_FilemgrFileInUseMenuDialog);
			} else {
				filemgrSetFileToDelete(file, g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum]->filetype);
#if VERSION >= VERSION_JPN_FINAL
				g_FilemgrFileToDelete[g_MpPlayerNum].fileid = file->fileid;
				g_FilemgrFileToDelete[g_MpPlayerNum].deviceserial = file->deviceserial;
#else
				g_FilemgrFileToDelete.fileid = file->fileid;
				g_FilemgrFileToDelete.deviceserial = file->deviceserial;
#endif
				menuPushDialog(&g_FilemgrConfirmDeleteMenuDialog);
			}
		}
	}

	return filemgrFileToCopyOrDeleteListMenuHandler(operation, item, data, true);
}

MenuItemHandlerResult filemgrFileToCopyListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct filelist *list = g_FileLists[0];

	if (list == NULL) {
		return 0;
	}

	if (operation == MENUOP_SET) {
		struct filelistfile *file = &list->files[data->list.value];

		if (file) {
			g_FilemgrFileToCopy.fileid = file->fileid;
			g_FilemgrFileToCopy.deviceserial = file->deviceserial;

#if VERSION >= VERSION_NTSC_1_0
			filemgrGetFileName(g_Menus[g_MpPlayerNum].fm.filename, file);
#endif
			filemgrPushSelectLocationDialog(g_Menus[g_MpPlayerNum].fm.filetypeplusone, g_Menus[g_MpPlayerNum].fm.filetypeplusone - 1);
		}
	}

	return filemgrFileToCopyOrDeleteListMenuHandler(operation, item, data, false);
}

MenuDialogHandlerResult filemgrCopyOrDeleteListMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_CLOSE) {
		if (g_Menus[g_MpPlayerNum].fm.isdeletingforsave == true) {
			g_Menus[g_MpPlayerNum].fm.isdeletingforsave = false;
		} else {
			filelistCreate(0, FILETYPE_GAME);
			g_Menus[g_MpPlayerNum].fm.filetypeplusone = 0;
		}
	}

	return 0;
}

void filemgrPushDeleteFileDialog(s32 listnum)
{
	struct filelist *list;

	g_Menus[g_MpPlayerNum].fm.listnum = listnum;
	g_Menus[g_MpPlayerNum].fm.isdeletingforsave = false;
	g_Menus[g_MpPlayerNum].fm.filetypeplusone = 1;

	list = g_FileLists[g_Menus[g_MpPlayerNum].fm.listnum];

	if (list) {
		g_Menus[g_MpPlayerNum].fm.filetypeplusone = list->filetype + 1;
	}

	menuPushDialog(&g_FilemgrDeleteMenuDialog);
}

MenuItemHandlerResult pakDeleteGameNoteMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		OSPfsState *note = &g_EditingPak->notes[g_Menus[g_MpPlayerNum].fm.noteindex];
		s32 result;

		g_Menus[g_MpPlayerNum].fm.unke24 = g_Menus[g_MpPlayerNum].fm.unke24 | (1 << g_Menus[g_MpPlayerNum].fm.device);

		menuPopDialog();

		result = pakDeleteGameNote(g_Menus[g_MpPlayerNum].fm.device,
				note->company_code, note->game_code, note->game_name, note->ext_name);

		g_Menus[g_MpPlayerNum].fm.device1 = g_Menus[g_MpPlayerNum].fm.device;

		if (result != PAK_ERR1_OK) {
			filemgrPushErrorDialog(FILEERROR_DELETENOTEFAILED);
		}
	}

	return 0;
}

/**
 * Handler for the note listing in the controller pak menu.
 */
MenuItemHandlerResult pakGameNoteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 x;
	s32 y;
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	OSPfsState *note;
#if VERSION >= VERSION_JPN_FINAL
	char tmpname[52];
	char tmpext[20];
	char generalbuffer[80];
	char extbuffer[80];
	char pagesbuffer[80];
#elif VERSION >= VERSION_NTSC_1_0
	char tmpname[40];
	char tmpext[12];
	char generalbuffer[60];
	char extbuffer[60];
	char pagesbuffer[60];
#else
	char tmpname[20];
	char tmpext[8];
	char generalbuffer[28];
	char extbuffer[28];
	char pagesbuffer[28];
#endif
	s32 textwidth;
	s32 textheight;

	if (g_EditingPak == NULL) {
		return 0;
	}

	switch (operation) {
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0x0fffff;
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = 16;
		break;
	case MENUOP_RENDER:
		gdl = data->type19.gdl;
		renderdata = data->type19.renderdata2;
		note = &g_EditingPak->notes[data->list.unk04];

		// Render note number (1-16)
		sprintf(generalbuffer, "%d:\n", data->list.unk04 + 1);
		x = renderdata->x + 4;
		y = renderdata->y + 1;
		gdl = textRenderProjected(gdl, &x, &y, generalbuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);

		// Prepare buffers for remaining text
		if (g_EditingPak->notesinuse[data->list.unk04] == 1) {
			pakN64FontCodeToAscii(note->game_name, tmpname, 16);
			pakN64FontCodeToAscii(note->ext_name, tmpext, 4);

			tmpext[1] = '\0';

			sprintf(generalbuffer, "%s\n", tmpname);
			sprintf(extbuffer, "%s\n", tmpext);
			sprintf(pagesbuffer, "%d\n", note->file_size / 256);
		} else {
			sprintf(generalbuffer, _("Empty\n")); // "Empty"
			sprintf(pagesbuffer, _("--\n")); // "--"
			sprintf(extbuffer, "", tmpname, tmpext);
		}

		// Render note name
		x = renderdata->x + 20;
		y = renderdata->y + 1;
		gdl = textRenderProjected(gdl, &x, &y, generalbuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);

		// Render ext character (for when a game has multiple notes)
		x = renderdata->x + (VERSION == VERSION_JPN_FINAL ? 220 : 190);
		y = renderdata->y + 1;
		gdl = textRenderProjected(gdl, &x, &y, extbuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);

		// Render number of pages
		textMeasure(&textheight, &textwidth, pagesbuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);

		x = renderdata->x + renderdata->width - textwidth - 6;
		y = renderdata->y + 1;
		gdl = textRenderProjected(gdl, &x, &y, pagesbuffer, g_CharsHandelGothicSm, g_FontHandelGothicSm,
				renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);

		return (uintptr_t) gdl;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = VERSION == VERSION_JPN_FINAL ? LINEHEIGHT - 1 : LINEHEIGHT;
		break;
	case MENUOP_SET:
		if (g_EditingPak->notesinuse[data->list.value] == true) {
			g_Menus[g_MpPlayerNum].fm.noteindex = data->list.value;
			menuPushDialog(&g_PakDeleteNoteMenuDialog);
		}
		break;
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
const char var7f1b34dc[] = "GOT OKed!\n";
const char var7f1b34e8[] = "Try to find last opened file...\n";
#endif

/**
 * Controller pak note listing dialog.
 */
MenuDialogHandlerResult pakGameNotesMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_TICK) {
		if (g_Menus[g_MpPlayerNum].curdialog
				&& g_Menus[g_MpPlayerNum].curdialog->definition == dialogdef) {
			PakErr1 ret = pak0f1168c4(g_Menus[g_MpPlayerNum].fm.device, &g_EditingPak);

			if (ret != PAK_ERR1_OK) {
				menuCloseDialog();
				g_EditingPak = NULL;
				g_Menus[g_MpPlayerNum].fm.device1 = g_Menus[g_MpPlayerNum].fm.device;

				if (ret == PAK_ERR1_NOPAK) {
					filemgrPushErrorDialog(FILEERROR_PAKREMOVED);
				}

				menuUpdateCurFrame();
			}
		}
	}

	return 0;
}

char *pakMenuTextPagesFree(struct menuitem *item)
{
	if (g_EditingPak == NULL) {
		sprintf(g_StringPointer, _("Pages Free: \n")); // "Pages Free: "
	} else {
		sprintf(g_StringPointer, _("Pages Free: %d\n"), g_EditingPak->pagesfree); // "Pages Free: %d"
	}

	return g_StringPointer;
}

char *pakMenuTextPagesUsed(struct menuitem *item)
{
	if (g_EditingPak == NULL) {
		sprintf(g_StringPointer2, _("Pages Used: \n")); // "Pages Used: "
	} else {
		sprintf(g_StringPointer2, _("Pages Used: %d\n"), g_EditingPak->pagesused); // "Pages Used: %d"
	}

	return g_StringPointer2;
}

char *pakMenuTextStatusMessage(struct menuitem *item)
{
	ubool haspdnote = false;
	ubool hasemptynote = false;
	s32 i;

	if (g_EditingPak == NULL) {
		return _("Perfect Dark note already exists\non this Controller Pak.\n"); // "Perfect Dark note already exists on this Controller Pak."
	}

	for (i = 0; i < ARRAYCOUNT(g_EditingPak->notes); i++) {
		if (g_EditingPak->notesinuse[i] == true) {
			if (g_EditingPak->notes[i].company_code == ROM_COMPANYCODE
					&& g_EditingPak->notes[i].game_code == ROM_GAMECODE) {
				haspdnote = true;
			}
		} else {
			hasemptynote = true;
		}
	}

	if (haspdnote) {
		return _("Perfect Dark note already exists\non this Controller Pak.\n"); // "Perfect Dark note already exists on this Controller Pak."
	}

	if (g_EditingPak->pagesfree < 28 || !hasemptynote) {
		return _("Controller Pak is too full to save note- 1 note\nand 28 pages required to save to Controller Pak.\n"); // "Controller Pak is too full to save note- 1 note and 28 pages required to save to Controller Pak."
	}

	return _("There is enough space for\nPerfect Dark note.\n"); // "There is enough space for Perfect Dark note."
}

/**
 * Title for the note listing dialog.
 */
char *pakMenuTextEditingPakName(struct menuitem *item)
{
	return filemgrGetDeviceName(g_Menus[g_MpPlayerNum].fm.device);
}

/**
 * A pak selection on the controller pak menu.
 *
 * Selecting one takes you to the note listing dialog.
 */
MenuItemHandlerResult pakSelectionMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_CHECKDISABLED) {
		if (!mempakIsOkay((s8)item->param)) {
			return true;
		}
	}

	if (operation == MENUOP_SET) {
		g_Menus[g_MpPlayerNum].fm.device = item->param;
		menuPushDialog(&g_PakGameNotesMenuDialog);
	}

	return 0;
}

/**
 * Controller pak menu.
 *
 * Lists the connected controller paks.
 */
MenuDialogHandlerResult pakChoosePakMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
#if VERSION >= VERSION_NTSC_1_0
	switch (operation) {
	case MENUOP_OPEN:
		joySetPfsPollInterval(3);
		g_Menus[g_MpPlayerNum].fm.unke24 = 0;
		break;
	case MENUOP_TICK:
		var80062944 = 1;
		break;
	case MENUOP_CLOSE:
		if (g_Vars.stagenum != STAGE_BOOTPAKMENU) {
			s32 i;

			for (i = 0; i < MAX_PLAYERS; i++) {
				if (g_Menus[g_MpPlayerNum].fm.unke24 & (1 << i)) {
					g_Vars.pakstocheck &= 0xfff0;
					g_Vars.pakstocheck |= 0x0008;
					g_Vars.pakstocheck |= 1 << (i + 8);
				}
			}
		}
		joySetDefaultPfsPollInterval();
		break;
	}
#else
	switch (operation) {
	case MENUOP_OPEN:
		joySetPfsPollInterval(3);
		joySetPfsPollInterval(-1);
		g_Menus[g_MpPlayerNum].fm.unke24 = 0;
		break;
	case MENUOP_TICK:
		var80062944 = 1;
		break;
	case MENUOP_CLOSE:
		{
			s32 i;

			for (i = 0; i < MAX_PLAYERS; i++) {
				if (g_Menus[g_MpPlayerNum].fm.unke24 & (1 << i)) {
					pak0f1169c8(i, STAGE_BOOTPAKMENU == g_Vars.stagenum);
				}
			}
		}
		joySetDefaultPfsPollInterval();
		pak0f1189d0();
		break;
	}
#endif

	return 0;
}

MenuItemHandlerResult filemgrOpenCopyFileMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Menus[g_MpPlayerNum].fm.filetypeplusone = item->param + 1;
		filelistCreate(0, item->param);

#if VERSION >= VERSION_NTSC_1_0
		g_Menus[g_MpPlayerNum].fm.listnum = 0;
#endif
		g_Menus[g_MpPlayerNum].fm.isdeletingforsave = false;

		menuPushDialog(&g_FilemgrCopyMenuDialog);
	}

	return 0;
}

MenuItemHandlerResult filemgrOpenDeleteFileMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		g_Menus[g_MpPlayerNum].fm.filetypeplusone = item->param + 1;
		filelistCreate(0, item->param);
		g_Menus[g_MpPlayerNum].fm.unke3e = -1;
		filemgrPushDeleteFileDialog(0);
	}

	return 0;
}

MenuItemHandlerResult filemgrAgentNameKeyboardMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;

	if (!g_FileLists[0]) {
		return 0;
	}

	switch (operation) {
	case MENUOP_GETTEXT:
		strcpy(name, g_GameFile.name);
		break;
	case MENUOP_SETTEXT:
		strcpy(g_GameFile.name, name);
		break;
	case MENUOP_SET:
		filemgrPushSelectLocationDialog(0, FILETYPE_GAME);
		g_Menus[g_MpPlayerNum].fm.unke2c = 1;
		break;
	}

	return 0;
}

MenuItemHandlerResult filemgrChooseAgentListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 x;
	s32 y;
	s32 pass;
	s32 i;
	s32 j;
	Gfx *gdl;
	struct menuitemrenderdata *renderdata;
	s32 texturenum;
	struct filelistfile *file;
	char name[12];
	u8 stage;
	u8 difficulty;
	u32 time;
	s32 days;
	s32 hours;
	s32 minutes;
	s32 seconds;
	char buffer[100];
	s32 textwidth;
	s32 textheight;

	if (g_FileLists[0] == NULL) {
		return 0;
	}

	switch (operation) {
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0x0fffff;
		break;
	case MENUOP_25:
		pass = false;

		if (data->list.unk04 == 1) {
			if (data->list.groupstartindex == 1 && g_Menus[g_MpPlayerNum].fm.unke2c == 1) {
				for (i = 0; i < g_FileLists[0]->numfiles; i++) {
					if (g_GameFileGuid.fileid == g_FileLists[0]->files[i].fileid
							&& g_GameFileGuid.deviceserial == g_FileLists[0]->files[i].deviceserial) {
						data->list.value = i;
					}
				}

				g_Menus[g_MpPlayerNum].fm.unke2c = 0;
			}

			if (g_FileLists[0]->updatedthisframe) {
				pass = true;
			}
		} else {
			pass = true;
			g_Menus[g_MpPlayerNum].fm.unke2c = 0;
		}

		if (pass && g_Vars.bossfileid) {
			for (j = 0; j < g_FileLists[0]->numfiles; j++) {
				if (g_Vars.bossfileid == g_FileLists[0]->files[j].fileid
						&& g_Vars.bossdeviceserial == g_FileLists[0]->files[j].deviceserial) {
					data->list.value = j;
					g_Vars.bossfileid = 0;
				}
			}
		}

#ifndef PLATFORM_N64
		if (g_FileAutoSelect >= 0 && g_FileLists[0] && g_FileLists[0]->numfiles > 0) {
			const u32 tmp = data->list.value;
			data->list.value = g_FileAutoSelect;
			g_FileAutoSelect = -1;
			filemgrChooseAgentListMenuHandler(MENUOP_SET, item, data);
			data->list.value = tmp;
		}
#endif
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = g_FileLists[0]->numfiles + 1;
		break;
	case MENUOP_RENDER:
		gdl = data->type19.gdl;
		texturenum = 12;
		file = NULL;
		renderdata = data->type19.renderdata2;
		seconds = 0;
		minutes = 0;
		hours = 0;
		days = 0;

		if (data->list.unk04 != g_FileLists[0]->numfiles) {
			file = &g_FileLists[0]->files[data->list.unk04];

			if (file) {
				gamefileGetOverview(file->name, name, &stage, &difficulty, &time);

				seconds = time % 60;
				time = time / 60;

				// The stage returned by the above function is offset by 1.
				// 0 = New recruit
				// 1 = Defection
				// ...
				// This is correctly capping it to Skedar Ruins
				if (stage > SOLOSTAGEINDEX_SKEDARRUINS + 1) {
					stage = SOLOSTAGEINDEX_SKEDARRUINS + 1;
				}

				if (difficulty > DIFF_PA) {
					difficulty = DIFF_PA;
				}

				texturenum = stage + 12;

				days = time / 1440;
				hours = (time - days * 1440) / 60;
				minutes = (time - days * 1440) - hours * 60;
			}
		}

		gDPPipeSync(gdl++);
		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);

		texSelect(&gdl, &g_TexGeneralConfigs[texturenum], 2, 0, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetTextureFilter(gdl++, G_TF_POINT);
		gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, renderdata->colour);
		gDPSetCombineLERP(gdl++,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
				TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);

		gSPTextureRectangle(gdl++,
				((renderdata->x + 4) << 2) * g_ScaleX,
				(renderdata->y + 2) << 2,
				((renderdata->x + 60) << 2) * g_ScaleX,
				(renderdata->y + 38) << 2,
				G_TX_RENDERTILE, 0, 1152, 1024 / g_ScaleX, -1024);

		x = renderdata->x + 62;
		y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 3 : 4);
		gdl = text0f153628(gdl);

		if (data->list.unk04 == g_FileLists[0]->numfiles) {
			// "New Agent..."
			gdl = textRenderProjected(gdl, &x, &y, _("New Agent...\n"),
					g_CharsHandelGothicMd, g_FontHandelGothicMd, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);
		} else if (file) {
			// Render file name
			gdl = textRenderProjected(gdl, &x, &y, name,
					g_CharsHandelGothicMd, g_FontHandelGothicMd, renderdata->colour, viGetWidth(), viGetHeight(), 0, 1);

			// Prepare and render stage name
			y = renderdata->y + (VERSION == VERSION_JPN_FINAL ? 16 : 18);
			x = renderdata->x + 62;

			if (stage > 0) {
				sprintf(buffer, "%s %s",
						_(g_SoloStages[stage - 1].name1),
						_(g_SoloStages[stage - 1].name2));
			} else {
				// "New Recruit"
				strcpy(buffer, _("New Recruit"));
			}

			strcat(buffer, "\n");
			gdl = textRenderProjected(gdl, &x, &y, buffer,
					g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

			// Prepare and render mission time
			x = renderdata->x + 62;
			y++;

#if VERSION == VERSION_JPN_FINAL
			x -= 3;
#endif

			if (days > 0) {
				// "Mission Time:"
				sprintf(buffer, "%s %d:%02d:%02d", _("Mission Time:"), days, hours, minutes);
			} else {
				// "Mission Time:"
				sprintf(buffer, "%s %02d:%02d", _("Mission Time:"), hours, minutes);
			}

			// Useless - textwidth and textheight are not used
#if VERSION >= VERSION_JPN_FINAL
			textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, -1);
#else
			textMeasure(&textheight, &textwidth, buffer, g_CharsHandelGothicSm, g_FontHandelGothicSm, 0);
#endif

			gdl = textRenderProjected(gdl, &x, &y, buffer,
					g_CharsHandelGothicSm, g_FontHandelGothicSm, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);

			// Render seconds part of mission time (uses a smaller font)
			y += (VERSION == VERSION_JPN_FINAL) ? 3 : 1;
			x++;
			sprintf(buffer, ".%02d", seconds);
			gdl = textRenderProjected(gdl, &x, &y, buffer,
					g_CharsHandelGothicXs, g_FontHandelGothicXs, renderdata->colour, viGetWidth(), viGetHeight(), 0, 0);
		}
		gdl = text0f153780(gdl);
		return (uintptr_t) gdl;
	case MENUOP_GETOPTIONHEIGHT:
		data->list.value = 40;
		break;
	case MENUOP_SET:
		if (data->list.value == g_FileLists[0]->numfiles) {
			// New agent
			gamefileLoadDefaults(&g_GameFile);
			menuPushDialog(&g_FilemgrEnterNameMenuDialog);
		} else {
			struct filelistfile *file = &g_FileLists[0]->files[data->list.value];

			if (file) {
				g_GameFileGuid.fileid = file->fileid;
				g_GameFileGuid.deviceserial = file->deviceserial;
				filemgrSaveOrLoad(&g_GameFileGuid, FILEOP_LOAD_GAME, 0);
			}
		}
		break;
	case MENUOP_GETOPTGROUPCOUNT:
		data->list.value = g_FileLists[0]->numdevices + 1;
		break;
	case MENUOP_GETOPTGROUPTEXT:
		if (data->list.value >= g_FileLists[0]->numdevices) {// TODO - Lang: Fix it, return value
			return (uintptr_t)_("New...\n"); // "New..."
		}
		return filemgrGetDeviceNameOrStartIndex(0, operation, data->list.value);
	case MENUOP_GETGROUPSTARTINDEX:
		if (data->list.value >= g_FileLists[0]->numdevices) {
			data->list.groupstartindex = g_FileLists[0]->numfiles;
		} else {
			data->list.groupstartindex = filemgrGetDeviceNameOrStartIndex(0, operation, data->list.value);
		}
		return 0;
	}

	return 0;
}

MenuDialogHandlerResult filemgrMainMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	s32 i;

	switch (operation) {
	case MENUOP_OPEN:
		g_Menus[g_MpPlayerNum].fm.filetypeplusone = 0;

		filelistCreate(0, FILETYPE_GAME);
		mpInit(true);

		// Set MP player names to "Player 1" through 4 if blank
		for (i = 0; i < MAX_PLAYERS; i++) {
			if (g_PlayerConfigsArray[i].base.name[0] == '\0') {
				sprintf(g_PlayerConfigsArray[i].base.name, "%s %d\n", _("Player"), i + 1);
			}
		}
		break;
	case MENUOP_CLOSE:
		func0f110bf8();
		break;
#if VERSION < VERSION_NTSC_1_0
	case MENUOP_TICK:
		filelistsTick();
		break;
#endif
	}

	return 0;
}

bool filemgrConsiderPushingFileSelectDialog(void)
{
	if (g_Menus[g_MpPlayerNum].openinhibit == 0) {
		g_Menus[g_MpPlayerNum].playernum = 0;
		menuPushRootDialog(&g_FilemgrFileSelectMenuDialog, MENUROOT_FILEMGR);

#if PAL
		if (g_Vars.language >= 6) {
			menuPushDialog(&g_ChooseLanguageMenuDialog);
		}
#endif

		return true;
	}

	return false;
}

void bootmenuReset(void)
{
	s32 prevplayernum = g_MpPlayerNum;
	g_MpPlayerNum = 0;
	menuPushRootDialog(&g_PakChoosePakMenuDialog, MENUROOT_BOOTPAKMGR);
	g_MpPlayerNum = prevplayernum;
}

struct menuitem g_FilemgrSelectLocationMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Where\n"), // "Where"
		0,// gettext_noop("Spaces\n"), // "Spaces" TODO - Lang: Fix it
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		SAVEDEVICE_GAMEPAK,
		0,
		&filemgrMenuTextLocationName2,
		(uintptr_t)&filemgrMenuTextSaveLocationSpaces,
		filemgrSelectLocationMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		SAVEDEVICE_CONTROLLERPAK1,
		0,
		&filemgrMenuTextLocationName2,
		(uintptr_t)&filemgrMenuTextSaveLocationSpaces,
		filemgrSelectLocationMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		SAVEDEVICE_CONTROLLERPAK2,
		0,
		&filemgrMenuTextLocationName2,
		(uintptr_t)&filemgrMenuTextSaveLocationSpaces,
		filemgrSelectLocationMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		SAVEDEVICE_CONTROLLERPAK3,
		0,
		&filemgrMenuTextLocationName2,
		(uintptr_t)&filemgrMenuTextSaveLocationSpaces,
		filemgrSelectLocationMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		SAVEDEVICE_CONTROLLERPAK4,
		0,
		&filemgrMenuTextLocationName2,
		(uintptr_t)&filemgrMenuTextSaveLocationSpaces,
		filemgrSelectLocationMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Delete Files...\n"), // "Delete Files..."
		0,
		filemgrDeleteFilesForSaveMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		filemgrCancelSaveMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrSelectLocationMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Select Location\n"), // "Select Location"
	g_FilemgrSelectLocationMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrConfirmDeleteMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		&filemgrMenuTextDeleteFileName,
		0,
		filemgrFileNameMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT,
		&filemgrMenuTextDeviceName,
		0,
		filemgrDeviceNameMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Are you sure you want\nto delete this file?\n"), // "Are you sure you want to delete this file?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("OK\n"), // "OK"
		0,
		filemgrConfirmDeleteMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrConfirmDeleteMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Warning\n"), // "Warning"
	g_FilemgrConfirmDeleteMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrFileInUseMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		&filemgrMenuTextDeleteFileName,
		0,
		filemgrFileNameMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_SMALLFONT,
		&filemgrMenuTextDeviceName,
		0,
		filemgrDeviceNameMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
#if VERSION >= VERSION_NTSC_1_0
		&filemgrMenuTextFileInUseDescription,
#else
		0x54a0,
#endif
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Cancel\n"), // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileInUseMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Error\n"), // "Error"
	g_FilemgrFileInUseMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

struct menuitem g_FilemgrDeleteMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Select a file to delete:\n"), // "Select a file to delete:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		1,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		0,
		0,
		filemgrFileToDeleteListMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_DARKERBG,
		gettext_noop("Press B Button to exit.\n"), // "Press B Button to exit."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrDeleteMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Delete File\n"), // "Delete File"
	g_FilemgrDeleteMenuItems,
	filemgrCopyOrDeleteListMenuDialog,
	0,
	NULL,
};

struct menuitem g_FilemgrCopyMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Select a file to copy:\n"), // "Select a file to copy:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		0,
		0,
		filemgrFileToCopyListMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE | MENUITEMFLAG_DARKERBG,
		gettext_noop("Press B Button to exit.\n"), // "Press B Button to exit."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrCopyMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Copy File\n"), // "Copy File"
	g_FilemgrCopyMenuItems,
	filemgrCopyOrDeleteListMenuDialog,
	0,
	NULL,
};

struct pakdata *g_EditingPak = NULL;

struct menuitem g_PakDeleteNoteMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Are you sure you\nwant to delete\nthis game note?\n"), // "Are you sure you want to delete this game note?"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("No\n"), // "No"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Yes\n"), // "Yes"
		0,
		pakDeleteGameNoteMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakDeleteNoteMenuDialog = {
	MENUDIALOGTYPE_DANGER,
	gettext_noop("Delete Game Note\n"), // "Delete Game Note"
	g_PakDeleteNoteMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_PakGameNotesMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Delete Game Notes:\n"), // "Delete Game Notes:"
		(uintptr_t)&pakMenuTextEditingPakName,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		"", // previous: 0x0000010e,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Note\n"), // "Note"
		(uintptr_t)gettext_noop("Pages\n"), // "Pages" TODO - Lang: Fix it
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"",// previous: 0x000000c8,
		0x0000006e,
		pakGameNoteListMenuHandler,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&pakMenuTextPagesFree,
		(uintptr_t)&pakMenuTextPagesUsed,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		&pakMenuTextStatusMessage,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Press the B Button to exit.\n"), // "Press the B Button to exit."
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakGameNotesMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Notes\n"), // "Game Notes"
	g_PakGameNotesMenuItems,
	pakGameNotesMenuDialog,
	0,
	NULL,
};

struct menuitem g_PakChoosePakMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Use this menu to delete game notes\nfrom your Controller Paks.\n"), // "Use this menu to delete game notes from your Controller Paks."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING,
		gettext_noop("Choose Controller Pak to Edit:\n"), // "Choose Controller Pak to Edit:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		gettext_noop("Controller Pak 1\n"), // "Controller Pak 1"
		0,
		pakSelectionMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		1,
		0,
		gettext_noop("Controller Pak 2\n"), // "Controller Pak 2"
		0,
		pakSelectionMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		2,
		0,
		gettext_noop("Controller Pak 3\n"), // "Controller Pak 3"
		0,
		pakSelectionMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		3,
		0,
		gettext_noop("Controller Pak 4\n"), // "Controller Pak 4"
		0,
		pakSelectionMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		gettext_noop("Exit\n"), // "Exit"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_PakChoosePakMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Controller Pak Menu\n"), // "Controller Pak Menu"
	g_PakChoosePakMenuItems,
	pakChoosePakMenuDialog,
	0,
	NULL,
};

struct menuitem g_FilemgrOperationsMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Copy:\n"), // "Copy:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		FILETYPE_GAME,
		0,
		gettext_noop("Single Player Agent File\n"), // "Single Player Agent File"
		0,
		filemgrOpenCopyFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		FILETYPE_MPPLAYER,
		0,
		gettext_noop("Combat Simulator Player File\n"), // "Combat Simulator Player File"
		0,
		filemgrOpenCopyFileMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Delete:\n"), // "Delete:"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		FILETYPE_GAME,
		0,
		gettext_noop("Single Player Agent File\n"), // "Single Player Agent File"
		0,
		filemgrOpenDeleteFileMenuHandler,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		FILETYPE_MPPLAYER,
		0,
		gettext_noop("Combat Simulator Player File\n"), // "Combat Simulator Player File"
		0,
		filemgrOpenDeleteFileMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_OPENSDIALOG,
		gettext_noop("Delete Game Notes...\n"), // "Delete Game Notes..."
		0,
		(void *)&g_PakChoosePakMenuDialog,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrOperationsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Game Files\n"), // "Game Files"
	g_FilemgrOperationsMenuItems,
	NULL,
	MENUDIALOGFLAG_IGNOREBACK,
	NULL,
};

struct menuitem g_FilemgrEnterNameMenuItems[] = {
	{
		MENUITEMTYPE_KEYBOARD,
		0,
		0,
		0,
		0x00000001,
		filemgrAgentNameKeyboardMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrEnterNameMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Enter Agent Name\n"), // "Enter Agent Name"
	g_FilemgrEnterNameMenuItems,
	NULL,
	0,
	NULL,
};

struct menuitem g_FilemgrFileSelectMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LESSLEFTPADDING | MENUITEMFLAG_DARKERBG,
		gettext_noop("Choose Your Reality\n"), // "Choose Your Reality"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LIST_CUSTOMRENDER,
		"",// previous: 0x000000f5,
		0,
		filemgrChooseAgentListMenuHandler,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_FilemgrFileSelectMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	gettext_noop("Perfect Dark\n"), // "Perfect Dark"
	g_FilemgrFileSelectMenuItems,
	filemgrMainMenuDialog,
	MENUDIALOGFLAG_IGNOREBACK,
	&g_FilemgrOperationsMenuDialog,
};
