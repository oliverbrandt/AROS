#ifndef FDSK_DEVICE_H
#define FDSK_DEVICE_H

/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <aros/libcall.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <dos/dos.h>

struct HostDiskBase
{
    struct Device 		device;
    struct SignalSemaphore 	sigsem;
    struct MsgPort 		port;
    struct MinList 		units;
    STRPTR			DiskDevice;
    APTR			HostLibBase;
    APTR			KernelHandle;
    struct HostInterface       *iface;
    int			       *errnoPtr;
};

#define HostLibBase hdskBase->HostLibBase

struct unit
{
    struct Message 		msg;
    struct HostDiskBase		*hdskBase;
    STRPTR                      filename;
    ULONG 			unitnum;
    ULONG			usecount;
    struct MsgPort 		port;
    file_t 			file;
    UBYTE			flags;
    ULONG			changecount;
    struct MinList 		changeints;
};

/* Unit flags */
#define UNIT_READONLY 0x01
#define UNIT_DEVICE   0x02

ULONG Host_Open(struct unit *Unit);
void Host_Close(struct unit *Unit);
LONG Host_Read(struct unit *Unit, APTR buf, ULONG size, ULONG *ioerr);
LONG Host_Write(struct unit *Unit, APTR buf, ULONG size, ULONG *ioerr);
ULONG Host_Seek(struct unit *Unit, ULONG pos);
ULONG Host_Seek64(struct unit *Unit, ULONG pos, ULONG pos_hi);
ULONG Host_GetGeometry(struct unit *Unit, struct DriveGeometry *dg);

#endif
