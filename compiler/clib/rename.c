/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc: ANSI C function rename()
    Lang: english
*/

#include <proto/dos.h>
#include "__errno.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

	int rename (

/*  SYNOPSIS */
	const char * oldpath,
	const char * newpath)

/*  FUNCTION
	Renames a file or directory.

    INPUTS
	oldpath - Complete path to existing file or directory.
	newpath - Complete path to the new file or directory.

    RESULT
	0 on success and -1 on error. In case of an error, errno is set.
	
    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
    if (!Rename ((STRPTR)oldpath,(STRPTR)newpath))
    {
	GETUSER;

	errno = IoErr2errno (IoErr());
	return -1;
    }

    return 0;

} /* rename */

