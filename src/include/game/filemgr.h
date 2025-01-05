#ifndef IN_GAME_FILEMGR_H
#define IN_GAME_FILEMGR_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_ChooseLanguageMenuDialog;

char *filemgrGetDeviceName(s32 index);
char *filemgrMenuTextDeviceName(struct menuitem *item);
void filemgrGetSelectName(char *buffer, struct filelistfile *file, u32 filetype);
char *filemgrMenuTextDeleteFileName(struct menuitem *item);
void filemgrSetDevice1BySerial(s32 deviceserial);
void filemgrSetDevice1ByFile(struct filelistfile *file);
void filemgrSetFileToDelete(struct filelistfile *file, s32 filetype);
char *filemgrMenuTextFailReason(struct menuitem *item);
char *filemgrMenuTextDeviceNameForError(struct menuitem *item);
void filemgrPushErrorDialog(u16 errnum);
uintptr_t filemgrGetDeviceNameOrStartIndex(s32 listnum, s32 operation, s32 optionindex);
char *filemgrMenuTextErrorTitle(struct menuitem *item);
char *filemgrMenuTextFileType(struct menuitem *item);
void func0f10898c(void);
void filemgrHandleSuccess(void);
void filemgrEraseCorruptFile(void);
char *filemgrMenuTextInsertOriginalPak(struct menuitem *item);
void filemgrRetrySave(s32 arg0);
bool filemgrAttemptOperation(s32 device, bool closeonsuccess);
bool filemgrSaveOrLoad(struct fileguid *guid, s32 fileop, uintptr_t playernum);
void filemgrDeleteCurrentFile(void);
void func0f1097d0(s32 device);
void filemgrSaveGameToDevice(s32 device);
void filemgrGetFileName(char *dst, struct filelistfile *file);
void filemgrGetRenameName(char *buffer);
void filemgrSetRenameName(char *name);
bool filemgrIsNameAvailable(s32 arg0);
void filemgrSaveToDevice(void);
char *filemgrMenuTextDeviceNameContainingDuplicateFile(struct menuitem *item);
char *filemgrMenuTextDuplicateFileName(struct menuitem *item);
char *filemgrMenuTextLocationName2(struct menuitem *item);
char *filemgrMenuTextSaveLocationSpaces(struct menuitem *item);
void filemgrPushSelectLocationDialog(s32 arg0, u32 filetype);
char *filemgrMenuTextFileInUseDescription(struct menuitem *item);
Gfx *filemgrRenderPerfectHeadThumbnail(Gfx *gdl, struct menuitemrenderdata *renderdata, s32 filenum, s32 deviceserial);
bool filemgrIsFileInUse(struct filelistfile *file);
MenuItemHandlerResult filemgrFileToCopyOrDeleteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data, bool isdelete);
void filemgrPushDeleteFileDialog(s32 listnum);
char *pakMenuTextPagesFree(struct menuitem *item);
char *pakMenuTextPagesUsed(struct menuitem *item);
char *pakMenuTextStatusMessage(struct menuitem *item);
char *pakMenuTextEditingPakName(struct menuitem *item);
bool filemgrConsiderPushingFileSelectDialog(void);
void bootmenuReset(void);
MenuItemHandlerResult filemgrChooseAgentListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakGameNoteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileToCopyListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileToDeleteListMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuDialogHandlerResult filemgrInsertOriginalPakMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult filemgrCopyOrDeleteListMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult pakGameNotesMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult pakChoosePakMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult filemgrMainMenuDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult filemgrDeviceNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileNameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDeviceNameForErrorMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrRetrySaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrCancelSave2MenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrAcknowledgeFileLostMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrReinsertedOkMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrReinsertedCancelMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDuplicateRenameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDuplicateCancelMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrCancelSaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDeleteFilesForSaveMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrConfirmDeleteMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakDeleteGameNoteMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakSelectionMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrAgentNameKeyboardMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrOpenCopyFileMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrOpenDeleteFileMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrConfirmRenameMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrSaveElsewhereYesMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrSelectLocationMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);

extern const char var7f1b3214[];
extern const char var7f1b3218[];
extern const char var7f1b321c[];
extern const char var7f1b3220[];

#endif
