/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Change the position in a stream
    Lang: english
*/

#include <errno.h>

/*****************************************************************************

    NAME */
#include <stdio.h>

	int fsetpos (

/*  SYNOPSIS */
	FILE		* stream,
	const fpos_t	* pos)

/*  FUNCTION
	Change the current position in a stream. This function is eqivalent
	to fseek() with whence set to SEEK_SET. However, on some systems
	fpos_t may be a complex structure, so this routine may be the only
	way to portably reposition a stream.

    INPUTS
	stream - Modify this stream
	pos - The new position in the stream.

    RESULT
	0 on success and -1 on error. If an error occurred, the global
	variable errno is set.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	fgetpos()

    INTERNALS

    HISTORY

******************************************************************************/
{
    int retval;

    if ( pos == NULL )
    {
	errno = EINVAL;
	return -1;
    }

    retval = fseek (stream, *pos, SEEK_SET);

    return retval;
} /* fsetpos */

