#ifndef DOS_DOS_H
#define DOS_DOS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef AROS_MACHINE_H
#include <aros/machine.h>
#endif

#define DOSNAME "dos.library"

struct DateStamp
{
   LONG ds_Days;
   LONG ds_Minute;
   LONG ds_Tick;
};

struct FileInfoBlock
{
    LONG fib_DiskKey;
    LONG fib_DirEntryType;
    UBYTE fib_FileName[108];
    LONG fib_Protection;
    LONG fib_EntryType;
    LONG fib_Size;
    LONG fib_NumBlocks;
    struct DateStamp fib_Date;
    UBYTE fib_Comment[80];
    UWORD fib_OwnerUID;
    UWORD fib_OwnerGID;
    UBYTE fib_Reserved[32];
};

#define MODE_OLDFILE		1005
#define MODE_NEWFILE		1006
#define MODE_READWRITE		1004

#define SHARED_LOCK		-2
#define EXCLUSIVE_LOCK		-1

#define OFFSET_BEGINNING	-1
#define OFFSET_CURRENT		0
#define OFFSET_END		1

#define RETURN_OK		0
#define RETURN_WARN		5
#define RETURN_ERROR		10
#define RETURN_FAIL		20

#define SIGBREAKB_CTRL_C	12
#define SIGBREAKB_CTRL_D	13
#define SIGBREAKB_CTRL_E	14
#define SIGBREAKB_CTRL_F	15
#define SIGBREAKF_CTRL_C	0x1000
#define SIGBREAKF_CTRL_D	0x2000
#define SIGBREAKF_CTRL_E	0x4000
#define SIGBREAKF_CTRL_F	0x8000

#define ITEM_EQUAL		-2
#define ITEM_ERROR		-1
#define ITEM_NOTHING		0
#define ITEM_UNQUOTED		1
#define ITEM_QUOTED		2

#define DOS_FILEHANDLE		0
#define DOS_EXALLCONTROL	1
#define DOS_FIB 		2
#define DOS_STDPKT		3
#define DOS_CLI 		4
#define DOS_RDARGS		5

#define TICKS_PER_SECOND	50

#define FIBB_DELETE		0
#define FIBB_EXECUTE		1
#define FIBB_WRITE		2
#define FIBB_READ		3
#define FIBB_ARCHIVE		4
#define FIBB_PURE		5
#define FIBB_SCRIPT		6
#define FIBB_GRP_DELETE 	8
#define FIBB_GRP_EXECUTE	9
#define FIBB_GRP_WRITE		10
#define FIBB_GRP_READ		11
#define FIBB_OTR_DELETE 	12
#define FIBB_OTR_EXECUTE	13
#define FIBB_OTR_WRITE		14
#define FIBB_OTR_READ		15
#define FIBF_DELETE		0x1
#define FIBF_EXECUTE		0x2
#define FIBF_WRITE		0x4
#define FIBF_READ		0x8
#define FIBF_ARCHIVE		0x10
#define FIBF_PURE		0x20
#define FIBF_SCRIPT		0x40
#define FIBF_GRP_DELETE 	0x100
#define FIBF_GRP_EXECUTE	0x200
#define FIBF_GRP_WRITE		0x400
#define FIBF_GRP_READ		0x800
#define FIBF_OTR_DELETE 	0x1000
#define FIBF_OTR_EXECUTE	0x2000
#define FIBF_OTR_WRITE		0x4000
#define FIBF_OTR_READ		0x8000

#define ERROR_NO_FREE_STORE		103
#define ERROR_TASK_TABLE_FULL		105
#define ERROR_BAD_TEMPLATE		114
#define ERROR_BAD_NUMBER		115
#define ERROR_REQUIRED_ARG_MISSING	116
#define ERROR_KEY_NEEDS_ARG		117
#define ERROR_TOO_MANY_ARGS		118
#define ERROR_UNMATCHED_QUOTES		119
#define ERROR_LINE_TOO_LONG		120
#define ERROR_FILE_NOT_OBJECT		121
#define ERROR_INVALID_RESIDENT_LIBRARY	122
#define ERROR_NO_DEFAULT_DIR		201
#define ERROR_OBJECT_IN_USE		202
#define ERROR_OBJECT_EXISTS		203
#define ERROR_DIR_NOT_FOUND		204
#define ERROR_OBJECT_NOT_FOUND		205
#define ERROR_BAD_STREAM_NAME		206
#define ERROR_OBJECT_TOO_LARGE		207
#define ERROR_ACTION_NOT_KNOWN		209
#define ERROR_INVALID_COMPONENT_NAME	210
#define ERROR_INVALID_LOCK		211
#define ERROR_OBJECT_WRONG_TYPE 	212
#define ERROR_DISK_NOT_VALIDATED	213
#define ERROR_DISK_WRITE_PROTECTED	214
#define ERROR_RENAME_ACROSS_DEVICES	215
#define ERROR_DIRECTORY_NOT_EMPTY	216
#define ERROR_TOO_MANY_LEVELS		217
#define ERROR_DEVICE_NOT_MOUNTED	218
#define ERROR_SEEK_ERROR		219
#define ERROR_COMMENT_TOO_BIG		220
#define ERROR_DISK_FULL 		221
#define ERROR_DELETE_PROTECTED		222
#define ERROR_WRITE_PROTECTED		223
#define ERROR_READ_PROTECTED		224
#define ERROR_NOT_A_DOS_DISK		225
#define ERROR_NO_DISK			226
#define ERROR_NO_MORE_ENTRIES		232
#define ERROR_IS_SOFT_LINK		233
#define ERROR_OBJECT_LINKED		234
#define ERROR_BAD_HUNK			235
#define ERROR_NOT_IMPLEMENTED		236
#define ERROR_RECORD_NOT_LOCKED 	240
#define ERROR_LOCK_COLLISION		241
#define ERROR_LOCK_TIMEOUT		242
#define ERROR_UNLOCK_ERROR		243

#endif
