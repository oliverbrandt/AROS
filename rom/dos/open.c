/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.12  1996/12/09 13:53:35  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.11  1996/11/22 12:28:16  aros
    Fixed bug for Open("CONSOLE:") to make run work again

    Revision 1.9  1996/10/24 15:50:33  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.8  1996/09/21 14:14:23  digulla
    Hand DOSBase to DoName()

    Revision 1.7  1996/09/17 16:17:23  digulla
    BADDR() instead of MKBADDR()

    Revision 1.6  1996/09/13 17:50:07  digulla
    Use IPTR

    Revision 1.5  1996/09/11 13:02:22  digulla
    Open() and Lock() are two different functions now (M. Fleischer)

    Revision 1.4  1996/08/13 13:52:49  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced AROS_LA by AROS_LHA

    Revision 1.3  1996/08/12 14:20:38  digulla
    Added aliases

    Revision 1.2  1996/08/01 17:40:55  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include "dos_intern.h"

#define NEWLIST(l)                          \
((l)->lh_Head=(struct Node *)&(l)->lh_Tail, \
 (l)->lh_Tail=NULL,                         \
 (l)->lh_TailPred=(struct Node *)(l))

/*****************************************************************************

    NAME */
#include <clib/dos_protos.h>

	AROS_LH2(BPTR, Open,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, name,       D1),
	AROS_LHA(LONG,   accessMode, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 5, Dos)

/*  FUNCTION
	Opens a file for read and/or write depending on the accessmode given.

    INPUTS
	name	   - NUL terminated name of the file.
	accessMode - One of MODE_OLDFILE   - open existing file
			    MODE_NEWFILE   - delete old, create new file
					     exclusive lock
			    MODE_READWRITE - open new one if it doesn't exist

    RESULT
	Handle to the file or 0 if the file couldn't be opened.
	IoErr() gives additional information in that case.

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

    struct FileHandle *ret;
    BPTR con, ast;
    LONG error;

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
	iofs->IOFS.io_Command=FSA_OPEN_FILE;

	/* io_Args[0] is the name which is set by DoName(). */
	switch(accessMode)
	{
	    case MODE_OLDFILE:
		iofs->io_Args[1]=FMF_WRITE|FMF_READ;
		ast=con=me->pr_CIS;
		break;
	    case MODE_NEWFILE:
		iofs->io_Args[1]=FMF_LOCK|FMF_CREATE|FMF_CLEAR|FMF_WRITE|FMF_READ;
		con=me->pr_COS;
		ast=me->pr_CES?me->pr_CES:me->pr_COS;
		break;
	    case MODE_READWRITE:
		iofs->io_Args[1]=FMF_CREATE|FMF_WRITE|FMF_READ;
		con=me->pr_COS;
		ast=me->pr_CES?me->pr_CES:me->pr_COS;
		break;
	    default:
		iofs->io_Args[1]=accessMode;
		ast=con=me->pr_CIS;
		break;
	}
	iofs->io_Args[2]=FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE;
	if(!Stricmp(name,"CONSOLE:"))
	{
	    iofs->IOFS.io_Device=((struct FileHandle *)BADDR(con))->fh_Device;
	    iofs->IOFS.io_Unit	=((struct FileHandle *)BADDR(con))->fh_Unit;
	    iofs->io_Args[0]=(IPTR)"";
	    (void)DoIO(&iofs->IOFS);
	    error=me->pr_Result2=iofs->io_DosError;
	}else if(!Stricmp(name,"*"))
	{
	    iofs->IOFS.io_Device=((struct FileHandle *)BADDR(ast))->fh_Device;
	    iofs->IOFS.io_Unit	=((struct FileHandle *)BADDR(ast))->fh_Unit;
	    iofs->io_Args[0]=(IPTR)"";
	    (void)DoIO(&iofs->IOFS);
	    error=me->pr_Result2=iofs->io_DosError;
	}else
	    error=DoName(iofs,name,DOSBase);
	if(!error)
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
} /* Open */
