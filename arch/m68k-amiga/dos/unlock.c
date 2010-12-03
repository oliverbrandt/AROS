/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id: close.c 34705 2010-10-13 20:30:16Z jmcmullan $

    Desc:
    Lang: English
*/

#define DEBUG 1
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include "dos_intern.h"
#include <aros/debug.h>

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(BOOL, UnLock,

/*  SYNOPSIS */
	AROS_LHA(BPTR, lock, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 15, Dos)

/*
    FUNCTION
	Free a lock created with Lock().

    INPUTS
	lock -- The lock to free

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Get pointer to filehandle */
    struct FileLock *fl = BADDR(lock);

    ASSERT_VALID_PTR_OR_NULL(fh);
    D(bug("UnLock(%x)\n", fl));

    /* 0 handles are OK */
    if(lock == BNULL)
	return 0;

    return dopacket1(DOSBase, NULL, fl->fl_Task, ACTION_FREE_LOCK, lock) != 0;

    AROS_LIBFUNC_EXIT
} /* UnLock */
