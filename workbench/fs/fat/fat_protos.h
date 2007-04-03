/*
 * fat.handler - FAT12/16/32 filesystem handler
 *
 * Copyright � 2006 Marek Szyprowski
 * Copyright � 2007 The AROS Development Team
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 *
 * $Id$
 */

#ifndef FAT_HANDLER_PROTO_H
#define FAT_HANDLER_PROTO_H

/* fat */
LONG ReadFATSuper (struct FSSuper *s);
void FreeFATSuper(struct FSSuper *s);
LONG CompareFATSuper(struct FSSuper *s1, struct FSSuper *s2);

LONG GetVolumeInfo(struct FSSuper *sb, struct VolumeInfo *volume);

LONG InitExtent(struct FSSuper *sb, struct Extent *ext, ULONG start_cluster);
LONG NextExtent(struct FSSuper *sb, struct Extent *ext);
LONG SeekExtent(struct FSSuper *sb, struct Extent *ext, ULONG dst_sector);

void CountFreeClusters(struct FSSuper *sb);

/* file */
LONG File_Read(struct ExtFileLock *fl, ULONG togo, void *buffer, LONG *result);

/* names.c */
/*
LONG GetLongName(struct FSSuper *sb, struct DirCache *dc, struct FATDirEntry *de, ULONG entry, STRPTR dest, UBYTE *dlen);
LONG GetShortName(struct FATDirEntry *de, STRPTR dest, UBYTE *dlen);
*/

/* diskchange */
void ProcessDiskChange (void);
void DoDiskInsert();
void DoDiskRemove();
void SendVolumePacket(struct DosList *vol, ULONG action);

/* diskio */
LONG InitDevice(struct FileSysStartupMsg *fssm, LONG blocksize);
LONG DoRawRead (ULONG n, void *buff);
LONG InitDiskHandler (struct FileSysStartupMsg *fssm, ULONG *ressigbit);
void CleanupDiskHandler(ULONG diskchgsig_bit);

/* info.c */
void FillDiskInfo (struct InfoData *id);
 
/* lock.c */
LONG TryLockObj(struct ExtFileLock *fl, UBYTE *name, LONG namelen, LONG access, BPTR *result);
LONG LockFile(ULONG entry, ULONG cluster, LONG axs, BPTR *res);
LONG LockRoot(LONG axs, BPTR *res);
LONG LockParent(struct ExtFileLock *ld, LONG axs, BPTR *res);
LONG IsLockable(ULONG ino, LONG mode);
void FreeLock(struct ExtFileLock *fl);
LONG CopyLock(struct ExtFileLock *src_fl, BPTR *res);

/* packet.c */
void ProcessPackets(void);

#include "fat_inlines.h"

/* new definitions as we refactor the code */

/* direntry.c */
LONG InitDirHandle(struct FSSuper *sb, ULONG cluster, struct DirHandle *dh);
LONG ReleaseDirHandle(struct DirHandle *dh);

LONG GetDirEntry(struct DirHandle *dh, ULONG index, struct DirEntry *de);
LONG GetNextDirEntry(struct DirHandle *dh, struct DirEntry *de);

LONG GetDirEntryByName(struct DirHandle *dh, STRPTR name, ULONG namelen, struct DirEntry *de);
LONG GetDirEntryByPath(struct DirHandle *dh, STRPTR path, ULONG pathlen, struct DirEntry *de);

LONG GetParentDir(struct DirHandle *dh, struct DirEntry *de);

LONG GetDirShortName(struct DirEntry *de, STRPTR name, ULONG *len);
LONG GetDirLongName(struct DirEntry *de, STRPTR name, ULONG *len);

/* fat.c */
void ConvertDate(UWORD date, UWORD time, struct DateStamp *ds);
LONG FillFIB(struct ExtFileLock *fl, struct FileInfoBlock *fib);

#endif

