/*
    Copyright (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Formats a message and makes sure the user will see it.
    Lang: english
*/
#include <aros/config.h>
#include <proto/exec.h>
#include "exec_private.h"

#if (AROS_FLAVOUR & AROS_FLAVOUR_NATIVE)
#define SysBase 	*(void **)4
#endif

/*****************************************************************************

    NAME */
#include <proto/arossupport.h>

	void RawPutChars (

/*  SYNOPSIS */
	const UBYTE * str,
	int	      len)

/*  FUNCTION
	Emits len bytes of fmt via RawPutChar()

    INPUTS
	str - string to print
	len - how many bytes of str to print

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
    while (*str && len --)
	RawPutChar (*str ++);
} /* RawPutChars */

