/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Free a structure created by CreateExtIO()
    Lang: english
*/

#include <proto/exec.h>

/*****************************************************************************

    NAME */
#include <exec/io.h>
#include <proto/alib.h>

	void DeleteExtIO (

/*  SYNOPSIS */
	struct IORequest * ioreq)

/*  FUNCTION
	Free a structure created by CreateExtIO().

    INPUTS
	ioreq - The returnvalue of CreateExtIO(). Must be
	    non-NULL.

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	CreateStdIO(), CreateExt()

    INTERNALS

    HISTORY

******************************************************************************/
{
    /* Erase some fields to enforce crashes */
    ioreq->io_Message.mn_Node.ln_Type = -1L;

    ioreq->io_Device = (struct Device *)-1L;
    ioreq->io_Unit   = (struct Unit *)-1L;

    /* Free the memory */
    FreeMem(ioreq,ioreq->io_Message.mn_Length);
} /* DeleteExtIO */

