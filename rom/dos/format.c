/*
    Copyright (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Format a device.
    Lang: English
*/
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH3(BOOL, Format,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, devicename, D1),
	AROS_LHA(STRPTR, volumename, D2),
	AROS_LHA(ULONG,  dostype,    D3),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 119, Dos)

/*  FUNCTION
	Initialise a filesystem for use by the system. This instructs
	a filesystem to write out the data that it uses to describe the
	device.

	The device should already have been formatted.

    INPUTS
	devicename	- Name of the device to format.
	volumename	- The name you wish the volume to be called.
	dostype		- The DOS type you wish on the disk.

    RESULT
	!= 0 if the format was successful, 0 otherwise.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    BOOL            success = 0;

    /* Get space for I/O request. Use stackspace for now. */
    struct IOFileSys iofs;
    
    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_FORMAT, DOSBase);
    
    iofs.IOFS.io_Device = GetDevice(devicename, &iofs.IOFS.io_Unit,
				    DOSBase);
    
    if (iofs.IOFS.io_Device == NULL)
    {
	return 0;
    }

    iofs.io_Union.io_FORMAT.io_VolumeName = volumename;
    iofs.io_Union.io_FORMAT.io_DosType = dostype;
    
    /* Send the request. */
    DosDoIO(&iofs.IOFS);
    
    /* Set error code */
    if (iofs.io_DosError == 0)
    {
	success = 1;
    }
    else
    {
	SetIoErr(iofs.io_DosError);
    }

    return success;

    AROS_LIBFUNC_EXIT
} /* Format */
