/*
    (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(LONG, Relabel,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, drive, D1),
	AROS_LHA(STRPTR, newname, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 120, Dos)

/*  FUNCTION

    Change names of a volume.

    INPUTS

    drive    --  The name of the device to rename (including the ':').
    newname  --  The new name for the device (without the ':').

    RESULT

    A boolean telling whether the name change was successful or not.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

    28.04.2000  SDuvan  implemented

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    LONG error;
    struct IOFileSys iofs;

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_RELABEL, DOSBase);

    iofs.IOFS.io_Device = GetDevice(drive, NULL, DOSBase);

    if(iofs.IOFS.io_Device == NULL)
    {
	return DOSFALSE;
    }

    iofs.io_Union.io_RELABEL.io_NewName = newname;
    iofs.io_Union.io_RELABEL.io_Result  = FALSE;

    error = DoIO(&iofs.IOFS);

    SetIoErr(error);

    if(error != 0)
    {
	return DOSFALSE;
    }

    return iofs.io_Union.io_RELABEL.io_Result ? DOSTRUE : DOSFALSE;

    AROS_LIBFUNC_EXIT
} /* Relabel */
