#include <aros/debug.h>
#include <aros/symbolsets.h>
#include <devices/trackdisk.h>
#include <proto/exec.h>
#include <proto/hostlib.h>

#include "hostdisk_host.h"
#include "hostdisk_device.h"

static ULONG error(ULONG winerr)
{
    switch(winerr)
    {
/*	case ERROR_SEEK_ERROR:
	    return TDERR_SeekError;*/
	    
	case ERROR_WRITE_PROTECT:
	    return TDERR_WriteProt;

/*	case ERROR_NO_DISK:
	    return TDERR_DiskChanged;*/

	default:
	    return TDERR_NotSpecified;
    }
}

ULONG Host_Open(struct unit *Unit)
{
    ULONG attrs;

    
    Forbid();
    attrs = Unit->hdskBase->iface->GetFileAttributes(Unit->filename);
    Unit->file = Unit->hdskBase->iface->CreateFile(Unit->filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
						   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Permit();

    if (Unit->file == (APTR)-1)
	return TDERR_NotSpecified;

//  Unit->flags = (attrs & FILE_ATTRIBUTE_READONLY) ? UNIT_READONLY : 0;
    Unit->flags = 0;
    if (attrs & FILE_ATTRIBUTE_DEVICE)
	Unit->flags |= UNIT_DEVICE;

    D(bug("hostdisk: Unit flags 0x%02X\n", Unit->flags));
    return 0;
}

void Host_Close(struct unit *Unit)
{
    Forbid();
    Unit->hdskBase->iface->CloseHandle(Unit->file);
    Permit();
}

LONG Host_Read(struct unit *Unit, APTR buf, ULONG size, ULONG *ioerr)
{
    ULONG resSize;
    ULONG ret;
    ULONG err;

    Forbid();
    ret = Unit->hdskBase->iface->ReadFile(Unit->file, buf, size, &resSize, NULL);
    err = Unit->hdskBase->iface->GetLastError();
    Permit();

    if (ret)
	return resSize;

    *ioerr = error(err);
    return -1;
}

LONG Host_Write(struct unit *Unit, APTR buf, ULONG size, ULONG *ioerr)
{
    ULONG resSize;
    ULONG ret;
    ULONG err;

    Forbid();
    ret = Unit->hdskBase->iface->WriteFile(Unit->file, buf, size, &resSize, NULL);
    err = Unit->hdskBase->iface->GetLastError();
    Permit();

    if (ret)
	return resSize;

    *ioerr = error(err);
    return -1;
}

ULONG Host_Seek(struct unit *Unit, ULONG pos)
{
    ULONG ret;

    Forbid();
    ret = Unit->hdskBase->iface->SetFilePointer(Unit->file, pos, NULL, FILE_BEGIN);
    Permit();

    if (ret != -1)
	return 0;

    return TDERR_SeekError;
}

ULONG Host_Seek64(struct unit *Unit, ULONG pos, ULONG pos_hi)
{
    ULONG ret;

    Forbid();
    ret = Unit->hdskBase->iface->SetFilePointer(Unit->file, pos, &pos_hi, FILE_BEGIN);
    Permit();

    if (ret != -1)
	return 0;

    return TDERR_SeekError;
}

ULONG Host_GetGeometry(struct unit *Unit, struct DriveGeometry *dg)
{
    ULONG len, err;
    DISK_GEOMETRY geom;
    ULONG ret;

    if (Unit->flags & UNIT_DEVICE)
    {
	Forbid();
	len = Unit->hdskBase->iface->DeviceIoControl(Unit->file, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
						     &geom, sizeof(geom), &ret, NULL);
	err = Unit->hdskBase->iface->GetLastError();
	Permit();

	D(bug("hostdisk: IOCTL_DISK_GET_DRIVE_GEOMETRY result: %d\n", len));
	if (len)
	{
	    dg->dg_SectorSize   = geom.BytesPerSector;
	    dg->dg_Heads        = geom.TracksPerCylinder;
	    dg->dg_TrackSectors = geom.SectorsPerTrack;
	    dg->dg_Cylinders    = geom.Cylinders;
	    dg->dg_CylSectors   = dg->dg_TrackSectors * dg->dg_Heads;
	    dg->dg_TotalSectors = dg->dg_CylSectors * dg->dg_Cylinders;
	    dg->dg_BufMemType   = MEMF_PUBLIC;
	    dg->dg_DeviceType   = DG_DIRECT_ACCESS;
	    dg->dg_Flags        = 0; //DGF_REMOVABLE;
 
	    D(bug("hostdisk: Sector size      : %u\n", dg->dg_SectorSize));
	    D(bug("hostdisk: Heads            : %u\n", dg->dg_Heads));
	    D(bug("hostdisk: Sectors per track: %u\n", dg->dg_TrackSectors));
	    D(bug("hostdisk: Cylinders        : %u\n", dg->dg_Cylinders));
 
	    return 0;
        }
    }
    else
    {
	Forbid();
	len = Unit->hdskBase->iface->GetFileSize(Unit->file, NULL);
	err = Unit->hdskBase->iface->GetLastError();
	Permit();

	D(bug("hostdisk: Image file length: %d\n", len));
	if (len != -1)
	{
	    dg->dg_SectorSize   = 512;
	    dg->dg_Heads        = 16;
	    dg->dg_TrackSectors = 63;
	    dg->dg_TotalSectors = len / dg->dg_SectorSize;
	    dg->dg_Cylinders    = dg->dg_TotalSectors / (dg->dg_Heads * dg->dg_TrackSectors);
	    dg->dg_CylSectors   = dg->dg_Heads * dg->dg_TrackSectors;
	    dg->dg_BufMemType   = MEMF_PUBLIC;
	    dg->dg_DeviceType   = DG_DIRECT_ACCESS;
	    dg->dg_Flags        = 0; //DGF_REMOVABLE;

	    return 0;
	}
    }

    D(bug("hostdisk: Host_GetGeometry(): Windows error %u\n", err));
    return error(err);
}

static const char *KernelSymbols[] = {
    "CreateFileA",
    "CloseHandle",
    "ReadFile",
    "WriteFile",
    "SetFilePointer",
    "GetFileAttributesA",
    "GetFileSize",
    "DeviceIoControl",
    "GetLastError",
    NULL
};

static int Host_Init(struct HostDiskBase *hdskBase)
{
    ULONG r;

    HostLibBase = OpenResource("hostlib.resource");
    D(bug("[hostdisk] HostLibBase: 0x%p\n", HostLibBase));
    if (!HostLibBase)
	return FALSE;

    hdskBase->KernelHandle = HostLib_Open("kernel32.dll", NULL);
    if (!hdskBase->KernelHandle)
	return FALSE;

    hdskBase->iface = (struct HostInterface *)HostLib_GetInterface(hdskBase->KernelHandle, KernelSymbols, &r);
    if ((!hdskBase->iface) || r)
	return FALSE;

    hdskBase->DiskDevice = "\\\\.\\PhysicalDrive%ld";
	
    return TRUE;
}

static int Host_Cleanup(struct HostDiskBase *hdskBase)
{
    if (!HostLibBase)
	return TRUE;

    if (hdskBase->iface)
	HostLib_DropInterface((APTR *)hdskBase->iface);

    if (hdskBase->KernelHandle)
	HostLib_Close(hdskBase->KernelHandle, NULL);

    return TRUE;
}

ADD2INITLIB(Host_Init, 0)
ADD2EXPUNGELIB(Host_Cleanup, 0)
