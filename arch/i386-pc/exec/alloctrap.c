/*
    (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Allocate a trap
    Lang: english
*/
#include "exec_intern.h"
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(LONG, AllocTrap,

/*  SYNOPSIS */
	AROS_LHA(long, trapNum, D0),

/*  LOCATION */
	struct ExecBase *, SysBase, 57, Exec)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    exec_lib.fd and clib/exec_protos.h
	23-12-98    schulz  No traps allowed for i386 now.

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct ExecBase *,SysBase)

    return -1L;
    AROS_LIBFUNC_EXIT
} /* AllocTrap */
