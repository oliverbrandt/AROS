/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.3  1996/12/09 13:53:20  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.2  1996/10/24 15:50:23  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.1  1996/09/11 12:54:44  digulla
    A couple of new DOS functions from M. Fleischer

    Desc:
    Lang: english
*/
#include <clib/exec_protos.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <clib/dos_protos.h>

	AROS_LH2(BOOL, AddBuffers,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, devicename, D1),
	AROS_LHA(LONG,   numbuffers, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 122, Dos)

/*  FUNCTION
	Add or remove cache memory from a filesystem.

    INPUTS
	devicename - NUL terminated dos device name.
	numbuffers - Number of buffers to add. May be negative.

    RESULT
	!=0 on success (IoErr() gives the actual number of buffers),
	0 else (IoErr() gives the error code).

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
    
    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);
    struct DosList *dl;
    BOOL success=0;

    dl=LockDosList(LDF_DEVICES|LDF_READ);
    dl=FindDosEntry(dl,devicename,LDF_DEVICES);
    if(dl!=NULL)
    {

        /* Get pointer to I/O request. Use stackspace for now. */
        struct IOFileSys io,*iofs=&io;

        /* Prepare I/O request. */
        iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;
        iofs->IOFS.io_Message.mn_ReplyPort   =&me->pr_MsgPort;
        iofs->IOFS.io_Message.mn_Length      =sizeof(struct IOFileSys);
        iofs->IOFS.io_Device =dl->dol_Device;
        iofs->IOFS.io_Unit   =dl->dol_Unit;
        iofs->IOFS.io_Command=FSA_MORE_CACHE;
        iofs->IOFS.io_Flags  =0;
        iofs->io_Args[0]=numbuffers;

        /* Send the request. */
        DoIO(&iofs->IOFS);
        
        /* Set error code */
        if(!iofs->io_DosError)
        {
            me->pr_Result2=iofs->io_Args[0];
            success=1; 
	}else
	    me->pr_Result2=iofs->io_DosError;
    }else
        me->pr_Result2=ERROR_DEVICE_NOT_MOUNTED;
    /* All Done. */
    UnLockDosList(LDF_DEVICES|LDF_READ);
    return success;
    AROS_LIBFUNC_EXIT
} /* AddBuffers */
