/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.4  1996/12/09 13:53:33  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.3  1996/10/24 15:50:32  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.2  1996/09/21 14:14:23  digulla
    Hand DOSBase to DoName()

    Revision 1.1  1996/09/11 12:54:46  digulla
    A couple of new DOS functions from M. Fleischer

    Desc:
    Lang: english
*/
#include <clib/exec_protos.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <clib/dos_protos.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <clib/dos_protos.h>

	AROS_LH2(BPTR, Lock,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, name,       D1),
	AROS_LHA(LONG,   accessMode, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 14, Dos)

/*  FUNCTION
	Gets a lock on a file or directory. There may be more than one
	shared lock on a file but only one if it is an exclusive one.
	Locked files or directories may not be deleted.

    INPUTS
	name	   - NUL terminated name of the file or directory.
	accessMode - One of SHARED_LOCK
			    EXCLUSIVE_LOCK

    RESULT
	Handle to the file or directory or 0 if the object couldn't be locked.
	IoErr() gives additional information in that case.

    NOTES
	The lock structure returned by this function is different
	from that of AmigaOS (in fact it is identical to a filehandle).
	Do not try to read any internal fields.

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

    struct FileHandle *ret;

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);

    /* Create filehandle */
    ret=(struct FileHandle *)AllocDosObject(DOS_FILEHANDLE,NULL);
    if(ret!=NULL)
    {
	/* Get pointer to I/O request. Use stackspace for now. */
	struct IOFileSys io,*iofs=&io;

	/* Prepare I/O request. */
	iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;
	iofs->IOFS.io_Message.mn_ReplyPort   =&me->pr_MsgPort;
	iofs->IOFS.io_Message.mn_Length      =sizeof(struct IOFileSys);
	iofs->IOFS.io_Flags=0;
	iofs->IOFS.io_Command=FSA_OPEN;
	/* io_Args[0] is the name which is set by DoName(). */
	switch(accessMode)
	{
	    case EXCLUSIVE_LOCK:
		iofs->io_Args[1]=FMF_LOCK|FMF_READ;
		break;
	    case SHARED_LOCK:
		iofs->io_Args[1]=FMF_READ;
		break;
	    default:
		iofs->io_Args[1]=accessMode;
		break;
	}
	if(!DoName(iofs,name,DOSBase))
	{
	    ret->fh_Device=iofs->IOFS.io_Device;
	    ret->fh_Unit  =iofs->IOFS.io_Unit;
	    return MKBADDR(ret);
	}
	FreeDosObject(DOS_FILEHANDLE,ret);
    }else
	me->pr_Result2=ERROR_NO_FREE_STORE;
    return 0;
    AROS_LIBFUNC_EXIT
} /* Lock */
