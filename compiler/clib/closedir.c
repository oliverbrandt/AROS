/*
    Copyright � 1995-2003, The AROS Development Team. All rights reserved.
    $Id$

    POSIX function closedir().
*/

#include <proto/dos.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "__open.h"

/*****************************************************************************

    NAME */
#include <dirent.h>

	int closedir(

/*  SYNOPSIS */
	DIR *dir)

/*  FUNCTION
	 Closes a directory

    INPUTS
	dir - the directory stream pointing to the directory being closed

    RESULT
	The  closedir()  function  returns  0  on success or -1 on
 	failure.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
 	close(), opendir(),  readdir(), rewinddir(), seekdir(),
	telldir(), scandir()

    INTERNALS

******************************************************************************/
{
    fdesc *desc;

    if (!dir)
    {
	errno = EFAULT;
	return -1;
    }

    desc = __getfdesc(dir->fd);
    if (!desc)
    {
	errno = EBADF;
	return -1;
    }

    __setfdesc(dir->fd, NULL);

    UnLock(desc->fh);
    FreeDosObject(DOS_FIB, dir->priv);
    free(dir);

    return 0;
}
